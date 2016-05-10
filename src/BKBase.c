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

#include "BKBase.h"

#define BK_STATUS_IDX(status) ((status) - BK_RETURN_TYPE)

static char const * const statusNames [] =
{
	[BK_STATUS_IDX (BK_ALLOCATION_ERROR)]        = "Allocation error",
	[BK_STATUS_IDX (BK_INVALID_ATTRIBUTE)]       = "Invalid argument",
	[BK_STATUS_IDX (BK_INVALID_VALUE)]           = "Invalid value",
	[BK_STATUS_IDX (BK_INVALID_STATE)]           = "Invalid state",
	[BK_STATUS_IDX (BK_INVALID_NUM_CHANNELS)]    = "Invalid number of channels",
	[BK_STATUS_IDX (BK_INVALID_NUM_FRAMES)]      = "Invalid number of frames",
	[BK_STATUS_IDX (BK_INVALID_NUM_BITS)]        = "Invalid number of bits",
	[BK_STATUS_IDX (BK_INVALID_RETURN_VALUE)]    = "Invalid return value",
	[BK_STATUS_IDX (BK_FILE_ERROR)]              = "File error",
	[BK_STATUS_IDX (BK_FILE_NOT_READABLE_ERROR)] = "File not readable",
	[BK_STATUS_IDX (BK_FILE_NOT_WRITABLE_ERROR)] = "File not writable",
	[BK_STATUS_IDX (BK_FILE_NOT_SEEKABLE_ERROR)] = "File not seekable",
	[BK_STATUS_IDX (BK_OTHER_ERROR)]             = "Other error",
};

char const * const BKVersion = BK_VERSION;

char const * BKStatusGetName (BKEnum status)
{
	char const * name = "Unknown status";

	if (status & BK_RETURN_TYPE && status <= BK_OTHER_ERROR) {
		name = statusNames [BK_STATUS_IDX (status)];
	}

	return name;
}
