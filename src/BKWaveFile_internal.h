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

#ifndef _BK_WAVE_FILE_INTERNAL_H_
#define _BK_WAVE_FILE_INTERNAL_H_

typedef struct BKWaveFileHeader     BKWaveFileHeader;
typedef struct BKWaveFileHeaderFmt  BKWaveFileHeaderFmt;
typedef struct BKWaveFileHeaderData BKWaveFileHeaderData;

struct BKWaveFileHeader
{
	char     chunkID [4];
	uint32_t chunkSize;
	char     format [4];
};

struct BKWaveFileHeaderFmt
{
	char     subchunkID [4];
	uint32_t subchunkSize;
	uint16_t audioFormat;
	uint16_t numChannels;
	uint32_t sampleRate;
	uint32_t byteRate;
	uint16_t blockAlign;
	uint16_t bitsPerSample;
};

struct BKWaveFileHeaderData
{
	char     subchunkID [4];
	uint32_t subchunkSize;
	char     data [];
};

static BKInt BKSystemIsBigEndian (void)
{
	union { BKUInt i; char c [4]; } sentinel;

	sentinel.i = 0x01020304;

	return sentinel.c[0] == 0x01;
}

static uint32_t BKInt32Reverse (uint32_t i)
{
	char c;
	union { uint32_t i; char c [4]; } sentinel;

	sentinel.i = i;
	c = sentinel.c [0]; sentinel.c [0] = sentinel.c [3]; sentinel.c [3] = c;
	c = sentinel.c [1]; sentinel.c [1] = sentinel.c [2]; sentinel.c [2] = c;
	i = sentinel.i;

	return i;
}

static uint16_t BKInt16Reverse (uint16_t i)
{
	char c;
	union { uint16_t i; char c [2]; } sentinel;

	sentinel.i = i;
	c = sentinel.c [0]; sentinel.c [0] = sentinel.c [1]; sentinel.c [1] = c;
	i = sentinel.i;

	return i;
}

#endif /* ! _BK_WAVE_FILE_INTERNAL_H_ */
