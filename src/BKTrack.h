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

#ifndef _BK_TRACK_H_
#define _BK_TRACK_H_

#include "BKUnit_internal.h"
#include "BKInstrument_internal.h"
#include "BKInterpolation.h"

#define BK_EFFECT_FLAG_SHIFT (16 - BK_EFFECT_TYPE - 1)

typedef struct BKTrack         BKTrack;
typedef struct BKDividerState  BKDividerState;
typedef struct BKArpeggioState BKArpeggioState;

enum
{
	BKVolumeSlideFlag              = 1 << (BK_EFFECT_VOLUME_SLIDE  + BK_EFFECT_FLAG_SHIFT),
	BKPanningSlideFlag             = 1 << (BK_EFFECT_PANNING_SLIDE + BK_EFFECT_FLAG_SHIFT),
	BKPortamentoFlag               = 1 << (BK_EFFECT_PORTAMENTO    + BK_EFFECT_FLAG_SHIFT),
	BKTremoloFlag                  = 1 << (BK_EFFECT_TREMOLO       + BK_EFFECT_FLAG_SHIFT),
	BKVibratoFlag                  = 1 << (BK_EFFECT_VIBRATO       + BK_EFFECT_FLAG_SHIFT),
	BKInstrumentFlag               = 1 << 0,
	BKArpeggioFlag                 = 1 << 1,
	BKPanningEnabledFlag           = 1 << 2,
	BKTriangleIgnoresVolumeFlag    = 1 << 3,
	BKIgnoreVolumeFlag             = 1 << 4,
	BKTrackAttrUpdateFlagVolume    = 1 << 5,
	BKTrackAttrUpdateFlagNote      = 1 << 6,
	BKTrackAttrUpdateFlagDutyCycle = 1 << 7,
	BKEffectMask                   = BKPortamentoFlag | BKVolumeSlideFlag
	| BKPanningSlideFlag | BKTremoloFlag | BKVibratoFlag,
};

struct BKDividerState
{
	BKInt divider;
	BKInt counter;
};

struct BKArpeggioState
{
	BKInt offset;
	BKInt delta;
	BKInt count;
	BKInt notes [BK_MAX_ARPEGGIO];
};

struct BKTrack
{
	BKUnit            unit;
	BKUInt            flags;
	BKDivider         divider;
	BKDividerState    arpeggioDivider;
	BKDividerState    instrDivider;
	BKDividerState    effectDivider;
	BKInt             waveform;
	BKInt             dutyCycle;
	BKData          * sample;
	BKFInt20          samplePitch;
	BKInt             masterVolume;
	BKSlideState      volume;
	BKSlideState      panning;
	BKInt             curNote;
	BKSlideState      note;
	BKFInt20          pitch;
	BKIntervalState   tremolo;
	BKSlideState      tremoloDelta;
	BKSlideState      tremoloSteps;
	BKIntervalState   vibrato;
	BKSlideState      vibratoDelta;
	BKSlideState      vibratoSteps;
	BKArpeggioState   arpeggio;
	BKInstrumentState instrState;
};

/**
 * Initialize track
 *
 * Disposing with `BKDispose` detaches the object from the context
 */
extern BKInt BKTrackInit (BKTrack * track, BKEnum waveform);

/**
 * Allocate track
 */
extern BKInt BKTrackAlloc (BKTrack ** outTrack, BKEnum waveform);

/**
 * Reset track values and buffer state
 */
extern void BKTrackReset (BKTrack * track);

/**
 * Reset track values
 */
extern void BKTrackClear (BKTrack * track);

/**
 * Attach to context
 *
 * Errors:
 * BK_INVALID_STATE if already attached to a context
 */
extern BKInt BKTrackAttach (BKTrack * track, BKContext * ctx);

/**
 * Detach from context
 */
extern void BKTrackDetach (BKTrack * track);

/**
 * Set attribute
 *
 * BK_MASTER_VOLUME
 *   Master volume which BK_VOLUME is multiplied by
 * BK_VOLUME
 *   General volume
 * BK_PANNING
 *   Negative values will pan to left, positive to right
 *   Only takes effect if context has exactly 2 channels
 * BK_NOTE
 *   Set note between BK_MIN_NOTE and BK_MAX_NOTE
 *   Value must be multiplied by BK_FINT20_UNIT
 *   To release the note the value is BK_NOTE_RELEASE
 *   To mute the note the value is BK_NOTE_MUTE
 * BK_INSTRUMENT
 *   Set an instrument object
 * BK_PITCH
 *   Set note pitch. This value will be added to every note.
 * BK_ARPEGGIO_DIVIDER
 *   Divider value for arpeggio step
 * BK_EFFECT_DIVIDER
 *   Divider value for effect step
 * BK_INSTRUMENT_DIVIDER
 *   Divider value for instrument step
 * BK_TRIANGLE_IGNORES_VOLUME
 *   Triangle ignores volume setting. Default is 1.
 * BK_EFFECT_VOLUME_SLIDE
 *   Volume slide effect value [0]; number of ticks
 * BK_EFFECT_PANNING_SLIDE
 *   Panning slide effect value [0]; number of ticks
 * BK_EFFECT_PORTMENTO
 *   Portamento effect value [0]; number of ticks
 *
 * All other attributes will be forwarded to the underlaying unit
 *
 * Errors:
 * BK_INVALID_ATTRIBUTE if attribute is unknown
 * BK_INVALID_VALUE if value is invalid for this attribute
 */
