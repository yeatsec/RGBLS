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
#define	ADC0_PATH	"/sys/devices/ocp.2/helper.14/AIN0"
#define ADC1_PATH	"/sys/devices/ocp.2/helper.14/AIN1"


// double-buffer data members
double buff[2][BUFF_SIZE];
pthread_mutex_t buff_mutex;
pthread_cond_t buff_cond;
int adc_finished;
int fft_finished;

// rgb_strip resources
rgb_strip strips[NUM_STRIPS];
rgb_strip matrix;

// sampling resources
int adc_fds[2];

// control adc
sem_t timer_sem;

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
	fd = adc_fds[chan];
	char value[4];
	read(fd, &value, 4);
	return atoi(value);
}

static void timersignalhandler(int sig)
{
	sem_post(&timer_sem);
}

static void * adc_routine(void * arg)
{
	// TODO - start timer
	unsigned int buff_index = 0; // initialize to first buffer
	while(1)
	{
		adc_finished = 0;	// only adc_routine can set this value
		int rc = sem_wait(&timer_sem);
		if (rc == -1 && errno == EINTR)
			continue;
		if (rc == -1)
		{
			printf("timer_routine failed on sem_wait\n")
			exit(-1);
		}
		// read the adc and update double-buffer
		//
		// if the double-buffer is full, wait for fft to send data and then swap
		// set adc_finished to 1, signal the condition
	}
	return NULL;
}

static void * fft_routine(void * arg)
{
	unsigned int buff_index = 1; // initialize to second arr
	while(1)
	{
		// calculate fft
		
		// once fft finished, calculate colors
		
		// set strips
		
		// set matrix
		
		// send formatted messages via opc_client
		
		// wait for adc_thread if needed
		
		// signal buffer swap
		
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
	// for debugging
	for (int i = 0; i < 30; ++i)
	{
		if (opc_client_send_formatted(1,0,&strips[0]));
	}
	// initialize timer semaphore to zero, register signal
	if (sem_init(&timer_sem, 0, 0))
	{
		printf("sem_init error\n");
		return 3;
	}
	signal(SIGALRM, timersignalhandler); // register the timer signal
	// initialize mutex and condition variable
	
	// TODO - initialize button input
	printf("deallocating resources\n");
	// close server
	if (opc_client_close())
	{
		printf("client close error\n");
		return 4;
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

