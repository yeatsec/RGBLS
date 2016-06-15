#include <stdlib.h>
#include <stdio.h>
//#include <pthread.h>
//#include <semaphore.h>
// #include <rt.h> // is this a library?? will probably need this for the linker

//#include "fftlib/fftlib.h"
#include "opc_client.h"

// constants
#define	BUFF_BITS	10
#define	BUFF_SIZE	1024
#define	STRIP_LENGTH	60
#define	NUM_STRIPS	4
#define	MATRIX_STRIP_LENGTH	256
#define	PORT	7890
#define	SERVER_ADDRESS	"::1"

// double-buffer data members
double sample_buff[BUFF_SIZE];
double fft_buff[BUFF_SIZE];

// rgb_strip resources
rgb_strip strips[NUM_STRIPS];
rgb_strip matrix;

// control flow
//sem_t timer_sem;

int main(void)
{
	// initialize strip resources
	for (int i = 0; i < NUM_STRIPS; ++i)
	{
		opc_client_rgb_strip_init(&(strips[i]), STRIP_LENGTH);
	}
	opc_client_rgb_strip_init(&matrix, MATRIX_STRIP_LENGTH);
	// initialize FFT singleton
	// int ret;
	// ret = fftlib_init();
	// set up network connection with OPS server
	if(opc_client_init(PORT, SERVER_ADDRESS))
	{
		exit(1);
	}
	
	for (int i = 0; i < 30; ++i)
	{
		if (opc_client_send_formatted(0,0,&strips[0]));
	}
	// initialize I/O for buttons
	// initialize state in free-mode
	// timer-based semaphore-controlled ADC sampling
	// register signal for semaphore timer
	printf("deallocating resources\n");
	// close server
	if (opc_client_close())
	{
		printf("client close error\n");
	}
	
	for (int i = 0; i < NUM_STRIPS; ++i)
	{
		opc_client_rgb_strip_destroy(&(strips[i]));
	}
	opc_client_rgb_strip_destroy(&matrix);
	return 0;
}

