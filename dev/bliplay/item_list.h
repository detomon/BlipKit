/**
 * Copyright (c) 2012-2014 Simon Schoenenberger
 * http://blipkit.monoxid.net/
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

#ifndef _BK_ITEM_LIST_H_
#define _BK_ITEM_LIST_H_

#include <stdlib.h>

/**
 * Resizable item list
 */
typedef struct {
	BKUInt capacity;
	BKUInt length;
	BKInt  items [];
} item_list;

/**
 * Allocate new item list with initial capacity
 */
static BKInt * item_list_alloc (BKUInt capacity)
{
	item_list * list;

	if (capacity < 16)
		capacity = 16;

	list = malloc (sizeof (item_list) + capacity * sizeof (BKInt));

	if (list) {
		list -> capacity = capacity;
		list -> length   = 0;

		return list -> items;
	}

	return NULL;
}

/**
 * Free item list
 */
static void item_list_free (BKInt ** list_ref)
{
	BKInt     * list;
	item_list * listPtr;

	list = * list_ref;

	if (list) {
		listPtr = (void *) list - ((uintptr_t) & ((item_list *) 0) -> items);
		free (listPtr);
		* list_ref = NULL;
	}
}

/**
 * Add list to item list and resize if capacity is exceeded
 * Returns 0 if no error occurred otherwise -1
 */
static BKInt item_list_add (BKInt ** list_ref, BKInt n)
{
	BKInt     * list;
	item_list * listPtr;
	BKUInt      capacity;

	list = * list_ref;

	if (list == NULL) {
		list = item_list_alloc (0);

		if (list == NULL)
			return -1;

		* list_ref = list;
	}

	listPtr = (void *) list - ((uintptr_t) & ((item_list *) 0) -> items);

	if (listPtr -> length >= listPtr -> capacity) {
		capacity = listPtr -> capacity * 2;
		listPtr = realloc (listPtr, sizeof (item_list) + capacity * sizeof (BKInt));

		if (listPtr == NULL)
			return -1;

		listPtr -> capacity = capacity;
		list = listPtr -> items;
		* list_ref = list;
	}

	listPtr -> items [listPtr -> length ++] = n;

	return 0;
}

/**
 * Get length of item list
 */
static BKUInt item_list_length (BKInt * list)
{
	item_list * listPtr;

	if (list) {
		listPtr = (void *) list - ((uintptr_t) & ((item_list *) 0) -> items);

		return listPtr -> length;
	}

	return 0;
}

#endif /* ! _BK_ITEM_LIST_H_ */
