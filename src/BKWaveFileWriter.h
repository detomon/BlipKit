/**
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

#ifndef _BK_WAVE_FILE_WRITER_H_
#define _BK_WAVE_FILE_WRITER_H_

#include "BKBase.h"
#include "BKData.h"

typedef struct BKWaveFileWriter BKWaveFileWriter;

struct BKWaveFileWriter
{
	BKObject object;
	FILE   * file;
	BKSize   initOffset;
	BKInt    sampleRate;
	BKInt    numChannels;
	BKInt    numBits;
	BKSize   fileSize;
	BKSize   dataSize;
	BKInt    reverseEndian;
};

/**
 * Initialize WAVE file writer object
 *
 * Prepare a writer object to write frames to an opened and writable file
 * `file`. Number of channels `numChannels` defines the layout of the frames
 * which will be appended. `sampleRate` defines the sample rate the given frames.
 * `numBits` must be set to 8 or 16. If not given, the default is 16.
 *
 * The file and terminated and closed when disposing with `BKDispose`
 */
extern BKInt BKWaveFileWriterInit (BKWaveFileWriter * writer, FILE * file, BKInt numChannels, BKInt sampleRate, BKInt numBits);

/**
 * Append frames to WAVE file
 *
 * Write `frames` with length `numFrames` to WAVE file. The number of frames
 * should be a multiple of `numChannels` given at initialization.
 */
extern BKInt BKWaveFileWriterAppendFrames (BKWaveFileWriter * writer, BKFrame const * frames, BKInt numFrames);

/**
 * Terminate WAVE file
 *
 * If no more frames will be appended this function must be called to set the
 * required WAVE header values.
 */
extern BKInt BKWaveFileWriterTerminate (BKWaveFileWriter * writer);

/**
 * Write data object to WAVE file
 *
 * `file` must be an opened and writable file. `data` is a data object
 * containing the sound data. Data objects do not carry the sample rate of their
 * frames so the sample rate has to be given with `sampleRate`. `numBits` must
 * be set to 8 or 16. If not given, the default is 16.
 */
extern BKInt BKWaveFileWriteData (FILE * file, BKData const * data, BKInt sampleRate, BKInt numBits);

#endif /* ! _BK_WAVE_FILE_WRITER_H_ */
