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

#ifndef _BK_UNIT_H_
#define _BK_UNIT_H_

#include "BKContext.h"
#include "BKData_internal.h"

enum
{
	BKUnitFlagSampleSustainRange = 1 << 0, // has `BK_SAMPLE_SUSTAIN_RANGE` set
	BKUnitFlagSampleSustainJump  = 1 << 1, // should jump immediately to release phase
	BKUnitFlagRelease            = 1 << 2, // set release phase
	BKUnitFlagsClearMask         = ~7,
};

typedef struct BKUnitFuncs BKUnitFuncs;

/**
 * Units are attached to a context and generate samples with a specified
 * waveform
 *
 * All functions return 0 on success and values < 0 on error
 */

typedef BKInt (* BKUnitRunFunc)   (void * unit, BKFUInt20 endTime);
typedef BKInt (* BKUnitEndFunc)   (void * unit, BKFUInt20 time);
typedef void  (* BKUnitResetFunc) (void * unit);

struct BKUnit
{
	BKObject object;

	// context
	BKContext     * ctx;
	BKUnitRunFunc   run;
	BKUnitEndFunc   end;
	BKUnitResetFunc reset;

	// linking
	BKUnit * prevUnit;
	BKUnit * nextUnit;

	// time
	BKFUInt20 time;
	BKFUInt20 period;
	BKInt     lastPulse [BK_MAX_CHANNELS];

	// waveform
	BKEnum waveform;
	BKUInt dutyCycle;

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
		BKUInt      repeatMode;
		BKUInt      repeatCount;
		BKUInt      sustainOffset; // relative to `offset`
		BKUInt      sustainEnd;    // relative to `offset`
		BKFInt20    timeFrac;
		BKFInt20    period;
		BKCallback  callback;
		BKFrame   * frames;
	} sample;
};

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
extern void BKUnitDispose (BKUnit * unit) BK_DEPRECATED_FUNC ("Use 'BKDispose' instead");

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
 *   Set waveform: BK_SQUARE, BK_TRIANGLE, BK_NOISE, BK_SAWTOOTH, BK_SINE
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
 * BK_SAMPLE
 *   Set a sample object to play
 * BK_SAMPLE_RANGE
 *   Frames range of the sample to play
 *   The first value is the start offset and the second one is the end offset
 *   If the end offset is less than start offset, th sample is played reversed
 *   Set to NULL to reset
 *   Will be reset to default when changing sample
 *   Disables `BK_SAMPLE_SUSTAIN_RANGE` when updating
 * BK_SAMPLE_REPEAT
 *   Set sample repeat mode: `BK_REPEAT`, `BK_PALINDROME`
 *   Default is `BK_NO_REPEAT`
 *   Use `BK_SAMPLE_CALLBACK` to have more control
 *   Will be ignored if `BK_SAMPLE_CALLBACK` is set
 * BK_SAMPLE_PERIOD
 *   Set speed at which the sample is played
 *   Default is BK_FINT20_UNIT
 * BK_SAMPLE_SUSTAIN_RANGE
 *   Set range to repeat when sample is played
 *   `BK_SAMPLE_REPEAT` does not affect sustain range
 *   Will be disabled when changing sample
 * BK_SAMPLE_IMMED_RELEASE
 *   Jump immediately to release phase when `BK_FLAG_RELEASE` is set
 * BK_FLAG_RELEASE
 *   Set release phase
 * BK_HALT_SILENT_PHASE
 *   Phase does stop cycling when muted or volume is 0
 *   This is automatically set when using BK_TRIANGLE and disabled on other waveforms
 *   It can reduce clicking noise when waveform is similar to triangle or sine
 *
 * Errors:
 * BK_INVALID_ATTRIBUTE if attribute is unknown
 * BK_INVALID_VALUE if value is invalid for this attribute
 * BK_INVALID_STATE if attribute can't be set in current unit state
 *   This happens when trying to set BK_SAMPLE_OFFSET or BK_SAMPLE_END when no
 *   sample is set
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
 * BK_SAMPLE_RANGE
 *   Set sample repeat range as BKInt[2]
 *   The first value defines the start offset in frames
 *   The second value defines the end offset in frames
 *   If the end position is less than the start position, the sample is played in reverse
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
 * BK_SAMPLE_RANGE
 *  Get sample repeat range
 *
 * Errors:
 * BK_INVALID_ATTRIBUTE if attribute is unknown
 */
extern BKInt BKUnitGetPtr (BKUnit const * unit, BKEnum attr, void * outPtr);

#endif /* ! _BK_UNIT_H_ */
