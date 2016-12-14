#include "BKByteBuffer.h"

#define SEG_MIN_CAPACITY 4096

static BKByteBufferSegment * BKByteBufferSegmentAllocate (BKUSize size)
{
	BKByteBufferSegment * seg;

	if (size < SEG_MIN_CAPACITY) {
		size = SEG_MIN_CAPACITY;
	}

	seg = malloc (sizeof (*seg) + size);

	if (!seg) {
		return NULL;
	}

	*seg = (BKByteBufferSegment) {0};
	seg -> size = size;

	return seg;
}

static void BKByteBufferFreeSegments (BKByteBuffer * buf)
{
	BKByteBufferSegment * seg, * next;

	for (seg = buf -> first; seg; seg = next) {
		next = seg -> next;
		free (seg);
	}
}

void BKByteBufferDispose (BKByteBuffer * buf)
{
	BKByteBufferFreeSegments (buf);
	*buf = (BKByteBuffer) {NULL};
}

void BKByteBufferClear (BKByteBuffer * buf)
{
	buf -> cur    = buf -> first;
	buf -> ptr    = buf -> cur -> data;
	buf -> ptrEnd = buf -> cur -> data + buf -> cur -> size;
}

static BKUSize BKByteBufferRemainingSize (BKByteBuffer const * buf)
{
	BKUSize size = 0;
	BKByteBufferSegment const * seg;

	if (buf -> cur) {
		seg = buf -> cur -> next;
		size = buf -> ptrEnd - buf -> ptr;

		for (; seg; seg = seg -> next) {
			size += seg -> size;
		}
	}

	return size;
}

BKUSize BKByteBufferSize (BKByteBuffer const * buf)
{
	BKUSize size = 0;
	BKByteBufferSegment const * seg;

	for (seg = buf -> first; seg != buf -> cur; seg = seg -> next) {
		size += seg -> size;
	}

	if (buf -> cur) {
		size += buf -> ptr - buf -> cur -> data;
	}

	return size;
}

BKInt BKByteBufferReserve (BKByteBuffer * buf, BKUSize size)
{
	BKUSize segSize;
	BKUSize remaining = BKByteBufferRemainingSize (buf);
	BKByteBufferSegment * seg;

	if (remaining < size) {
		segSize = size - remaining;
		seg = BKByteBufferSegmentAllocate (segSize);

		if (!seg) {
			return -1;
		}

		if (buf -> last) {
			buf -> last -> next = seg;
			buf -> last = seg;

			if (buf -> ptr >= buf -> ptrEnd) {
				seg = buf -> cur -> next;
				buf -> ptr = seg -> data;
				buf -> ptrEnd = seg -> data + seg -> size;
			}
		}
		else {
			buf -> cur = seg;
			buf -> first = seg;
			buf -> last = seg;
			buf -> ptr = seg -> data;
			buf -> ptrEnd = seg -> data + seg -> size;
		}
	}
	else if (buf -> ptr >= buf -> ptrEnd) {
		seg = buf -> cur;

		if (seg -> next) {
			seg = seg -> next;
			buf -> ptr = seg -> data;
			buf -> ptrEnd = seg -> data + seg -> size;
		}
	}

	return 0;
}

BKInt BKByteBufferMakeContinuous (BKByteBuffer * buf)
{
	BKUSize size;
	BKByteBufferSegment * segment;

	if (buf -> cur != buf -> first) {
		size = BKByteBufferSize (buf);
		segment = BKByteBufferSegmentAllocate (size);

		if (!segment) {
			return -1;
		}

		BKByteBufferCopy (buf, segment -> data);
		BKByteBufferFreeSegments (buf);

		buf -> cur = segment;
		buf -> last = segment;
		buf -> first = segment;
		buf -> ptr = segment -> data + size;
		buf -> ptrEnd = &segment -> data [segment -> size];
	}

	return 0;
}

BKInt _BKByteBufferWrite (BKByteBuffer * buf, void const * bytes, BKUSize size)
{
	BKInt res;
	BKUSize remaining;
	BKByteBufferSegment * seg;

	if ((res = BKByteBufferReserve (buf, size)) != 0) {
		return res;
	}

	remaining = buf -> ptrEnd - buf -> ptr;
	remaining = size < remaining ? size : remaining;
	memcpy (buf -> ptr, bytes, remaining);
	buf -> ptr += remaining;
	bytes += remaining;
	size -= remaining;

	while (size) {
		if (buf -> ptr >= buf -> ptrEnd) {
			buf -> cur = buf -> cur -> next;
			seg = buf -> cur;
			buf -> ptr = seg -> data;
			buf -> ptrEnd = seg -> data + seg -> size;
		}

		remaining = buf -> ptrEnd - buf -> ptr;
		remaining = size < remaining ? size : remaining;
		memcpy (buf -> ptr, bytes, remaining);
		buf -> ptr += remaining;
		bytes += remaining;
		size -= remaining;
	}

	return 0;
}

BKUSize BKByteBufferCopy (BKByteBuffer const * buf, void * outBytes)
{
	BKUSize size = 0;
	BKByteBufferSegment * seg;

	for (seg = buf -> first; seg != buf -> cur; seg = seg -> next) {
		memcpy (outBytes, seg -> data, seg -> size);
		outBytes += seg -> size;
		size += seg -> size;
	}

	seg = buf -> cur;

	if (seg) {
		memcpy (outBytes, seg -> data, buf -> ptr - seg -> data);
		size += buf -> ptr - seg -> data;
	}

	return size;
}
