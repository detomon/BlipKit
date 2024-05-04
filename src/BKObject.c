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

#include "BKObject.h"

BKInt BKObjectInit(void* object, BKClass const* isa, BKSize guardSize) {
	BKObject* obj = object;

	if (obj == NULL || isa == NULL) {
		return BK_ALLOCATION_ERROR;
	}

	if (isa->instanceSize != guardSize) {
		return BK_ALLOCATION_ERROR;
	}

	memset(obj, 0, isa->instanceSize);

	obj->flags = BKObjectFlagInitialized;
	obj->isa = isa;

	return 0;
}

BKInt BKObjectAlloc(void** outObject, BKClass const* isa, BKSize extraSize) {
	BKObject* obj;

	if (outObject == NULL) {
		return BK_ALLOCATION_ERROR;
	}

	*outObject = NULL;

	if (isa == NULL || isa->instanceSize == 0) {
		return BK_ALLOCATION_ERROR;
	}

	obj = malloc(isa->instanceSize + extraSize);

	if (obj == NULL) {
		return BK_ALLOCATION_ERROR;
	}

	memset(obj, 0, isa->instanceSize);

	obj->flags = BKObjectFlagInitialized | BKObjectFlagAllocated;
	obj->isa = isa;

	*outObject = obj;

	return 0;
}

BKInt BKSetAttr(void* object, BKEnum attr, BKInt value) {
	BKObject* obj = object;
	BKClass const* isa;

	if (obj == 0) {
		return BK_INVALID_STATE;
	}

	isa = obj->isa;

	if (isa->setAttr == NULL) {
		return BK_INVALID_STATE;
	}

	return isa->setAttr(obj, attr, value);
}

BKInt BKGetAttr(void const* object, BKEnum attr, BKInt* outValue) {
	BKObject const* obj = object;
	BKClass const* isa;

	if (obj == 0) {
		return BK_INVALID_STATE;
	}

	isa = obj->isa;

	if (isa->getAttr == NULL) {
		return BK_INVALID_STATE;
	}

	return isa->getAttr(obj, attr, outValue);
}

BKInt BKSetPtr(void* object, BKEnum attr, void* ptr, BKSize size) {
	BKObject* obj = object;
	BKClass const* isa;

	if (obj == 0) {
		return BK_INVALID_STATE;
	}

	isa = obj->isa;

	if (isa->setPtr == NULL) {
		return BK_INVALID_STATE;
	}

	return isa->setPtr(obj, attr, ptr, size);
}

BKInt BKGetPtr(void const* object, BKEnum attr, void* outPtr, BKSize size) {
	BKObject const* obj = object;
	BKClass const* isa;

	if (obj == 0) {
		return BK_INVALID_STATE;
	}

	isa = obj->isa;

	if (isa->getPtr == NULL) {
		return BK_INVALID_STATE;
	}

	return isa->getPtr(obj, attr, outPtr, size);
}

void BKDispose(void* object) {
	BKObject* obj = object;
	BKClass const* isa;
	BKUInt flags;

	if (obj == NULL) {
		return;
	}

	isa = obj->isa;

	if (isa == NULL) {
		return;
	}

	flags = obj->flags;

	if (!(flags & BKObjectFlagInitialized)) {
		return;
	}

	if (isa->dispose) {
		isa->dispose(obj);
	}

	memset(obj, 0, isa->instanceSize);

	if (flags & BKObjectFlagAllocated) {
		free(obj);
	}
}
