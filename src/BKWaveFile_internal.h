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
 */

#ifndef _BK_WAVE_FILE_INTERNAL_H_
#define _BK_WAVE_FILE_INTERNAL_H_

typedef struct BKWaveFileHeader     BKWaveFileHeader;
typedef struct BKWaveFileHeaderFmt  BKWaveFileHeaderFmt;
typedef struct BKWaveFileHeaderData BKWaveFileHeaderData;

/**
 * A WAVE file header.
 */
struct BKWaveFileHeader
{
	char     chunkID [4]; ///< The string "RIFF".
	uint32_t chunkSize;   ///< The file length without header.
	char     format [4];  ///< The string "WAVE".
};

/**
 * The format description header.
 */
struct BKWaveFileHeaderFmt
{
	char     subchunkID [4]; ///< The string "data".
	uint32_t subchunkSize;   ///< The data chunk size without header.
	uint16_t audioFormat;    ///< The value "1" for PCM.
	uint16_t numChannels;    ///< The number of channels.
	uint32_t sampleRate;     ///< The sample rate.
	uint32_t byteRate;       ///< sampleRate * numChannels * numBytes.
	uint16_t blockAlign;     ///< numChannels * numBytes.
	uint16_t bitsPerSample;  ///< Number or bits per sample.
};

/**
 * The data chunk header.
 */
struct BKWaveFileHeaderData
{
	char     subchunkID [4]; ///< The string "data".
	uint32_t subchunkSize;   ///< The chunk size without header.
	char     data [];        ///< The data.
};

/**
 * Check if the system uses big endian order.
 */
BK_INLINE BKInt BKSystemIsBigEndian (void)
{
	union { BKUInt i; char c [4]; } sentinel;

	sentinel.i = 0x01020304;

	return sentinel.c[0] == 0x01;
}

/**
 * Reverse byte order of a 32 bit integer.
 */
BK_INLINE uint32_t BKInt32Reverse (uint32_t i)
{
	char c;
	union { uint32_t i; char c [4]; } sentinel;

	sentinel.i = i;
	c = sentinel.c [0]; sentinel.c [0] = sentinel.c [3]; sentinel.c [3] = c;
	c = sentinel.c [1]; sentinel.c [1] = sentinel.c [2]; sentinel.c [2] = c;
	i = sentinel.i;

	return i;
}

/**
 * Reverse byte order of a 16 bit integer.
 */
BK_INLINE uint16_t BKInt16Reverse (uint16_t i)
{
	char c;
	union { uint16_t i; char c [2]; } sentinel;

	sentinel.i = i;
	c = sentinel.c [0]; sentinel.c [0] = sentinel.c [1]; sentinel.c [1] = c;
	i = sentinel.i;

	return i;
}

#endif /* ! _BK_WAVE_FILE_INTERNAL_H_ */
