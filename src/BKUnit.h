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

#ifndef _BK_UNIT_H_
#define _BK_UNIT_H_

#include "BKContext.h"
#include "BKData_internal.h"

typedef struct BKUnitFuncs BKUnitFuncs;

/**
 * Units are attached to a context and generate samples with a specified
 * waveform
 *
 * All functions return 0 on success and values < 0 on error
 */

struct BKUnit
{
	// context
	BKContext   * ctx;
	BKUnitFuncs * funcs;

	// linking
	BKUnit * prevUnit;
	BKUnit * nextUnit;

	// waveform
	BKEnum waveform;
	BKUInt dutyCycle;

	// time
	BKFUInt20 time;
	BKFUInt20 period;
	BKInt     lastPulse [BK_MAX_CHANNELS];

	// volume
	BKInt volume [BK_MAX_CHANNELS];
	BKInt mute;

	// phase
	struct {
		BKUInt phase;  // contains noise seed and sample offset
		BKUInt wrap;
		BKInt  wrapCount;
		BKUInt count;
		BKInt  haltSilence;
	} phase;

	// samples
	struct {
		BKDataState dataState;
		BKUInt      numChannels;
		BKUInt      length;
		BKUInt      offset;
		BKUInt      end;
		BKUInt      repeat;
		BKFUInt20   timeFrac;
		BKFUInt20   period;
		BKCallback  callback;
		BKFrame   * frames;
	} sample;
};

struct BKUnitFuncs
{
	BKInt (* run)     (void * unit, BKFUInt20 endTime);
	void  (* end)     (void * unit, BKFUInt20 time);
	void  (* reset)   (void * unit);
	BKInt (* getAttr) (void const * unit, BKEnum attr, BKInt * outValue);
	BKInt (* setAttr) (void * unit, BKEnum attr, BKInt value);
	BKInt (* getPtr)  (void const * unit, BKEnum attr, void * outPtr);
	BKInt (* setPtr)  (void * unit, BKEnum attr, void * ptr);
};

/**
 * Generate functions used by unit
 */
extern BKUnitFuncs const BKUnitFuncsStruct;

/**
 * Initialize unit
 *
 * Errors:
 * -1
 */
extern BKInt BKUnitInit (BKUnit * unit, BKEnum waveform);

/**
 * Dispose unit
 */
extern void BKUnitDispose (BKUnit * unit);

/**
 * Attach to context
 *
 * Errors:
 * BK_INVALID_STATE if already attached to a context
 */
extern BKInt BKUnitAttach (BKUnit * unit, BKContext * ctx);

/**
 * Detach from context
 */
extern void BKUnitDetach (BKUnit * unit);

/**
 * Set attribute
 *
 * BK_WAVEFORM
 *   Set waveform: BK_SQUARE, BK_TRIANGLE, BK_NOISE, BK_SAWTOOTH
 * BK_DUTY_CYCLE
 *   Set duty cycle of square wave (BK_SQUARE). Other waveforms are not affected.
 *   Value may be between 1 and 15. Values outside of this range are clamped.
 *   Default is 4 (25%)
 * BK_PERIOD
 *   Set period of wave phase
 * BK_PHASE
 *   Set wave phase; If waveform is BK_SAMPLE the sample offset is set
 * BK_PHASE_WRAP
 *   Set number of phases after phase should be resetted
 *   This may be used for the noise waveform to create a special effect
 *   Samples are not affected by this value
 * BK_VOLUME
 *   Set volume of all channels
 * BK_VOLUME_0 - BK_VOLUME_7
 *   Set volume of specific channel
 * BK_MUTE
 *   Has the same effect as setting the volume to 0 but does not change volume settings
 *   Can eighter be 0 or 1
 *   Default is 0
 * BK_SAMPLE_OFFSET
 *   Frames offset from which the sample should start playing
 *   When repeating the sample it will start playing again at this offset
 *   Default is 0
 * BK_SAMPLE_END
 *   End (but not including) frame offset to which the sample should be played
 *   Default is number of frames of the current sample
 *   If set to 0 this is set to the maximum offset
 * BK_SAMPLE_REPEAT
 *   Repeat sample if set to 1
 *   Default is 0
 *   Use `BK_SAMPLE_CALLBACK` to have more control
 *   Does nothing if `BK_SAMPLE_CALLBACK` is set
 * BK_SAMPLE_PERIOD
 *   Set speed at which the sample is played
 *   Default is BK_FINT20_UNIT
 * BK_HALT_SILENT_PHASE
 *   Phase does stop cycling when muted or volume is 0
 *   This is automatically set when using BK_TRIANGLE and disabled on other waveforms
 *   It can reduce clicking noise when waveform is similar to triangle or sine
 *
 * Errors:
 * BK_INVALID_ATTRIBUTE if attribute is unknown
 * BK_INVALID_VALUE if value is invalid for this attribute
 */
extern BKInt BKUnitSetAttr (BKUnit * unit, BKEnum attr, BKInt value);

/**
 * Get attribute
 *
 * BK_WAVEFORM
 * BK_DUTY_CYCLE
 * BK_PERIOD
 * BK_PHASE
 * BK_PHASE_WRAP
 * BK_VOLUME_0 - BK_VOLUME_7
 * BK_MUTE
 * BK_SAMPLE_REPEAT
 * BK_SAMPLE_PERIOD
 *
 * Errors:
 * BK_INVALID_ATTRIBUTE if attribute is unknown
 */
extern BKInt BKUnitGetAttr (BKUnit const * unit, BKEnum attr, BKInt * outValue);

/**
 * Set pointer
 *
 * BK_SAMPLE_CALLBACK
 *   Set sample event callback
 *   Sample is repeated if the callback returns `BK_SAMPLE_REPEAT`
 * BK_WAVEFORM
 *   Set custom waveform via a `BKData` object.
 *   Number of channels is ignored
 * BK_SAMPLE
 *   Set sample to play via a `BKData` object. Sample is only played once.
 *   If it should be repeated set attribute `BK_SAMPLE_REPEAT` to a value greater 1 or
 *   set `BK_SAMPLE_CALLBACK`
 *
 * Errors:
 * BK_INVALID_ATTRIBUTE if attribute is unknown
 * BK_INVALID_VALUE if pointer is invalid for this attribute
 * BK_INVALID_NUM_CHANNELS if the sample's number of channels does not match that of the context
 */
extern BKInt BKUnitSetPtr (BKUnit * unit, BKEnum attr, void * ptr);

/**
 * Get pointer
 *
 * BK_SAMPLE_CALLBACK
 *   Get sample callback
 * BK_SAMPLE
 *   Get `BKData` object if waveform is BK_CUSTOM or BK_SAMPLE
 *
 * Errors:
 * BK_INVALID_ATTRIBUTE if attribute is unknown
 */
extern BKInt BKUnitGetPtr (BKUnit const * unit, BKEnum attr, void * outPtr);

#endif /* ! _BK_UNIT_H_ */
