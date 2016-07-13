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
 * A WAVE file reader.
 *
 * @code{.c}
 * BKWaveFileReader reader;
 * BKInt numChannels, sampleRate, numFrames
 * BKFrame * frames;
 *
 * BKWaveFileReaderInit (& reader, file);
 * BKWaveFileReaderReadHeader (& reader, & numChannels, & sampleRate, & numFrames);
 *
 * frames = malloc (numFrames * numChannels);
 * BKWaveFileReaderReadFrames (& reader, frames);
 *
 * BKDispose (& reader);
 * @endcode
 */

#ifndef _BK_WAVE_FILE_READER_H_
#define _BK_WAVE_FILE_READER_H_

#include "BKBase.h"
#include "BKData.h"

typedef struct BKWaveFileReader BKWaveFileReader;

/**
 * The WAVE file reader struct.
 */
struct BKWaveFileReader
{
	BKObject object;      ///< The general object.
	FILE   * file;        ///< The file to be read from.
	BKInt    sampleRate;  ///< The sample rate.
	BKInt    numChannels; ///< Number of channels.
	BKInt    numBits;     ///< Number of bits.
	BKInt    numFrames;   ///< Number of frames.
	BKSize   dataSize;    ///< Size if data chunk.
};

/**
 * Initialize WAVE file reader object.
 *
 * Only PCM format with 8 or 16 bits are supported. The file is not closed when
 * disposing with `BKDispose`.
 *
 * @param reader The reader to initialize.
 * @param file The file to write to.
 * @param 0 on success.
 */
extern BKInt BKWaveFileReaderInit (BKWaveFileReader * reader, FILE * file);

/**
 * Read header of WAVE file.
 *
 * Returns the number of channels (`outNumChannels`) and sample rate
 * (`outSampleRate`). The number of frames contained in one channel is returned
 * in `outNumFrames`.
 *
 * @param reader The reader to read the header from.
 * @param outNumChannels Outputs the number of channels.
 * @param outSampleRate Outputs the sample rate.
 * @param outNumFrames Outputs number of frames.
 * @return 0 on success.
 */
extern BKInt BKWaveFileReaderReadHeader (BKWaveFileReader * reader, BKInt * outNumChannels, BKInt * outSampleRate, BKInt * outNumFrames);

/**
 * Read frames of WAVE file.
 *
 * Buffer `outFrames` must be large enough to hold `outNumChannels` * `outNumFrames`
 * frames returned by `BKWaveFileReaderReadHeader`. If number of bits is 8, the frames
 * are converted to 16 bits.
 *
 * @param reader The reader to read the frames from.
 * @param outFrames The frame buffer to read into.
 * @return 0 on success
 */
extern BKInt BKWaveFileReaderReadFrames (BKWaveFileReader * reader, BKFrame outFrames []);

#endif /* ! _BK_WAVE_FILE_READER_H_ */
