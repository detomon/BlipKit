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

#ifndef _BK_BLIP_READER_H_
#define _BK_BLIP_READER_H_

#include "BKBase.h"

typedef struct BKBlipReader    BKBlipReader;
typedef struct BKBlipArgument  BKBlipArgument;
typedef struct BKBlipCommand   BKBlipCommand;

typedef long (* BKBlipReadCallback) (void * buffer, size_t size, void * userInfo);

struct BKBlipReader
{
	unsigned char    * buffer;
	unsigned char    * bufferPtr;
	size_t             bufferCapacity;
	BKBlipArgument   * argBuffer;
	BKBlipArgument   * argPtr;
	size_t             argBufferCapacity;
	unsigned char    * data;
	unsigned char    * dataPtr;
	size_t             dataSize;
	BKBlipReadCallback read;
	void             * userInfo;

};

struct BKBlipArgument
{
	char const * arg;
	size_t       size;
};

struct BKBlipCommand
{
	char const           * name;
	size_t                 nameSize;
	BKBlipArgument const * args;
	size_t                 argCount;
};

/**
 * 
 */
extern BKInt BKBlipReaderInit (BKBlipReader * reader, char const * data, size_t dataSize, BKBlipReadCallback read, void * userInfo);

/**
 * 
 */
extern void BKBlipReaderDispose (BKBlipReader * reader);

/**
 *
 */
extern BKInt BKBlipReaderNextCommand (BKBlipReader * reader, BKBlipCommand * command);

#endif /* ! _BK_BLIP_READER_H_ */
