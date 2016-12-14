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
 *
 * A memory pool for fast allocation of memory blocks with the same size.
 *
 * Memory blocks are allocated in linked segments containing multiple blocks at
 * once. If a segment is full, an new segment is allocated and linked to the
 * previous one. Free blocks are single linked and reused when requested.
 */

#ifndef _BK_BLOCK_POOL_H_
#define _BK_BLOCK_POOL_H_

#include "BKBase.h"

typedef struct BKBlockPool        BKBlockPool;
typedef struct BKBlockPoolSegment BKBlockPoolSegment;
typedef struct BKBlockPoolBlock   BKBlockPoolBlock;

/**
 * A linked memory segment containing multiple blocks.
 */
struct BKBlockPoolSegment
{
	BKBlockPoolSegment * nextSegment; ///< The next block segment.
	char data [];                     ///< Segment blocks.
};

/**
 * An unused linked block.
 */
struct BKBlockPoolBlock
{
	BKBlockPoolBlock * nextBlock; ///< The next free block.
};

/**
 * The memory pool struct.
 */
struct BKBlockPool
{
	BKUSize              blockSize;    ///< The block size.
	BKUSize              segmentSize;  ///< The usable segment size.
	BKBlockPoolSegment * lastSegment;  ///< The last linked memory segment
	void               * freePtr;      ///< The next free block of the last segment.
	BKBlockPoolSegment * firstSegment; ///< The first memory segment.
	BKBlockPoolBlock   * freeBlocks;   ///< A linked list of reusable blocks.
};

/**
 * Initialize block pool.
 *
 * @param blockPool The block pool to be initialized.
 * @param blockSize The block size to be allocated.
 * @param segmentCapacity The number of blocks in a segment. When 0 is given, the default is used.
 * @return 0 on success.
 */
extern BKInt BKBlockPoolInit (BKBlockPool * blockPool, BKUSize blockSize, BKUSize segmentCapacity);

/**
 * Dispose block pool and free all segments.
 *
 * @param blockPool The block pool to be disposed.
 */
extern void BKBlockPoolDispose (BKBlockPool * blockPool);

/**
 * Ensure that at least one block is present.
 *
 * @param blockPool The block pool to ensure a block for.
 * @return 0 on success.
 */
extern BKInt BKBlockPoolEnsureBlock (BKBlockPool * blockPool);

/**
 * Allocate block.
 *
 * @param blockPool The block pool to allocate a block from.
 * @return A new allocated and emptied block.
 */
BK_INLINE void * BKBlockPoolAlloc (BKBlockPool * blockPool);

/**
 * Free block.
 *
 * @param blockPool The block pool to free a block from.
 * @param block The block to be freed.
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
		block = (BKBlockPoolBlock *) _BKBlockPoolAlloc (blockPool);
	}

	return block;
}

BK_INLINE void BKBlockPoolFree (BKBlockPool * blockPool, void * block)
{
	BKBlockPoolBlock * blockPtr = (BKBlockPoolBlock *) block;

	if (!blockPtr) {
		return;
	}

	blockPtr -> nextBlock = blockPool -> freeBlocks;
	blockPool -> freeBlocks = blockPtr;
}

#endif /* ! _BK_TK_BLOCK_POOL_H_ */
