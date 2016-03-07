/**
 * Copyright (c) 2016 Simon Schoenenberger
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
	BKUSize              blockSize;
	BKUSize              segmentSize;
	BKBlockPoolSegment * lastSegment;
	void               * freePtr;
	BKBlockPoolSegment * firstSegment;
	BKBlockPoolBlock   * freeBlocks;
};

/**
 * Initialize block pool
 */
extern BKInt BKBlockPoolInit (BKBlockPool * blockPool, BKUSize blockSize, BKUSize segmentCapacity);

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
