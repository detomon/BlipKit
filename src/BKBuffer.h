/**
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

#ifndef _BK_BUFFER_H_
#define _BK_BUFFER_H_

#include "BKBase.h"

#define BK_STEP_SHIFT 5
#define BK_STEP_UNIT (1 << BK_STEP_SHIFT)
#define BK_STEP_FRAC (BK_STEP_UNIT - 1)

#define BK_STEP_WIDTH 16
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
 * Step phases
 */
extern BKInt const BKBufferStepPhases [BK_STEP_UNIT][BK_STEP_WIDTH];

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
BK_INLINE BKInt BKBufferAddPulse (BKBuffer * buf, BKFUInt20 time, BKFrame pulse);

/**
 * Add single frame at time offset
 */
BK_INLINE BKInt BKBufferAddFrame (BKBuffer * buf, BKFUInt20 time, BKFrame frame);

/**
 * Set time of last update
 */
BK_INLINE BKInt BKBufferEnd (BKBuffer * buf, BKFUInt20 time);

/**
 * Advance time
 */
BK_INLINE BKInt BKBufferShift (BKBuffer * buf, BKFUInt20 time);

/**
 * Read frames
 */
extern BKInt BKBufferRead (BKBuffer * buf, BKFrame outFrames [], BKUInt size, BKUInt interlace);

/**
 * Get current buffer size
 */
BK_INLINE BKInt BKBufferSize (BKBuffer const * buf);

/**
 * Clear data
 */
extern void BKBufferClear (BKBuffer * buf);


BK_INLINE BKInt BKBufferEnd (BKBuffer * buf, BKFUInt20 time)
{
	BKUInt offset;

	time   = buf -> time + time;
	offset = time >> BK_FINT20_SHIFT;

	if (offset > buf -> capacity) {
		buf -> capacity = offset;
	}

	return 0;
}

BK_INLINE BKInt BKBufferShift (BKBuffer * buf, BKFUInt20 time)
{
	BKFUInt20 maxShift;

	maxShift = buf -> capacity << BK_FINT20_SHIFT;

	if (buf -> time + time > maxShift) {
		time = maxShift - buf -> time;
	}

	buf -> time += time;

	return 0;
}

BK_INLINE BKInt BKBufferSize (BKBuffer const * buf)
{
	return buf -> time >> BK_FINT20_SHIFT;
}

BK_INLINE BKInt BKBufferAddPulse (BKBuffer * buf, BKFUInt20 time, BKFrame pulse)
{
	BKUInt frac;
	BKUInt offset;
	BKInt * frames;
	BKInt const * phase;

	time   = buf -> time + time;
	offset = time >> BK_FINT20_SHIFT;

	frac = time & BK_FINT20_FRAC;               // frame fraction
	frac >>= (BK_FINT20_SHIFT - BK_STEP_SHIFT); // step fraction

	phase  = BKBufferStepPhases [frac];
	frames = & buf -> frames [offset];

/*#if BK_USE_INTRINSIC
	__m128i stepv, pulsev, framesv;

	pulsev = _mm_set1_epi32 (pulse);

	// add step
	for (BKInt i = 0; i < BK_STEP_WIDTH; i += 4) {
		stepv   = _mm_load_si128 ((__m128i *) & phase [i]);
		framesv = _mm_loadu_si128 ((__m128i *) & frames [i]);
		framesv = _mm_add_epi32 (framesv, _mm_mullo_epi32 (stepv, pulsev));
		_mm_storeu_si128 ((__m128i *) & frames [i], framesv);
	}
#else*/
	// add step
	for (BKInt i = 0; i < BK_STEP_WIDTH; i ++) {
		frames [i] += phase [i] * pulse;
	}
/*#endif*/

	return 0;
}

BK_INLINE BKInt BKBufferAddFrame (BKBuffer * buf, BKFUInt20 time, BKFrame frame)
{
	BKUInt offset;

	time   = buf -> time + time;
	offset = time >> BK_FINT20_SHIFT;

	buf -> frames [offset] += BK_MAX_VOLUME * frame;

	return 0;
}

#endif /* ! _BK_BUFFER_H_ */
