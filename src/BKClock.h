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

#ifndef _BK_CLOCK_H_
#define _BK_CLOCK_H_

#include "BKBase.h"
#include "BKTime.h"

/**
 * A clock is attached to a context and ticks in a specified interval.
 * At each tick its callback is fired.
 *
 * A divider is attached to a clock and fires its callback every time the clock
 * ticks which it is attached to. The divider counter is then decremented by 1.
 * If it reaches 0 the callback is fired and the counter is reset.
 */

typedef struct BKDivider      BKDivider;
typedef struct BKDividerGroup BKDividerGroup;
typedef struct BKClock        BKClock;

struct BKDivider
{
	BKDividerGroup * group;
	BKDivider      * prevDivider;
	BKDivider      * nextDivider;
	BKCallback       callback;
	BKInt            divider;
	BKInt            counter;
};

struct BKDividerGroup
{
	BKDivider * firstDivider;
	BKDivider * lastDivider;
};

struct BKClock
{
	BKUInt         flags;
	void         * ctx;
	BKClock      * prevClock;
	BKClock      * nextClock;
	BKTime         period;
	BKTime         time;
	BKTime         nextTime;
	BKUInt         counter;
	BKCallback     callback;
	BKDividerGroup dividers;
};

/**
 * Initialize clock
 */
extern BKInt BKClockInit (BKClock * clock, BKTime period, BKCallback * callback);

/**
 * Dispose clock
 */
extern void BKClockDispose (BKClock * clock);

/**
 * Attach clock
 * If `beforeClock` is not NULL `clock` is attached before `beforeClock`
 * `beforeClock` can have the values BK_LAST_ELEMENT_PTR and BK_FIRST_ELEMENT_PTR
 *
 * Errors:
 * BK_INVALID_STATE if clock is already attached to a context
 */
extern BKInt BKClockAttach (BKClock * clock, BKContext * ctx, BKClock * beforeClock);

/**
 * Detach clock
 */
extern void BKClockDetach (BKClock * clock);

/**
 * Set period
 */
extern void BKClockSetPeriod (BKClock * clock, BKTime period);

/**
 * Reset clock
 */
extern void BKClockReset (BKClock * clock);

/**
 * Advance clock by time
 */
extern void BKClockAdvance (BKClock * clock, BKFUInt20 period);

/**
 * Tick clock and attached dividers
 */
extern BKInt BKClockTick (BKClock * clock);

/**
 * Initialize divider
 */
extern BKInt BKDividerInit (BKDivider * divider, BKUInt count, BKCallback * callback);

/**
 * Dispose divider
 */
extern void BKDividerDispose (BKDivider * divider);

/**
 * Add divider to clock
 */
extern BKInt BKDividerAttachToClock (BKDivider * divider, BKClock * clock);

/**
 * Add divider to group
 */
extern BKInt BKDividerAttachToGroup (BKDivider * divider, BKDividerGroup * group);

/**
 * Detach divider from clock
 */
extern void BKDividerDetach (BKDivider * divider);

/**
 * Decrement divider counter of divider list by 1
 * If 0 is reached call callback function and reset counter
 */
extern BKInt BKDividerTick (BKDivider * divider, BKCallbackInfo * info);

/**
 * Reset divider counter
 */
extern void BKDividerReset (BKDivider * divider);

#endif /* !_BK_CLOCK_H_ */
