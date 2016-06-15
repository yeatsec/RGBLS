/*
 * fftlib.c
 *
 *  Created on: Nov 25, 2017
 *      Author: esdev
 */

#include <stdlib.h>
//#include <stdio.h>
#include "fftlib.h"

#define	BITS	5
#define	SIZE	32
#define	HALF_SIZE	16

// singleton data members
static const double real_roots[SIZE] = {1.0, 0.98078528, 0.92387953, 0.83146961, 0.70710678, 0.55557023, 0.38268343, 0.19509032, 0.0, -0.19509032, -0.38268343, -0.55557023, -0.70710678, -0.83146961, -0.92387953, -0.98078528, -1.0, -0.98078528, -0.92387953, -0.83146961, -0.70710678, -0.55557023, -0.38268343, -0.19509032, -0.0, 0.19509032, 0.38268343, 0.55557023, 0.70710678, 0.83146961, 0.92387953, 0.98078528};
static const double imag_roots[SIZE] = {0.0, 0.19509032, 0.38268343, 0.55557023, 0.70710678, 0.83146961, 0.92387953, 0.98078528, 1.0, 0.98078528, 0.92387953, 0.83146961, 0.70710678, 0.55557023, 0.38268343, 0.19509032, 0.0, -0.19509032, -0.38268343, -0.55557023, -0.70710678, -0.83146961, -0.92387953, -0.98078528, -1.0, -0.98078528, -0.92387953, -0.83146961, -0.70710678, -0.55557023, -0.38268343, -0.19509032};
static double imag[SIZE];

static const int inds[SIZE] = {0, 16, 8, 24, 4, 20, 12, 28, 2, 18, 10, 26, 6, 22, 14, 30, 1, 17, 9, 25, 5, 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31};
static int pow_of_two(int exponent)
{
	int ans = 1;
	for (int i = 0; i < exponent; ++i)
	{
		ans *= 2;
	}
	return ans;
}

/* input: real (time) data
 * output: spectra energy (magnitude squared)
 */
// TODO - CHECK SHUFFLING
int fftlib_spectra(double * real)
{
	double temp[SIZE];
	// reorder spectra data
	for (int i = 0; i < SIZE; ++i)
		temp[i] = real[inds[i]];

	for (int i = 0; i < SIZE; ++i)
		real[i] = temp[i];

	/*printf("START UNPROCESSED SPECTRA\n");
	// initialize imaginary to 0
	for (int i = 0; i < SIZE; ++i)
	{
		printf("%f\n", real[i]);
		imag[i] = 0.0;
	}
	*/
	for (int stage = 0; stage < BITS; ++stage)
	{
		int half_frame = pow_of_two(stage);
		int frame = 2 * half_frame;
		int rootSpacing = pow_of_two(BITS-1-stage);
		for (int a = 0; a < SIZE; a += frame)
		{
			for (int b = 0; b < half_frame; ++b)
			{
				int left_butterfly_index = a + b;
				int right_butterfly_index = left_butterfly_index + half_frame;
				int left_root_index = b * rootSpacing;
				int right_root_index = (b + half_frame) * rootSpacing;
				/* First Half of Butterfly Calculation */
				double real_left = real[left_butterfly_index] + (real[right_butterfly_index] * real_roots[left_root_index]) - (imag[right_butterfly_index] * imag_roots[left_root_index]);
				double imag_left = imag[left_butterfly_index] + (real[right_butterfly_index] * imag_roots[left_root_index]) + (imag[right_butterfly_index] * real_roots[left_root_index]);
				/* Second Half of Butterfly Calculation */
				double real_right = real[left_butterfly_index] + (real[right_butterfly_index] * real_roots[right_root_index]) - (imag[right_butterfly_index] * imag_roots[right_root_index]);
				double imag_right = imag[left_butterfly_index] + (real[right_butterfly_index] * imag_roots[right_root_index]) + (imag[right_butterfly_index] * real_roots[right_root_index]);
				real[left_butterfly_index] = real_left;
				imag[left_butterfly_index] = imag_left;
				real[right_butterfly_index] = real_right;
				imag[right_butterfly_index] = imag_right;
			}
		}
	}
	/* For Debugging Purposes
	printf("START REALS\n");
	for (int i = 0; i < SIZE; ++i)
		printf("%f\n", real[i]);
	printf("START IMAGINARIES\n");
	for (int i = 0; i < SIZE; ++i)
		printf("%f\n", imag[i]);
	printf("START INTERNAL ENERGIES\n");
	*/
	for (int i = 0; i < HALF_SIZE; ++i)
	{
		real[i] = (real[i] * real[i]) + (imag[i] * imag[i]);
	}
	return 0;
}

