/*
 * fftlib.h
 *
 *  Created on: Nov 25, 2017
 *      Author: esdev
 */

#ifndef FFTLIB_H_
#define FFTLIB_H_

int fftlib_init(int _size);

int fftlib_spectra(double * buffer);

int fftlib_free();

#endif /* FFTLIB_H_ */