extern BKInt BKTrackSetAttr (BKTrack * track, BKEnum attr, BKInt value) BK_DEPRECATED_FUNC ("Use 'BKSetAttr' instead");

/**
 * Get attribute
 *
 * BK_MASTER_VOLUME
 * BK_VOLUME
 * BK_PANNING
 * BK_NOTE
 * BK_ARPEGGIO_DIVIDER
 * BK_EFFECT_DIVIDER
 * BK_INSTRUMENT_DIVIDER
 * BK_TRIANGLE_IGNORES_VOLUME
 * BK_EFFECT_VOLUME_SLIDE
 * BK_EFFECT_PANNING_SLIDE
 * BK_EFFECT_PORTMENTO
 *
 * All other attributes will be forwarded to the underlaying unit
 *
 * Errors:
 * BK_INVALID_ATTRIBUTE if attribute is unknown
 */
extern BKInt BKTrackGetAttr (BKTrack const * track, BKEnum attr, BKInt * outValue) BK_DEPRECATED_FUNC ("Use 'BKGetAttr' instead");

/**
 * Set pointer
 *
 * BK_INSTRUMENT
 *   Set current instrument
 *   This is a pointer to an initialized `BKInstrument` object
 * BK_ARPEGGIO
 *   Set arpeggio sequence via an integer array
 *   First element contains number of notes. The following elements contain
 *   delta note values
 *   A maximum of BK_MAX_ARPEGGIO notes can be set
 *   To disable arpeggio set pointer to NULL or first element to 0
 *   BKInt values [] = {2, 3 * BK_FINT20_UNIT, 7 * BK_FINT20_UNIT};
 *
 * Effects
 *
 * Each effect requires at least one value
 * Effect can be disabled by setting a NULL pointer or setting the first value to 0
 *
 * BK_EFFECT_VOLUME_SLIDE
 *   values [0] sets number of effect ticks to slide to new volume
 *   values [1] unused
 * BK_EFFECT_PANNING_SLIDE
 *   Only takes effect if context has exactly 2 channels
 *   values [0] sets number of effect ticks to slide to new panning
 *   values [1] unused
 * BK_EFFECT_PORTAMENTO
 *   values [0] sets number of effect ticks to slide to new tone
 *   values [1] unused
 * BK_EFFECT_TREMOLO
 *   values [0] sets number of effect ticks to complete interval
 *   values [1] sets delta volume
 * BK_EFFECT_VIBRATO
 *   values [0] sets number of effect ticks to complete interval
 *   values [1] sets delta note
 *
 * All other attributes will be forwarded to the underlaying unit
 *
 * Errors:
 * BK_INVALID_ATTRIBUTE if attribute is unknown
 * BK_INVALID_VALUE if pointer is invalid for this attribute
 * BK_INVALID_NUM_CHANNELS if number of channels of sample does not match that of context
 */
extern BKInt BKTrackSetPtr (BKTrack * track, BKEnum attr, void * ptr) BK_DEPRECATED_FUNC ("Use 'BKSetPtr' instead");

/**
 * Get pointer
 *
 * BK_INSTRUMENT
 *   Sets pointer to a `BKInstrument` object
 * BK_ARPEGGIO
 *   Sets pointer to arpeggio sequence
 *   Pointer should have type `BKInt [BK_MAX_ARPEGGIO + 1]`
 *   First element contains number of arpeggio notes
 *   If no arpeggio is set first element is 0
 *
 * Effect parameters
 *
 * BK_EFFECT_VOLUME_SLIDE
 * BK_EFFECT_PANNING_SLIDE
 * BK_EFFECT_PORTAMENTO
 * BK_EFFECT_TREMOLO
 * BK_EFFECT_VIBRATO
 *
 * All other attributes will be forwarded to the underlaying unit
 *
 * Errors:
 * BK_INVALID_ATTRIBUTE if attribute is unknown
 */
extern BKInt BKTrackGetPtr (BKTrack const * track, BKEnum attr, void * outPtr) BK_DEPRECATED_FUNC ("Use 'BKGetPtr' instead");

/**
 * Enable or disable effects
 *
 * Each effect requires at least one value
 * Effect can be disabled by setting a NULL pointer or setting the first value to 0
 *
 * BK_EFFECT_VOLUME_SLIDE
 *   values [0] sets number of effect ticks to slide to new volume
 * BK_EFFECT_PANNING_SLIDE
 *   Only takes effect if context has exactly 2 channels
 *   values [0] sets number of effect ticks to slide to new panning
 * BK_EFFECT_PORTAMENTO
 *   values [0] sets number of effect ticks to slide to new tone
 * BK_EFFECT_TREMOLO
 *   values [0] sets number of effect ticks to complete interval
 *   values [1] sets delta volume
 *   values [2] sets the number of ticks to slide to the new values (optional)
 * BK_EFFECT_VIBRATO
 *   values [0] sets number of effect ticks to complete interval
 *   values [1] sets delta note
 *   values [2] sets the number of ticks to slide to the new values (optional)
 */
extern BKInt BKTrackSetEffect (BKTrack * track, BKEnum effect, void const * ptr, BKUInt size);

/**
 * Get effect parameters
 *
 * BK_EFFECT_VOLUME_SLIDE
 * BK_EFFECT_PANNING_SLIDE
 * BK_EFFECT_PORTAMENTO
 * BK_EFFECT_TREMOLO
 * BK_EFFECT_VIBRATO
 */
extern BKInt BKTrackGetEffect (BKTrack const * track, BKEnum effect, void * outValues, BKUInt size);

#endif /* ! _BK_TRACK_H_ */
