/**
 * Copyright (c) 2012-2013 Simon Schoenenberger
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

static BKEnum BKUnitCallSampleCallback (BKUnit * unit, BKEnum event);

BKUnitFuncs const BKUnitFuncsStruct =
{
	.run     = (void *) BKUnitRun,
	.end     = (void *) BKUnitEnd,
	.reset   = (void *) BKUnitReset,
	.getAttr = (void *) BKUnitGetAttr,
	.setAttr = (void *) BKUnitSetAttr,
	.getPtr  = (void *) BKUnitGetPtr,
	.setPtr  = (void *) BKUnitSetPtr,
};

static BKInt BKUnitTrySetData (BKUnit * unit, BKData * data, BKEnum type, BKEnum event)
{
	BKContext * ctx = unit -> ctx;
	
	switch (type) {
		// set data as custom waveform
		case BK_WAVEFORM: {
			if (data && event != BK_DATA_STATE_EVENT_DISPOSE) {				
				if (data -> numSamples >= 2) {
					unit -> waveform          = BK_CUSTOM;
					unit -> phase.count       = data -> numSamples;
					unit -> phase.haltSilence = 1;
					unit -> phase.phase       = 0;
				}
				else {
					return BK_INVALID_NUM_SAMPLES;
				}
			}
			// data was disposed
			else {
				unit -> waveform = 0;
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
				else if (data -> numSamples < 2) {
					return BK_INVALID_NUM_SAMPLES;
				}
				else {
					unit -> waveform           = BK_SAMPLE;
					unit -> phase.count        = 1;
					unit -> sample.count       = data -> numSamples;
					unit -> sample.numChannels = data -> numChannels;
					
					BKUnitCallSampleCallback (unit, BK_EVENT_SAMPLE_BEGIN);
				}
			}
			// disable sample
			else {
				unit -> waveform = 0;
			}
			
			unit -> phase.phase   = 0; // reset phase
			unit -> sample.repeat = 0;
			
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

	// unset data when failed
	if (res == 0) {
		BKDataStateSetData (& unit -> sample.dataState, data);
	}
	else {
		BKUnitSetAttr (unit, BK_WAVEFORM, 0);
	}

	return res;
}

static BKInt BKUnitSampleDataStateCallback (BKEnum event, BKUnit * unit)
{
	BKEnum type;
	
	switch (unit -> waveform) {
		case BK_CUSTOM: {
			type = BK_WAVEFORM;
			break;
		}
		case BK_SAMPLE: {
			type = BK_WAVEFORM;
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
	memset (unit, 0, sizeof (BKUnit));

	unit -> funcs = (BKUnitFuncs *) & BKUnitFuncsStruct;

	BKUnitSetAttr (unit, BK_DUTY_CYCLE, BK_DEFAULT_DUTY_CYCLE);
	BKUnitSetAttr (unit, BK_WAVEFORM, waveform);

	unit -> sample.period                     = BK_FINT20_UNIT;
	unit -> sample.time                       = 0;
	unit -> sample.dataState.callback         = (void *) BKUnitSampleDataStateCallback;
	unit -> sample.dataState.callbackUserInfo = unit;

	return 0;
}

void BKUnitDispose (BKUnit * unit)
{
	BKDataStateSetData (& unit -> sample.dataState, NULL);

	BKUnitDetach (unit);

	memset (unit, 0, sizeof (BKUnit));
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
				/*case 0: {
					unit -> phase.count = BK_SQUARE_PHASES;
					unit -> phase.haltSilence = 1;
					break;
				}*/
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
			else  {
				value = BKClamp (value, 0, unit -> sample.count - 1);
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
		}
		case BK_PERIOD: {
			unit -> period = BKMax (value, BK_MIN_PERIOD);
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
			unit -> sample.repeat = value;
			break;
		}
		case BK_SAMPLE_PERIOD: {
			unit -> sample.period = BKClamp (value, BK_MIN_SAMPLE_PERIOD, BK_MAX_SAMPLE_PERIOD);
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
			value = (unit -> waveform != BK_SAMPLE) ? unit -> phase.count : unit -> sample.count;
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
			value = unit -> sample.period;
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
	BKInt res;

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
		default: {
			return BK_INVALID_ATTRIBUTE;
			break;
		}
	}
	
	return 0;
}

