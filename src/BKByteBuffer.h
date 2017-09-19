/*
 * Copyright (c) 2012-2016 Simon Schoenenberger
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

#include "BKBase.h"

typedef struct BKByteBuffer        BKByteBuffer;
typedef struct BKByteBufferSegment BKByteBufferSegment;

struct BKByteBufferSegment
{
	BKUSize               size;
	BKByteBufferSegment * next;
	uint8_t               data [];
};

struct BKByteBuffer
{
	uint8_t             * ptr;
	uint8_t             * ptrEnd;
	BKByteBufferSegment * cur;
	BKByteBufferSegment * first;
	BKByteBufferSegment * last;
};

/**
 * Initialize byte buffer
 */
#define BK_BYTE_BUFFER_INIT ((BKByteBuffer) {NULL})

/**
 * Free byte buffer
 */
extern void BKByteBufferDispose (BKByteBuffer * buf);

/**
 * Clear byte buffer and reuse allocated buffer
 */
extern void BKByteBufferClear (BKByteBuffer * buf);

/**
 * Append single `byte`
 */
BK_INLINE BKInt BKByteBufferAppendByte (BKByteBuffer * buf, uint8_t byte);

/**
 * Append `bytes` with `size`
 */
BK_INLINE BKInt BKByteBufferAppendBytes (BKByteBuffer * buf, void const * bytes, BKUSize size);

/**
 * Append 16 bit integer
 */
BK_INLINE BKInt BKByteBufferAppendInt16 (BKByteBuffer * buf, uint16_t i);

/**
 * Append 32 bit integer
 */
BK_INLINE BKInt BKByteBufferAppendInt32 (BKByteBuffer * buf, uint32_t i);

/**
 * Append 64 bit integer
 */
BK_INLINE BKInt BKByteBufferAppendInt64 (BKByteBuffer * buf, uint64_t i);

/**
 * Get size of buffer
 */
extern BKUSize BKByteBufferSize (BKByteBuffer const * buf);

/**
 * Reserve space for at least `size` bytes
 */
extern BKInt BKByteBufferReserve (BKByteBuffer * buf, BKUSize size);

/**
 * Merge byte into single segment
 */
extern BKInt BKByteBufferMakeContinuous (BKByteBuffer * buf);

/**
 * Copy byte into continuous buffer
 *
 * `outBytes` needs to have space for at least the number of byte retrieved
 * with `BKByteBufferSize`
 */
extern BKUSize BKByteBufferCopy (BKByteBuffer const * buf, void * outBytes);


// --- Inline implementations

BK_INLINE BKInt BKByteBufferAppendByte (BKByteBuffer * buf, uint8_t byte)
{
	BKInt res;

	if (buf -> ptr + 1 > buf -> ptrEnd) {
		if ((res = BKByteBufferReserve (buf, 1)) != 0) {
			return res;
		}
	}

	*buf -> ptr ++ = byte;

	return 0;
}

extern BKInt _BKByteBufferWrite (BKByteBuffer * buf, void const * bytes, BKUSize size);

BK_INLINE BKInt BKByteBufferAppendBytes (BKByteBuffer * buf, void const * bytes, BKUSize size)
{
	BKInt res;

	if (buf -> ptr + size <= buf -> ptrEnd) {
		memcpy (buf -> ptr, bytes, size);
		buf -> ptr += size;
	}
	else if ((res = _BKByteBufferWrite (buf, bytes, size)) != 0) {
		return res;
	}

	return 0;
}

BK_INLINE BKInt BKByteBufferAppendInt16 (BKByteBuffer * buf, uint16_t i)
{
	BKInt res;

	if (buf -> ptr + sizeof (i) <= buf -> ptrEnd) {
		*(uint16_t *) buf -> ptr = i;
		buf -> ptr += sizeof (i);
	}
	else if ((res = _BKByteBufferWrite (buf, &i, sizeof (i))) != 0) {
		return res;
	}

	return 0;
}

BK_INLINE BKInt BKByteBufferAppendInt32 (BKByteBuffer * buf, uint32_t i)
{
	BKInt res;

	if (buf -> ptr + sizeof (i) <= buf -> ptrEnd) {
		*(uint32_t *) buf -> ptr = i;
		buf -> ptr += sizeof (i);
	}
	else if ((res = _BKByteBufferWrite (buf, &i, sizeof (i))) != 0) {
		return res;
	}

	return 0;
}

BK_INLINE BKInt BKByteBufferAppendInt64 (BKByteBuffer * buf, uint64_t i)
{
	BKInt res;

	if (buf -> ptr + sizeof (i) <= buf -> ptrEnd) {
		*(uint64_t *) buf -> ptr = i;
		buf -> ptr += sizeof (i);
	}
	else if ((res = _BKByteBufferWrite (buf, &i, sizeof (i))) != 0) {
		return res;
	}

	return 0;
}

#endif /* ! _BK_BYTE_BUFFER_H_ */
