/*
 * Copyright (c) 2012-2015 Simon Schoenenberger
 * http://blipkit.audio
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
#include "BKFFT.h"
#include "BKObject.h"

extern BKClass BKFFTClass;

/**
 * Returns the next higher power of 2 which is the same as ceil(log2(value)).
 */
static BKUSize BKLog2 (BKSize value)
{
	BKUSize shift = 0;

	value -= 1;

	while (value) {
		value >>= 1;
		shift ++;
	}

	return shift;
}

/**
 * Initialize array of binary reversed indices to map the input samples to their
 * decomposed position as needed by the transform algorithm.
 */
static void BKFFTBitRevMapMake (BKInt indices [], BKUSize numIndices)
{
	BKUInt rev = 0, mask;
	BKUInt halfLen = (BKUInt) numIndices / 2;

	for (BKSize i = 0; i < numIndices - 1; i ++) {
		indices [i] = rev;
		mask = halfLen;

		while (rev >= mask) {
			rev -= mask;
			mask >>= 1;
		}

		rev += mask;
	}

	indices [numIndices - 1] = (BKUInt) numIndices - 1;
}

/**
 * Initialize unit sine and cosine waves.
 *
 * Due to the binary nature of the transform algorithm only one pre-calculated
 * sine and cosine wave is needed for all levels which then is strided with
 * different step size.
 */
static void BKFFTUnitWaveMake (BKComplex points [], BKUSize numPoints)
{
	BKComplex x;

	for (int i = 0; i < numPoints; i ++) {
		// W[i] = cos(PI * i / N) - j sin(PI * i / N)
		x = BKComplexMake (cos (M_PI * i / numPoints), -sin (M_PI * i / numPoints));
		points [i] = x;
	}
}

/**
 * Sort points by their bit reversal index.
 *
 * Sorting again will put them to their previous position.
 */
static void BKFFTSortBitReversed (BKComplex points [], BKUSize numPoints, BKInt bitRevMap [])
{
	BKInt ri;
	BKComplex x;

	for (BKInt i = 0; i < numPoints; i ++) {
		ri = bitRevMap [i];

		if (ri > i) {
			x           = points [i];
			points [i]  = points [ri];
			points [ri] = x;
		}
	}
}

/**
 * Multiply complex parts of every point in `points` with length `numPoints` by
 * `factor`.
 */
static void BKComplexListScale (BKComplex points [], BKUSize numPoints, BKComplexComp factor)
{
	BKComplex x;

	for (BKInt i = 0; i < numPoints; i ++) {
		x = points [i];
		points [i] = BKComplexMake (
			BKComplexReal (x) * factor,
			BKComplexImag (x) * factor
		);
	}
}

/**
 * Invert the sign of the imaginary part of every point of `points` with length
 * `numPoints`.
 */
static void BKComplexListConj (BKComplex points [], BKUSize numPoints)
{
	for (BKInt i = 0; i < numPoints; i ++) {
		points [i] = BKComplexConj (points [i]);
	}
}

/**
 * Convert `points` with length `numPoints` to their polar form
 */
static void BKComplexListToPolar (BKComplex points [], BKUSize numPoints)
{
	BKComplex x;
	BKComplexComp re, im;

	for (BKInt i = 0; i < numPoints; i ++) {
		x  = points [i];
		re = BKComplexReal (x);
		im = BKComplexImag (x);

		points [i] = BKComplexMake (
			hypot (re, im),
			atan2 (im, re)
		);
	}
}

/**
 * Convert `points` with length `numPoints` to their rectangular form.
 */
static void BKComplexListToRectangular (BKComplex points [], BKUSize numPoints)
{
	BKComplex x;
	BKComplexComp mag, ph;

	for (BKInt i = 0; i < numPoints; i ++) {
		x   = points [i];
		mag = BKComplexReal (x);
		ph  = BKComplexImag (x);

		points [i] = BKComplexMake (
			cos (ph) * mag,
			sin (ph) * mag
		);
	}
}

/**
 * Copy real part of `points` with length `numPoints` to `re`.
 */
static void BKComplexListCopyReal (BKComplexComp re [], BKComplex const points [], BKUSize numPoints)
{
	for (BKInt i = 0; i < numPoints; i ++) {
		re [i] = BKComplexReal (points [i]);
	}
}