/*
BKInt BKUnitSetAttr (BKUnit * unit, BKEnum attr, BKInt value)
{
	return unit -> funcs -> setAttr (unit, attr, value);
}

BKInt BKUnitGetAttr (BKUnit const * unit, BKEnum attr, BKInt * outValue)
{
	return unit -> funcs -> getAttr (unit, attr, outValue);
}

BKInt BKUnitSetPtr (BKUnit * unit, BKEnum attr, void * ptr)
{
	return unit -> funcs -> setPtr (unit, attr, ptr);
}

BKInt BKUnitGetPtr (BKUnit const * unit, BKEnum attr, void * outPtr)
{
	return unit -> funcs -> getPtr (unit, attr, outPtr);
}
*/

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
		case BK_CUSTOM: {
			amp = unit -> sample.dataState.data -> samples [phase];
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
				
				BKBufferUpdateStep (channel, time, chanDelta);
			}
			
			/*
			// run until time
			for (time = unit -> time; time < endTime; time += unit -> period) {
				pulse = BKUnitNextPhase (unit);
				delta = (pulse * volume) >> BK_VOLUME_SHIFT;

				// "scale" last pulse to current volume
				// this reduces click noisees if volume is lower than before
				chanDelta = delta - ((lastPulse * volume) >> BK_VOLUME_SHIFT);
				lastPulse = pulse;
				
				BKBufferUpdateStep (channel, time, chanDelta);
			}
			*/

			unit -> lastPulse [i] = lastPulse;
		}
	}

	return time;
}

static BKInt BKUnitResetSample (BKUnit * unit)
{
	BKInt numWrap = 0;
	BKInt halt    = 0;

	if (unit -> phase.wrap) {
		if (unit -> phase.wrap < unit -> sample.count) {
			numWrap = unit -> phase.wrap;
		}
		else {
			unit -> phase.phase = 0;
		}
	}
	else {
		numWrap = unit -> sample.count;
	}
	
	if (numWrap) {
		// phase %= numWrap
		do {
			unit -> phase.phase -= numWrap;
		}
		while (unit -> phase.phase >= numWrap);
	}

	// callback function
	if (unit -> sample.callback.func) {
		BKInt ret = BKUnitCallSampleCallback (unit, BK_EVENT_SAMPLE_RESET);
		
		// repeat sample if BK_SAMPLE_REPEAT is returned from callback
		if (ret != BK_SAMPLE_REPEAT)
			halt = 1;
	}
	// repeat attribute
	else if (unit -> sample.repeat > 0) {
		if (unit -> sample.repeat != BK_INT_MAX)
			unit -> sample.repeat --;
	}
	// no repeat
	else {
		halt = 1;
	}

	if (halt)
		BKUnitSetPtr (unit, BK_SAMPLE, NULL);

	return halt;
}

/**
 * Fills buffer with sample to specified time
 * Calls sample callback if sample has ended and asks if it should be repeated
 */
static BKFUInt20 BKUnitRunSample (BKUnit * unit, BKFUInt20 endTime)
{
	BKFUInt20  time, lastTime;
	BKInt      volume;
	BKBuffer * channel;
	BKInt      pulse, delta, chanDelta;
	BKFrame  * samples;
	BKData   * data;

	data = unit -> sample.dataState.data;

	for (time = unit -> time; time < endTime; time += BK_FINT20_UNIT) {
		BKInt reset = 0;

		samples = & data -> samples [unit -> phase.phase * unit -> sample.numChannels];

		// update each channel
		for (BKInt i = 0; i < unit -> ctx -> numChannels; i ++) {
			channel = & unit -> ctx -> channels [i];
			volume  = unit -> volume [i];
			pulse   = samples [unit -> sample.numChannels == 1 ? 0 : i];
			delta   = (pulse * volume) >> BK_VOLUME_SHIFT;
			
			chanDelta = delta - unit -> lastPulse [i];
			unit -> lastPulse [i] = delta;
			
			BKBufferUpdateSample (channel, time, chanDelta);
		}
		
		// advance phase
		lastTime = unit -> sample.time;
		unit -> sample.time += unit -> sample.period;
		unit -> phase.phase += (unit -> sample.time >> BK_FINT20_SHIFT) - (lastTime >> BK_FINT20_SHIFT);
		unit -> sample.time &= BK_FINT20_FRAC;
		
		// reset if sample ended
		if (unit -> phase.phase >= unit -> sample.count) {
			reset = 1;
		}
		// reset if phase wrap exceeded
		else if (unit -> phase.wrap && unit -> phase.phase > unit -> phase.wrap) {
			reset = 1;
		}

		if (reset) {
			if (BKUnitResetSample (unit) == 1)
				break;
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

void BKUnitReset (BKUnit * unit)
{
	BKContext * ctx = unit -> ctx;
	
	unit -> waveform        = 0;
	unit -> phase.phase     = 0;
	unit -> phase.wrap      = 0;
	unit -> phase.wrapCount = 0;
	unit -> period          = 0;
	unit -> time            = 0;

	BKUnitSetData (unit, BK_WAVEFORM, NULL);
	BKUnitSetData (unit, BK_SAMPLE, NULL);

	unit -> sample.period                     = BK_FINT20_UNIT;
	unit -> sample.time                       = 0;
	unit -> sample.dataState.callback         = (void *) BKUnitSampleDataStateCallback;
	unit -> sample.dataState.callbackUserInfo = unit;

	if (ctx) {
		for (BKInt i = 0; i < ctx -> numChannels; i ++)
			unit -> lastPulse [i] = 0;
	}
}
