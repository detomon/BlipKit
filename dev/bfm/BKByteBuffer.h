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

#ifndef _BK_BYTE_BUFFER_H_
#define _BK_BYTE_BUFFER_H_

/**
 * FIFO byte buffer
 * Write operations append data at the end of the buffer
 * Read operations read data from the beginning of the buffer
 * The storage behaviour can be controlled with `BKByteBufferInitOption`
 *
 * `BKByteBufferSource` can be used if data should be retreived dynamically
 * (see `BKByteBufferSource` for more info)
 */

#include "BKByteBufferSource.h"

typedef struct BKByteBuffer        BKByteBuffer;
typedef struct BKByteBufferSegment BKByteBufferSegment;

struct BKByteBuffer
{
	unsigned              info;         //
	BKUSize               capacity;     // Capacity of active segments
	BKUSize               readSize;     // Capacity of read segments
	BKByteBufferSegment * firstSegment; //
	BKByteBufferSegment * freeSegments; // Reusable segments; only single linked
	BKByteBufferSegment * writeSegment; //
	char                * writeCursor;  //
	char                * writeDataEnd; //
	BKByteBufferSegment * readSegment;  //
	char                * readCursor;   //
	char                * readDataEnd;  //
	BKByteBufferSource  * source;       // Called when buffer runs empty
};

struct BKByteBufferSegment
{
	BKByteBufferSegment * nextSegment;
	BKByteBufferSegment * previousSegment;
	BKUSize               capacity;
	char                  data [];
};

enum BKByteBufferInitOption
{
	BKByteBufferOptionKeepBytes         = 1 << 0,
	BKByteBufferOptionContinuousStorage = 1 << 1,
};

enum BKByteBufferOption
{
	BKByteBufferOptionDiscardReaded     = 1 << 2,
	BKByteBufferOptionReuseStorage      = 1 << 3,
};

enum BKByteBufferSeek
{
	BKByteBufferSeekRestore             = 1 << 0,
	BKByteBufferSeekSet                 = 1 << 1,
};

/**
 * Initialze buffer struct `buffer`
 * At least `initSize` bytes are reserved
 * A combination of the following options may be used:
 *
 * `BKByteBufferOptionKeepBytes`:
 *   If set, read bytes are kept in buffer
 *   If not set, read bytes are automatically discarded
 * `BKByteBufferOptionContinuousStorage`:
 *   If set, bytes are hold in a continuous memory block (slower)
 *   If not set, bytes are hold in segmented memory blocks (faster)
 *
 *  Returns false if initialization fails
 */
extern BKInt BKByteBufferInit (BKByteBuffer * buffer, BKUSize initSize, unsigned options);

/**
 * Free buffer and discard data
 */
extern void BKByteBufferDispose (BKByteBuffer * buffer);

/**
 * Set buffer source
 * see `BKByteBufferSource` for more info
 */
extern void BKByteBufferSetSource (BKByteBuffer * buffer, BKByteBufferSource * source);

/**
 * Read a maximum of `size` bytes at beginning of buffer into `bytes`
 * If `bytes` is NULL only the read cursor is moved
 * Returns the number of bytes read
 * Returns 0 if end of buffer is reached
 */
extern BKSize BKByteBufferReadBytes (BKByteBuffer * buffer, void * bytes, BKUSize size);

/**
 * Read one byte at beginning of buffer
 * Returns -1 if end of buffer is reached
 */
extern int BKByteBufferReadByte (BKByteBuffer * buffer);

/**
 * Returns a pointer to the unread bytes
 * If option `BKByteBufferOptionContinuousStorage` is not set NULL is returned
 */
extern void * BKByteBufferGetBytes (BKByteBuffer * buffer);

/**
 * Append `size` bytes at end of buffer
 * Returns `size` otherwise -1 if no memory could be allocated
 */
extern BKSize BKByteBufferWriteBytes (BKByteBuffer * buffer, const void * bytes, BKUSize size);

/**
 * Append one byte at end of buffer
 * Returns 1 otherwise -1 if no memory could be allocated
 */
extern BKSize BKByteBufferWriteByte (BKByteBuffer * buffer, unsigned char byte);

/**
 * Get number of remaining bytes to read
 */
extern BKUSize BKByteBufferGetSize (BKByteBuffer * buffer);

/**
 * Get offset of read cursor
 * If option `BKByteBufferOptionKeepBytes` is not set -1 is returned
 */
extern BKSize BKByteBufferGetOffset (BKByteBuffer * buffer);

/**
 * Seek read cursor to specific offset
 * Either option `BKByteBufferSeekRestore` or `BKByteBufferSeekSet` should be set
 *
 * `BKByteBufferSeekRestore`:
 *   Restore maximum `offset` number of bytes
 *   The actual number of restored bytes is returned
 * `BKByteBufferSeekSet`:
 *   Set read cursor to absolute position
 *   If offset is outside of valid range -1 is returned
 *   If buffer was not initialized with option `BKByteBufferOptionKeepBytes` -1 is returned
 *   The absolute position is returned
 */
extern BKSize BKByteBufferSeek (BKByteBuffer * buffer, BKSize offset, unsigned options);

/**
 * Remove data from buffer
 * A combination of the following options may be used:
 *
 * `BKByteBufferOptionDiscardReaded`:
 *   Discard read bytes
 *   If not set all bytes are discarded
 * `BKByteBufferOptionReuseStorage`:
 *   Allocated storage is not freed but reused for new data
 */
extern void BKByteBufferClear (BKByteBuffer * buffer, unsigned options);

#endif /* ! _BK_BYTE_BUFFER_H_ */
