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

#include "BKUnit_internal.h"
#include "BKData_internal.h"

extern BKClass BKUnitClass;

static BKFrame const BKSinePhases [BK_SINE_PHASES] =
{
	     0,   6392,  12539,  18204,
	 23169,  27244,  30272,  32137,
	 32767,  32137,  30272,  27244,
	 23169,  18204,  12539,   6392,
	     0,  -6392, -12539, -18204,
	-23169, -27244, -30272, -32137,
	-32767, -32137, -30272, -27244,
	-23169, -18204, -12539,  -6392,
};

static BKEnum BKUnitCallSampleCallback (BKUnit * unit, BKEnum event);

static BKInt BKUnitTrySetData (BKUnit * unit, BKData * data, BKEnum type, BKEnum event)
{
	BKContext * ctx = unit -> ctx;

	if (ctx == NULL) {
		return BK_INVALID_STATE;
	}

	switch (type) {
		// set data as custom waveform
		case BK_WAVEFORM: {
			if (data && event != BK_DATA_STATE_EVENT_DISPOSE) {
				if (data -> numFrames >= 2) {
					unit -> waveform           = BK_CUSTOM;
					unit -> phase.count        = data -> numFrames;
					unit -> phase.haltSilence  = 1;
					unit -> phase.phase        = 0;
					unit -> sample.offset      = 0;
					unit -> sample.end         = data -> numFrames;
					unit -> sample.frames      = data -> frames;
					unit -> sample.repeatCount = 0;
				}
				else {
					return BK_INVALID_NUM_FRAMES;
				}
			}
			// data was disposed
			else if (unit -> waveform == BK_CUSTOM) {
				unit -> waveform      = 0;
				unit -> sample.offset = 0;
				unit -> sample.end    = 0;
				unit -> sample.frames = NULL;
				return -1;
			}

			break;
		}
		// set sample to play
		case BK_SAMPLE: {
			if (data && event != BK_DATA_STATE_EVENT_DISPOSE) {
				// number of channels must eighter be 1 or equal to
				// number of context channels
				if (data -> numChannels != 1 && data -> numChannels != ctx -> numChannels) {
					return BK_INVALID_NUM_CHANNELS;
				}
				else if (data -> numFrames < 2) {
					return BK_INVALID_NUM_FRAMES;
				}
				else {
					unit -> waveform           = BK_SAMPLE;
					unit -> phase.count        = 1;  // prevent divion by 0
					unit -> sample.length      = data -> numFrames;
					unit -> sample.numChannels = data -> numChannels;
					unit -> sample.offset      = 0;
					unit -> sample.end         = data -> numFrames;
					unit -> sample.frames      = data -> frames;
					unit -> phase.phase        = 0; // reset phase
					unit -> sample.repeat      = 0;
					unit -> sample.repeatCount = 0;

					BKUnitCallSampleCallback (unit, BK_EVENT_SAMPLE_BEGIN);
				}
			}
			// disable sample
			else if (unit -> waveform == BK_SAMPLE) {
				unit -> waveform      = 0;
				unit -> sample.offset = 0;
				unit -> sample.end    = 0;
				unit -> sample.frames = NULL;
				unit -> phase.phase   = 0; // reset phase
				unit -> sample.repeat = 0;
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

static BKInt BKUnitSetData (BKUnit * unit, BKEnum type, BKData * data)
{
	BKInt res;

	res = BKUnitTrySetData (unit, data, type, BK_DATA_STATE_EVENT_RESET);

	if (res == 0) {
		BKDataStateSetData (& unit -> sample.dataState, data);
	}
	// unset data when failed
	else {
		switch (unit -> waveform) {
			case BK_CUSTOM:
			case BK_SAMPLE: {
				unit -> waveform = 0;
				break;
			}
		}
	}

	return res;
}

BKInt BKUnitSampleDataStateCallback (BKEnum event, BKUnit * unit)
{
	BKEnum type;

	switch (unit -> waveform) {
		case BK_CUSTOM: {
			type = BK_WAVEFORM;
			break;
		}
		case BK_SAMPLE: {
			type = BK_SAMPLE;
			break;
		}
		default: {
			return -1;
			break;
		}
	}

	// reset data
	return BKUnitTrySetData (unit, unit -> sample.dataState.data, type, event);
}

static BKEnum BKUnitCallSampleCallback (BKUnit * unit, BKEnum event)
{
	BKEnum result = 0;
	BKCallbackInfo info;

	if (unit -> sample.callback.func) {
		memset (& info, 0, sizeof (BKCallbackInfo));
		info.object = unit;
		info.event  = event;

		unit -> sample.callback.func (& info, unit -> sample.callback.userInfo);
	}

	return result;
}

BKInt BKUnitInit (BKUnit * unit, BKEnum waveform)
{
	if (BKObjectInit (unit, & BKUnitClass, sizeof (*unit)) < 0) {
		return -1;
	}

	unit -> run   = (BKUnitRunFunc) BKUnitRun;
	unit -> end   = (BKUnitEndFunc) BKUnitEnd;
	unit -> reset = (BKUnitResetFunc) BKUnitReset;

	BKSetAttr (unit, BK_DUTY_CYCLE, BK_DEFAULT_DUTY_CYCLE);
	BKSetAttr (unit, BK_WAVEFORM, waveform);

	unit -> sample.period                     = BK_FINT20_UNIT;
	unit -> sample.timeFrac                   = 0;
	unit -> sample.dataState.callback         = (void *) BKUnitSampleDataStateCallback;
	unit -> sample.dataState.callbackUserInfo = unit;

	return 0;
}

void BKUnitDisposeObject (BKUnit * unit)
{
	BKDataStateSetData (& unit -> sample.dataState, NULL);

	BKUnitDetach (unit);
}

void BKUnitDispose (BKUnit * unit)
{
	BKDispose (unit);
}

BKInt BKUnitAttach (BKUnit * unit, BKContext * ctx)
{
	if (unit -> ctx == NULL) {
		unit -> prevUnit = ctx -> lastUnit;
		unit -> nextUnit = NULL;
		unit -> ctx      = ctx;
		unit -> time     = ctx -> deltaTime;  // shift time to context time

		if (ctx -> lastUnit) {
			ctx -> lastUnit -> nextUnit = unit;
			ctx -> lastUnit = unit;
		}
		// is first unit
		else {
			ctx -> firstUnit = unit;
			ctx -> lastUnit  = unit;
		}
	}
	else {
		return BK_INVALID_STATE;
	}

	return 0;
}

void BKUnitDetach (BKUnit * unit)
{
	BKContext * ctx = unit -> ctx;

	if (ctx) {
		if (unit -> prevUnit) {
			unit -> prevUnit -> nextUnit = unit -> nextUnit;
		}
		// unit ist first unit
		else {
			ctx -> firstUnit = unit -> nextUnit;
		}

		if (unit -> nextUnit) {
			unit -> nextUnit -> prevUnit = unit -> prevUnit;
		}
		// unit is last unit
		else {
			ctx -> lastUnit = unit -> prevUnit;
		}

		unit -> ctx  = NULL;
		unit -> time = 0;
	}
}

static void BKUnitUpdateSampleRange (BKUnit * unit, BKInt offset, BKInt end)
{
	BKInt sampleLength;
	BKInt oldOffset;
	BKInt newOffset, newLength;
	BKInt newPhase;

	if (unit -> sample.dataState.data == NULL)
		return;

	sampleLength = unit -> sample.dataState.data -> numFrames;

	if (offset < 0) offset += sampleLength + 1;
	offset = BKClamp (offset, 0, sampleLength);

	if (end < 0) end += sampleLength + 1;
	end = BKClamp (end, 0, sampleLength);

	oldOffset = BKMin (unit -> sample.offset, unit -> sample.end);
	newOffset = BKMin (offset, end);
	newLength = BKAbs (offset - end);

	newPhase = unit -> phase.phase + (oldOffset - newOffset);

	// reset phase if it would overlap the new range
	if (newPhase < 0 || newPhase >= newLength)
		newPhase = 0;

	unit -> sample.offset = offset;
	unit -> sample.end    = end;
	unit -> sample.frames = & unit -> sample.dataState.data -> frames [newOffset * unit -> sample.numChannels];
	unit -> sample.length = newLength;
	unit -> sample.period = BKAbs (unit -> sample.period);

	// reverse sample period
	if (offset > end)
		unit -> sample.period = -unit -> sample.period;

	// if not playing yet
	if (unit -> sample.repeatCount == 0) {
		// set sample phase to begin if sample phase is at end
		if (unit -> sample.offset < unit -> sample.end && unit -> phase.phase == unit -> sample.length - 1) {
			newPhase = 0;
		}
		// set sample phase to end if sample phase is at begin
		else if (unit -> phase.phase == 0) {
			newPhase = unit -> sample.length - 1;
		}
	}

	unit -> phase.phase = newPhase;
}

BKInt BKUnitSetAttr (BKUnit * unit, BKEnum attr, BKInt value)
{
	switch (attr) {
		case BK_DUTY_CYCLE: {
			value = BKClamp (value, BK_MIN_DUTY_CYCLE, BK_MAX_DUTY_CYCLE);

			if (unit -> dutyCycle != value) {
				if (unit -> waveform == BK_SQUARE) {
					// reduce clicking noise
					if (value > unit -> dutyCycle && unit -> phase.phase < value)
						unit -> phase.phase = 0;
				}

				unit -> dutyCycle = value;
			}

			break;
		}
		case BK_WAVEFORM: {
			switch (value) {
				case 0: {
					unit -> phase.count = BK_SQUARE_PHASES;
					unit -> phase.haltSilence = 1;
					break;
				}
				case BK_SQUARE: {
					unit -> phase.count = BK_SQUARE_PHASES;
					unit -> phase.haltSilence = 1;
					break;
				}
				case BK_TRIANGLE: {
					unit -> phase.count = BK_TRIANGLE_PHASES;
					unit -> phase.haltSilence = 1;
					break;
				}
				case BK_NOISE: {
					unit -> phase.count = BK_NOISE_PHASES;
					unit -> phase.haltSilence = 0;
					break;
				}
				case BK_SAWTOOTH: {
					unit -> phase.count = BK_SAWTOOTH_PHASES;
					unit -> phase.haltSilence = 1;
					break;
				}
				case BK_SINE: {
					unit -> phase.count = BK_SINE_PHASES;
					unit -> phase.haltSilence = 1;
					break;
				}
				default: {
					return BK_INVALID_VALUE;
					break;
				}
			}

			if (unit -> waveform != value) {
				unit -> phase.phase = 0;
				unit -> waveform    = value;
			}

			BKDataStateSetData (& unit -> sample.dataState, NULL);

			break;
		}
		case BK_PHASE: {
			if (unit -> waveform != BK_SAMPLE) {
				value = BKClamp (value, 0, unit -> phase.count - 1);
			}
			else {
				// reset to sample start
				if (value < 0) {
					if (unit -> sample.end > unit -> sample.offset) {
						value = 0;
					}
					else {
						value = unit -> sample.length - 1;
					}
				}

				value = BKClamp (value, 0, unit -> sample.length - 1);
				BKUnitCallSampleCallback (unit, BK_EVENT_SAMPLE_BEGIN);
			}

			unit -> phase.phase = value;

			break;
		}
		case BK_PHASE_WRAP: {
			value = value ? BKMax (value, 2) : 0;
			unit -> phase.phase     = 0;
			unit -> phase.wrap      = value;
			unit -> phase.wrapCount = value;
			break;
		}
		case BK_PERIOD: {
			value = BKMax (BKAbs (value), BK_MIN_PERIOD);
			unit -> period = value;
			break;
		}
		case BK_VOLUME: {
			value = BKClamp (value, 0, BK_MAX_VOLUME);

			for (BKInt i = 0; i < BK_MAX_CHANNELS; i ++)
				unit -> volume [i] = value;

			break;
		}
		case BK_VOLUME_0:
		case BK_VOLUME_1:
		case BK_VOLUME_2:
		case BK_VOLUME_3:
		case BK_VOLUME_4:
		case BK_VOLUME_5:
		case BK_VOLUME_6:
		case BK_VOLUME_7: {
			value = BKClamp (value, 0, BK_MAX_VOLUME);
			unit -> volume [attr - BK_VOLUME_0] = value;
			break;
		}
		case BK_MUTE: {
			unit -> mute = value ? 1 : 0;
			break;
		}
		case BK_SAMPLE_REPEAT: {
			switch (value) {
				case BK_NO_REPEAT:
				case BK_REPEAT:
				case BK_PALINDROME: {
					break;
				}
				default: {
					value = BK_NO_REPEAT;
					break;
				}
			}

			unit -> sample.repeat = value;
			break;
		}
		case BK_SAMPLE_PERIOD: {
			value = BKAbs (value);
			value = BKClamp (value, BK_MIN_SAMPLE_PERIOD, BK_MAX_SAMPLE_PERIOD);

			// reverse if negative
			if (unit -> sample.period < 0) {
				value = -value;
			}

			unit -> sample.period = value;
			break;
		}
		case BK_HALT_SILENT_PHASE: {
			unit -> phase.haltSilence = (value != 0);
			break;
		}
		default: {
			return BK_INVALID_ATTRIBUTE;
			break;
		}
	}

	return 0;
}

BKInt BKUnitGetAttr (BKUnit const * unit, BKEnum attr, BKInt * outValue)
{
	BKInt value = 0;

	switch (attr) {
		case BK_WAVEFORM: {
			value = unit -> waveform;
			break;
		}
		case BK_DUTY_CYCLE: {
			value = unit -> dutyCycle;
			break;
		}
		case BK_PERIOD: {
			value = unit -> period;
			break;
		}
		case BK_PHASE: {
			value = unit -> phase.phase;
			break;
		}
		case BK_PHASE_WRAP: {
			value = unit -> phase.wrap;
			break;
		}
		case BK_NUM_PHASES: {
			value = (unit -> waveform != BK_SAMPLE) ? unit -> phase.count : unit -> sample.length;
			break;
		}
		case BK_VOLUME_0:
		case BK_VOLUME_1:
		case BK_VOLUME_2:
		case BK_VOLUME_3:
		case BK_VOLUME_4:
		case BK_VOLUME_5:
		case BK_VOLUME_6:
		case BK_VOLUME_7: {
			value = unit -> volume [attr - BK_VOLUME_0];
			break;
		}
		case BK_MUTE: {
			value = unit -> mute;
			break;
		}
		case BK_SAMPLE_REPEAT: {
			value = unit -> sample.repeat;
			break;
		}
		case BK_SAMPLE_PERIOD: {
			value = BKAbs (unit -> sample.period);
			break;
		}
		case BK_HALT_SILENT_PHASE: {
			value = unit -> phase.haltSilence;
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

BKInt BKUnitSetPtr (BKUnit * unit, BKEnum attr, void * ptr)
{
	BKInt   res;
	BKInt * values;

	switch (attr) {
		case BK_SAMPLE_CALLBACK: {
			// set sample callback
			if (ptr) {
				unit -> sample.callback = * (BKCallback *) ptr;
			}
			// unset callback
			else {
				memset (& unit -> sample.callback, 0, sizeof (BKCallback));
			}

			break;
		}
		case BK_WAVEFORM:
		case BK_SAMPLE: {
			res = BKUnitSetData (unit, attr, ptr);

			if (res < 0)
				return res;

			break;
		}
		case BK_SAMPLE_RANGE: {
			if (unit -> waveform != BK_SAMPLE)
				return BK_INVALID_STATE;

			values = ptr;

			if (values == NULL) {
				values [0] = 0;
				values [1] = -1;
			}

			BKUnitUpdateSampleRange (unit, values [0], values [1]);

			break;
		}
		default: {
			return BK_INVALID_ATTRIBUTE;
			break;
		}
	}

	return 0;
}

BKInt BKUnitGetPtr (BKUnit const * unit, BKEnum attr, void * outPtr)
{
	void ** ptrRef = outPtr;
	BKInt * values = outPtr;

	switch (attr) {
		case BK_SAMPLE_CALLBACK: {
			* ptrRef = (BKCallback *) & unit -> sample.callback;
			break;
		}
		case BK_SAMPLE: {
			// data may be set but is invalid; only return if valid
			switch (unit -> waveform) {
				case BK_CUSTOM:
				case BK_SAMPLE: {
					* ptrRef = unit -> sample.dataState.data;
					break;
				}
				default: {
					* ptrRef = NULL;
					break;
				}
			}
			break;
		}
		case BK_SAMPLE_RANGE: {
			values [0] = unit -> sample.offset;
			values [1] = unit -> sample.end;
			break;
		}
		default: {
			return BK_INVALID_ATTRIBUTE;
			break;
		}
	}

	return 0;
}

/**
 * Get next phase of current waveform
 * Returns 0 if no waveform is set
 */
static BKInt BKUnitNextPhase (BKUnit * unit)
{
	BKInt  amp = 0;
	BKUInt phase, phase0;

	phase = unit -> phase.phase;

	// wrap phase
	if (unit -> phase.wrap) {
		if (-- unit -> phase.wrapCount <= 0) {
			unit -> phase.wrapCount = unit -> phase.wrap;
			phase = 0;
		}
	}

	switch (unit -> waveform) {
		case BK_SQUARE: {
			 // shift phase to reduce volume peak at begin
			phase0 = (phase + BK_SQUARE_PHASES / 4) & (BK_SQUARE_PHASES - 1);
			amp    = phase0 < unit -> dutyCycle ? BK_MAX_VOLUME : 0;
			phase  = (phase + 1) & (BK_SQUARE_PHASES - 1);
			break;
		}
		case BK_TRIANGLE: {
			phase0 = (phase + BK_TRIANGLE_PHASES / 4) & (BK_TRIANGLE_PHASES - 1);
			amp    = (phase0 < (BK_TRIANGLE_PHASES / 2) ? phase0 : (BK_TRIANGLE_PHASES - phase0 - 1)) - 8;
			amp    = amp * BK_MAX_VOLUME / (BK_TRIANGLE_PHASES / 2);
			phase  = (phase + 1) & (BK_TRIANGLE_PHASES - 1);
			break;
		}
		case BK_NOISE: {
			phase = phase ?: 0x4a41;  // must not be 0
			amp   = ((phase >> 0) ^ (phase >> 2) ^ (phase >> 3) ^ (phase >> 5)) & 1;
			phase = (phase >> 1) | (amp << 15);
			amp   = amp ? BK_MAX_VOLUME / 2 : -BK_MAX_VOLUME / 2;
			break;
		}
		case BK_SAWTOOTH: {
			amp = ((BK_SAWTOOTH_PHASES - phase - 1) * (BK_MAX_VOLUME / (BK_SAWTOOTH_PHASES - 1)));
			phase ++;
			if (phase >= BK_SAWTOOTH_PHASES)
				phase = 0;
			break;
		}
		case BK_SINE: {
			amp   = BKSinePhases [phase] / 2;
			phase = (phase + 1) & (BK_SINE_PHASES - 1);
			break;
		}
		case BK_CUSTOM: {
			amp = unit -> sample.frames [phase];
			phase ++;
			if (phase >= unit -> phase.count)
				phase = 0;
			break;
		}
	}

	unit -> phase.phase = phase;

	return amp;
}

/**
 * Fills buffer with waveform to specified time
 */
static BKFUInt20 BKUnitRunWaveform (BKUnit * unit, BKFUInt20 endTime, BKInt advanceSilentPhase)
{
	BKFUInt20  time = 0;
	BKInt      volume;
	BKBuffer * channel;
	BKInt      pulse, lastPulse, delta, chanDelta;
	BKInt      origPhase, origPhaseWrapCount;

	origPhase          = unit -> phase.phase;
	origPhaseWrapCount = unit -> phase.wrapCount;

	// update each channel
	for (BKInt i = 0; i < unit -> ctx -> numChannels; i ++) {
		volume = unit -> volume [i];

		// mute sets volume to 0
		if (unit -> mute)
			volume = 0;

		if (volume || advanceSilentPhase) {
			channel   = & unit -> ctx -> channels [i];
			lastPulse = unit -> lastPulse [i];

			unit -> phase.phase     = origPhase;
			unit -> phase.wrapCount = origPhaseWrapCount;

			// run until time
			for (time = unit -> time; time < endTime; time += unit -> period) {
				pulse = BKUnitNextPhase (unit);
				delta = (pulse * volume) >> BK_VOLUME_SHIFT;

				chanDelta = delta - lastPulse;
				lastPulse = delta;

				BKBufferAddPulse (channel, time, chanDelta);
			}

			unit -> lastPulse [i] = lastPulse;
		}
	}

	return time;
}

/**
 * Wrap sample phase if it exceeds the sample range boundary
 */
static BKInt BKUnitWrapSamplePhase (BKUnit * unit)
{
	BKInt resetDir = 0;
	BKInt length   = unit -> sample.length;

	// reset if phase exceeds end
	if ((BKInt) unit -> phase.phase >= (BKInt) unit -> sample.length) {
		// phase %= length
		while ((BKInt) unit -> phase.phase >= length) {
			unit -> phase.phase -= length;
		}
		resetDir = 1;
	}
	// reset if phase exceeds end (reversed)
	else if ((BKInt) unit -> phase.phase < 0) {
		// phase %= length
		while ((BKInt) unit -> phase.phase < 0) {
			unit -> phase.phase += length;
		}
		resetDir = -1;
	}

	return resetDir;
}

static BKInt BKUnitResetSample (BKUnit * unit)
{
	BKInt repeat, resetDir;
	BKInt mode = 0;
	BKInt halt = 0;

	repeat = unit -> sample.repeat;

	if (unit -> sample.callback.func) {
		repeat = BKUnitCallSampleCallback (unit, BK_EVENT_SAMPLE_RESET);
	}

	resetDir = BKUnitWrapSamplePhase (unit);

	switch (repeat) {
		default:
		case BK_NO_REPEAT: {
			mode = 0;
			break;
		}
		case BK_REPEAT: {
			mode = resetDir;
			break;
		}
		case BK_PALINDROME: {
			if (resetDir == 1) {
				unit -> phase.phase = unit -> sample.length - unit -> phase.phase - 1;
			}
			else if (resetDir == -1) {
				unit -> phase.phase = unit -> sample.length - unit -> phase.phase;
			}
			mode = -resetDir;
			break;
		}
	}

	// halt
	if (mode == 0) {
		halt = 1;
	}
	// set direction
	else {
		unit -> sample.period = BKAbs (unit -> sample.period);

		// reverse
		if (mode == -1) {
			unit -> sample.period = -unit -> sample.period;
		}
	}

	unit -> sample.repeatCount ++;

	if (halt) {
		BKSetAttr (unit, BK_MUTE, 1);
	}

	return halt;
}

/**
 * Fills buffer with sample to specified time
 * Calls sample callback if sample has ended and asks if it should be repeated
 */
static BKFUInt20 BKUnitRunSample (BKUnit * unit, BKFUInt20 endTime)
{
	BKFInt20   time, lastTime;
	BKInt      volume;
	BKBuffer * channel;
	BKInt      pulse, delta, chanDelta;
	BKFrame  * frames;

	// muted
	if (unit -> mute) {
		return endTime;
	}

	// prevent invalid sample length
	if (BKAbs ((BKInt) unit -> sample.end - (BKInt) unit -> sample.offset) < 2)
		return endTime;

	for (time = unit -> time; time < endTime; time += BK_FINT20_UNIT) {
		frames = & unit -> sample.frames [unit -> phase.phase * unit -> sample.numChannels];

		// update each channel
		for (BKInt i = 0; i < unit -> ctx -> numChannels; i ++) {
			channel = & unit -> ctx -> channels [i];
			volume  = unit -> volume [i];
			pulse   = frames [unit -> sample.numChannels == 1 ? 0 : i];
			delta   = (pulse * volume) >> BK_VOLUME_SHIFT;

			chanDelta = delta - unit -> lastPulse [i];
			unit -> lastPulse [i] = delta;

			BKBufferAddPulse (channel, time, chanDelta);
		}

		// advance phase
		lastTime = unit -> sample.timeFrac;
		unit -> sample.timeFrac += unit -> sample.period; // may be negative
		unit -> phase.phase += (unit -> sample.timeFrac >> BK_FINT20_SHIFT) - (lastTime >> BK_FINT20_SHIFT);
		unit -> sample.timeFrac &= BK_FINT20_FRAC;

		// check phase boundary
		if ((BKInt) unit -> phase.phase < 0 || (BKInt) unit -> phase.phase >= (BKInt) unit -> sample.length) {
			if (BKUnitResetSample (unit) == 1) {
				break;
			}
		}
	}

	return time;
}

BKInt BKUnitRun (BKUnit * unit, BKFUInt20 endTime)
{
	BKContext * ctx  = unit -> ctx;
	BKFUInt20   time = unit -> time;
	BKBuffer  * channel;

	if (unit -> period) {
		switch (unit -> waveform) {
			case BK_SQUARE:
			case BK_TRIANGLE:
			case BK_NOISE:
			case BK_SAWTOOTH:
			case BK_SINE:
			case BK_CUSTOM: {
				time = BKUnitRunWaveform (unit, endTime, (unit -> phase.haltSilence == 0));
				break;
			}
			case BK_SAMPLE: {
				time = BKUnitRunSample (unit, endTime);
				break;
			}
		}
	}

	// advance time in case less data was written
	if (time < endTime)
		time = endTime;

	// advance buffer capacity
	for (BKInt i = 0; i < ctx -> numChannels; i ++) {
		channel = & ctx -> channels [i];
		BKBufferEnd (channel, time);
	}

	unit -> time = time;

	return 0;
}

/**
 * Shift unit time
 */
void BKUnitEnd (BKUnit * unit, BKFUInt20 time)
{
	unit -> time -= time;
}

void BKUnitClear (BKUnit * unit)
{
	BKUnitSetData (unit, BK_WAVEFORM, NULL);
	BKUnitSetData (unit, BK_SAMPLE, NULL);

	unit -> phase.wrap      = 0;
	unit -> phase.wrapCount = 0;

	unit -> sample.offset                     = 0;
	unit -> sample.end                        = 0;
	unit -> sample.repeat                     = 0;
	unit -> sample.repeatCount                = 0;
	unit -> sample.period                     = BK_FINT20_UNIT;
	unit -> sample.timeFrac                   = 0;
	unit -> sample.dataState.callback         = (void *) BKUnitSampleDataStateCallback;
	unit -> sample.dataState.callbackUserInfo = unit;

	// reset sample range
	if (unit -> sample.dataState.data)
		unit -> sample.end = unit -> sample.dataState.data -> numFrames;

	// reverse period if needed
	if (unit -> sample.offset > unit -> sample.end)
		unit -> sample.period = -BKAbs (unit -> sample.period);
}

void BKUnitReset (BKUnit * unit)
{
	BKUnitClear (unit);

	unit -> period      = 0;
	unit -> waveform    = 0;
	unit -> phase.phase = 0;
	unit -> time = 0;

	for (BKInt i = 0; i < BK_MAX_CHANNELS; i ++)
		unit -> lastPulse [i] = 0;
}

static BKInt BKUnitSetPtrSize (BKUnit * unit, BKEnum attr, void * ptr, BKSize size)
{
	return BKUnitSetPtr (unit, attr, ptr);
}

static BKInt BKUnitGetPtrSize (BKUnit * unit, BKEnum attr, void * outPtr, BKSize size)
{
	return BKUnitSetPtr (unit, attr, outPtr);
}

BKClass BKUnitClass =
{
	.instanceSize = sizeof (BKUnit),
	.dispose      = (BKDisposeFunc) BKUnitDisposeObject,
	.setAttr      = (BKSetAttrFunc) BKUnitSetAttr,
	.getAttr      = (BKGetAttrFunc) BKUnitGetAttr,
	.setPtr       = (BKSetPtrFunc)  BKUnitSetPtrSize,
	.getPtr       = (BKGetPtrFunc)  BKUnitGetPtrSize,
};
