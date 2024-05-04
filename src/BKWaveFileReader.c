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

#include "BKWaveFileReader.h"
#include "BKWaveFile_internal.h"
#include <fcntl.h>
#include <stdio.h>

extern BKClass BKWaveFileReaderClass;

static BKInt BKCheckFile(FILE* file) {
	if (fseek(file, 0, SEEK_CUR)) {
		return BK_FILE_NOT_SEEKABLE_ERROR;
	}

	int fd = fileno(file);
	int mode = fcntl(fd, F_GETFL) & O_ACCMODE;

	if (mode < 0) {
		return BK_FILE_ERROR;
	}

	if (mode != O_RDWR && mode != O_RDONLY) {
		return BK_FILE_NOT_READABLE_ERROR;
	}

	return 0;
}

BKInt BKWaveFileReaderInit(BKWaveFileReader* reader, FILE* file) {
	BKInt res = BKCheckFile(file);

	if (res != 0) {
		return res;
	}

	if (BKObjectInit(reader, &BKWaveFileReaderClass, sizeof(*reader)) < 0) {
		return -1;
	}

	reader->file = file;

	return 0;
}

static void BKWaveFileReaderDispose(BKWaveFileReader* reader) {
}

static void BKWaveFileHeaderFmtRead(BKWaveFileHeaderFmt* headerFmt) {
	if (BKSystemIsBigEndian()) {
		headerFmt->subchunkSize = BKInt32Reverse(headerFmt->subchunkSize);
		headerFmt->audioFormat = BKInt16Reverse(headerFmt->audioFormat);
		headerFmt->numChannels = BKInt16Reverse(headerFmt->numChannels);
		headerFmt->sampleRate = BKInt32Reverse(headerFmt->sampleRate);
		headerFmt->byteRate = BKInt32Reverse(headerFmt->byteRate);
		headerFmt->blockAlign = BKInt16Reverse(headerFmt->blockAlign);
		headerFmt->bitsPerSample = BKInt16Reverse(headerFmt->bitsPerSample);
	}
}

static void BKWaveFileHeaderDataRead(BKWaveFileHeaderData* headerData) {
	if (BKSystemIsBigEndian()) {
		headerData->subchunkSize = BKInt32Reverse(headerData->subchunkSize);
	}
}

BKInt BKWaveFileReaderReadHeader(BKWaveFileReader* reader, BKInt* outNumChannels, BKInt* outSampleRate, BKInt* outNumFrames) {
	if (reader->sampleRate == 0) {
		BKWaveFileHeader header;
		BKSize size = fread(&header, 1, sizeof(header), reader->file);

		if (size < sizeof(header)) {
			return -1;
		}

		if (memcmp(header.chunkID, "RIFF", 4) != 0) {
			return -1;
		}

		if (memcmp(header.format, "WAVE", 4) != 0) {
			return -1;
		}

		BKWaveFileHeaderFmt headerFmt;
		BKWaveFileHeaderData headerData;

		size = fread(&headerFmt, 1, sizeof(headerFmt), reader->file);

		if (size < sizeof(headerFmt)) {
			return -1;
		}

		BKWaveFileHeaderFmtRead(&headerFmt);

		if (memcmp(headerFmt.subchunkID, "fmt ", 4) != 0) {
			return -1;
		}

		if (headerFmt.audioFormat != 1) {
			return -1;
		}

		if (headerFmt.bitsPerSample != 8 && headerFmt.bitsPerSample != 16) {
			return -1;
		}

		do {
			size = fread(&headerData, 1, sizeof(headerData), reader->file);

			if (size < sizeof(headerData)) {
				return -1;
			}

			BKWaveFileHeaderDataRead(&headerData);

			if (memcmp(headerData.subchunkID, "data", 4) == 0) {
				break;
			}

			// seek to next subchunk
			fseek(reader->file, headerData.subchunkSize, SEEK_CUR);
		}
		while (1);

		reader->sampleRate = headerFmt.sampleRate;
		reader->numChannels = headerFmt.numChannels;
		reader->numBits = headerFmt.bitsPerSample;

		BKSize frameSize;

		switch (reader->numBits) {
			case 8: {
				frameSize = 1;
				break;
			}
			default:
			case 16: {
				frameSize = 2;
				break;
			}
		}

		reader->dataSize = headerData.subchunkSize;

		// read everything after data chunk if size not set
		if (!reader->dataSize) {
			BKSize size;
			BKSize cur = ftell(reader->file);

			if (fseek(reader->file, 0, SEEK_END) < 0) {
				return -1;
			}

			size = ftell(reader->file);
			fseek(reader->file, cur, SEEK_SET);
			reader->dataSize = size - cur;
		}

		reader->numFrames = (BKInt)reader->dataSize / reader->numChannels / frameSize;
	}

	*outNumChannels = reader->numChannels;
	*outSampleRate = reader->sampleRate;
	*outNumFrames = reader->numFrames;

	return 0;
}

BKInt BKWaveFileReaderReadFrames(BKWaveFileReader* reader, BKFrame outFrames[]) {
	char buffer[1024];
	BKSize frameSize;
	BKUInt readMask;

	switch (reader->numBits) {
		case 8: {
			frameSize = 1;
			readMask = 0;
			break;
		}
		default:
		case 16: {
			frameSize = 2;
			readMask = 1;
			break;
		}
	}

	BKSize writeSize = 0;
	BKInt reverseEndian = BKSystemIsBigEndian();
	BKSize remainingSize = reader->dataSize;
	readMask = ~readMask;

	while (remainingSize) {
		BKSize readSize = BKMin(remainingSize, sizeof(buffer));
		readSize = readSize & readMask;
		readSize = fread(buffer, 1, readSize, reader->file);
		readSize = readSize & readMask;

		// truncated
		if (readSize == 0) {
			break;
		}

		char* bufferPtr = buffer;

		remainingSize -= readSize;
		writeSize += readSize;

		switch (frameSize) {
			case 1: {
				while (readSize) {
					BKFrame frame = (((BKFrame)(*(uint8_t*)bufferPtr)) - 128) << 8;
					(*outFrames++) = frame;

					bufferPtr += frameSize;
					readSize -= frameSize;
				}
				break;
			}
			case 2: {
				while (readSize) {
					BKFrame frame = (*(BKFrame*)bufferPtr);

					if (reverseEndian) {
						frame = BKInt16Reverse(frame);
					}

					(*outFrames++) = frame;

					bufferPtr += frameSize;
					readSize -= frameSize;
				}
				break;
			}
		}
	}

	// empty trailing byte if any
	memset(outFrames, 0, reader->dataSize - writeSize);

	return 0;
}

BKClass BKWaveFileReaderClass = {
	.instanceSize = sizeof(BKWaveFileReader),
	.dispose = (BKDisposeFunc)BKWaveFileReaderDispose,
};
