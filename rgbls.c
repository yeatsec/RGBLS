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
sem_t swap_finished;

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
		buff[buff_index][elem_index] = (double) adc(sampling_channel);
		elem_index = (++elem_index)%BUFF_SIZE;
		// if the double-buffer is full, wait for fft to send data and then swap
		if (!elem_index)	// looped around and is full
		{
			signal(SIGALRM, timersignalignore);
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
			signal(SIGALRM, timersignalhandler);
		}
	}
	return NULL;
}

static void * fft_routine(void * arg)
{
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
		if (sem_wait(&adc_finished))
		{
			printf("fft_finished sem_wait error\n");
			exit(421);
		}
		// signal buffer swap
		buff_index = (++buff_index)%2;
	}
	return NULL;
}

int main(void)
{
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
	signal(SIGALRM, timersignalhandler); // register the timer signal
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
	if (pthread_create(&fft_thread, NULL, fft_routine, NULL))
	{
		printf("fft_thread create failed\n");
		return 7;
	}
	// TODO - initialize button input
	void * ret;
	pthread_join(adc_thread, &ret);
	pthread_join(fft_thread, &ret);
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

