/**
 * Copyright (c) 2012-2014 Simon Schoenenberger
 * http://blipkit.monoxid.net/
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <math.h>
#include <stdio.h>
#include <limits.h>
#include "BKFFT.h"
#include "BKComplex.h"

int main (int argc, char const * argv [])
{
	// the buffer will be 16 sample wide
	int n = 16;

	// sample buffer
	BKComplexComp x [n];

	// create FFT object
	BKFFT * fft = BKFFTCreate (n);

	printf ("Input:\n");

	// fill samples
	// 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
	for (BKInt i = 0; i < n; i ++) {
		x [i] = (i < 6) ? 1.0 : 0.0;

		printf ("%4d % lf:\n", i, x [i]);
	}

	// fill whole buffer
	BKFFTSamplesLoad (fft, x, n, 0);

	// transform samples forward
	BKFFTTransform (fft, 0);

	printf ("\nTransformed output:\n");

	// print frequency domain
	for (BKInt i = 0; i < fft -> numSamples; i ++) {
		BKComplex x = fft -> output [i];

		BKComplexComp re = BKComplexReal (x);
		BKComplexComp im = BKComplexImag (x);

		printf ("%4d % lf % lf\n", i, re, im);
	}

	// transform backwards
	BKFFTTransform (fft, BKFFTTransformOptionInvert);

	printf ("\nReversed output:\n");

	// print frequency domain which corresponds now to the input due to the
	// inversion (except some rounding noise)
	for (BKInt i = 0; i < fft -> numSamples; i ++) {
		BKComplex x = fft -> output [i];

		BKComplexComp re = BKComplexReal (x);
		BKComplexComp im = BKComplexImag (x);

		printf ("%4d % lf % lf\n", i, re, im);
	}

	BKFFTDispose (fft);

	return 0;
}
