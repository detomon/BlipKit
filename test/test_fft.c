#include "test.h"
#include "BKFFT.h"

int main (int argc, char const * argv [])
{
	BKInt res;
	BKFFT * fft = INVALID_PTR;

	// the buffer will be 16 sample wide
	int n = 16;

	// sample buffer
	BKComplexComp x [n];
	BKComplexComp y [n];

	// create FFT object

	res = BKFFTAlloc (& fft, n);

	if (res != 0) {
		fprintf (stderr, "Allocation failed\n");
		return 99;
	}

	if (fft == INVALID_PTR || fft == NULL) {
		fprintf (stderr, "Invalid pointer\n");
		return 99;
	}

	// fill samples
	// 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
	for (BKInt i = 0; i < n; i ++) {
		x [i] = (i < 6) ? 1.0 : 0.0;
		y [i] = (i < 6) ? 1.0 : 0.0;
	}

	// fill whole buffer
	BKFFTSamplesLoad (fft, x, n, 0);

	// transform samples forward
	BKFFTTransform (fft, 0);

	// transform backwards
	BKFFTTransform (fft, BK_FFT_TRANS_INVERT);

	// print frequency domain which corresponds now to the input due to the
	// inversion (except some rounding noise)
	for (BKInt i = 0; i < fft -> numSamples; i ++) {
		BKComplex x = fft -> output [i];

		BKComplexComp rex = BKComplexReal (x);
		BKComplexComp imx = BKComplexImag (x);
		BKComplexComp rey = y [i];

		if (BKAbs (rex - rey) > 0.000001) {
			fprintf (stderr, "Sample at index %d differs from origin value (%f %f)\n", i, rex, rey);
			return 99;
		}

		if (BKAbs (imx) > 0.000001) {
			fprintf (stderr, "Imaginary part at index %d not 0 %f\n", i, imx);
			return 99;
		}
	}

	BKDispose (fft);

	return 0;
}
