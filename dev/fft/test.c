#include <math.h>
#include <stdio.h>
#include <limits.h>
#include "BKFFT.h"
#include "BKComplex.h"

int main (int argc, char const * argv [])
{
	int n = 16;
	BKComplexComp x [n];

	BKFFT * fft = BKFFTCreate (n);

	for (BKInt i = 0; i < n; i ++) {
		//x [i] = sin (2.0 * M_PI * i / n * 1) + 2;
		//x [i] += sin (2.0 * M_PI * i / n * 2);

		x [i] = i < 6 ? 1 : 0;
	}

	BKFFTSamplesPush (fft, x, n);

	BKFFTTransform (fft, BKFFTTransformOptionNormalize);

	for (BKInt i = 0; i < fft -> numSamples; i ++) {
		BKComplex x = fft -> output [i];

		BKComplexComp real = BKComplexReal (x);
		BKComplexComp imag = BKComplexImag (x);

		BKComplexComp m = sqrt (real * real + imag * imag);
		BKComplexComp p = atan2 (imag, real);

		printf ("%4d %lf %lf\n", i, real, imag);
	}

	BKFFTDispose (fft);

	return 0;
}
