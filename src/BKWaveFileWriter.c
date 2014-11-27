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

#include "BKWaveFileWriter.h"
#include "BKWaveFile_internal.h"
#include <fcntl.h>

/**
 * https://ccrma.stanford.edu/courses/422/projects/WaveFormat/
 */

extern BKClass BKWaveFileWriterClass;

static BKWaveFileHeader const waveFileHeader =
{
	.chunkID   = "RIFF",
	.chunkSize = 0,
	.format    = "WAVE",
};

static BKWaveFileHeaderFmt const waveFileHeaderFmt =
{
	.subchunkID    = "fmt ",
	.subchunkSize  = 16,
	.audioFormat   = 1,
	.numChannels   = 2,
	.sampleRate    = 44100,
	.byteRate      = 44100 * 2 * 16/8,
	.blockAlign    = 2 * 16/8,
	.bitsPerSample = 16,
};

static BKWaveFileHeaderData const waveFileHeaderData =
{
	.subchunkID   = "data",
	.subchunkSize = 0,
};

static BKInt BKCheckFile (FILE * file)
{
	int fd, mode;

	if (fseek (file, 0, SEEK_CUR)) {
		return BK_FILE_NOT_SEEKABLE_ERROR;
	}

	fd   = fileno (file);
	mode = fcntl (fd, F_GETFL) & O_ACCMODE;

	if (mode < 0) {
		return BK_FILE_ERROR;
	}

	if (mode != O_RDWR && mode != O_WRONLY) {
		return BK_FILE_NOT_WRITABLE_ERROR;
	}

	return 0;
}

BKInt BKWaveFileWriterInit (BKWaveFileWriter * writer, FILE * file, BKInt numChannels, BKInt sampleRate)
{
	BKInt res;

	if ((res = BKCheckFile (file)) != 0) {
		return res;
	}

	if (BKObjectInit (writer, & BKWaveFileWriterClass, sizeof (*writer)) < 0) {
		return -1;
	}

	writer -> file          = file;
	writer -> initOffset    = ftell (file);
	writer -> sampleRate    = sampleRate;
	writer -> numChannels   = numChannels;
	writer -> reverseEndian = BKSystemIsBigEndian ();

	return 0;
}

void BKWaveFileWriterDispose (BKWaveFileWriter * writer)
{
}

static BKInt BKWaveFileWriterWriteHeader (BKWaveFileWriter * writer)
{
	BKInt                numChannels, sampleRate, numBits;
	BKWaveFileHeader     header;
	BKWaveFileHeaderFmt  fmtHeader;
	BKWaveFileHeaderData dataHeader;

	header     = waveFileHeader;
	fmtHeader  = waveFileHeaderFmt;
	dataHeader = waveFileHeaderData;

	numChannels = writer -> numChannels;
	sampleRate  = writer -> sampleRate;
	numBits     = 16;

	fmtHeader.numChannels   = numChannels;
	fmtHeader.sampleRate    = sampleRate;
	fmtHeader.bitsPerSample = numBits;
	fmtHeader.blockAlign    = numChannels * numBits / 8;
	fmtHeader.byteRate      = sampleRate * fmtHeader.blockAlign;

	if (writer -> reverseEndian) {
		fmtHeader.subchunkSize  = BKInt32Reverse (fmtHeader.subchunkSize);
		fmtHeader.numChannels   = BKInt16Reverse (fmtHeader.numChannels);
		fmtHeader.sampleRate    = BKInt32Reverse (fmtHeader.sampleRate);
		fmtHeader.bitsPerSample = BKInt16Reverse (fmtHeader.bitsPerSample);
		fmtHeader.blockAlign    = BKInt16Reverse (fmtHeader.blockAlign);
		fmtHeader.byteRate      = BKInt32Reverse (fmtHeader.byteRate);
	}

	fwrite (& header, sizeof (header), 1, writer -> file);
	fwrite (& fmtHeader, sizeof (fmtHeader), 1, writer -> file);
	fwrite (& dataHeader, sizeof (dataHeader), 1, writer -> file);

	writer -> fileSize += ftell (writer -> file) - 8;

	return 0;
}

BKInt BKWaveFileWriterAppendFrames (BKWaveFileWriter * writer, BKFrame const * frames, BKInt numFrames)
{
	BKSize  dataSize;
	BKSize  writeSize;
	BKSize  bufferSize = 1024;
	BKFrame buffer [bufferSize];

	if (writer -> dataSize == 0) {
		if (BKWaveFileWriterWriteHeader (writer) < 0) {
			return -1;
		}
	}

	dataSize = sizeof (BKFrame) * numFrames;

	if (writer -> reverseEndian) {
		while (numFrames) {
			writeSize = BKMin (bufferSize, numFrames);

			for (BKInt i = 0; i < writeSize; i ++) {
				buffer [i] = BKInt16Reverse (frames [i]);
			}

			frames += writeSize;
			numFrames -= writeSize;

			fwrite (buffer, sizeof (BKFrame), writeSize, writer -> file);
		}
	}
	else {
		fwrite (frames, sizeof (BKFrame), numFrames, writer -> file);
	}

	writer -> fileSize += dataSize;
	writer -> dataSize += dataSize;

	return 0;
}

BKInt BKWaveFileWriterTerminate (BKWaveFileWriter * writer)
{
	BKSize   offset;
	uint32_t chunkSize;

	offset = writer -> initOffset + offsetof (BKWaveFileHeader, chunkSize);
	chunkSize = writer -> fileSize;

	if (writer -> reverseEndian) {
		chunkSize = BKInt32Reverse (chunkSize);
	}

	fseek (writer -> file, offset, SEEK_SET);
	fwrite (& chunkSize, sizeof (chunkSize), 1, writer -> file);

	offset = writer -> initOffset + sizeof (BKWaveFileHeader) + sizeof (BKWaveFileHeaderFmt);
	offset += offsetof (BKWaveFileHeaderData, subchunkSize);

	chunkSize = writer -> dataSize;

	if (writer -> reverseEndian) {
		chunkSize = BKInt32Reverse (chunkSize);
	}

	fseek (writer -> file, offset, SEEK_SET);
	fwrite (& chunkSize, sizeof (chunkSize), 1, writer -> file);

	fseek (writer -> file, 0, SEEK_END);
	fflush (writer -> file);

	return 0;
}

BKInt BKWaveFileWriteData (FILE * file, BKData const * data, BKInt sampleRate)
{
	BKWaveFileWriter writer;

	if (BKWaveFileWriterInit (& writer, file, sampleRate, data -> numChannels) < 0) {
		return -1;
	}

	BKWaveFileWriterAppendFrames (& writer, data -> frames, data -> numFrames * data -> numChannels);
	BKWaveFileWriterTerminate (& writer);

	BKDispose (& writer);

	return 0;
}

BKClass BKWaveFileWriterClass =
{
	.instanceSize = sizeof (BKWaveFileWriter),
	.dispose      = (BKDisposeFunc) BKWaveFileWriterDispose,
};
