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

#include "BKTrack.h"
#include "BKTone.h"

extern BKInt const sequenceDefaultValue [BK_MAX_SEQUENCES];

void BKTrackReset (BKTrack * track);

static void BKTrackUpdateUnit (BKTrack * track);
static void BKTrackSetNote (BKTrack * track, BKInt note);
static void BKTrackSetInstrument (BKTrack * track, BKInstrument * instrument);

static BKUnitFuncs const BKTrackFuncsStruct =
{
	.run     = (void *) BKUnitRun,
	.end     = (void *) BKUnitEnd,
	.reset   = (void *) BKTrackReset,
	.getAttr = (void *) BKTrackGetAttr,
	.setAttr = (void *) BKTrackSetAttr,
	.getPtr  = (void *) BKTrackGetPtr,
	.setPtr  = (void *) BKTrackSetPtr,
};

static BKInt BKTrackInstrStateCallback (BKEnum event, BKTrack * track)
{
	switch (event) {
		case BK_INSTR_STATE_EVENT_DISPOSE: {
			BKTrackSetInstrument (track, NULL);
			break;
		}
		case BK_INSTR_STATE_EVENT_RESET: {
			BKTrackUpdateUnit (track);
			break;
		}
		case BK_INSTR_STATE_EVENT_MUTE: {
			BKTrackSetNote (track, BK_NOTE_MUTE);
			break;
		}
	}

	return 0;
}

/** 
 * Decrement state counter
 * If the state counter reaches 0 the counter is reset and 1 is returned
 */
static BKInt BKDividerStateTick (BKDividerState * dividerState)
{
	BKInt tick = 0;
	
	if (dividerState -> counter == 0) {
		dividerState -> counter = dividerState -> divider;
		tick = 1;
	}

	dividerState -> counter --;
	
	return tick;
}

static void BKTrackUpdateUnitVolume (BKTrack * track)
{
	BKInt volume, volume0, volume1;
	BKInt panning;
	BKInt tremolo;
	BKInt value;

	volume = BKSlideStateGetValue (& track -> volume);

	// ignore volume except it is 0
	if ((track -> flags & BKIgnoreVolumeFlag) && volume != 0)
		volume = BK_MAX_VOLUME;

	panning = BKSlideStateGetValue (& track -> panning);

	if (track -> flags & BKInstrumentFlag) {
		value = track -> instrState.states[BK_SEQUENCE_VOLUME].value;

		if ((track -> flags & BKIgnoreVolumeFlag) == 0 || value == 0)
			volume = (volume * value) >> BK_VOLUME_SHIFT;

		panning += track -> instrState.states[BK_SEQUENCE_PANNING].value;
	}

	if (track -> flags & BKTremoloFlag) {
		if ((track -> flags & BKIgnoreVolumeFlag) == 0) {
			tremolo = BKIntervalStateGetValue (& track -> tremolo);
			// make absolute and invert interval value
			tremolo = BK_MAX_VOLUME - BKAbs (tremolo);
			volume  = (volume * tremolo) >> BK_VOLUME_SHIFT;
		}
	}

	volume = BKClamp (volume, 0, BK_MAX_VOLUME);
	volume = (volume * track -> masterVolume) >> BK_VOLUME_SHIFT;

	if (panning && (track -> flags & BKPanningEnabledFlag)) {
		panning = BKClamp (panning, -BK_MAX_VOLUME, +BK_MAX_VOLUME);
		volume0 = panning > 0 ? (((BK_MAX_VOLUME - panning) * volume) >> BK_VOLUME_SHIFT) : volume;
		volume1 = panning < 0 ? (((BK_MAX_VOLUME + panning) * volume) >> BK_VOLUME_SHIFT) : volume;

		BKUnitSetAttr (& track -> unit, BK_VOLUME_0, volume0);
		BKUnitSetAttr (& track -> unit, BK_VOLUME_1, volume1);
	}
	else {
		BKUnitSetAttr (& track -> unit, BK_VOLUME, volume);
	}
}

