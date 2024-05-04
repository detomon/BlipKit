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
 * A WAVE file writer.
 *
 * @code{.c}
 * BKWaveFileWriter writer;
 *
 * BKWaveFileWriterInit (& writer, file, 2, 44100, 16);
 * BKWaveFileWriterAppendFrames (& writer, frames, numFrames);
 * BKWaveFileWriterTerminate (& writer);
 * BKDispose (& writer);
 * @endcode
 */

#ifndef _BK_WAVE_FILE_WRITER_H_
#define _BK_WAVE_FILE_WRITER_H_

#include "BKBase.h"
#include "BKData.h"

typedef struct BKWaveFileWriter BKWaveFileWriter;

/**
 * The WAVE file writer struct.
 */
struct BKWaveFileWriter {
	BKObject object;	 ///< The general object.
	FILE* file;			 ///< The file to be written to.
	BKSize initOffset;	 ///< The initial file cursor offset. Used to update the header when terminating.
	BKInt sampleRate;	 ///< The sample rate to be written to the header.
	BKInt numChannels;	 ///< Number of channels to be written to the header.
	BKInt numBits;		 ///< Number of bits to be used to write the frames.
	BKSize fileSize;	 ///< The number of bytes of the data chunk.
	BKSize dataSize;	 ///< The number of data bytes written to the data chunk.
	BKInt reverseEndian; ///< Whether the endian order should be reversed.
						 ///< 16 bit frames are written in little-endian order.
						 ///< If the system uses big-endian order, the given frame bytes has to be reversed.
};

/**
 * Initialize WAVE file writer object.
 *
 * Prepare a writer object to write frames to an opened and writable file
 * `file`. Number of channels `numChannels` defines the layout of the frames
 * which will be appended. `sampleRate` defines the sample rate the given frames.
 * `numBits` must be set to 8 or 16. If not given, the default is 16.
 *
 * The file is not closed when the reader is disposed with `BKDispose`.
 *
 * @param writer The WAVE file writer to initialize.
 * @param file The file to write to.
 * @param numChannels The number of channels to write.
 * @param sampleRate The sample rate to write.
 * @param numBits The number of bits to write. Can be 8 or 16.
 * @return 0 on success.
 */
extern BKInt BKWaveFileWriterInit(BKWaveFileWriter* writer, FILE* file, BKInt numChannels, BKInt sampleRate, BKInt numBits);

/**
 * Append frames to WAVE file.
 *
 * Write `frames` with length `numFrames` to WAVE file. The number of frames
 * should be a multiple of `numChannels` given at initialization.
 *
 * @param writer The writer to append frames to.
 * @param frames The frames to append.
 * @param numFrames The number of frames to append.
 * @return 0 on success.
 */
extern BKInt BKWaveFileWriterAppendFrames(BKWaveFileWriter* writer, BKFrame const* frames, BKInt numFrames);

/**
 * Terminate WAVE file.
 *
 * If no more frames will be appended, this function must be called to set the
 * required WAVE header values.
 *
 * @param writer The writer to terminate.
 * @return 0 on success.
 */
extern BKInt BKWaveFileWriterTerminate(BKWaveFileWriter* writer);

/**
 * Write data object to WAVE file.
 *
 * `file` must be an opened and writable file. `data` is a data object
 * containing the sound data. Data objects do not carry the sample rate of their
 * frames so the sample rate has to be given with `sampleRate`. `numBits` must
 * be set to 8 or 16. If not given, the default is 16.
 *
 * @param file The file to write to.
 * @param data The data object to write.
 * @param sampleRate The sample rate to write.
 * @param numBits The number of bits to write.
 * @return 0 on success. Can be 8 or 16.
 */
extern BKInt BKWaveFileWriteData(FILE* file, BKData const* data, BKInt sampleRate, BKInt numBits);

#endif /* ! _BK_WAVE_FILE_WRITER_H_ */
