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

#include "BKClock.h"
#include "BKContext.h"

enum
{
	BK_CLOCK_FLAG_RESET     = 1 << 0,
	BK_CLOCK_FLAG_COPY_MASK = 0,
};

extern BKClass BKClockClass;
extern BKClass BKDividerClass;

BKInt BKClockInitGeneric (BKClock * clock, BKTime period, BKCallback * callback)
{
	if (callback) {
		clock -> callback = (* callback);
	}

	clock -> time     = BK_TIME_ZERO;
	clock -> nextTime = BK_TIME_ZERO;
	clock -> period   = period;

	return 0;
}

BKInt BKClockInit (BKClock * clock, BKTime period, BKCallback * callback)
{
	if (BKObjectInit (clock, & BKClockClass, sizeof (*clock)) < 0) {
		return -1;
	}

	if (BKClockInitGeneric (clock, period, callback) < 0) {
		return -1;
	}

	return 0;
}

BKInt BKClockAlloc (BKClock ** outClock, BKTime period, BKCallback * callback)
{
	if (BKObjectAlloc ((void **) outClock, & BKClockClass, 0) < 0) {
		return -1;
	}

	if (BKClockInitGeneric (*outClock, period, callback) < 0) {
		return -1;
	}

	return 0;
}

void BKClockDispose (BKClock * clock)
{
	BKDivider * nextDivider;

	BKClockDetach (clock);

	for (BKDivider * divider = clock -> dividers.firstDivider; divider; divider = nextDivider) {
		nextDivider = divider;
		BKDividerDetach (divider);
	}
}

BKInt BKClockAttach (BKClock * clock, BKContext * ctx, BKClock * beforeClock)
{
	if (clock -> ctx == NULL) {
		clock -> ctx = ctx;
		BKClockReset (clock);

		if (beforeClock == BK_FIRST_ELEMENT_PTR)
			beforeClock = ctx -> firstClock;

		if (beforeClock == NULL) {
			clock -> prevClock = ctx -> lastClock;
			clock -> nextClock = NULL;

			if (ctx -> lastClock) {
				ctx -> lastClock -> nextClock = clock;
				ctx -> lastClock = clock;
			}
			// is first clock
			else {
				ctx -> firstClock = clock;
				ctx -> lastClock  = clock;
			}
		}
		// previous clock must be in same context
		else if (beforeClock -> ctx == ctx) {
			clock -> prevClock = beforeClock -> prevClock;
			clock -> nextClock = beforeClock;

			if (beforeClock == ctx -> firstClock) {
				ctx -> firstClock = clock;
			}
			else {
				beforeClock -> prevClock -> nextClock = clock;
			}

			beforeClock -> prevClock = clock;
		}
		else {
			return BK_INVALID_VALUE;
		}
	}
	else {
		return BK_INVALID_STATE;
	}

	return 0;
}

void BKClockDetach (BKClock * clock)
{
	BKContext * ctx = clock -> ctx;

	if (ctx) {
		if (clock -> prevClock) {
			clock -> prevClock -> nextClock = clock -> nextClock;
		}
		// is first clock
		else {
			ctx -> firstClock = clock -> nextClock;
		}

		if (clock -> nextClock) {
			clock -> nextClock -> prevClock = clock -> prevClock;
		}
		// is last clock
		else {
			ctx -> lastClock = clock -> prevClock;
		}

		clock -> ctx = NULL;
		BKClockReset (clock);
	}
}

void BKClockSetPeriod (BKClock * clock, BKTime period)
{
	clock -> period = period;
}

void BKClockReset (BKClock * clock)
{
	BKContext * ctx = clock -> ctx;

	if (ctx)
		ctx -> flags |= BK_CONTEXT_FLAG_CLOCK_RESET;

	for (BKDivider * divider = clock -> dividers.firstDivider; divider; divider = divider -> nextDivider)
		BKDividerReset (divider);

	clock -> object.flags &= ~BK_CLOCK_FLAG_RESET; // clear reset flag
	clock -> counter  = 0;
	clock -> time     = BK_TIME_ZERO;
	clock -> nextTime = BK_TIME_ZERO;
}

void BKClockAdvance (BKClock * clock, BKFUInt20 period)
{
	clock -> time = BKTimeAddFUInt20 (clock -> time, period);
}