static void BKTrackUpdateUnitNote (BKTrack * track)
{
	BKInt note = track -> note.value;
	BKInt period;

	if (track -> unit.ctx == NULL)
		return;
	
	if (track -> flags & BKArpeggioFlag)
		note += track -> arpeggio.delta;

	if (track -> flags & BKInstrumentFlag)
		note += track -> instrState.states[BK_SEQUENCE_ARPEGGIO].value;

	if (track -> flags & BKVibratoFlag)
		note += track -> vibrato.value;

	period = BKTonePeriodLookup (note, track -> unit.ctx -> sampleRate) / track -> unit.phase.count;		

	if (track -> waveform != BK_SAMPLE) {
		BKUnitSetAttr (& track -> unit, BK_PERIOD, period);
	}
	else {
#warning Special case for samples!
		// ...
	}
}

static void BKTrackUpdateUnitDutyCycle (BKTrack * track)
{
	BKInt dutyCycle = track -> dutyCycle;

	if (track -> flags & BKInstrumentFlag) {
		if (track -> instrState.states[BK_SEQUENCE_DUTY_CYCLE].value)
			dutyCycle = track -> instrState.states[BK_SEQUENCE_DUTY_CYCLE].value;
	}

	BKUnitSetAttr (& track -> unit, BK_DUTY_CYCLE, dutyCycle);
}

static void BKTrackUpdateUnit (BKTrack * track)
{
	BKTrackUpdateUnitVolume (track);
	BKTrackUpdateUnitNote (track);
	BKTrackUpdateUnitDutyCycle (track);
}

static void BKTrackUpdateIgnoreVolume (BKTrack * track)
{
	if (track -> waveform == BK_TRIANGLE) {
		if (track -> flags & BKTriangleIgnoresVolumeFlag) {
			track -> flags |= BKIgnoreVolumeFlag;
			return;
		}
	}

	track -> flags &= ~BKIgnoreVolumeFlag;
}

static void BKTrackArpeggioTick (BKTrack * track)
{
	track -> arpeggio.delta = track -> arpeggio.notes [track -> arpeggio.offset];
	track -> arpeggio.offset ++;

	if (track -> arpeggio.offset >= track -> arpeggio.count)
		track -> arpeggio.offset = 0;
}

static void BKTrackInstrumentTick (BKTrack * track, BKInt level)
{
	BKInstrumentStateTick (& track -> instrState, level);
}

static void BKTrackEffectTick (BKTrack * track)
{
	if (track -> flags & BKPortamentoFlag) {
		BKSlideStateStep (& track -> note);
	}

	if (track -> flags & BKVolumeSlideFlag) {
		BKSlideStateStep (& track -> volume);
	}

	if (track -> flags & BKPanningSlideFlag) {
		BKSlideStateStep (& track -> panning);
	}

	if (track -> flags & BKTremoloFlag) {
		BKIntervalStateStep (& track -> tremolo);
	}

	if (track -> flags & BKVibratoFlag) {
		BKIntervalStateStep (& track -> vibrato);
	}
}

static BKEnum BKTrackTick (BKCallbackInfo * info, BKTrack * track)
{
	BKInt tick;

	if (track -> flags & BKArpeggioFlag) {
		if (BKDividerStateTick (& track -> arpeggioDivider))
			BKTrackArpeggioTick (track);
	}

	if (track -> flags & BKInstrumentFlag) {
		tick = BKDividerStateTick (& track -> instrDivider);
		BKTrackInstrumentTick (track, tick ? 1 : 0);
	}

	BKTrackUpdateUnit (track);
	
	if (track -> flags & BKEffectMask) {
		if (BKDividerStateTick (& track -> effectDivider))
			BKTrackEffectTick (track);
	}

	return 0;
}

