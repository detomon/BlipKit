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