BKInt BKDividerTick (BKDivider * divider, BKCallbackInfo * info)
{
	BKInt reset = 0;

	// tick each attached divider
	for (; divider; divider = divider -> nextDivider) {
		if (divider -> counter <= 0) {
			reset = 1;

			if (divider -> callback.func) {
				info -> event   = BK_EVENT_DIVIDER;
				info -> divider = divider -> divider;

				divider -> callback.func (info, divider -> callback.userInfo);

				divider -> divider = BKMax (1, info -> divider);
			}

			divider -> counter = divider -> divider;
		}

		divider -> counter --;
	}

	return reset;
}

BKInt BKClockTick (BKClock * clock)
{
	BKInt result = 0;
	BKCallbackInfo info;

	if (BKTimeIsEqual (clock -> time, clock -> nextTime)) {
		memset (& info, 0, sizeof (BKCallbackInfo));
		info.object   = clock;
		info.event    = BK_EVENT_CLOCK;
		info.nextTime = clock -> time;

		if (clock -> callback.func)
			result = clock -> callback.func (& info, clock -> callback.userInfo);

		BKDividerTick (clock -> dividers.firstDivider, & info);

		clock -> counter ++;

		// set next time and new period from callback
		if (BKTimeIsGreater (info.nextTime, clock -> time)) {
			clock -> period   = BKTimeSub (info.nextTime, clock -> time);
			clock -> nextTime = info.nextTime;
		}
		// set next period
		else {
			clock -> nextTime = BKTimeAdd (clock -> time, clock -> period);
		}
	}

	return result;
}

static BKInt BKDividerInitGeneric (BKDivider * divider, BKUInt count, BKCallback * callback)
{
	divider -> divider = BKMax (1, count);
	divider -> counter = 0;

	if (callback) {
		divider -> callback = * callback;
	}

	return 0;
}

BKInt BKDividerInit (BKDivider * divider, BKUInt count, BKCallback * callback)
{
	if (BKObjectInit (divider, & BKDividerClass, sizeof (*divider)) < 0) {
		return -1;
	}

	if (BKDividerInitGeneric (divider, count, callback) < 0) {
		return -1;
	}

	return 0;
}

BKInt BKDividerAlloc (BKDivider ** outDivider, BKUInt count, BKCallback * callback)
{
	if (BKObjectAlloc ((void **) outDivider, & BKDividerClass, 0) < 0) {
		return -1;
	}

	if (BKDividerInitGeneric (*outDivider, count, callback) < 0) {
		return -1;
	}

	return 0;
}

void BKDividerDispose (BKDivider * divider)
{
	BKDividerGroup * group = divider -> group;

	if (divider -> group) {
		if (divider -> prevDivider) {
			divider -> prevDivider -> nextDivider = divider -> nextDivider;
		}
		// is first clock
		else {
			group -> firstDivider = divider -> nextDivider;
		}

		if (divider -> nextDivider) {
			divider -> nextDivider -> prevDivider = divider -> prevDivider;
		}
		// is last clock
		else {
			group -> lastDivider = divider -> prevDivider;
		}

		divider -> group = NULL;
	}
}

BKInt BKDividerAttachToClock (BKDivider * divider, BKClock * clock)
{
	return BKDividerAttachToGroup (divider, & clock -> dividers);
}

BKInt BKDividerAttachToGroup (BKDivider * divider, BKDividerGroup * group)
{
	if (divider -> group == NULL) {
		divider -> prevDivider = group -> lastDivider;
		divider -> nextDivider = NULL;

		if (group -> lastDivider) {
			group -> lastDivider -> nextDivider = divider;
		}
		// is first clock
		else {
			group -> firstDivider = divider;
		}

		group -> lastDivider = divider;

		divider -> group = group;
	}
	else {
		return BK_INVALID_STATE;
	}

	return 0;
}

void BKDividerDetach (BKDivider * divider)
{
	BKDividerGroup * group = divider -> group;

	if (group) {
		if (divider -> prevDivider) {
			divider -> prevDivider -> nextDivider = divider -> nextDivider;
		}
		// is first divider
		else {
			group -> firstDivider = divider -> nextDivider;
		}

		if (divider -> nextDivider) {
			divider -> nextDivider -> prevDivider = divider -> prevDivider;
		}
		// is last divider
		else {
			group -> lastDivider = divider -> prevDivider;
		}

		divider -> group = NULL;
	}
}

void BKDividerReset (BKDivider * divider)
{
	divider -> counter = 0;
}

BKClass BKClockClass =
{
	.instanceSize = sizeof (BKClock),
	.dispose      = (BKDisposeFunc) BKClockDispose,
};

BKClass BKDividerClass =
{
	.instanceSize = sizeof (BKDivider),
	.dispose      = (BKDisposeFunc) BKDividerDispose,
};
