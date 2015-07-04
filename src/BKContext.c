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

#include "BKContext.h"
#include "BKUnit.h"

extern BKClass BKContextClass;

static BKEnum BKContextTick (BKCallbackInfo * info, BKContext * ctx)
{
	BKDividerTick (ctx -> beatDividers.firstDivider, info);
	BKDividerTick (ctx -> effectDividers.firstDivider, info);

	return 0;
}

/**
 * Update effect and beat clock period dependant of sample rate
 */
static void BKContextUpdateMasterClocks (BKContext * ctx)
{
	BKTime clockPeriod;
	BKCallback callback;

	clockPeriod = BKTimeFromSeconds (ctx, 1.0 / BK_DEFAULT_CLOCK_RATE);

	callback.func     = (BKCallbackFunc) BKContextTick;
	callback.userInfo = ctx;

	BKClockInit (& ctx -> masterClock, clockPeriod, & callback);  // then advance effects
}

static BKInt BKContextInitGeneric (BKContext * ctx, BKUInt numChannels, BKUInt sampleRate)
{
	BKBuffer * channel;

	ctx -> sampleRate  = BKClamp (sampleRate, BK_MIN_SAMPLE_RATE, BK_MAX_SAMPLE_RATE);
	ctx -> numChannels = BKClamp (numChannels, 1, BK_MAX_CHANNELS);
	ctx -> channels    = malloc (sizeof (BKBuffer) * ctx -> numChannels);

	BKContextUpdateMasterClocks (ctx);

	if (ctx -> channels == NULL)
		return BK_ALLOCATION_ERROR;

	for (BKInt i = 0; i < ctx -> numChannels; i ++) {
		channel = & ctx -> channels [i];

		if (BKBufferInit (channel) < 0)
			return BK_ALLOCATION_ERROR;
	}

	return 0;
}

BKInt BKContextInit (BKContext * ctx, BKUInt numChannels, BKUInt sampleRate)
{
	if (BKObjectInit (ctx, & BKContextClass, sizeof (*ctx)) < 0) {
		return -1;
	}

	if (BKContextInitGeneric (ctx, numChannels, sampleRate) < 0) {
		return -1;
	}

	return 0;
}

extern BKInt BKContextAlloc (BKContext ** outCtx, BKUInt numChannels, BKUInt sampleRate)
{
	if (BKObjectAlloc ((void **) outCtx, & BKContextClass, 0) < 0) {
		return -1;
	}

	if (BKContextInitGeneric (*outCtx, numChannels, sampleRate) < 0) {
		return -1;
	}

	return 0;
}

static void BKContextDisposeObject (BKContext * ctx)
{
	BKUnit   * nextUnit;
	BKClock  * nextClock;
	BKBuffer * channel;

	for (BKUnit * unit = ctx -> firstUnit; unit; unit = nextUnit) {
		nextUnit = unit -> nextUnit;
		BKUnitDetach (unit);
	}

	for (BKClock * clock = ctx -> firstClock; clock; clock = nextClock) {
		nextClock = clock -> nextClock;
		BKClockDetach (clock);
	}

	for (BKInt i = 0; i < ctx -> numChannels; i ++) {
		channel = & ctx -> channels [i];
		BKBufferDispose (channel);
	}

	if (ctx -> channels)
		free (ctx -> channels);

	BKDispose (& ctx -> masterClock);
}

void BKContextDispose (BKContext * ctx)
{
	BKDispose (ctx);
}

BKInt BKContextSetAttrInt (BKContext * ctx, BKEnum attr, BKInt value)
{
	switch (attr) {
		case BK_ARPEGGIO_DIVIDER:
		case BK_EFFECT_DIVIDER:
		case BK_INSTRUMENT_DIVIDER: {
			/*for (BKUnit * unit = ctx -> firstUnit; unit; unit = unit -> nextUnit) {
				if (unit -> funcs -> setAttr)
					unit -> funcs -> setAttr (unit, attr, value);
			}*/
		}
		default: {
			return BK_INVALID_ATTRIBUTE;
			break;
		}
	}

	return 0;
}

BKInt BKContextSetAttr (BKContext * ctx, BKEnum attr, BKInt value)
{
	return BKContextSetAttrInt (ctx, attr, value);
}

static BKInt BKContextGetAttrInt (BKContext const * ctx, BKEnum attr, BKInt * outValue)
{
	BKInt value = 0;

	switch (attr) {
		case BK_NUM_CHANNELS: {
			value = ctx -> numChannels;
			break;
		}
		case BK_SAMPLE_RATE: {
			value = ctx -> sampleRate;
			break;
		}
		default: {
			return BK_INVALID_ATTRIBUTE;
			break;
		}
	}

	* outValue = value;

	return 0;
}