BKInt BKTrackInit (BKTrack * track, BKEnum waveform)
{
	BKInt ret = 0;
	BKCallback callback;

	ret = BKUnitInit (& track -> unit, waveform);

	track -> flags      = BKTriangleIgnoresVolumeFlag;
	track -> unit.funcs = (BKUnitFuncs *) & BKTrackFuncsStruct;

	if (ret == 0) {
		// init waveform flags
		BKTrackSetAttr (track, BK_WAVEFORM, waveform);
		BKTrackReset (track);

		callback.func     = (BKCallbackFunc) BKTrackTick;
		callback.userInfo = track;

		BKDividerInit (& track -> divider, 1, & callback);

		BKSlideStateInit (& track -> volume, BK_MAX_VOLUME);
		BKSlideStateInit (& track -> panning, BK_MAX_VOLUME);
		BKSlideStateInit (& track -> note, 0);  // note resolution is already high enough

		BKIntervalStateInit (& track -> tremolo, BK_MAX_VOLUME);
		BKIntervalStateInit (& track -> vibrato, 0);  // note resolution is already high enough
	}

	return ret;
}

void BKTrackReset (BKTrack * track)
{
	BKEnum waveform = track -> waveform;

	BKTrackSetInstrument (track, NULL);
	
	// only clear fields after and including "arpeggioDivider" field
	memset (& track -> arpeggioDivider, 0, sizeof (BKTrack) - offsetof (BKTrack, arpeggioDivider));

	track -> flags &= (BKTriangleIgnoresVolumeFlag | BKIgnoreVolumeFlag | BKPanningEnabledFlag);

	BKUnitReset (& track -> unit);

	BKTrackSetAttr (track, BK_WAVEFORM, waveform);
	BKTrackSetAttr (track, BK_DUTY_CYCLE, BK_SQUARE_PHASES / 4);
	BKTrackSetAttr (track, BK_NOTE, BK_NOTE_MUTE);
	BKTrackSetAttr (track, BK_MUTE, 1);
	
	BKTrackSetAttr (track, BK_ARPEGGIO_DIVIDER, BK_DEFAULT_ARPEGGIO_DIVIDER);
	BKTrackSetAttr (track, BK_EFFECT_DIVIDER, BK_DEFAULT_EFFECT_DIVIDER);
	BKTrackSetAttr (track, BK_INSTRUMENT_DIVIDER, BK_DEFAULT_INSTR_DIVIDER);

	track -> instrState.callback         = (void *) BKTrackInstrStateCallback;
	track -> instrState.callbackUserInfo = track;
}

void BKTrackDispose (BKTrack * track)
{
	BKInstrumentStateSetInstrument (& track -> instrState, NULL);

	BKTrackDetach (track);
	BKUnitDispose (& track -> unit);
	memset (track, 0, sizeof (BKTrack));
}

BKInt BKTrackAttach (BKTrack * track, BKContext * ctx)
{
	BKInt ret;

	ret = BKUnitAttach (& track -> unit, ctx);

	if (ret == 0) {
		BKContextSetAttr (ctx, BK_CLOCK_TYPE_EFFECT, 1);

		BKContextAttachDivider (ctx, & track -> divider, BK_CLOCK_TYPE_EFFECT);

		if (ctx -> numChannels == 2)
			track -> flags |= BKPanningEnabledFlag;

		BKTrackUpdateUnit (track);
	}

	return ret;
}

void BKTrackDetach (BKTrack * track)
{
	BKDividerDetach (& track -> divider);
	BKUnitDetach (& track -> unit);

	track -> flags &= ~BKPanningEnabledFlag;
}

static void BKTrackSetVolume (BKTrack * track, BKInt volume)
{
	volume = BKClamp (volume, 0, BK_MAX_VOLUME);

	BKSlideStateSetValue (& track -> volume, volume);

	BKTrackUpdateUnitVolume (track);
}

static void BKTrackSetPanning (BKTrack * track, BKInt panning)
{
	panning = BKClamp (panning, -BK_MAX_VOLUME, +BK_MAX_VOLUME);

	BKSlideStateSetValue (& track -> panning, panning);

	BKTrackUpdateUnitVolume (track);
}

static void BKTrackSetInstrumentInitValues (BKTrack * track)
{
	if (track -> flags & BKInstrumentFlag) {
		if (track -> instrState.instrument -> sequences [BK_SEQUENCE_DUTY_CYCLE])
			BKSequenceStateSetValue (& track -> instrState.states [BK_SEQUENCE_DUTY_CYCLE], track -> dutyCycle);
	}
}

