#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <string.h> // memset

#include "fftlib.h"
#include "opc_client.h"

// constants
#define	BUFF_BITS	10
#define	BUFF_SIZE	1024
#define	STRIP_LENGTH	60
#define	NUM_STRIPS	4
#define	MATRIX_STRIP_LENGTH	256
#define	PORT	7890
#define	SERVER_ADDRESS	"::1"
#define	ADC0_PATH	"/sys/devices/ocp.2/helper.11/AIN0"
#define ADC1_PATH	"/sys/devices/ocp.2/helper.11/AIN1"

#define USEC	50	// desired adc sampling period in microseconds

// double-buffer data members
double buff[2][BUFF_SIZE];
sem_t adc_finished;
sem_t fft_finished;

// rgb_strip resources
rgb_strip strips[NUM_STRIPS];
rgb_strip matrix;

// sampling resources
int adc_fds[2];

// control adc
sem_t timer_sem;
unsigned int sampling_channel;


/*
* int adc(unsigned int chan)
* This function was written with help from Robert Edwards' blog.
* Available: embedded-basics.blogspot.com/2014/10/beaglebone-black-analog-input-pins.html
*/

int adc(unsigned int chan)
{
	if (chan > 1)
	{
		printf("invalid channel specified\n");
		exit(1);
	}
	int fd = adc_fds[chan];
	char value[4];
	read(fd, &value, 4);
	return atoi(value);
}

static void timersignalhandler(int sig)
{
	sem_post(&timer_sem);
}

static void timersignalignore(int sig)
{
}

static void * adc_routine(void * arg)
{
	// initialize both sigaction structs to configure each timersignal handler
	static struct sigaction sa_ignore;
	memset(&sa_ignore, 0, sizeof(sa_ignore));
	sa_ignore.sa_handler = timersignalignore;
	static struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = timersignalhandler;
	if(sigaction(SIGALRM, &sa, NULL))
		printf("set to timersignalhandler fail\n");
	printf("entered adc_routine, registering timersignalhandler and starting timer\n");
	// start timer
	struct itimerval tval = {
		.it_interval = { .tv_sec = 0, .tv_usec = USEC},
			.it_value = { .tv_sec = 0, .tv_usec = USEC}
	};
	setitimer(ITIMER_REAL, &tval, (struct itimerval*)0); /* start timer */
	unsigned int buff_index = 0; // initialize to first buffer
	unsigned int elem_index = 0; // first elem
	while(1)
	{
		int rc = sem_wait(&timer_sem);
		if (rc == -1 && errno == EINTR)
			continue;
		if (rc == -1)
		{
			printf("timer_routine failed on sem_wait\n");
			exit(-1);
		}
		// read the adc and update double-buffer
		double value = (double) adc(sampling_channel);
		buff[buff_index][elem_index] = value;
		elem_index = (++elem_index)%BUFF_SIZE;
		printf("%f\n");				// REMOVE LATER
		// if the double-buffer is full, wait for fft to send data and then swap
		if (!elem_index)	// looped around and is full
		{
			if(sigaction(SIGALRM, &sa_ignore, NULL))
				printf("set to timersignalignore fail\n");
			printf("registered SIGALARM to ignore\n");
			sem_post(&adc_finished);	// signal that the adc buffer is filled
			int fftr = sem_wait(&fft_finished);	// block if fft not finished
			if (fftr == -1 && errno != EINTR)
			{
				printf("sem_wait(&fft_finished) error\n");
				exit(420);
			}
			// do swap
			buff_index = (++buff_index)%2;
			// finished with swap
			if(sigaction(SIGALRM, &sa, NULL))
				printf("set to timersignalhandler fail\n");
		}
	}
	return NULL;
}

