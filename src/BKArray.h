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

/**
 * @file
 * Defines a generic resizable array
 */

#ifndef _BK_ARRAY_H_
#define _BK_ARRAY_H_

#include "BKBase.h"

typedef struct BKArray BKArray;

struct BKArray
{
	void  * items;
	BKUSize len;
	BKUSize cap;
	BKUSize itemSize;
};

/**
 * Initialize array struct
 */
#define BK_ARRAY_INIT(itemSize) (BKArray) {NULL, 0, 0, (itemSize)}

/**
 * Free allocated space
 */
extern void BKArrayDispose (BKArray * arr);

/**
 * Reserve space for `size` items
 */
extern BKInt BKArrayReserve (BKArray * arr, BKUSize size);

/**
 * Reserve and empty slots
 */
extern BKInt BKArrayResize (BKArray * arr, BKUSize size);

/**
 * Append empty item and return its pointer
 */
BK_INLINE void * BKArrayPush (BKArray * arr);

/**
 * Remove last item
 */
BK_INLINE void BKArrayPop (BKArray * arr);

/**
 * Get pointer to last item
 */
BK_INLINE void * BKArrayLast (BKArray const * arr);

/**
 * Append `count` `items`. The item size has to match that one given at
 * initialization.
 */
extern BKInt BKArrayAppendItems (BKArray * arr, void const * items, size_t count);

/**
 * Append items from `other` array
 */
extern BKInt BKArrayAppendArray (BKArray * arr, BKArray const * other);

/**
 * Get item at specified `index`. If `index` is out of bounds, `NULL` is returned.
 */
BK_INLINE void * BKArrayItemAt (BKArray const * arr, size_t index);

/**
 * Empty array but keep allocated space
 */
BK_INLINE void BKArrayEmpty (BKArray * arr);


// --- Inline implementations

BK_INLINE void * BKArrayPush (BKArray * arr)
{
	void * item;

	if (arr -> len >= arr -> cap) {
		if (BKArrayReserve (arr, 1) != 0) {
			return NULL;
		}
	}

	item = arr -> items + arr -> len * arr -> itemSize;
	memset (item, 0, arr -> itemSize);
	arr -> len ++;

	return item;
}

BK_INLINE void BKArrayPop (BKArray * arr)
{
	if (arr -> len > 0) {
		arr -> len --;
	}
}

BK_INLINE void * BKArrayLast (BKArray const * arr)
{
	return BKArrayItemAt (arr, arr -> len - 1);
}

BK_INLINE void * BKArrayItemAt (BKArray const * arr, size_t index)
{
	if (index >= arr -> len) {
		return NULL;
	}

	return arr -> items + arr -> itemSize * index;
}

BK_INLINE void BKArrayEmpty (BKArray * arr)
{
	arr -> len = 0;
}

#endif /* ! _BK_ARRAY_H_  */
