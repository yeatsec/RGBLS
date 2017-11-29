/*
 * fftlib.c
 *
 *  Created on: Nov 25, 2017
 *      Author: esdev
 */

#include <math.h>
#include <stdlib.h>
#include "fftlib.h"

// singleton data members
double * real_roots;
double * imag_roots;
double * imag;

int * inds;

int size;
int bits;

int pow_of_two(int exponent)
{
	int ans = 1;
	for (int i = 0; i < exponent; ++i)
	{
		ans *= 2;
	}
	return ans;
}


int fftlib_init(int _size_in_bits)
{
	// enforce 2^n
	bits = _size_in_bits;
	size = pow_of_two(_size_in_bits);

	// check if initialized
	if (real_roots || imag_roots || inds)
	{
		return 1;
	}

	// allocate resources for roots and indexes
	real_roots = (double *)malloc(size*sizeof(double));
	imag_roots = (double *)malloc(size*sizeof(double));

	imag = (double *)malloc(size*sizeof(double));

	inds = (int *)malloc(size*sizeof(int));

	for(int i = 0; i < size; ++i)
	{
		real_roots[i] = cos(2.0 * M_PI * ((double)i)/((double)size));
		imag_roots[i] = sin(2.0 * M_PI * ((double)i)/((double)size));
		inds[i] = i;
	}

	// TODO - Bit switching here
	

	return 0;
}

/* input: real (time) data
 * output: spectra energy (magnitude squared)
 */
int fftlib_spectra(double * real)
{
	// initialize imaginary to 0
	for (int i = 0; i < size; ++i)
	{
		imag[i] = 0.0;
	}
	for (int stage = 0; stage < bits; ++stage)
	{
		int half_frame = pow_of_two(stage);
		int frame = 2 * half_frame;
		int rootSpacing = pow_of_two(bits-1-stage);
		for (int a = 0; a < size; a += frame)
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
	for (int i = 0; i < size; ++i)
	{
		real[i] *= real[i] * imag[i] * imag[i];
	}
	return 0;
}

int fftlib_free()
{
	free(real_roots);
	free(imag_roots);
	free(imag);
	free(inds);
	return 0;
}

