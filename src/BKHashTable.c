#include "BKHashTable.h"
#include "BKString.h"

#define PLACEHOLDER_KEY ((char const *)1)

BKUSize const BKHashTableSizes [30] =
{
	0,
	7,
	13,
	31,
	61,
	127,
	251,
	509,
	1021,
	2039,
	4093,
	8191,
	16381,
	32749,
	65521,
	131071,
	262139,
	524287,
	1048573,
	2097143,
	4194301,
	8388593,
	16777213,
	33554393,
	67108859,
	134217689,
	268435399,
	536870909,
	1073741789,
	2147483647,
};

static BKUSize BKHashTableHash (char const * key)
{
	BKUSize hash;

	for (hash = 0; *key; key ++) {
		hash = (hash * 100003) ^ *key;
	}

	return hash;
}

static struct BKHashTableBucket * BKHashTableBucketLookup (struct BKHashTableBucket * buckets, BKUSize size, BKUSize hash, char const * key)
{
	BKUSize i = hash, perturb = hash;
	struct BKHashTableBucket * bucket = & buckets [i % size];

	while (bucket -> key) {
		if (bucket -> hash == hash) {
			if (bucket -> key != PLACEHOLDER_KEY && strcmp (bucket -> key, key) == 0) {
				break;
			}
		}

		i += (perturb >>= 5) + 1;
		bucket = &buckets [i % size];
	}

	return bucket;
}

static BKInt BKHashTableResize (BKHashTable * table, BKUSize capIdx)
{
	BKUSize size = BKHashTableSizes [capIdx];
	struct BKHashTableBucket * newBuckets;
	struct BKHashTableBucket * bucket, * newBucket;

	newBuckets = calloc (size, sizeof (*bucket));

	if (!newBuckets) {
		return -1;
	}

	for (BKUSize i = 0; i < BKHashTableSizes [table -> capIdx]; i ++) {
		bucket = & table -> buckets [i];

		if (bucket -> key > PLACEHOLDER_KEY) {
			newBucket = BKHashTableBucketLookup (newBuckets, size, bucket -> hash, bucket -> key);
			*newBucket = *bucket;
		}
	}

	free (table -> buckets);

	table -> occupied = table -> size;
	table -> capIdx   = capIdx;
	table -> buckets  = newBuckets;

	return 0;
}

void BKHashTableDispose (BKHashTable * table)
{
	struct BKHashTableBucket * bucket;

	for (BKUSize i = 0; i < BKHashTableSizes [table -> capIdx]; i ++) {
		bucket = &table -> buckets [i];

		if (bucket -> key > PLACEHOLDER_KEY) {
			free (bucket -> key);
		}
	}

	if (table -> buckets) {
		free (table -> buckets);
	}

	memset (table, 0, sizeof (*table));
}

static BKInt BKHashTableShouldGrow (BKHashTable const * table)
{
	// occupied * 1.5 > size => occupied > 2/3 * size
	return table -> occupied + (table -> occupied >> 1) > BKHashTableSizes [table -> capIdx];
}

static BKInt BKHashTableShouldShrink (BKHashTable const * table)
{
	// used < size / 4
	return table -> capIdx > 1 && table -> size < (BKHashTableSizes [table -> capIdx] >> 2);
}

BKInt BKHashTableLookup (BKHashTable const * table, char const * key, void ** outItem)
{
	BKUSize hash;
	BKUSize size = BKHashTableSizes [table -> capIdx];
	struct BKHashTableBucket * bucket;

	if (!size) {
		*outItem = NULL;

		return 0;
	}

	hash = BKHashTableHash (key);
	bucket = BKHashTableBucketLookup (table -> buckets, size, hash, key);

	if (bucket -> key <= PLACEHOLDER_KEY) {
		*outItem = NULL;

		return 0;
	}

	*outItem = bucket -> item;

	return 1;
}

BKInt BKHashTableLookupOrInsert (BKHashTable * table, char const * key, void *** outItemRef)
{
	BKUSize capIdx;
	BKUSize hash;
	BKUSize size;
	BKInt res = 0;
	struct BKHashTableBucket * bucket = NULL;

	capIdx = table -> capIdx;
	size = BKHashTableSizes [capIdx];

	if (!size || BKHashTableShouldGrow (table)) {
		capIdx ++;

		if (BKHashTableResize (table, capIdx) != 0) {
			return -1;
		}

		size = BKHashTableSizes [capIdx];
	}

	hash = BKHashTableHash (key);
	bucket = BKHashTableBucketLookup (table -> buckets, size, hash, key);

	if (bucket -> key <= PLACEHOLDER_KEY) {
		key = BKStrdup (key);

		if (!key) {
			return -1;
		}

		if (!bucket -> key) {
			table -> occupied ++;
		}

		bucket -> key = (void *) key;
		bucket -> hash = hash;
		table -> size ++;

		res = 1;
	}

	*outItemRef = &bucket -> item;

	return res;
}

BKInt BKHashTableRemove (BKHashTable * table, char const * key)
{
	BKUSize capIdx;
	BKUSize hash;
	BKUSize size;
	struct BKHashTableBucket * bucket;

	capIdx = table -> capIdx;
	size = BKHashTableSizes [capIdx];

	if (!size) {
		return 0;
	}

	hash = BKHashTableHash (key);
	bucket = BKHashTableBucketLookup (table -> buckets, size, hash, key);

	if (bucket -> key > PLACEHOLDER_KEY) {
		free (bucket -> key);
		bucket -> hash = 0;
		bucket -> key = (void *) PLACEHOLDER_KEY;
		bucket -> item = NULL;
		table -> size --;
	}

	if (BKHashTableShouldShrink (table)) {
		if (BKHashTableResize (table, capIdx - 1) != 0) {
			return -1;
		}
	}

	return 0;
}

void BKHashTableEmpty (BKHashTable * table)
{
	if (table -> occupied) {
		for (BKUSize i = 0; i < BKHashTableSizes [table -> capIdx]; i ++) {
			struct BKHashTableBucket* bucket = & table -> buckets [i];

			bucket -> key = NULL;
			bucket -> item = NULL;
		}
	}

	table -> size = 0;
	table -> occupied = 0;
}