static void BKTrackInstrumentAttack (BKTrack * track)
{
	BKTrackSetInstrumentInitValues (track);

	track -> instrDivider.counter = 0;
	BKInstrumentStateSetPhase (& track -> instrState, BK_SEQUENCE_PHASE_ATTACK);
}

static void BKTrackInstrumentRelease (BKTrack * track)
{
	BKInstrumentStateSetPhase (& track -> instrState, BK_SEQUENCE_PHASE_RELEASE);
}

static void BKTrackInstrumentMute (BKTrack * track)
{
	BKInstrumentStateSetPhase (& track -> instrState, BK_SEQUENCE_PHASE_MUTE);
}

static void BKTrackSetNote (BKTrack * track, BKInt note)
{
	if (note >= 0) {
		note = BKClamp (note, BK_MIN_NOTE << BK_FINT20_SHIFT, BK_MAX_NOTE << BK_FINT20_SHIFT);

		BKSlideStateSetValue (& track -> note, note);
		
		if (track -> curNote == -1) {
			// halt portamento
			BKSlideStateHalt (& track -> note, 1);

			if (track -> flags & BKInstrumentFlag)
				BKTrackInstrumentAttack (track);
		}

		track -> curNote         = note;
		track -> arpeggio.offset = 0;

		BKTrackUpdateUnit (track);  // instrument may update other values
		
		BKUnitSetAttr (& track -> unit, BK_MUTE, 0);
	}
	else if (note == BK_NOTE_RELEASE) {
		track -> curNote = -1;
 
		if (track -> flags & BKInstrumentFlag) {
			BKTrackInstrumentRelease (track);
			BKTrackUpdateUnit (track);  // instrument may update other values
		}
		else {
			// halt portamento
			BKSlideStateHalt (& track -> note, 0);

			BKUnitSetAttr (& track -> unit, BK_MUTE, 1);
		}
	}
	else if (note == BK_NOTE_MUTE) {
		track -> curNote = -1;

		// halt portamento
		BKSlideStateHalt (& track -> note, 1);

		if (track -> flags & BKInstrumentFlag)
			BKTrackInstrumentMute (track);
			
		BKUnitSetAttr (& track -> unit, BK_MUTE, 1);
	}
}

static void BKTrackSetInstrument (BKTrack * track, BKInstrument * instrument)
{
	BKInstrumentStateSetInstrument (& track -> instrState, instrument);
	
	if (instrument) {
		track -> flags |= BKInstrumentFlag;
		track -> instrDivider.counter = 0;

		BKTrackSetInstrumentInitValues (track);

		BKTrackInstrumentMute (track);
		BKTrackUpdateUnit (track);
	}
	else {
		track -> flags &= ~BKInstrumentFlag;
		
		if (track -> curNote == -1)
			BKTrackSetNote (track, BK_NOTE_MUTE);
	
	}
}

