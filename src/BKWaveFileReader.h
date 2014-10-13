/**
 * Copyright (c) 2014 Simon Schoenenberger
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

#ifndef _BK_WAVE_FILE_READER_H_
#define _BK_WAVE_FILE_READER_H_

#include <stdio.h>
#include "BKBase.h"
#include "BKData.h"

typedef struct BKWaveFileReader BKWaveFileReader;

struct BKWaveFileReader
{
	FILE * file;
	BKInt  sampleRate;
	BKInt  numChannels;
	BKInt  numBits;
	BKInt  numFrames;
	BKSize dataSize;
};

/**
 * Initialize WAVE file reader object
 *
 * Only PCM format with 8 or 16 bits can be read at the moment.
 */
extern BKInt BKWaveFileReaderInit (BKWaveFileReader * reader, FILE * file);

/**
 * Dispose reader object
 *
 * This doesn't close the file given at initialization.
 */
extern void BKWaveFileReaderDispose (BKWaveFileReader * reader);

/**
 * Read header of WAVE file
 *
 * Returns the number of channels (`outNumChannels`) and sample rate
 * (`outSampleRate`). The number of frames contained in one channel is returned
 * in `outNumFrames`.
 */
extern BKInt BKWaveFileReaderReadHeader (BKWaveFileReader * reader, BKInt * outNumChannels, BKInt * outSampleRate, BKInt * outNumFrames);

/**
 * Read frames of WAVE file
 *
 * Buffer `outFrames` must be large enough to hold `outNumChannels` * `outNumFrames`
 * frames returned by `BKWaveFileReaderReadHeader`. If number of bits is 8 frames
 * are converted to 16 bits.
 */
extern BKInt BKWaveFileReaderReadFrames (BKWaveFileReader * reader, BKFrame outFrames []);

#endif /* ! _BK_WAVE_FILE_READER_H_ */