BKInt BKFFTAlloc (BKFFT ** outFFT, BKUSize numSamples)
{
	BKFFT * fft;
	BKUSize size;
	BKUSize numBits;

	numSamples = BKMax (1, numSamples);
	numBits    = BKLog2 (numSamples);
	numSamples = (1 << numBits);  // set rounded up value

	// allocate all arrays at once
	// [BKFFT struct] + [input] + [output] + [bitRevMap] + [unitWaves]

	size = numSamples * sizeof (BKComplexComp)
	     + numSamples * sizeof (BKComplex)
	     + numSamples * sizeof (BKInt)
	     + numSamples * sizeof (BKComplex);

	if (BKObjectAlloc ((void **) & fft, & BKFFTClass, size) < 0) {
		return -1;
	}

	memset (& fft [1], 0, size);

	fft -> numSamples = numSamples;
	fft -> numBits    = numBits;
	fft -> input      = (void *) & fft [1];
	fft -> output     = (void *) fft -> input     + numSamples * sizeof (BKComplexComp);
	fft -> bitRevMap  = (void *) fft -> output    + numSamples * sizeof (BKComplex);
	fft -> unitWave   = (void *) fft -> bitRevMap + numSamples * sizeof (BKInt);

	BKFFTBitRevMapMake (fft -> bitRevMap, numSamples);
	BKFFTUnitWaveMake (fft -> unitWave, numSamples);

	*outFFT = fft;

	return 0;
}

static void BKFFTDispose (BKFFT * fft)
{
}

BKInt BKFFTSamplesLoad (BKFFT * fft, BKComplexComp const samples [], BKUSize numSamples, BKEnum options)
{
	BKComplex x;
	BKInt     bi;
	BKUSize   tailSize;

	if (numSamples > fft -> numSamples)
		numSamples = fft -> numSamples;

	tailSize = fft -> numSamples - numSamples;

	// shift existing samples to the left and append new samples
	if (options & BK_FFT_LOAD_SHIFT) {
		memmove (& fft -> input [0], & fft -> input [numSamples], tailSize * sizeof (BKComplexComp));
		memcpy (& fft -> input [tailSize], samples, numSamples * sizeof (BKComplexComp));
	}
	// overwrite existing sample and empty pending samples
	else {
		memcpy (& fft -> input [0], samples, numSamples * sizeof (BKComplexComp));
		memset (& fft -> input [tailSize], 0, tailSize * sizeof (BKComplexComp));
	}

	// remap samples to decomposed bit reversed index
	for (BKSize i = 0; i < fft -> numSamples; i ++) {
		x  = BKComplexMake (fft -> input [i], 0);
		bi = fft -> bitRevMap [i];

		fft -> output [bi] = x;
	}

	return 0;
}

/*
BKInt BKFFTSpectrumLoad (BKFFT * fft, BKComplex const points [], BKUSize numPoints)
{
	fprintf (stderr, "BKFFTSpectrumLoad: not implemented!\n");
	abort();

	return -1;
}
*/

/**
 * The FFT algorithm.
 */
static void BKFFTTransformForward (BKComplex points [], BKUSize numBits, BKComplex unitWave [])
{
	BKInt n = (1 << numBits);
	BKInt step, halfStep;
	BKInt waveStep;
	BKInt wi = 0;
	BKComplex u, t;

	for (BKInt l = 1; l <= numBits; l ++) {
		step     = (1 << l);
		halfStep = step / 2;
		waveStep = n / halfStep;
		wi       = 0;

		for (BKInt j = 1; j <= halfStep; j ++) {
			u = unitWave [wi];

			for (BKInt i = j - 1; i < n; i += step) {
				// Butterfly calculation
				//
				// T = X[i+halfStep] * U[wi]
				// X[i+halfStep] = X[i] - T
				// X[i] = X[i] + T

				t = BKComplexMult (points [i + halfStep], u);
				points [i + halfStep] = BKComplexSub (points [i], t);
				points [i] = BKComplexAdd (points [i], t);
			}

			wi += waveStep;
		}
	}
}

BKInt BKFFTTransform (BKFFT * fft, BKEnum options)
{
	if (options & BK_FFT_TRANS_INVERT) {
		if (options & BK_FFT_TRANS_POLAR)
			BKComplexListToRectangular (fft -> output, fft -> numSamples);

		BKFFTSortBitReversed (fft -> output, fft -> numSamples, fft -> bitRevMap);
		BKComplexListConj (fft -> output, fft -> numSamples);
	}

	BKFFTTransformForward (fft -> output, fft -> numBits, fft -> unitWave);

	if (options & BK_FFT_TRANS_NORMALIZE) {
		BKComplexComp factor;

		if (options & BK_FFT_TRANS_INVERT)
			factor = fft -> numSamples;
		else
			factor = 1.0 / fft -> numSamples;

		BKComplexListScale (fft -> output, fft -> numSamples, factor);
	}

	if (options & BK_FFT_TRANS_INVERT) {
		BKComplexListScale (fft -> output, fft -> numSamples, 1.0 / fft -> numSamples);
		BKComplexListCopyReal (fft -> input, fft -> output, fft -> numSamples);
	}
	else if (options & BK_FFT_TRANS_POLAR) {
		BKComplexListToPolar (fft -> output, fft -> numSamples);
	}

	return 0;
}

void BKFFTClear (BKFFT * fft)
{
	memset (fft -> input, 0, fft -> numSamples * sizeof (BKComplexComp));
	memset (fft -> output, 0, fft -> numSamples * sizeof (BKComplex));
}

BKClass BKFFTClass =
{
	.instanceSize = sizeof (BKFFT),
	.dispose      = (BKDisposeFunc) BKFFTDispose,
};
