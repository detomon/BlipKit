#ifndef _BK_ARRAY_H_
#define _BK_ARRAY_H_

#include "BKBase.h"

typedef struct BKArray BKArray;

struct BKArray
{
	void * items;
	size_t len;
	size_t cap;
	size_t itemSize;
};

/**
 * Initialize array struct
 */
#define BK_ARRAY_INIT(itemSize) ((BKArray) {NULL, 0, 0, (itemSize)})

/**
 * Dispose array
 */
extern void BKArrayDispose (BKArray * arr);

/**
 * Reserve space for `size` items
 */
extern BKInt BKArrayReserve (BKArray * arr, size_t size);

/**
 * Reserve and empty slots
 */
extern BKInt BKArrayResize (BKArray * arr, size_t size);

/**
 * Append empty item and return its pointer
 */
BK_INLINE void * BKArrayPush (BKArray * arr);

/**
 * Remove last item
 */
BK_INLINE void BKArrayPop (BKArray * arr);

/**
 * Get last item
 */
BK_INLINE void * BKArrayLast (BKArray * arr);

/**
 * Append `items` with `size`
 */
extern BKInt BKArrayAppendItems (BKArray * arr, void const * items, size_t size);

/**
 * Append items from `other` array
 */
extern BKInt BKArrayAppendArray (BKArray * arr, BKArray const * other);

/**
 * Get item at index
 */
BK_INLINE void * BKArrayItemAt (BKArray * arr, size_t idx);

/**
 * Empty aray and keep capacity
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

BK_INLINE void * BKArrayLast (BKArray * arr)
{
	return BKArrayItemAt (arr, arr -> len - 1);
}

BK_INLINE void * BKArrayItemAt (BKArray * arr, size_t idx)
{
	if (idx >= arr -> len) {
		return NULL;
	}

	return arr -> items + arr -> itemSize * idx;
}

BK_INLINE void BKArrayEmpty (BKArray * arr)
{
	arr -> len = 0;
}

#endif /* ! _BK_ARRAY_H_  */
