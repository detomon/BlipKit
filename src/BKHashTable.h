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

#ifndef _BK_HASH_TABLE_H_
#define _BK_HASH_TABLE_H_

#include "BKBase.h"

typedef struct BKHashTable         BKHashTable;
typedef struct BKHashTableIterator BKHashTableIterator;

struct BKHashTableBucket
{
	BKUSize hash;
	char  * key;
	union {
		void  * item;
		int64_t value;
	};
};

struct BKHashTable
{
	BKUSize size;
	BKUSize capIdx;
	BKUSize occupied;
	struct BKHashTableBucket * buckets;
};

struct BKHashTableIterator
{
	struct BKHashTableBucket const * buckets;
	struct BKHashTableBucket const * bucketsEnd;
};

extern BKUSize const BKHashTableSizes [];

/**
 * Initialize hash table
 */
#define BK_HASH_TABLE_INIT (BKHashTable) {0}

/**
 * Dispose hash table
 */
extern void BKHashTableDispose (BKHashTable * table);

/**
 * Lookup item by key
 *
 * Returns 1 if key is found
 */
extern BKInt BKHashTableLookup (BKHashTable const * table, char const * key, void ** outItem);

/**
 * Lookup or insert item by key
 *
 * Returns 1 if item has been inserted
 */
extern BKInt BKHashTableLookupOrInsert (BKHashTable * table, char const * key, void *** outItemRef);

/**
 * Remove item by key
 */
extern BKInt BKHashTableRemove (BKHashTable * table, char const * key);

/**
 * Get hash table size
 */
BK_INLINE BKUSize BKHashTableSize (BKHashTable const * table);

/**
 * Initialize hash table interator
 */
BK_INLINE void BKHashTableIteratorInit (BKHashTableIterator * itor, BKHashTable const * table);

/**
 * Get next key/item pair from hash table iterator
 */
BK_INLINE BKInt BKHashTableIteratorNext (BKHashTableIterator * itor, char const ** outKey, void ** outItem);

/**
 * Empty hash table and keep capacity
 */
extern void BKHashTableEmpty (BKHashTable * table);


// --- Inline implementations

BK_INLINE BKUSize BKHashTableSize (BKHashTable const * table)
{
	return table -> size;
}

BK_INLINE void BKHashTableIteratorInit (BKHashTableIterator * itor, BKHashTable const * table)
{
	itor -> buckets = (struct BKHashTableBucket const *) table -> buckets;
	itor -> bucketsEnd = (struct BKHashTableBucket const *) &table -> buckets [BKHashTableSizes [table -> capIdx]];
}

BK_INLINE BKInt BKHashTableIteratorNext (BKHashTableIterator * itor, char const ** outKey, void ** outItem)
{
	if (itor -> buckets >= itor -> bucketsEnd) {
		return 0;
	}

	while (itor -> buckets < itor -> bucketsEnd) {
		if (itor -> buckets -> key > (char const *)1) {
			*outKey = itor -> buckets -> key;
			*outItem = itor -> buckets -> item;
			itor -> buckets ++;

			return 1;
		}

		itor -> buckets ++;
	}

	return 0;
}

#endif /* ! _BK_STRING_H_  */
