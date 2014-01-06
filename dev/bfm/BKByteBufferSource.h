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

#ifndef _BK_BYTE_BUFFER_SOURCE_H_
#define _BK_BYTE_BUFFER_SOURCE_H_

#include "BKBase.h"

/**
 * Buffer source
 * Callback `read` is called every time the byte buffer runs empty
 * On success `read` should return a number greater than 0
 * If no more byte are available `read` should return 0
 * On error `read` should return -1
 */

typedef struct BKByteBufferSource BKByteBufferSource;
typedef BKSize (* BKByteBufferSourceReadHandle) (void * buffer, BKByteBufferSource * source);
typedef void (* BKByteBufferSourceDestroyHandle) (void * source);

/**
 * Field `private` can be used for private data 
 */
struct BKByteBufferSource
{
	BKByteBufferSourceReadHandle    read;
	BKByteBufferSourceDestroyHandle destroy;
	void                          * private [4];
};

/**
 * Initialize buffer source struct with file descriptor
 * File descriptor is not closed after calling `BKByteBufferSourceDestroy`
 * Returns `false` if an error occurred
 */
extern BKInt BKByteBufferFileSourceInit (BKByteBufferSource * source, int fildes);

/**
 * Initialize file buffer source struct with filename
 * Returns `false` if file couldn't be opened and raises an error
 */
extern BKInt BKByteBufferFileSourceInitWithFilename (BKByteBufferSource * source, const char * path);

/**
 * Destroy buffer source
 * Calls `destroy` of `source`
 */
extern void BKByteBufferSourceDestroy (BKByteBufferSource * source);

#endif /* ! _BK_BYTE_BUFFER_SOURCE_H_ */
