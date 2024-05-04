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

#include "BKData_internal.h"
#include "BKUnit_internal.h"

#define MAV BK_MAX_VOLUME
#define MIV -BK_MAX_VOLUME

extern BKClass BKUnitClass;

// clang-format off

static BKFrame const squarePhases[BK_SQUARE_PHASES + 1][BK_SQUARE_PHASES] = {
	{  0,   MAV, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
	{  0,   MAV, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
	{  0,   0,   MAV, MAV, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
	{  0,   0,   MAV, MAV, MAV, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
	{  0,   0,   0,   MAV, MAV, MAV, MAV, 0,   0,   0,   0,   0,   0,   0,   0,   0},
	{  0,   0,   0,   MAV, MAV, MAV, MAV, MAV, 0,   0,   0,   0,   0,   0,   0,   0},
	{  0,   0,   0,   0,   MAV, MAV, MAV, MAV, MAV, MAV, 0,   0,   0,   0,   0,   0},
	{  0,   0,   0,   0,   MAV, MAV, MAV, MAV, MAV, MAV, MAV, 0,   0,   0,   0,   0},
	{  0,   0,   0,   0,   0,   MAV, MAV, MAV, MAV, MAV, MAV, MAV, MAV, 0,   0,   0},

	{  0,   0,   0,   0,   0,   0,   MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, 0},
	{  0,   0,   0,   0,   0,   MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, 0},
	{  0,   0,   0,   0,   MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, 0},
	{  0,   0,   0,   MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, 0},
	{  0,   0,   MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, 0},
	{  0,   MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, 0},
	{  MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, 0},
	{  MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, MIV, 0},
};

static BKFrame const trianglePhases[BK_TRIANGLE_PHASES] = {
	     0,   2047,   4095,   6143,   8191,  10239,  12287,  14335,
	 14335,  12287,  10239,   8191,   6143,   4095,   2047,      0,
	 -2047,  -4095,  -6143,  -8191, -10239, -12287, -14335, -16383,
	-16383, -14335, -12287, -10239,  -8191,  -6143,  -4095,  -2047,
};

static BKFrame const sawtoothPhases[BK_SAWTOOTH_PHASES] = {
	 32766,  27305,  21844,  16383,  10922,   5461,      0,
};

static BKFrame const sinePhases[BK_SINE_PHASES] = {
	     0,   6392,  12539,  18204,  23169,  27244,  30272,  32137,
	 32767,  32137,  30272,  27244,  23169,  18204,  12539,   6392,
	     0,  -6392, -12539, -18204, -23169, -27244, -30272, -32137,
	-32767, -32137, -30272, -27244, -23169, -18204, -12539,  -6392,
};

// clang-format on

static BKEnum BKUnitCallSampleCallback(BKUnit* unit, BKEnum event);
static void BKUnitUpdateSampleSustainRange(BKUnit* unit, BKInt offset, BKInt end);

static BKInt BKUnitTrySetData(BKUnit* unit, BKData* data, BKEnum type, BKEnum event) {
	BKContext* ctx = unit->ctx;

	if (ctx == NULL) {
		return BK_INVALID_STATE;
	}

	switch (type) {
		// set data as custom waveform
		case BK_WAVEFORM: {
			if (data && event != BK_DATA_STATE_EVENT_DISPOSE) {
				if (data->numFrames >= 2) {
					unit->waveform = BK_CUSTOM;
					unit->phase.count = BKMin(data->numFrames, BK_WAVE_MAX_LENGTH);
					unit->phase.phase = 0;
					unit->sample.offset = 0;
					unit->sample.end = data->numFrames;
					unit->sample.frames = data->frames;
					unit->sample.repeatCount = 0;
				}
				else {
					return BK_INVALID_NUM_FRAMES;
				}
			}
			// data was disposed
			else if (unit->waveform == BK_CUSTOM) {
				unit->waveform = 0;
				unit->sample.offset = 0;
				unit->sample.end = 0;
				unit->sample.frames = NULL;
				return -1;
			}

			break;
		}
		// set sample to play
		case BK_SAMPLE: {
			if (data && event != BK_DATA_STATE_EVENT_DISPOSE) {
				// number of channels must eighter be 1 or equal to
				// number of context channels
				if (data->numChannels != 1 && data->numChannels != ctx->numChannels) {
					return BK_INVALID_NUM_CHANNELS;
				}
				else if (data->numFrames < 2) {
					return BK_INVALID_NUM_FRAMES;
				}
				else {
					unit->waveform = BK_SAMPLE;
					unit->phase.count = 1; // prevent divion by 0
					unit->sample.length = data->numFrames;
					unit->sample.numChannels = data->numChannels;
					unit->sample.offset = 0;
					unit->sample.end = data->numFrames;
					unit->sample.frames = data->frames;
					unit->sample.period = BKAbs(unit->sample.period);
					unit->phase.phase = 0; // reset phase
					unit->sample.repeatMode = 0;
					unit->sample.repeatCount = 0;
					unit->sample.sustainOffset = 0;
					unit->sample.sustainEnd = 0;

					BKUnitCallSampleCallback(unit, BK_EVENT_SAMPLE_BEGIN);

					BKBitUnset(unit->object.flags, BKUnitFlagSampleSustainRange | BKUnitFlagSampleSustainJump | BKUnitFlagRelease);
				}
			}
			// disable sample
			else if (unit->waveform == BK_SAMPLE) {
				unit->waveform = 0;
				unit->sample.offset = 0;
				unit->sample.end = 0;
				unit->sample.sustainOffset = 0;
				unit->sample.sustainEnd = 0;
				unit->sample.frames = NULL;
				unit->phase.phase = 0; // reset phase
				unit->sample.repeatMode = 0;

				BKBitUnset(unit->object.flags, BKUnitFlagSampleSustainRange | BKUnitFlagSampleSustainJump | BKUnitFlagRelease);
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

static BKInt BKUnitSetData(BKUnit* unit, BKEnum type, BKData* data) {
	BKInt res = BKUnitTrySetData(unit, data, type, BK_DATA_STATE_EVENT_RESET);

	if (res == 0) {
		BKDataStateSetData(&unit->sample.dataState, data);
	}
	// unset data when failed
	else {
		switch (unit->waveform) {
			case BK_CUSTOM:
			case BK_SAMPLE: {
				unit->waveform = 0;
				break;
			}
		}
	}

	return res;
}

BKInt BKUnitSampleDataStateCallback(BKEnum event, BKUnit* unit) {
	BKEnum type;

	switch (unit->waveform) {
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
	return BKUnitTrySetData(unit, unit->sample.dataState.data, type, event);
}

static BKEnum BKUnitCallSampleCallback(BKUnit* unit, BKEnum event) {
	BKEnum result = 0;

	if (unit->sample.callback.func) {
		BKCallbackInfo info = {
			.object = unit,
			.event = event,
		};

		unit->sample.callback.func(&info, unit->sample.callback.userInfo);
	}

	return result;
}

BKInt BKUnitInit(BKUnit* unit, BKEnum waveform) {
	if (BKObjectInit(unit, &BKUnitClass, sizeof(*unit)) < 0) {
		return -1;
	}

	unit->run = (BKUnitRunFunc)BKUnitRun;
	unit->end = (BKUnitEndFunc)BKUnitEnd;
	unit->reset = (BKUnitResetFunc)BKUnitReset;

	BKSetAttr(unit, BK_DUTY_CYCLE, BK_DEFAULT_DUTY_CYCLE);
	BKSetAttr(unit, BK_WAVEFORM, waveform);

	unit->sample.period = BK_FINT20_UNIT;
	unit->sample.timeFrac = 0;
	unit->sample.dataState.callback = (void*)BKUnitSampleDataStateCallback;
	unit->sample.dataState.callbackUserInfo = unit;

	return 0;
}

void BKUnitDisposeObject(BKUnit* unit) {
	BKDataStateSetData(&unit->sample.dataState, NULL);

	BKUnitDetach(unit);
}

BKInt BKUnitAttach(BKUnit* unit, BKContext* ctx) {
	if (unit->ctx == NULL) {
		unit->prevUnit = ctx->lastUnit;
		unit->nextUnit = NULL;
		unit->ctx = ctx;
		unit->time = ctx->deltaTime; // shift time to context time

		if (ctx->lastUnit) {
			ctx->lastUnit->nextUnit = unit;
			ctx->lastUnit = unit;
		}
		// is first unit
		else {
			ctx->firstUnit = unit;
			ctx->lastUnit = unit;
		}
	}
	else {
		return BK_INVALID_STATE;
	}

	return 0;
}

void BKUnitDetach(BKUnit* unit) {
	BKContext* ctx = unit->ctx;

	if (ctx) {
		if (unit->prevUnit) {
			unit->prevUnit->nextUnit = unit->nextUnit;
		}
		// unit ist first unit
		else {
			ctx->firstUnit = unit->nextUnit;
		}

		if (unit->nextUnit) {
			unit->nextUnit->prevUnit = unit->prevUnit;
		}
		// unit is last unit
		else {
			ctx->lastUnit = unit->prevUnit;
		}

		unit->ctx = NULL;
		unit->time = 0;
	}
}

static void BKUnitUpdateSampleRange(BKUnit* unit, BKInt offset, BKInt end) {
	if (unit->sample.dataState.data == NULL) {
		return;
	}

	BKInt sampleLength = unit->sample.dataState.data->numFrames;

	if (offset < 0) {
		offset += sampleLength + 1;
	}
	offset = BKClamp(offset, 0, sampleLength);

	if (end < 0) {
		end += sampleLength + 1;
	}
	end = BKClamp(end, 0, sampleLength);

	BKInt oldOffset = BKMin(unit->sample.offset, unit->sample.end);
	BKInt newOffset = BKMin(offset, end);
	BKInt newLength = BKAbs(offset - end);
	BKInt newPhase = unit->phase.phase + (oldOffset - newOffset);

	// reset phase if it would overlap the new range
	if (newPhase < 0 || newPhase >= newLength) {
		newPhase = 0;
	}

	unit->sample.offset = offset;
	unit->sample.end = end;
	unit->sample.frames = &unit->sample.dataState.data->frames[newOffset * unit->sample.numChannels];
	unit->sample.length = newLength;
	unit->sample.period = BKAbs(unit->sample.period);

	// reverse sample period
	if (offset > end) {
		unit->sample.period = -unit->sample.period;
	}

	// if not playing yet
	if (unit->sample.repeatCount == 0) {
		// set sample phase to begin if sample phase is at end
		if (unit->sample.offset < unit->sample.end && unit->phase.phase == unit->sample.length - 1) {
			newPhase = 0;
		}
		// set sample phase to end if sample phase is at begin
		else if (unit->phase.phase == 0) {
			newPhase = unit->sample.length - 1;
		}
	}

	unit->phase.phase = newPhase;
}

static void BKUnitUpdateSampleSustainRange(BKUnit* unit, BKInt offset, BKInt end) {
	if (unit->sample.dataState.data == NULL) {
		return;
	}

	BKInt sampleLength = unit->sample.dataState.data->numFrames;

	if (offset < 0) {
		offset += sampleLength + 1;
	}

	if (end < 0) {
		end += sampleLength + 1;
	}

	offset = BKClamp(offset, 0, sampleLength);
	end = BKClamp(end, 0, sampleLength);

	if (end < offset) {
		BKInt tmp = offset;
		offset = end;
		end = tmp;
	}

	BKBitSetCond(unit->object.flags, BKUnitFlagSampleSustainRange, offset != end);

	unit->sample.sustainOffset = offset;
	unit->sample.sustainEnd = end;
}

BKInt BKUnitSetAttr(BKUnit* unit, BKEnum attr, BKInt value) {
	switch (attr) {
		case BK_DUTY_CYCLE: {
			value = BKClamp(value, BK_MIN_DUTY_CYCLE, BK_MAX_DUTY_CYCLE);

			if (unit->dutyCycle != value) {
				if (unit->waveform == BK_SQUARE) {
					// reduce clicking noise
					if (value > unit->dutyCycle && unit->phase.phase < value) {
						unit->phase.phase = 0;
					}
				}

				unit->dutyCycle = value;
			}

			break;
		}
		case BK_WAVEFORM: {
			switch (value) {
				case 0: {
					unit->phase.count = BK_SQUARE_PHASES;
					break;
				}
				case BK_SQUARE: {
					unit->phase.count = BK_SQUARE_PHASES;
					break;
				}
				case BK_TRIANGLE: {
					unit->phase.count = BK_TRIANGLE_PHASES;
					break;
				}
				case BK_NOISE: {
					unit->phase.count = BK_NOISE_PHASES;
					break;
				}
				case BK_SAWTOOTH: {
					unit->phase.count = BK_SAWTOOTH_PHASES;
					break;
				}
				case BK_SINE: {
					unit->phase.count = BK_SINE_PHASES;
					break;
				}
				default: {
					return BK_INVALID_VALUE;
					break;
				}
			}

			if (unit->waveform != value) {
				unit->phase.phase = 0;
				unit->waveform = value;
			}

			BKDataStateSetData(&unit->sample.dataState, NULL);

			break;
		}
		case BK_PHASE: {
			if (unit->waveform != BK_SAMPLE) {
				value = BKClamp(value, 0, unit->phase.count - 1);
			}
			else {
				// reset to sample start
				if (value < 0) {
					if (unit->sample.end > unit->sample.offset) {
						value = 0;
					}
					else {
						value = unit->sample.length - 1;
					}
				}

				value = BKClamp(value, 0, unit->sample.length - 1);
				BKUnitCallSampleCallback(unit, BK_EVENT_SAMPLE_BEGIN);
			}

			unit->phase.phase = value;

			break;
		}
		case BK_PHASE_WRAP: {
			value = value ? BKMax(value, 2) : 0;
			unit->phase.phase = 0;
			unit->phase.wrap = value;
			unit->phase.wrapCount = value;
			break;
		}
		case BK_PERIOD: {
			value = BKMax(BKAbs(value), BK_MIN_PERIOD);
			unit->period = value;
			break;
		}
		case BK_VOLUME: {
			value = BKClamp(value, 0, BK_MAX_VOLUME);

			for (BKInt i = 0; i < BK_MAX_CHANNELS; i++) {
				unit->volume[i] = value;
			}

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
			value = BKClamp(value, 0, BK_MAX_VOLUME);
			unit->volume[attr - BK_VOLUME_0] = value;
			break;
		}
		case BK_MUTE: {
			unit->mute = value ? 1 : 0;
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

			unit->sample.repeatMode = value;
			break;
		}
		case BK_SAMPLE_PERIOD: {
			value = BKAbs(value);
			value = BKClamp(value, BK_MIN_SAMPLE_PERIOD, BK_MAX_SAMPLE_PERIOD);

			// reverse if negative
			if (unit->sample.period < 0) {
				value = -value;
			}

			unit->sample.period = value;
			break;
		}
		case BK_SAMPLE_IMMED_RELEASE: {
			value = value ? BKUnitFlagSampleSustainJump : 0;
			unit->object.flags = (unit->object.flags & ~BKUnitFlagSampleSustainJump) | value;
			break;
		}
		case BK_FLAG_RELEASE: {
			if (unit->object.flags & BKUnitFlagSampleSustainRange) {
				value = value ? BKUnitFlagRelease : 0;
				unit->object.flags = (unit->object.flags & ~BKUnitFlagRelease) | value;

				if (value) {
					if ((unit->object.flags & BKUnitFlagSampleSustainJump) && unit->mute == 0) {
						if (unit->sample.period > 0) {
							unit->phase.phase = unit->sample.sustainEnd;
						}
						else {
							unit->phase.phase = BKMax(0, (BKInt)unit->sample.sustainOffset - 1);
						}
					}
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

BKInt BKUnitGetAttr(BKUnit const* unit, BKEnum attr, BKInt* outValue) {
	BKInt value = 0;

	switch (attr) {
		case BK_WAVEFORM: {
			value = unit->waveform;
			break;
		}
		case BK_DUTY_CYCLE: {
			value = unit->dutyCycle;
			break;
		}
		case BK_PERIOD: {
			value = unit->period;
			break;
		}
		case BK_PHASE: {
			value = unit->phase.phase;
			break;
		}
		case BK_PHASE_WRAP: {
			value = unit->phase.wrap;
			break;
		}
		case BK_NUM_PHASES: {
			value = (unit->waveform != BK_SAMPLE) ? unit->phase.count : unit->sample.length;
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
			value = unit->volume[attr - BK_VOLUME_0];
			break;
		}
		case BK_MUTE: {
			value = unit->mute;
			break;
		}
		case BK_SAMPLE_REPEAT: {
			value = unit->sample.repeatMode;
			break;
		}
		case BK_SAMPLE_PERIOD: {
			value = BKAbs(unit->sample.period);
			break;
		}
		case BK_SAMPLE_IMMED_RELEASE: {
			value = (unit->object.flags & BKUnitFlagSampleSustainJump) ? 1 : 0;
			break;
		}
		case BK_FLAG_RELEASE: {
			value = (unit->object.flags & BKUnitFlagRelease) ? 1 : 0;
			break;
		}
		default: {
			return BK_INVALID_ATTRIBUTE;
			break;
		}
	}

	*outValue = value;

	return 0;
}

BKInt BKUnitSetPtr(BKUnit* unit, BKEnum attr, void* ptr) {
	switch (attr) {
		case BK_SAMPLE_CALLBACK: {
			// set sample callback
			if (ptr) {
				unit->sample.callback = *(BKCallback*)ptr;
			}
			// unset callback
			else {
				memset(&unit->sample.callback, 0, sizeof(BKCallback));
			}

			break;
		}
		case BK_WAVEFORM:
		case BK_SAMPLE: {
			BKInt res = BKUnitSetData(unit, attr, ptr);

			if (res < 0) {
				return res;
			}

			break;
		}
		case BK_SAMPLE_RANGE: {
			BKInt range[2];

			if (unit->waveform != BK_SAMPLE) {
				return BK_INVALID_STATE;
			}

			BKInt* values = ptr;

			if (values == NULL) {
				range[0] = 0;
				range[1] = -1;
				values = range;
			}

			BKUnitUpdateSampleRange(unit, values[0], values[1]);
			BKUnitUpdateSampleSustainRange(unit, 0, 0);

			break;
		}
		case BK_SAMPLE_SUSTAIN_RANGE: {
			BKInt range[2];

			if (unit->waveform != BK_SAMPLE) {
				return BK_INVALID_STATE;
			}

			BKInt* values = ptr;

			if (values == NULL || values[0] == values[1]) {
				range[0] = 0;
				range[1] = 0;
				values = range;
			}

			BKUnitUpdateSampleSustainRange(unit, values[0], values[1]);

			break;
		}
		default: {
			return BK_INVALID_ATTRIBUTE;
			break;
		}
	}

	return 0;
}

BKInt BKUnitGetPtr(BKUnit const* unit, BKEnum attr, void* outPtr) {
	void** ptrRef = outPtr;
	BKInt* values = outPtr;

	switch (attr) {
		case BK_SAMPLE_CALLBACK: {
			*ptrRef = (BKCallback*)&unit->sample.callback;
			break;
		}
		case BK_SAMPLE: {
			// data may be set but is invalid; only return if valid
			switch (unit->waveform) {
				case BK_CUSTOM:
				case BK_SAMPLE: {
					*ptrRef = unit->sample.dataState.data;
					break;
				}
				default: {
					*ptrRef = NULL;
					break;
				}
			}
			break;
		}
		case BK_SAMPLE_RANGE: {
			values[0] = unit->sample.offset;
			values[1] = unit->sample.end;
			break;
		}
		case BK_SAMPLE_SUSTAIN_RANGE: {
			values[0] = unit->sample.sustainOffset;
			values[1] = unit->sample.sustainEnd;
			break;
		}
		default: {
			return BK_INVALID_ATTRIBUTE;
			break;
		}
	}

	return 0;
}

static BKFUInt20 BKUnitRunWaveformSquare(BKUnit* unit, BKBuffer* channel, BKInt* lastPulseRef, BKInt volume, BKFUInt20 time, BKFUInt20 endTime) {
	BKInt dutyCycle = unit->dutyCycle;
	BKInt lastPulse = *lastPulseRef;
	BKUInt phase = unit->phase.phase;

	// run until time
	for (; time < endTime; time += unit->period) {
		BKInt pulse = squarePhases[dutyCycle][phase];
		phase = (phase + 1) & (BK_SQUARE_PHASES - 1);

		BKInt delta = (pulse * volume) >> BK_VOLUME_SHIFT;

		BKInt chanDelta = delta - lastPulse;
		lastPulse = delta;

		BKBufferAddPulse(channel, time, chanDelta);
	}

	unit->phase.phase = phase;
	*lastPulseRef = lastPulse;

	return time;
}

static BKFUInt20 BKUnitRunWaveformTriangle(BKUnit* unit, BKBuffer* channel, BKInt* lastPulseRef, BKInt volume, BKFUInt20 time, BKFUInt20 endTime) {
	BKInt lastPulse = *lastPulseRef;
	BKUInt phase = unit->phase.phase;

	// run until time
	for (; time < endTime; time += unit->period) {
		BKInt pulse = trianglePhases[phase];
		phase = (phase + 1) & (BK_TRIANGLE_PHASES - 1);

		BKInt delta = (pulse * volume) >> BK_VOLUME_SHIFT;

		BKInt chanDelta = delta - lastPulse;
		lastPulse = delta;

		BKBufferAddPulse(channel, time, chanDelta);
	}

	unit->phase.phase = phase;
	*lastPulseRef = lastPulse;

	return time;
}

static BKFUInt20 BKUnitRunWaveformNoise(BKUnit* unit, BKBuffer* channel, BKInt* lastPulseRef, BKInt volume, BKFUInt20 time, BKFUInt20 endTime) {
	BKInt lastPulse = *lastPulseRef;
	BKUInt phase = unit->phase.phase;
	BKUInt wrap = unit->phase.wrap;
	BKUInt wrapCount = unit->phase.wrapCount;

	// must not be 0
	if (!phase) {
		phase = 0x4a41;
	}

	// run until time
	for (; time < endTime; time += unit->period) {
		if (wrap) {
			if (--wrapCount <= 0) {
				wrapCount = wrap;
				phase = 0x4a41;
			}
		}

		BKInt pulse = ((phase >> 0) ^ (phase >> 2) ^ (phase >> 3) ^ (phase >> 5)) & 1;
		phase = (phase >> 1) | (pulse << 15);
		pulse = pulse ? BK_MAX_VOLUME / 2 : -BK_MAX_VOLUME / 2;

		BKInt delta = (pulse * volume) >> BK_VOLUME_SHIFT;

		BKInt chanDelta = delta - lastPulse;
		lastPulse = delta;

		BKBufferAddPulse(channel, time, chanDelta);
	}

	unit->phase.phase = phase;
	unit->phase.wrapCount = wrapCount;
	*lastPulseRef = lastPulse;

	return time;
}

static BKFUInt20 BKUnitRunWaveformSawtooth(BKUnit* unit, BKBuffer* channel, BKInt* lastPulseRef, BKInt volume, BKFUInt20 time, BKFUInt20 endTime) {
	BKInt lastPulse = *lastPulseRef;
	BKUInt phase = unit->phase.phase;

	// run until time
	for (; time < endTime; time += unit->period) {
		BKInt pulse = sawtoothPhases[phase];

		if (++phase >= BK_SAWTOOTH_PHASES) {
			phase = 0;
		}

		BKInt delta = (pulse * volume) >> BK_VOLUME_SHIFT;

		BKInt chanDelta = delta - lastPulse;
		lastPulse = delta;

		BKBufferAddPulse(channel, time, chanDelta);
	}

	unit->phase.phase = phase;
	*lastPulseRef = lastPulse;

	return time;
}

static BKFUInt20 BKUnitRunWaveformSine(BKUnit* unit, BKBuffer* channel, BKInt* lastPulseRef, BKInt volume, BKFUInt20 time, BKFUInt20 endTime) {
	BKInt lastPulse = *lastPulseRef;
	BKUInt phase = unit->phase.phase;

	// run until time
	for (; time < endTime; time += unit->period) {
		BKInt pulse = sinePhases[phase] / 2;
		phase = (phase + 1) & (BK_SINE_PHASES - 1);

		BKInt delta = (pulse * volume) >> BK_VOLUME_SHIFT;

		BKInt chanDelta = delta - lastPulse;
		lastPulse = delta;

		BKBufferAddPulse(channel, time, chanDelta);
	}

	unit->phase.phase = phase;
	*lastPulseRef = lastPulse;

	return time;
}

static BKFUInt20 BKUnitRunWaveformCustom(BKUnit* unit, BKBuffer* channel, BKInt* lastPulseRef, BKInt volume, BKFUInt20 time, BKFUInt20 endTime) {
	BKInt lastPulse = *lastPulseRef;
	BKUInt phase = unit->phase.phase;
	BKUInt wrap = unit->phase.wrap;
	BKUInt wrapCount = unit->phase.wrapCount;

	// run until time
	for (; time < endTime; time += unit->period) {
		if (wrap) {
			if (--wrapCount <= 0) {
				wrapCount = wrap;
				phase = 0;
			}
		}

		BKInt pulse = unit->sample.frames[phase];

		if (++phase >= unit->phase.count) {
			phase = 0;
		}

		BKInt delta = (pulse * volume) >> BK_VOLUME_SHIFT;

		BKInt chanDelta = delta - lastPulse;
		lastPulse = delta;

		BKBufferAddPulse(channel, time, chanDelta);
	}

	unit->phase.phase = phase;
	unit->phase.wrapCount = wrapCount;
	*lastPulseRef = lastPulse;

	return time;
}

/**
 * Fills buffer with waveform to specified time
 */
static BKFUInt20 BKUnitRunWaveform(BKUnit* unit, BKFUInt20 endTime) {
	BKFUInt20 time = 0;
	BKInt origPhase = unit->phase.phase;
	BKInt origWrapCount = unit->phase.wrapCount;

	if (unit->mute) {
		return time;
	}

	// update each channel
	for (BKInt i = 0; i < unit->ctx->numChannels; i++) {
		BKInt volume = unit->volume[i];

		if (!volume) {
			continue;
		}

		BKBuffer* channel = &unit->ctx->channels[i];
		BKInt lastPulse = unit->lastPulse[i];
		time = unit->time;

		unit->phase.phase = origPhase;
		unit->phase.wrapCount = origWrapCount;

		switch (unit->waveform) {
			case BK_SQUARE: {
				time = BKUnitRunWaveformSquare(unit, channel, &lastPulse, volume, time, endTime);
				break;
			}
			case BK_TRIANGLE: {
				time = BKUnitRunWaveformTriangle(unit, channel, &lastPulse, volume, time, endTime);
				break;
			}
			case BK_NOISE: {
				time = BKUnitRunWaveformNoise(unit, channel, &lastPulse, volume, time, endTime);
				break;
			}
			case BK_SAWTOOTH: {
				time = BKUnitRunWaveformSawtooth(unit, channel, &lastPulse, volume, time, endTime);
				break;
			}
			case BK_SINE: {
				time = BKUnitRunWaveformSine(unit, channel, &lastPulse, volume, time, endTime);
				break;
			}
			case BK_CUSTOM: {
				time = BKUnitRunWaveformCustom(unit, channel, &lastPulse, volume, time, endTime);
				break;
			}
		}

		unit->lastPulse[i] = lastPulse;
	}

	return time;
}

/**
 * Wrap sample phase in given range
 */
static BKInt BKUnitWrapSamplePhase(BKUnit* unit, BKInt min, BKInt max) {
	BKInt resetDir = 0;
	BKInt length = max - min;

	// reset if phase exceeds end
	if ((BKInt)unit->phase.phase >= max) {
		// phase %= length
		while ((BKInt)unit->phase.phase >= max) {
			unit->phase.phase -= length;
		}

		resetDir = 1;
	}
	// reset if phase exceeds end (reversed)
	else if ((BKInt)unit->phase.phase < min) {
		// phase %= length
		while ((BKInt)unit->phase.phase < min) {
			unit->phase.phase += length;
		}

		resetDir = -1;
	}

	return resetDir;
}

static BKInt BKUnitResetSample(BKUnit* unit) {
	BKInt mode = 0;
	BKInt halt = 0;

	BKInt repeatMode = unit->sample.repeatMode;

	if (unit->sample.callback.func) {
		repeatMode = BKUnitCallSampleCallback(unit, BK_EVENT_SAMPLE_RESET);
	}

	BKInt resetDir = BKUnitWrapSamplePhase(unit, 0, unit->sample.length);

	switch (repeatMode) {
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
				unit->phase.phase = unit->sample.length - unit->phase.phase - 1;
			}
			else if (resetDir == -1) {
				unit->phase.phase = unit->sample.length - unit->phase.phase;
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
		unit->sample.period = BKAbs(unit->sample.period);

		// reverse
		if (mode == -1) {
			unit->sample.period = -unit->sample.period;
		}
	}

	unit->sample.repeatCount++;

	if (halt) {
		BKSetAttr(unit, BK_MUTE, 1);
	}

	return halt;
}

/**
 * Fills buffer with sample to specified time
 * Calls sample callback if sample has ended and asks if it should be repeated
 */
static BKFUInt20 BKUnitRunSample(BKUnit* unit, BKFUInt20 endTime) {
	BKFInt20 time;

	// muted
	if (unit->mute) {
		return endTime;
	}

	// prevent invalid sample length
	if (BKAbs((BKInt)unit->sample.end - (BKInt)unit->sample.offset) < 2) {
		return endTime;
	}

	BKInt checkBounds = (unit->object.flags & BKUnitFlagSampleSustainRange) && !(unit->object.flags & BKUnitFlagRelease);

	for (time = unit->time; time < endTime; time += BK_FINT20_UNIT) {
		BKFrame* frames = &unit->sample.frames[unit->phase.phase * unit->sample.numChannels];

		// update each channel
		for (BKInt i = 0; i < unit->ctx->numChannels; i++) {
			BKBuffer* channel = &unit->ctx->channels[i];
			BKInt volume = unit->volume[i];
			BKInt pulse = frames[unit->sample.numChannels == 1 ? 0 : i];
			BKInt delta = (pulse * volume) >> BK_VOLUME_SHIFT;

			BKInt chanDelta = delta - unit->lastPulse[i];
			unit->lastPulse[i] = delta;

			BKBufferAddPulse(channel, time + unit->sample.timeFrac, chanDelta);
		}

		// advance phase
		BKFInt20 lastTime = unit->sample.timeFrac;
		unit->sample.timeFrac += unit->sample.period; // may be negative
		unit->phase.phase += (unit->sample.timeFrac >> BK_FINT20_SHIFT) - (lastTime >> BK_FINT20_SHIFT);
		unit->sample.timeFrac &= BK_FINT20_FRAC;

		// check phase boundary
		if ((BKInt)unit->phase.phase < 0 || (BKInt)unit->phase.phase >= (BKInt)unit->sample.length) {
			if (BKUnitResetSample(unit) == 1) {
				break;
			}
		}

		// check for sustain range boundary
		if (checkBounds) {
			if (unit->sample.period > 0) {
				if ((BKInt)unit->phase.phase >= (BKInt)unit->sample.sustainEnd) {
					unit->phase.phase = unit->sample.sustainOffset;
				}
			}
			else {
				if ((BKInt)unit->phase.phase < (BKInt)unit->sample.sustainOffset) {
					unit->phase.phase = unit->sample.sustainEnd - 1;
				}
			}
		}
	}

	return time;
}

BKInt BKUnitRun(BKUnit* unit, BKFUInt20 endTime) {
	BKContext* ctx = unit->ctx;
	BKFUInt20 time = unit->time;

	if (unit->period) {
		switch (unit->waveform) {
			case BK_SQUARE:
			case BK_TRIANGLE:
			case BK_NOISE:
			case BK_SAWTOOTH:
			case BK_SINE:
			case BK_CUSTOM: {
				time = BKUnitRunWaveform(unit, endTime);
				break;
			}
			case BK_SAMPLE: {
				time = BKUnitRunSample(unit, endTime);
				break;
			}
		}
	}

	// advance time in case less data was written
	if (time < endTime) {
		time = endTime;
	}

	// advance buffer capacity
	for (BKInt i = 0; i < ctx->numChannels; i++) {
		BKBuffer* channel = &ctx->channels[i];
		BKBufferEnd(channel, time);
	}

	unit->time = time;

	return 0;
}

/**
 * Shift unit time
 */
void BKUnitEnd(BKUnit* unit, BKFUInt20 time) {
	unit->time -= time;
}

void BKUnitClear(BKUnit* unit) {
	BKUnitSetData(unit, BK_WAVEFORM, NULL);
	BKUnitSetData(unit, BK_SAMPLE, NULL);

	unit->object.flags &= BKUnitFlagsClearMask;
	unit->phase.wrap = 0;
	unit->phase.wrapCount = 0;

	unit->sample.offset = 0;
	unit->sample.end = 0;
	unit->sample.repeatMode = 0;
	unit->sample.repeatCount = 0;
	unit->sample.sustainOffset = 0;
	unit->sample.sustainEnd = 0;
	unit->sample.period = BK_FINT20_UNIT;
	unit->sample.timeFrac = 0;
	unit->sample.dataState.callback = (void*)BKUnitSampleDataStateCallback;
	unit->sample.dataState.callbackUserInfo = unit;

	// reset sample range
	if (unit->sample.dataState.data) {
		unit->sample.end = unit->sample.dataState.data->numFrames;
	}

	// reverse period if needed
	if (unit->sample.offset > unit->sample.end) {
		unit->sample.period = -BKAbs(unit->sample.period);
	}
}

void BKUnitReset(BKUnit* unit) {
	BKUnitClear(unit);

	unit->waveform = 0;
	unit->phase.phase = 0;
	unit->time = 0;

	for (BKInt i = 0; i < BK_MAX_CHANNELS; i++) {
		unit->lastPulse[i] = 0;
	}
}

static BKInt BKUnitSetPtrSize(BKUnit* unit, BKEnum attr, void* ptr, BKSize size) {
	return BKUnitSetPtr(unit, attr, ptr);
}

static BKInt BKUnitGetPtrSize(BKUnit* unit, BKEnum attr, void* outPtr, BKSize size) {
	return BKUnitSetPtr(unit, attr, outPtr);
}

BKClass BKUnitClass = {
	.instanceSize = sizeof(BKUnit),
	.dispose = (BKDisposeFunc)BKUnitDisposeObject,
	.setAttr = (BKSetAttrFunc)BKUnitSetAttr,
	.getAttr = (BKGetAttrFunc)BKUnitGetAttr,
	.setPtr = (BKSetPtrFunc)BKUnitSetPtrSize,
	.getPtr = (BKGetPtrFunc)BKUnitGetPtrSize,
};
