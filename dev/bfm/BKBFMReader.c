/**
 * Copyright (c) 2012-2013 Simon Schoenenberger
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

#include "BKBFMReader.h"

enum
{
	BKBFMReaderFlagMagicRead = 1 << 0,
	BKBFMReaderFlagBinary    = 1 << 1,
	BKBFMReaderFlagText      = 1 << 2,
};

BKInt BKBFMReaderInit (BKBFMReader * reader)
{
	memset (reader, 0, sizeof (* reader));

	if (BKByteBufferInit (& reader -> buffer, 0, 0) != 0)
		return -1;

	return 0;
}

void BKBFMReaderDispose (BKBFMReader * reader)
{
	BKByteBufferDispose (& reader -> buffer);

	memset (reader, 0, sizeof (* reader));
}

static BKInt BKBFMReaderReadVarInt (BKBFMReader * reader)
{
	BKInt n = 0;
	BKInt c = 0;

	do {
		c = BKByteBufferReadByte (& reader -> buffer);
		n = (n << 7) | (c & 0x7F);
	}
	while (c & 0x80);

	// is negative
	if (n & 1) {
		n = -(n >> 1);
	}
	else {
		n >>= 1;
	}

	return n;
}

static BKInt BKBFMReaderReadMagic (BKBFMReader * reader)
{
	BKSize size;
	char   magic [14];

	// read magic token
	size = BKByteBufferReadBytes (& reader -> buffer, magic, sizeof (magic));

	if (size < sizeof (magic))
		return -1;

	// seek before header
	BKByteBufferSeek (& reader -> buffer, size, BKByteBufferSeekRestore);

	// format is binary
	if (memcmp (magic, "\x02\08\06bfm\x08\x08blip\x06\x02", 14) == 0) {
		reader -> flags |= BKBFMReaderFlagBinary;
	}
	// format is text
	else /*if (memcmp (magic, "[bfm:blip:1;", 12) == 0)*/ {
		reader -> flags |= BKBFMReaderFlagText;
	}

	reader -> flags |= BKBFMReaderFlagMagicRead;

	return 0;
}

BKInt BKBFMReaderNextToken (BKBFMReader * reader, BKBFMToken * outToken)
{
	if ((reader -> flags & BKBFMReaderFlagMagicRead) == 0) {
		if (BKBFMReaderReadMagic (reader) < 0)
			return -1;
	}

	// ...

	return 0;
}
