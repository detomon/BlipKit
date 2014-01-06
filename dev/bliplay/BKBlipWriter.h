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

#ifndef _BK_BLIP_WRITER_H_
#define _BK_BLIP_WRITER_H_

#include "BKBase.h"

typedef struct BKBlipWriter BKBlipWriter;

typedef long (* BKBlipWriterCallback) (char * data, size_t size, void * userInfo);

struct BKBlipWriter
{
	size_t               argCount;
	unsigned char      * buffer;
	unsigned char      * bufferPtr;
	size_t               bufferCapacity;
	BKBlipWriterCallback write;
	void               * userInfo;
};

/**
 *
 */
extern BKInt BKBlipWriterInit (BKBlipWriter * writer, BKBlipWriterCallback write, void * userInfo);

/**
 *
 */
extern void BKBlipWriterDispose (BKBlipWriter * writer);

/**
 *
 */
extern BKInt BKBlipWriterBeginCommand (BKBlipWriter * writer, char const * name);

/**
 *
 */
extern BKInt BKBlipWriterAddIntArg (BKBlipWriter * writer, long value);

/**
 *
 */
extern BKInt BKBlipWriterAddFloatArg (BKBlipWriter * writer, double value);

/**
 * If `size` is 0 length of string is determined by `strlen`
 */
extern BKInt BKBlipWriterAddStringArg (BKBlipWriter * writer, char const * string, size_t size);

/**
 * Add base64 encoded argument
 */
extern BKInt BKBlipWriterAddDataArg (BKBlipWriter * writer, void const * data, size_t size);

/**
 *
 */
extern BKInt BKBlipWriterEndCommand (BKBlipWriter * writer);

#endif /* ! _BK_BLIP_WRITER_H_ */
