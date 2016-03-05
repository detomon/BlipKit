#ifndef _BK_BLOCK_POOL_H_
#define _BK_BLOCK_POOL_H_

#include "BKBase.h"

typedef struct BKBlockPool        BKBlockPool;
typedef struct BKBlockPoolSegment BKBlockPoolSegment;
typedef struct BKBlockPoolBlock   BKBlockPoolBlock;

struct BKBlockPoolSegment
{
	BKBlockPoolSegment * nextSegment;
	char data [];
};

struct BKBlockPoolBlock
{
	BKBlockPoolBlock * nextBlock;
};

struct BKBlockPool
{
	BKSize               blockSize;
	BKSize               segmentSize;
	BKBlockPoolSegment * lastSegment;
	void               * freePtr;
	BKBlockPoolSegment * firstSegment;
	BKBlockPoolBlock   * freeBlocks;
};

/**
 * Initialize block pool
 */
extern BKInt BKBlockPoolInit (BKBlockPool * blockPool, BKSize blockSize, BKSize segmentCapacity);

/**
 * Dispose block pool
 */
extern void BKBlockPoolDispose (BKBlockPool * blockPool);

/**
 * Append segment (internal)
 */
extern BKInt BKBlockPoolAppendSegment (BKBlockPool * blockPool);

/**
 * Ensure that at least one block is present
 */
extern BKInt BKBlockPoolEnsureBlock (BKBlockPool * blockPool);

/**
 * Allocate block
 */
BK_INLINE void * BKBlockPoolAlloc (BKBlockPool * blockPool);

/**
 * Free block
 */
BK_INLINE void BKBlockPoolFree (BKBlockPool * blockPool, void * block);


// --- Inline implementations

extern void * _BKBlockPoolAlloc (BKBlockPool * blockPool);

BK_INLINE void * BKBlockPoolAlloc (BKBlockPool * blockPool)
{
	BKBlockPoolBlock * block;

	if (blockPool -> freeBlocks) {
		block = blockPool -> freeBlocks;
		blockPool -> freeBlocks = block -> nextBlock;
		memset (block, 0, blockPool -> blockSize);
	}
	else {
		block = _BKBlockPoolAlloc (blockPool);
	}

	return block;
}

BK_INLINE void BKBlockPoolFree (BKBlockPool * blockPool, void * block)
{
	BKBlockPoolBlock * blockPtr = block;

	if (!blockPtr) {
		return;
	}

	blockPtr -> nextBlock = blockPool -> freeBlocks;
	blockPool -> freeBlocks = blockPtr;
}

#endif /* ! _BK_TK_BLOCK_POOL_H_ */
