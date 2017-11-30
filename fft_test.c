#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "fftlib.h"

#define SIGNAL_LENTH	32

int main(void)
{
	double sig[SIGNAL_LENGTH];
	for (int i = 0; i < SIGNAL_LENGTH; ++i)
		sig[i] = cos(2 * M_PI * ((double)(i)) / ((double)(SIGNAL_LENGTH)));
	int ret = fftlib_spectra(sig);
	printf("return value %d\n", ret);
	for (int i = 0; i < SIGNAL_LENGTH; ++i)
		printf("%f\n", sig[i]);
	return 0;
}
