#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
// #include <rt.h> // is this a library?? will probably need this for the linker

#include "fftlib/fftlib.h"

// constants
const int BUFF_BITS = 10;
const int BUFF_SIZE = 1024;

// double-buffer data members
double sample_buff[BUFF_SIZE];
double fft_buff[BUFF_SIZE];

void * timer_thread(void * arg)
{
	
	return NULL;
}

int main(void)
{
	// initialize FFT singleton
	int ret;
	ret = fftlib_init();
	// set up network connection with OPS server
	// initialize I/O for buttons
	// initialize state in free-mode
	// timer-based semaphore-controlled ADC sampling
	// register signal for semaphore timer

}

