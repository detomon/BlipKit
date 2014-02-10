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

#ifndef _BK_FFT_H_
#define _BK_FFT_H_

#include <math.h>
#include "BKBase.h"
#include "BKComplex.h"

typedef enum
{
	BKFFTTransformOptionNormalized = 1 << 0,
	BKFFTTransformOptionInvert     = 1 << 1,
	BKFFTTransformOptionPolar      = 1 << 2,
} BKFFTTransformOption;

typedef enum
{
	BKFFTLoadingOptionShift = 1 << 0,
} BKFFTLoadingOption;

/**
 * The FFT object
 */
typedef struct
{
	BKUSize         numSamples; // number of samples
	BKUSize         numBits;    // convenient access to log2(numSamples)
	BKComplexComp * input;      // len: numSamples
	BKComplex     * output;     // len: numSamples
	BKInt         * bitRevMap;  // len: numSamples
	BKComplex     * unitWave;   // len: numSamples
} BKFFT;

/**
 * Create FFT object
 *
 * `numSamples` should be a power of 2 otherwise it is rounded up to the next
 * higher power of 2. If no memory could be allocated NULL is returned.
 */
extern BKFFT * BKFFTCreate (BKUSize numSamples);

/**
 * Dispose FFT object
 */
extern void BKFFTDispose (BKFFT * fft);

/**
 * Load new samples
 *
 * Replace input samples with `samples` with length `numSamples`. If less
 * samples than the available capacity is given the rest is filled with 0.
 * If too many samples are given they are truncated.
 * If option `BKFFTLoadingOptionShift` is set the old samples are shifted to the
 * left and the new samples are appended.
 */
extern BKInt BKFFTSamplesLoad (BKFFT * fft, BKComplexComp const samples [], BKUSize numSamples, BKEnum options);

/**
 * Transform input samples to output buffer `fft -> output`
 *
 * If option `BKFFTTransformOptionInvert` is set the points in the output buffer
 * are transformed back to the input samples.
 *
 * If option `BKFFTTransformOptionNormalized` is set the data in the output
 * buffer is normalized after transformation: X[i] = X[i] / N. In combination
 * with `BKFFTTransformOptionInvert` the points in the output buffer are
 * expected to be normalized already.
 *
 * If option `BKFFTTransformOptionPolar` is set the output points are converted
 * to their polar form after transformation. In combination with
 * `BKFFTTransformOptionInvert` the points in the output buffer are expected
 * to be in the polar form already.
 */
extern BKInt BKFFTTransform (BKFFT * fft, BKEnum options);

/**
 * Load spectrum points from f = 0.0 to f = 0.5 into output buffer
 *
 * This is useful to make an inverse transformation of a existing spectrum.
 * `numPoints` must be `numSamples` / 2 + 1
 */
// NOT IMPLEMENTED YET
//extern BKInt BKFFTSpectrumLoad (BKFFT * fft, BKComplex const points [], BKUSize numPoints);

/**
 * Set all points of input and output buffers to 0
 */
extern void BKFFTClear (BKFFT * fft);

#endif /* ! _BK_FFT_H_ */
