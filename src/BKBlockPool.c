#include "BKBlockPool.h"

#define DEFAULT_SEGMENT_CAPACITY 32

BKInt BKBlockPoolInit (BKBlockPool * blockPool, BKUSize blockSize, BKUSize segmentCapacity)
{
	memset (blockPool, 0, sizeof (*blockPool));

	if (!segmentCapacity) {
		segmentCapacity = DEFAULT_SEGMENT_CAPACITY;
	}

	blockPool -> blockSize = blockSize;
	blockPool -> segmentSize = blockSize * segmentCapacity;

	return 0;
}

void BKBlockPoolDispose (BKBlockPool * blockPool)
{
	BKBlockPoolSegment * segment, * nextSegment;

	for (segment = blockPool -> firstSegment; segment; segment = nextSegment) {
		nextSegment = segment -> nextSegment;
		free (segment);
	}

	memset (blockPool, 0, sizeof (*blockPool));
}

static BKInt BKBlockPoolAppendSegment (BKBlockPool * blockPool)
{
	BKBlockPoolSegment * segment;

	segment = malloc (sizeof (*segment) + blockPool -> segmentSize);

	if (!segment) {
		return -1;
	}

	memset (segment, 0, sizeof (*segment));

	if (blockPool -> firstSegment) {
		blockPool -> lastSegment -> nextSegment = segment;
	}
	else {
		blockPool -> firstSegment = segment;
	}

	blockPool -> lastSegment = segment;
	blockPool -> freePtr = segment -> data;

	return 0;
}

BKInt BKBlockPoolEnsureBlock (BKBlockPool * blockPool)
{
	void * block = BKBlockPoolAlloc (blockPool);

	if (!block) {
		return -1;
	}

	BKBlockPoolFree (blockPool, block);

	return 0;
}

void * _BKBlockPoolAlloc (BKBlockPool * blockPool)
{
	BKBlockPoolBlock * block;

	if (!blockPool -> freePtr || (void *) blockPool -> freePtr >= (void *) blockPool -> lastSegment -> data + blockPool -> segmentSize) {
		if (BKBlockPoolAppendSegment (blockPool) != 0) {
			return NULL;
		}
	}

	block = (void *) blockPool -> freePtr;
	blockPool -> freePtr += blockPool -> blockSize;
	memset (block, 0, blockPool -> blockSize);

	return block;
}