BKInt BKTrackSetAttr (BKTrack * track, BKEnum attr, BKInt value)
{
	BKInt ret = 0;
	BKInt values [2];

	switch (attr) {
		case BK_MASTER_VOLUME: {
			value = BKClamp (value, 0, BK_MAX_VOLUME);
			track -> masterVolume = value;
			BKTrackUpdateUnitVolume (track);
			break;
		}
		case BK_VOLUME: {
			BKTrackSetVolume (track, value);
			break;
		}
		case BK_PANNING: {
			BKTrackSetPanning (track, value);
			break;
		}
		case BK_WAVEFORM: {
			ret = BKUnitSetAttr (& track -> unit, BK_WAVEFORM, value);

			if (ret == 0) {
				BKUnitGetAttr (& track -> unit, BK_WAVEFORM, & track -> waveform);
				BKTrackUpdateIgnoreVolume (track);
			}

			break;
		}
		case BK_DUTY_CYCLE: {
			ret = BKUnitSetAttr (& track -> unit, BK_DUTY_CYCLE, value);

			if (ret == 0)
				BKUnitGetAttr (& track -> unit, BK_DUTY_CYCLE, & track -> dutyCycle);

			break;
		}
		case BK_NOTE: {
			BKTrackSetNote (track, value);
			break;
		}
		case BK_ARPEGGIO_DIVIDER: {
			track -> arpeggioDivider.divider = value;
			track -> arpeggioDivider.counter = 0;
			break;
		}
		case BK_EFFECT_DIVIDER: {
			track -> effectDivider.divider = value;
			track -> effectDivider.counter = 0;
			break;
		}
		case BK_INSTRUMENT_DIVIDER: {
			track -> instrDivider.divider = value;
			track -> instrDivider.counter = 0;
			break;
		}
		case BK_TRIANGLE_IGNORES_VOLUME: {
			if (value) {
				track -> flags |= BKTriangleIgnoresVolumeFlag;

				if (track -> waveform == BK_TRIANGLE)
					track -> flags |= BKIgnoreVolumeFlag;
			}
			else {
				track -> flags &= ~(BKTriangleIgnoresVolumeFlag | BKIgnoreVolumeFlag);
			}

			break;
		}
		case BK_EFFECT_VOLUME_SLIDE:
		case BK_EFFECT_PANNING_SLIDE:
		case BK_EFFECT_PORTAMENTO: {
			values [0] = value;
			values [1] = 0;

			return BKTrackSetPtr (track, attr, values);
			break;
		}
		default: {
			return BKUnitSetAttr (& track -> unit, attr, value);
			break;
		}
	}

	return ret;
}

BKInt BKTrackGetAttr (BKTrack const * track, BKEnum attr, BKInt * outValue)
{
	BKInt ret   = 0;
	BKInt value = 0;
	BKInt values [2];

	switch (attr) {
		case BK_MASTER_VOLUME: {
			value = track -> masterVolume;
			break;
		}
		case BK_VOLUME: {
			BKSlideStateGetValue (& track -> volume);
			break;
		}
		case BK_PANNING: {
			BKSlideStateGetValue (& track -> panning);
			break;
		}
		case BK_NOTE: {
			value = track -> curNote;
			break;
		}
		case BK_ARPEGGIO_DIVIDER: {
			value = track -> arpeggioDivider.divider;
			break;
		}
		case BK_EFFECT_DIVIDER: {
			value = track -> effectDivider.divider;
			break;
		}
		case BK_INSTRUMENT_DIVIDER: {
			value = track -> instrDivider.divider;
			break;
		}
		case BK_TRIANGLE_IGNORES_VOLUME: {
			value = (track -> flags & BKTriangleIgnoresVolumeFlag) != 0;
			break;
		}
		case BK_EFFECT_VOLUME_SLIDE:
		case BK_EFFECT_PANNING_SLIDE:
		case BK_EFFECT_PORTAMENTO: {
			ret = BKTrackGetPtr (track, attr, values);
			
			if (ret != 0)
				return ret;

			* outValue = values [0];
			break;
		}
		default: {
			return BKUnitGetAttr (& track -> unit, attr, outValue);
			break;
		}
	}
	
	* outValue = value;

	return 0;
}

static BKInt BKTrackSetEffect (BKTrack * track, BKEnum effect, BKInt const values [2])
{
	static BKInt const emptyValues [2] = {0, 0};

	BKInt flag;
	
	if (values == NULL)
		values = emptyValues;
	
	switch (effect) {
		case BK_EFFECT_VOLUME_SLIDE: {
			BKSlideStateSetSteps (& track -> volume, values [0]);
			BKTrackUpdateUnitVolume (track);
			break;
		}
		case BK_EFFECT_PORTAMENTO: {
			BKSlideStateSetSteps (& track -> note, values [0]);
			BKTrackUpdateUnitNote (track);
			break;
		}
		case BK_EFFECT_PANNING_SLIDE: {
			BKSlideStateSetSteps (& track -> panning, values [0]);
			BKTrackUpdateUnitVolume (track);
			break;
		}
		case BK_EFFECT_TREMOLO: {
			BKIntervalStateSetDeltaAndSteps (& track -> tremolo, values [1], values [0]);
			BKTrackUpdateUnitVolume (track);
			break;
		}
		case BK_EFFECT_VIBRATO: {
			BKIntervalStateSetDeltaAndSteps (& track -> vibrato, values [1], values [0]);
			BKTrackUpdateUnitNote (track);
			break;
		}
		default: {
			return BK_INVALID_ATTRIBUTE;
			break;
		}
	}

	flag = (1 << (effect - BK_EFFECT_TYPE - 1 + BK_EFFECT_FLAG_SHIFT));

	if (values [0]) {
		track -> flags |= flag;
	}
	else {
		track -> flags &= ~flag;
	}

	return 0;
}

