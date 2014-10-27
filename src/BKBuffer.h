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

#ifndef _BK_BUFFER_H_
#define _BK_BUFFER_H_

#include "BKBase.h"

#define BK_STEP_SHIFT 5
#define BK_STEP_UNIT (1 << BK_STEP_SHIFT)
#define BK_STEP_FRAC (BK_STEP_UNIT - 1)

#define BK_STEP_WIDTH 15
#define BK_HIGH_PASS_SHIFT 22

#define BK_BUFFER_CAPACITY ((1 << (BK_INT_SHIFT - BK_FINT20_SHIFT)) + BK_STEP_WIDTH + 1)

#if BK_BUFFER_CAPACITY > 4112
#error Capacity exceeds 4112?
#endif

typedef struct BKBuffer BKBuffer;

/**
 * Buffer
 */
struct BKBuffer
{
	BKFUInt20 time;
	BKUInt    capacity;                     // dynamic capacity
	BKInt     accum;                        // amplitude accumulator
	BKInt     frames [BK_BUFFER_CAPACITY];  // frame buffer
};

/**
 * Initialize buffer
 */
extern BKInt BKBufferInit (BKBuffer * buf);

/**
 * Dispose buffer
 */
extern void BKBufferDispose (BKBuffer * buf);

/**
 * Add pulse at time offset
 */
extern BKInt BKBufferAddPulse (BKBuffer * buf, BKFUInt20 time, BKFrame pulse);

/**
 * Add single frame at time offset
 */
extern BKInt BKBufferAddFrame (BKBuffer * buf, BKFUInt20 time, BKFrame frame);

/**
 * Set time of last update
 */
extern BKInt BKBufferEnd (BKBuffer * buf, BKFUInt20 time);

/**
 * Advance time
 */
extern BKInt BKBufferShift (BKBuffer * buf, BKFUInt20 time);

/**
 * Read frames
 */
extern BKInt BKBufferRead (BKBuffer * buf, BKFrame outFrames [], BKUInt size, BKUInt interlace);

/**
 * Get current buffer size
 */
extern BKInt BKBufferSize (BKBuffer const * buf);

/**
 * Clear data
 */
extern void BKBufferClear (BKBuffer * buf);

#endif /* ! _BK_BUFFER_H_ */
