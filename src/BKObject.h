/*
 * Copyright (c) 2012-2015 Simon Schoenenberger
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

#ifndef _BK_OBJECT_H_
#define _BK_OBJECT_H_

#include "BKBase.h"

/**
 * Object types.
 */
typedef struct BKClass  BKClass;
typedef struct BKObject BKObject;

/**
 * Instance functions.
 */
typedef BKInt (* BKSetAttrFunc) (void * object, BKEnum attr, BKInt value);
typedef BKInt (* BKGetAttrFunc) (void const * object, BKEnum attr, BKInt * outValue);
typedef BKInt (* BKSetPtrFunc)  (void * object, BKEnum attr, void * ptr, BKSize size);
typedef BKInt (* BKGetPtrFunc)  (void const * object, BKEnum attr, void * outPtr, BKSize size);
typedef BKInt (* BKDisposeFunc) (void * object);

/**
 * Object flags.
 */
enum BKObjectFlag
{
	BKObjectFlagInitialized = 1 << 24,          ///< Set if object was initialized.
	BKObjectFlagAllocated   = 1 << 25,          ///< Set if object was allocated.
	BKObjectFlagLocked      = 1 << 26,          ///< Used to prevent recursion.
	BKObjectFlagMask        = ~((1 << 24) - 1), ///< Mask matching object flags
	BKObjectFlagUsableMask  = (1 << 24) - 1,    ///< Mask matching private usable flags.
};

/**
 * The class.
 */
struct BKClass
{
	BKUInt        flags;        ///< Class flags.
	BKSize        instanceSize; ///< Size of object instance.
	BKDisposeFunc dispose;      ///< Dispose function.
	BKSetAttrFunc setAttr;      ///< Set attribute function.
	BKGetAttrFunc getAttr;      ///< Get attribute function.
	BKSetPtrFunc  setPtr;       ///< Set pointer function.
	BKGetPtrFunc  getPtr;       ///< Get pointer function.
};

/**
 * The object.
 */
struct BKObject
{
	BKUInt flags;        ///< The object flags.
	BKClass const * isa; ///< The object class.
};

/**
 * Initialize object with given class. @p guardSize is checked against the class'
 * instance size to prevent overflow.
 *
 * @param object The object to initialize.
 * @param isa The object class.
 * @param guardSize Has to match the instance size given in @p isa.
 * @return 0 on success.
 */
extern BKInt BKObjectInit (void * object, BKClass const * isa, BKSize guardSize);

/**
 * Allocate and a new object with class. Optionally @p extraSize bytes are
 * reserved after object. The extra bytes are not emptied.
 *
 * @param outObject A reference to an object pointer.
 * @param isa The object class.
 * @param extraSize The additional number of bytes to reserve after the object.
 * @return 0 on success.
 *   BK_ALLOCATION_ERROR if @p guardSize differs from instance size given by class.
 */
extern BKInt BKObjectAlloc (void ** outObject, BKClass const * isa, BKSize extraSize);

/**
 * Set an integer attribute.
 *
 * @param object The object to set the attribute to.
 * @param attr The attribute to set.
 * @param value The attribute value to set.
 * @return 0 on success.
 *   BK_INVALID_ATTRIBUTE if attribute is unknown by object.
 *   BK_INVALID_VALUE if attribute value is invalid.
 *   BK_INVALID_STATE if object does not support setting attributes or is NULL.
 */
extern BKInt BKSetAttr (void * object, BKEnum attr, BKInt value);

/**
 * Get an integer attribute.
 *
 * @param object The object to get the attribute from.
 * @param attr The attribute to get.
 * @param outValue A reference to be set to the attribute value.
 * @return 0 on success.
 *   BK_INVALID_ATTRIBUTE if attribute is unknown by object.
 *   BK_INVALID_STATE if object does not support setting attributes or is NULL.
 */
extern BKInt BKGetAttr (void const * object, BKEnum attr, BKInt * outValue);

/**
 * Set a pointer attribute.
 *
 * @param object The object to set the attribute to.
 * @param attr The attribute to set.
 * @param ptr The attribute value.
 * @param The size of the attribute in bytes.
 * @return 0 on success.
 *   BK_INVALID_ATTRIBUTE if attribute is unknown by object
 *   BK_INVALID_VALUE if attribute value is invalid
 *   BK_INVALID_STATE if object does not support setting pointers or is NULL
 */
extern BKInt BKSetPtr (void * object, BKEnum attr, void * ptr, BKSize size);

/**
 * Get a pointer attribute.
 *
 * @param object The object to get the attribute from.
 * @param attr The attribute to get.
 * @param outPtr A reference in which the value will be copied into.
 * @param size The size of the referenced value.
 * @return 0 on success.
 *   BK_INVALID_ATTRIBUTE if attribute is unknown by object
 *   BK_INVALID_STATE if object does not support getting pointers or is NULL
 */
extern BKInt BKGetPtr (void const * object, BKEnum attr, void * outPtr, BKSize size);

/**
 * Dispose object. Calls the class' dispose function. If the object was
 * initialized with BKObjectInit, the object is emptied If the object was
 * allocated with BKObjectAlloc, the object is freed.
 *
 * @param object The object to dispose.
 */
extern void BKDispose (void * object);

#endif /* ! _BK_OBJECT_H_ */
