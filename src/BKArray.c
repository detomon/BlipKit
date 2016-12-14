#include "BKArray.h"

void BKArrayDispose (BKArray * arr)
{
	if (arr -> cap) {
		free (arr -> items);
		arr -> len = 0;
		arr -> cap = 0;
		arr -> items = NULL;
	}
}

BKInt BKArrayReserve (BKArray * arr, BKUSize size)
{
	void * items;
	BKUSize cap;

	cap = arr -> len + size;
	cap = (cap + 8ULL) & ~(8ULL - 1);

	if (cap >= arr -> cap) {
		cap += cap >> 1; // +1/2
		items = realloc (arr -> items, cap * arr -> itemSize);

		if (!items) {
			return -1;
		}

		arr -> items = items;
		arr -> cap = cap;
	}

	return 0;
}

BKInt BKArrayResize (BKArray * arr, BKUSize size)
{
	if (size > arr -> len) {
		if (arr -> cap < size) {
			if (BKArrayReserve (arr, size - arr -> len) != 0) {
				return -1;
			}
		}

		memset ((char *) arr -> items + arr -> len * arr -> itemSize, 0, (size - arr -> len) * arr -> itemSize);
		arr -> len = size;
	}

	return 0;
}

BKInt BKArrayAppendItems (BKArray * arr, void const * items, size_t count)
{
	if (BKArrayReserve (arr, count) != 0) {
		return -1;
	}

	memcpy (arr -> items + arr -> len * arr -> itemSize, items, count * arr -> itemSize);
	arr -> len += count;

	return 0;
}

BKInt BKArrayAppendArray (BKArray * str, BKArray const * other)
{
	return BKArrayAppendItems (str, other -> items, other -> len);
}