BKInt BKContextGetAttr (BKContext const * ctx, BKEnum attr, BKInt * outValue)
{
	return BKContextGetAttrInt (ctx, attr, outValue);
}

static BKInt BKContextSetPtrObj (BKContext * ctx, BKEnum attr, void * ptr, BKSize size)
{
	switch (attr) {
		case BK_CLOCK_PERIOD: {
			BKTime time = * (BKTime *) ptr;

			if (BKTimeIsGreater (time, BK_TIME_ZERO)) {
				BKClockSetPeriod (& ctx -> masterClock, time);
			}
			else {
				return BK_INVALID_VALUE;
			}

			break;
		}
		default: {
			return BK_INVALID_ATTRIBUTE;
			break;
		}
	}

	return 0;
}

BKInt BKContextSetPtr (BKContext * ctx, BKEnum attr, void * ptr)
{
	return BKContextSetPtrObj (ctx, attr, ptr, 0);
}

static BKInt BKContextGetPtrObj (BKContext const * ctx, BKEnum attr, void * outPtr, BKSize size)
{
	switch (attr) {
		case BK_CLOCK_PERIOD: {
			BKTime * timeRef = outPtr;

			* timeRef = ctx -> masterClock.period;
			break;
		}
		case BK_TIME: {
			BKTime * timeRef = outPtr;

			* timeRef = ctx -> currentTime;
			break;
		}
		default: {
			return BK_INVALID_ATTRIBUTE;
			break;
		}
	}

	return 0;
}

BKInt BKContextGetPtr (BKContext const * ctx, BKEnum attr, void * outPtr)
{
	return BKContextGetPtrObj (ctx, attr, outPtr, 0);
}

BKInt BKContextGenerate (BKContext * ctx, BKFrame outFrames [], BKUInt size)
{
	BKUInt endTime;
	BKUInt chunkSize;
	BKUInt remainingSize;
	BKUInt writeSize;
	BKInt  result;

	remainingSize = size;
	writeSize     = 0;

	do {
		chunkSize = remainingSize;

		if (chunkSize > BK_MAX_GENERATE_SAMPLES)
			chunkSize = BK_MAX_GENERATE_SAMPLES;

		endTime = chunkSize << BK_FINT20_SHIFT;
		result = BKContextEnd (ctx, endTime);

		if (result < 0)
			return result;

		chunkSize = BKContextRead (ctx, outFrames, chunkSize);

		writeSize += chunkSize;

		// no track has written data
		if (chunkSize == 0)
			chunkSize = remainingSize;

		remainingSize -= chunkSize;
		outFrames += chunkSize * ctx -> numChannels;
	}
	while (remainingSize);

	return writeSize;
}

BKInt BKContextGenerateToTime (BKContext * ctx, BKTime endTime, BKInt (* write) (BKFrame inFrames [], BKUInt size, void * info), void * info)
{
	BKInt numFrames = 0;
	BKTime deltaTime;
	BKFUInt20 period;

	while (BKTimeIsLess (ctx -> currentTime, endTime)) {
		deltaTime = BKTimeSub (endTime, ctx -> currentTime);

		if (BKTimeIsLessFUInt20 (deltaTime, BK_INT_MAX / 3)) {
			period = BKTimeGetFUInt20 (deltaTime);
		}
		else {
			period = BK_INT_MAX / 3;
		}

		BKContextEnd (ctx, period);

		// write frames if buffer filled or end time is reached
		if (BKBufferSize (& ctx -> channels [0]) >= BK_MAX_GENERATE_SAMPLES || BKTimeIsGreaterEqual (ctx -> currentTime, endTime)) {
			BKInt size;
			BKFrame frames [ctx -> numChannels * BK_MAX_GENERATE_SAMPLES];

			size = BKContextRead (ctx, frames, BK_MAX_GENERATE_SAMPLES);
			numFrames += size;

			if (write (frames, size, info) != 0)
				return BK_INVALID_RETURN_VALUE;
		}
	}

	return numFrames;
}

/**
 * Get next clock tick period of all clocks
 * Increment clock times by that period and set new time if some clock ticks
 */
static BKFUInt20 BKClocksAdvance (BKContext * ctx, BKClock * clocks, BKInt * error)
{
	BKClock * clock;
	BKTime    nextTime;
	BKTime    deltaTime;
	BKFUInt20 period;

	do {
		nextTime = BK_TIME_MAX;
		ctx -> flags &= ~BK_CONTEXT_FLAG_CLOCK_RESET; // clear clock reset flag

		for (clock = clocks; clock; clock = clock -> nextClock) {
			BKClockTick (clock);

			// get next time to tick
			if (BKTimeIsLess (clock -> nextTime, nextTime))
				nextTime = clock -> nextTime;
		}
	}
	while (ctx -> flags & BK_CONTEXT_FLAG_CLOCK_RESET);

	deltaTime = BKTimeSub (nextTime, ctx -> currentTime);

	if (BKTimeIsLessFUInt20 (deltaTime, BK_INT_MAX / 3)) {
		period = BKTimeGetFUInt20 (deltaTime);
	}
	else {
		period = BK_INT_MAX / 3;
	}

	// advance
	for (clock = clocks; clock; clock = clock -> nextClock)
		BKClockAdvance (clock, period);

	ctx -> currentTime = BKTimeAddFUInt20 (ctx -> currentTime, period);

	return period;
}

