#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "fftlib.h"
//
#define SIGNAL_LENGTH	32
#define	PI	3.14159
int main(void)
{
	static double sig[SIGNAL_LENGTH];
	printf("START TIME SIGNAL\n");
	for (int i = 0; i < SIGNAL_LENGTH; ++i) {
		sig[i] = 1.8 * cos(2 * PI * ((double)(i)) / ((double)(SIGNAL_LENGTH)));
		printf("%f\n", sig[i]);
	}
	int ret = fftlib_spectra(sig);
	printf("return value %d\n", ret);
	printf("START SPECTRA ENERGIES\n");
	for (int i = 0; i < SIGNAL_LENGTH/2; ++i)
		printf("%f\n", sig[i]);
	return 0;
}
