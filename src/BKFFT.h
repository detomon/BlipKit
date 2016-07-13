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

 /**
  * @file
  *
  * The Fast Fourier Transformation.
  */

#ifndef _BK_FFT_H_
#define _BK_FFT_H_

#include "BKObject.h"
#include "BKComplex.h"

/**
 * Transformations options.
 */
enum
{
	BK_FFT_TRANS_NORMALIZE = 1 << 0, ///< Normalize output after transformation.
	BK_FFT_TRANS_INVERT    = 1 << 1, ///< Invert transformation from output to input.
	BK_FFT_TRANS_POLAR     = 1 << 2, ///< Convert to or from polar representation,
	                                 ///< depending on if BK_FFT_TRANS_INVERT is set.
};

/**
 * Sample loading options.
 */
enum
{
	BK_FFT_LOAD_SHIFT = 1 << 0, ///< Shift existing sample to the left,
	                            ///< otherwise overwrite existing samples.
};

/**
 * The FFT object.
 */
typedef struct
{
	BKObject        object;     ///< The general object.
	BKUSize         numSamples; ///< Number of samples contained in buffer.
	BKUSize         numBits;    ///< Convenient access to log2(numSamples).
	BKComplexComp * input;      ///< The input samples. Length: numSamples
	BKComplex     * output;     ///< The output samples. Length: numSamples
	BKInt         * bitRevMap;  ///< Bit reversion map used for transformation. Length: numSamples
	BKComplex     * unitWave;   ///< Unit sine wave used for transformation. Length: numSamples
} BKFFT;

/**
 * Allocate FFT object.
 *
 * @p numSamples should be a power of 2 otherwise it is rounded up to the next
 * higher power of 2.
 *
 * @param outFFT A reference to an FFT object pointer.
 * @param numSamples Number of samples to used for the buffer.
 * @return 0 on success.
 */
extern BKInt BKFFTAlloc (BKFFT ** outFFT, BKUSize numSamples);

/**
 * Load new samples.
 *
 * Replace input samples with @p samples with length @p numSamples. If less
 * samples than the available capacity is given the rest is filled with 0.
 * If too many samples are given they are truncated. If option BK_FFT_LOAD_SHIFT
 * is set the old samples are shifted to the left and the new samples are
 * appended.
 *
 * @param fft The FFT object to load samples in.
 * @param samples The samples to be loaded.
 * @param numSamples The number of samples to be loaded.
 * @param options The loading options.
 * @return 0 on success. No errors are defined yet.
 */
extern BKInt BKFFTSamplesLoad (BKFFT * fft, BKComplexComp const samples [], BKUSize numSamples, BKEnum options);

/**
 * Transform input samples to output buffer `fft -> output`.
 *
 * If the option BK_FFT_TRANS_INVERT is set the points in the output buffer
 * are transformed back to the input samples.
 *
 * If option BK_FFT_TRANS_NORMALIZE is set the data in the output buffer is
 * normalized after transformation: X[i] = X[i] / N. In combination with
 * BK_FFT_TRANS_INVERT the points in the output buffer are expected to be
 * normalized already.
 *
 * If option BK_FFT_TRANS_POLAR is set the output points are convertedto their
 * polar form after transformation. In combination with BK_FFT_TRANS_INVERT the
 * points in the output buffer are expected to be in the polar form already.
 *
 * @param fft The FFT object to transform.
 * @param options The transform options.
 * @return 0 on success. No errors are defined yet.
 */
extern BKInt BKFFTTransform (BKFFT * fft, BKEnum options);

/**
 * Load spectrum points from f = 0.0 to f = 0.5 into output buffer.
 *
 * This is useful to make an inverse transformation of a existing spectrum.
 * `numPoints` must be `numSamples` / 2 + 1
 */
// NOT IMPLEMENTED YET
//extern BKInt BKFFTSpectrumLoad (BKFFT * fft, BKComplex const points [], BKUSize numPoints);

/**
 * Set all points of input and output buffers to 0.
 *
 * @param fft The FFT object to be emptied.
 */
extern void BKFFTClear (BKFFT * fft);

#endif /* ! _BK_FFT_H_ */