static BKInt BKTrackGetEffect (BKTrack const * track, BKEnum effect, BKInt values [2])
{
	switch (effect) {
		case BK_EFFECT_VOLUME_SLIDE: {
			values [0] = track -> volume.steps;
			values [1] = track -> volume.step;
			break;
		}
		case BK_EFFECT_PORTAMENTO: {
			values [0] = track -> note.steps;
			values [1] = track -> note.step;
			break;
		}
		case BK_EFFECT_PANNING_SLIDE: {
			values [0] = track -> panning.steps;
			values [1] = track -> panning.step;
			break;
		}
		case BK_EFFECT_TREMOLO: {
			values [0] = track -> tremolo.steps;
			values [1] = track -> tremolo.delta;
			break;
		}
		case BK_EFFECT_VIBRATO: {
			values [0] = track -> vibrato.steps;
			values [1] = track -> vibrato.delta;
			break;
		}
		default: {
			memset (values, 0, sizeof (BKInt [2]));
			return BK_INVALID_ATTRIBUTE;
			break;
		}
	}
	
	return 0;
}

BKInt BKTrackSetPtr (BKTrack * track, BKEnum attr, void * ptr)
{
	BKInt res;

	switch (attr & BK_ATTR_TYPE_MASK) {
		case BK_EFFECT_TYPE: {
			return BKTrackSetEffect (track, attr, ptr);
			break;
		}
		default: {
			switch (attr) {
				case BK_WAVEFORM:
				case BK_SAMPLE: {
					res = BKUnitSetPtr (& track -> unit, attr, ptr);
					
					if (res != 0)
						return res;
					
					BKTrackUpdateIgnoreVolume (track);
					
					break;
				}
				case BK_INSTRUMENT: {
					BKTrackSetInstrument (track, ptr);
					break;
				}
				case BK_ARPEGGIO: {
					BKInt * arpeggio = ptr;
					BKUInt  count;
					
					track -> arpeggio.delta  = 0;
					track -> arpeggio.offset = 0;

					if (arpeggio && arpeggio [0] > 0) {
						count = BKMin (arpeggio [0], BK_MAX_ARPEGGIO);
						track -> arpeggio.count = count;
						memcpy (track -> arpeggio.notes, & arpeggio [1], count * sizeof (BKInt));
						track -> flags |= BKArpeggioFlag;
					}
					else {
						track -> arpeggio.count = 0;
						track -> flags &= ~BKArpeggioFlag;
					}
					
					break;
				}
				default: {
					return BKUnitSetPtr (& track -> unit, attr, ptr);
					break;
				}
			}

			break;
		}
	}
	
	return 0;
}

BKInt BKTrackGetPtr (BKTrack const * track, BKEnum attr, void * outPtr)
{
	void ** ptrRef = outPtr;

	switch (attr & BK_ATTR_TYPE_MASK) {
		case BK_EFFECT_TYPE: {
			return BKTrackGetEffect (track, attr, outPtr);
			break;
		}
		default: {
			switch (attr) {
				case BK_INSTRUMENT: {
					* ptrRef = track -> instrState.instrument;
					break;
				}
				case BK_ARPEGGIO: {
					BKInt * arpeggio = outPtr;
					
					arpeggio [0] = track -> arpeggio.count;
					memcpy (& arpeggio [1], track -> arpeggio.notes, sizeof (BKInt) * track -> arpeggio.count);
					
					break;
				}
				default: {
					return BKUnitGetPtr (& track -> unit, attr, outPtr);
					break;
				}
			}

			break;
		}
	}
		
	return 0;
}
