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

#ifndef _BK_OBJECT_H_
#define _BK_OBJECT_H_

#include "BKBase.h"

/**
 * Object types
 */
typedef struct BKClass  BKClass;
typedef struct BKObject BKObject;

/**
 * Instance functions
 */
typedef BKInt (* BKSetAttrFunc) (void * object, BKEnum attr, BKInt value);
typedef BKInt (* BKGetAttrFunc) (void const * object, BKEnum attr, BKInt * outValue);
typedef BKInt (* BKSetPtrFunc)  (void * object, BKEnum attr, void * ptr, BKSize size);
typedef BKInt (* BKGetPtrFunc)  (void const * object, BKEnum attr, void * outPtr, BKSize size);
typedef BKInt (* BKDisposeFunc) (void * object);

/**
 * Object flags
 */
enum BKObjectFlag
{
	BKObjectFlagInitialized = 1 << 24,
	BKObjectFlagAllocated   = 1 << 25,
	BKObjectFlagUsableMask  = (1 << 24) - 1,
};

/**
 * Class
 */
struct BKClass
{
	BKUInt        flags;
	BKSize        instanceSize;
	BKDisposeFunc dispose;
	BKSetAttrFunc setAttr;
	BKGetAttrFunc getAttr;
	BKSetPtrFunc  setPtr;
	BKGetPtrFunc  getPtr;
};

/**
 * Abstract object
 */
struct BKObject
{
	BKUInt flags;
	BKClass const * isa;
};

/**
 * Initialize object
 *
 * Empty given struct and set class `isa`
 * `guardSize` is checked against the class' instance size to prevent overflow
 * Returns 0 on success
 *
 * Return errors:
 * BK_ALLOCATION_ERROR if `guardSize` differs from instance size given by class
 */
extern BKInt BKObjectInit (void * object, BKClass const * isa, BKSize guardSize);

/**
 * Allocate object
 *
 * Allocate and a new object with class `isa`
 * Optionally `extraSize` bytes are reserved after object
 * Returns 0 on success
 *
 * Return errors:
 * BK_ALLOCATION_ERROR if allocation failed
 */
extern BKInt BKObjectAlloc (void ** outObject, BKClass const * isa, BKSize extraSize);

/**
 * Set attribute
 *
 * Set a single integer attribute
 * Returns 0 on success
 *
 * Return errors:
 * BK_INVALID_ATTRIBUTE if attribute is unknown by object
 * BK_INVALID_VALUE if attribute value is invalid
 * BK_INVALID_STATE if object does not support setting attributes or is NULL
 */
extern BKInt BKSetAttr (void * object, BKEnum attr, BKInt value);

/**
 * Get attribute
 *
 * Get a single integer attribute
 * Returns 0 on success
 *
 * Return errors:
 * BK_INVALID_ATTRIBUTE if attribute is unknown by object
 * BK_INVALID_STATE if object does not support getting attributesor or is NULL
 */
extern BKInt BKGetAttr (void const * object, BKEnum attr, BKInt * outValue);

/**
 * Set pointer
 *
 * Set a pointer attribute with size `size`
 * Returns 0 on success
 *
 * Return errors:
 * BK_INVALID_ATTRIBUTE if attribute is unknown by object
 * BK_INVALID_VALUE if attribute value is invalid
 * BK_INVALID_STATE if object does not support setting pointers or is NULL
 */
extern BKInt BKSetPtr (void * object, BKEnum attr, void * ptr, BKSize size);

/**
 * Get pointer
 *
 * Get a pointer attribute with size `size`
 * Returns 0 on success
 *
 * Return errors:
 * BK_INVALID_ATTRIBUTE if attribute is unknown by object
 * BK_INVALID_STATE if object does not support getting pointers or is NULL
 */
extern BKInt BKGetPtr (void const * object, BKEnum attr, void * outPtr, BKSize size);

/**
 * Dispose object
 *
 * Calls the class' dispose function
 * If the object was initialized with `BKObjectInit`, the object is emptied
 * If the object was allocated with `BKObjectAlloc`, the object freed
 */
extern void BKDispose (void * object);

#endif /* ! _BK_OBJECT_H_ */
