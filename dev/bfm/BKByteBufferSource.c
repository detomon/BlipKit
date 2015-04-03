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

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "BKByteBuffer.h"

#define FILE_READ_BUFFER 0x4000

struct BKByteBufferFileSource
{
	BKByteBufferSourceReadHandle    read;
	BKByteBufferSourceDestroyHandle destroy;
	int                             fildes;
};

static BKSize BKByteBufferFileSourceRead (BKByteBuffer * buffer, struct BKByteBufferFileSource * source)
{
	BKSize readSize;
	char    bytes [FILE_READ_BUFFER];
	
	readSize = read (source -> fildes, bytes, FILE_READ_BUFFER);

	if (readSize > 0) {
		readSize = BKByteBufferWriteBytes (buffer, bytes, readSize);
		
		//if (readSize < 0)
		//	moErrorRaiseAppend ("BKByteBufferFileSourceRead", "Couldn't write bytes into buffer");
	}
	else if (readSize < 0) {
		//moErrorRaise ("BKByteBufferFileSourceRead", "Couldn't read file");
		readSize = -1;
	}

	return readSize;
}

static void BKByteBufferFileSourceDestroy (struct BKByteBufferFileSource * source)
{
	close (source -> fildes);
}

BKInt BKByteBufferFileSourceInit (BKByteBufferSource * source, int fildes)
{
	struct BKByteBufferFileSource * stdSource = (struct BKByteBufferFileSource *) source;

	stdSource -> read    = (BKByteBufferSourceReadHandle)    BKByteBufferFileSourceRead;
	stdSource -> destroy = (BKByteBufferSourceDestroyHandle) BKByteBufferFileSourceDestroy;	
	stdSource -> fildes  = fildes;

	return 0;
}

BKInt BKByteBufferFileSourceInitWithFilename (BKByteBufferSource * source, const char * path)
{
	int fildes = open (path, O_RDONLY);

	if (fildes != -1) {
		if (BKByteBufferFileSourceInit (source, fildes)) {
			return 0;
		}
		else {
			//moErrorRaise ("BKByteBufferFileSourceInitWithFilename", "Couldn't initialize buffer source");
			close (fildes);
		}
	}
	else {
		//moErrorRaise ("BKByteBufferFileSourceInitWithFilename", "Couldn't open file '%s'", path);
	}

	return -1;
}

void BKByteBufferSourceDestroy (BKByteBufferSource * source)
{
	if (source -> destroy)
		source -> destroy (source);

	memset (source, 0, sizeof (BKByteBufferSource));
}