BKInt BKContextRun (BKContext * ctx, BKFUInt20 endTime)
{
	BKFUInt20 time, clockDelta;
	BKUnit  * unit;
	BKInt     result;

	if (ctx -> firstClock) {
		for (time = ctx -> deltaTime; time < endTime;) {
			result = 0;
			clockDelta = BKClocksAdvance (ctx, ctx -> firstClock, & result);

			if (result < 0)
				return result;

			// set new end time
			time += clockDelta;

			// run units
			for (unit = ctx -> firstUnit; unit; unit = unit -> nextUnit)
				unit -> run (unit, time);
		}

		ctx -> deltaTime = time;
	}
	else {
		// run units
		for (unit = ctx -> firstUnit; unit; unit = unit -> nextUnit)
			unit -> run (unit, endTime);
	}

	return endTime;
}

BKInt BKContextEnd (BKContext * ctx, BKFUInt20 endTime)
{
	BKUnit   * unit;
	BKBuffer * channel;
	BKInt      result;

	result = BKContextRun (ctx, endTime);

	if (result < 0)
		return result;

	// end clock time
	ctx -> deltaTime -= endTime;

	// end units
	for (unit = ctx -> firstUnit; unit; unit = unit -> nextUnit)
		unit -> end (unit, endTime);

	// shift channel buffers
	for (BKInt i = 0; i < ctx -> numChannels; i ++) {
		channel = & ctx -> channels [i];
		BKBufferShift (channel, endTime);
	}

	return endTime;
}

BKInt BKContextSize (BKContext const * ctx)
{
	// assuming every buffer has the same size
	return BKBufferSize (& ctx -> channels [0]);
}

BKInt BKContextRead (BKContext * ctx, BKFrame outFrames [], BKUInt size)
{
	BKBuffer * channel;

	// read channels
	for (BKInt i = 0; i < ctx -> numChannels; i ++) {
		channel = & ctx -> channels [i];
		// interlace into `outFrames`
		size = BKBufferRead (channel, & outFrames [i], size, ctx -> numChannels);
	}

	return size;
}

void BKContextReset (BKContext * ctx)
{
	BKUnit   * unit;
	BKBuffer * channel;
	BKClock  * clock;

	ctx -> deltaTime   = 0;
	ctx -> currentTime = BK_TIME_ZERO;

	for (unit = ctx -> firstUnit; unit; unit = unit -> nextUnit) {
		if (unit -> reset)
			unit -> reset (unit);
	}

	for (clock = ctx -> firstClock; clock; clock = clock -> nextClock)
		BKClockReset (clock);

	for (BKInt i = 0; i < ctx -> numChannels; i ++) {
		channel = & ctx -> channels [i];
		BKBufferClear (channel);
	}
}

BKInt BKContextAttachDivider (BKContext * ctx, BKDivider * divider, BKEnum type)
{
	BKClock        * clock = & ctx -> masterClock;
	BKDividerGroup * group = NULL;

	switch (type) {
		case BK_CLOCK_TYPE_EFFECT: {
			group = & ctx -> effectDividers;
			break;
		}
		case BK_CLOCK_TYPE_BEAT: {
			group = & ctx -> beatDividers;
			break;
		}
		default: {
			return BK_INVALID_ATTRIBUTE;
			break;
		}
	}

	if (group) {
		// Check if required values set
		if (BKTimeIsGreater (clock -> period, BK_TIME_ZERO)) {
			BKDividerAttachToGroup (divider, group);
			BKClockAttach (clock, ctx, ctx -> firstClock);
		}
		else {
			return BK_INVALID_VALUE;
		}
	}
	else {
		return BK_INVALID_ATTRIBUTE;
	}

	return 0;
}

BKClass BKContextClass =
{
	.instanceSize = sizeof (BKContext),
	.dispose      = (BKDisposeFunc) BKContextDisposeObject,
	.setAttr      = (BKSetAttrFunc) BKContextSetAttrInt,
	.getAttr      = (BKGetAttrFunc) BKContextGetAttrInt,
	.setPtr       = (BKSetPtrFunc)  BKContextSetPtrObj,
	.getPtr       = (BKGetPtrFunc)  BKContextGetPtrObj,
};