static void * fft_routine(void * arg)
{
	printf("entered fft_routine, blocking SIG_ALRM\n");
	// deregister signal for self
	sigset_t no_sigalrm;
	if(sigemptyset(&no_sigalrm))
		printf("unable to empty sig set for fft_thread\n");
	if(sigaddset(&no_sigalrm, SIGALRM))
		printf("unable to configure sigset_t for fft_thread\n");
	if(sigprocmask(SIG_BLOCK, &no_sigalrm, NULL))
		printf("unable to mask sigalrm for fft_thread\n");
	unsigned int buff_index = 1; // initialize to second arr
	while(1)
	{
		// calculate fft
		if (fftlib_spectra(buff[buff_index]))
			continue;
		
		// once fft finished, calculate colors
		rgb color;
		color.red = 255;
		color.green = 0;
		color.blue = 255;
		
		// set strips
		for (unsigned int strip_index = 0; strip_index < NUM_STRIPS; ++strip_index)
		{
			for (unsigned int led_index = 0; led_index < STRIP_LENGTH; ++led_index)
			{
				strips[strip_index].rgb_leds[led_index].red = color.red;
				strips[strip_index].rgb_leds[led_index].green = color.green;
				strips[strip_index].rgb_leds[led_index].blue = color.blue;
			}
		}
		// set matrix
		// nah
		// send formatted messages via opc_client
		for (unsigned int strip_index = 0; strip_index < NUM_STRIPS; ++strip_index)
		{
			if(opc_client_send_formatted((char) strip_index, 0, &(strips[strip_index])))
			{
				printf("opc_client_send_formatted error\n");
			}
		}
		// wait for adc_thread if needed
		sem_post(&fft_finished);
		while (sem_wait(&adc_finished) && errno == EINTR)
		{
			continue;	// sem_wait interrupted by signal
		}
		// signal buffer swap
		buff_index = (++buff_index)%2;
	}
	return NULL;
}

int main(void)
{
	sem_t program_over;
	if (sem_init(&program_over, 0, 0))
	{
		printf("program_over init failed\n");
		return 1;
	}
	// initialize strip resources
	for (int i = 0; i < NUM_STRIPS; ++i)
	{
		opc_client_rgb_strip_init(&(strips[i]), STRIP_LENGTH);
	}
	opc_client_rgb_strip_init(&matrix, MATRIX_STRIP_LENGTH);

	// initialize sampling resources
	adc_fds[0] = open(ADC0_PATH, O_RDONLY);
	adc_fds[1] = open(ADC1_PATH, O_RDONLY);
	if (adc_fds[0] < 0 || adc_fds[1] < 0)
	{
		printf("adc open failure\n");
		return 1;
	}
	// set up network connection with OPS server
	if(opc_client_init(PORT, SERVER_ADDRESS))
	{
		return 2;
	}
	// initialize timer semaphore to zero, register signal
	if (sem_init(&timer_sem, 0, 0))
	{
		printf("sem_init error\n");
		return 3;
	}
	printf("registering SIGALRM\n");
	// initialize semaphores
	if (sem_init(&adc_finished, 0, 0))
	{
		printf("adc semaphore init error\n");
		return 4;
	}
	if (sem_init(&fft_finished, 0, 0))
	{
		printf("fft semaphore init error\n");
		return 5;
	}
	// initialize pthreads and send them on their way
	pthread_t adc_thread;
	pthread_t fft_thread;
	if (pthread_create(&adc_thread, NULL, adc_routine, NULL))
	{
		printf("adc_thread create failed\n");
		return 6;
	}
	sigset_t no_alrm;
	if(sigemptyset(&no_alrm))
		printf("main thread unable to clear no_alrm set\n");
	if(sigaddset(&no_alrm, SIGALRM))
		printf("main thread unable to add no_alrm to set\n");
	if(sigprocmask(SIG_BLOCK, &no_alrm, NULL))
		printf("main thread unable to block SIG_ALRM\n");
	printf("created adc thread\n");
	if (pthread_create(&fft_thread, NULL, fft_routine, NULL))
	{
		printf("fft_thread create failed\n");
		return 7;
	}
	printf("created fft thread \n");
	// TODO - initialize button input
	while(sem_wait(&program_over)==-1 && errno == EINTR)
		continue;
	printf("deallocating resources\n");
	// close server
	if (opc_client_close())
	{
		printf("client close error\n");
		return 6;
	}
	
	for (int i = 0; i < NUM_STRIPS; ++i)
	{
		opc_client_rgb_strip_destroy(&(strips[i]));
	}
	opc_client_rgb_strip_destroy(&matrix);
	close(adc_fds[0]);	// close file descriptors for adcs
	close(adc_fds[1]);
	return 0;
}

