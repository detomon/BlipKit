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

#define BK_TRACK_EFFECT_MAX_STEPS (1 << 16)

extern BKInt const sequenceDefaultValue [BK_MAX_SEQUENCES];

void BKTrackReset (BKTrack * track);

static void BKTrackUpdateUnit (BKTrack * track);
static BKInt BKTrackRun (BKTrack * track, BKFUInt20 endTime);
static void BKTrackSetNote (BKTrack * track, BKInt note);
static void BKTrackSetInstrument (BKTrack * track, BKInstrument * instrument);
static void BKTrackInstrumentUpdateFlags (BKTrack * track);

static BKUnitFuncs const BKTrackFuncsStruct =
{
	.run     = (void *) BKTrackRun,
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
			BKTrackInstrumentUpdateFlags (track);
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
	BKInt note;
	BKInt period;

	if (track -> unit.ctx == NULL)
		return;

	note = BKSlideStateGetValue (& track -> note);

	if (track -> flags & BKArpeggioFlag)
		note += track -> arpeggio.delta;

	if (track -> flags & BKInstrumentFlag)
		note += track -> instrState.states[BK_SEQUENCE_ARPEGGIO].value;

	if (track -> flags & BKVibratoFlag)
		note += BKIntervalStateGetValue (& track -> vibrato);

	note += track -> pitch;

	if (track -> waveform != BK_SAMPLE) {
		period = BKTonePeriodLookup (note, track -> unit.ctx -> sampleRate) / track -> unit.phase.count;
		BKUnitSetAttr (& track -> unit, BK_PERIOD, period);
	}
	else {
		period = BKLog2PeriodLookup (note + track -> samplePitch);
		BKUnitSetAttr (& track -> unit, BK_SAMPLE_PERIOD, period);
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
	if (track -> flags & BKTrackAttrUpdateFlagVolume) {
		BKTrackUpdateUnitVolume (track);
		track -> flags &= ~BKTrackAttrUpdateFlagVolume;
	}

	if (track -> flags & BKTrackAttrUpdateFlagNote) {
		BKTrackUpdateUnitNote (track);
		track -> flags &= ~BKTrackAttrUpdateFlagNote;
	}

	if (track -> flags & BKTrackAttrUpdateFlagDutyCycle) {
		BKTrackUpdateUnitDutyCycle (track);
		track -> flags &= ~BKTrackAttrUpdateFlagDutyCycle;
	}
}

static void BKTrackUpdateIgnoreVolume (BKTrack * track)
{
	track -> flags |= BKTrackAttrUpdateFlagVolume;

	if (track -> waveform == BK_TRIANGLE) {
		if (track -> flags & BKTriangleIgnoresVolumeFlag) {
			track -> flags |= BKIgnoreVolumeFlag;
			return;
		}
	}

	track -> flags &= ~BKIgnoreVolumeFlag;
}

static void BKTrackArpeggioSetNotes (BKTrack * track, BKInt * arpeggio, BKInt count)
{
	track -> arpeggio.count = count;

	track -> arpeggio.delta  = 0;
	track -> arpeggio.offset = 0;

	if (count > 0) {
		memcpy (track -> arpeggio.notes, arpeggio, count * sizeof (BKInt));
		track -> flags |= BKArpeggioFlag;
	}
	else {
		track -> flags &= ~BKArpeggioFlag;
	}

	track -> flags |= BKTrackAttrUpdateFlagNote;
}

static void BKTrackArpeggioTick (BKTrack * track)
{
	track -> arpeggio.delta = track -> arpeggio.notes [track -> arpeggio.offset];
	track -> arpeggio.offset ++;

	if (track -> arpeggio.offset >= track -> arpeggio.count)
		track -> arpeggio.offset = 0;

	track -> flags |= BKTrackAttrUpdateFlagNote;
}

static void BKTrackInstrumentTick (BKTrack * track, BKInt level)
{
	BKInstrumentStateTick (& track -> instrState, level);
	BKTrackInstrumentUpdateFlags (track);
}

static void BKTrackInstrumentUpdateFlags (BKTrack * track)
{
	if (track -> instrState.states[BK_SEQUENCE_VOLUME].sequence)
		track -> flags |= BKTrackAttrUpdateFlagVolume;

	if (track -> instrState.states[BK_SEQUENCE_PANNING].sequence)
		track -> flags |= BKTrackAttrUpdateFlagVolume;

	if (track -> instrState.states[BK_SEQUENCE_ARPEGGIO].sequence)
		track -> flags |= BKTrackAttrUpdateFlagNote;

	if (track -> instrState.states[BK_SEQUENCE_DUTY_CYCLE].sequence)
		track -> flags |= BKTrackAttrUpdateFlagDutyCycle;
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
		if (track -> tremoloDelta.steps) {
			BKSlideStateStep (& track -> tremoloDelta);
			BKSlideStateStep (& track -> tremoloSteps);

			BKIntervalStateSetDeltaAndSteps (
				& track -> tremolo,
				BKSlideStateGetValue (& track -> tremoloDelta),
				BKSlideStateGetValue (& track -> tremoloSteps)
			);
		}

		BKIntervalStateStep (& track -> tremolo);
	}

	if (track -> flags & BKVibratoFlag) {
		if (track -> vibratoDelta.steps) {
			BKSlideStateStep (& track -> vibratoDelta);
			BKSlideStateStep (& track -> vibratoSteps);

			BKIntervalStateSetDeltaAndSteps (
				& track -> vibrato,
				BKSlideStateGetValue (& track -> vibratoDelta),
				BKSlideStateGetValue (& track -> vibratoSteps)
			);
		}

		BKIntervalStateStep (& track -> vibrato);
	}
}

static void BKTrackEffectUpdateFlags (BKTrack * track)
{
	if (track -> flags & BKPortamentoFlag) {
		track -> flags |= BKTrackAttrUpdateFlagNote;
	}

	if (track -> flags & BKVolumeSlideFlag) {
		track -> flags |= BKTrackAttrUpdateFlagVolume;
	}

	if (track -> flags & BKPanningSlideFlag) {
		track -> flags |= BKTrackAttrUpdateFlagVolume;
	}

	if (track -> flags & BKTremoloFlag) {
		track -> flags |= BKTrackAttrUpdateFlagVolume;
	}

	if (track -> flags & BKVibratoFlag) {
		track -> flags |= BKTrackAttrUpdateFlagNote;
	}
}

static BKEnum BKTrackTick (BKCallbackInfo * info, BKTrack * track)
{
	BKInt tick;

	// 1. Update effect flags

	BKTrackEffectUpdateFlags (track);

	// 2. Tick arpeggio and instrument

	if (track -> flags & BKArpeggioFlag) {
		if (BKDividerStateTick (& track -> arpeggioDivider)) {
			BKTrackArpeggioTick (track);
		}
	}

	if (track -> flags & BKInstrumentFlag) {
		tick = BKDividerStateTick (& track -> instrDivider);
		BKTrackInstrumentTick (track, tick ? 1 : 0);
	}

	// 3. Update unit

	BKTrackUpdateUnit (track);

	// 4. Tick effects

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
		BKSlideStateInit (& track -> tremoloDelta, BK_MAX_VOLUME);
		BKSlideStateInit (& track -> tremoloSteps, BK_TRACK_EFFECT_MAX_STEPS);

		BKIntervalStateInit (& track -> vibrato, 0);  // note resolution is already high enough
		BKSlideStateInit (& track -> vibratoDelta, 0);
		BKSlideStateInit (& track -> vibratoSteps, BK_TRACK_EFFECT_MAX_STEPS);
	}

	return ret;
}

static BKInt BKTrackRun (BKTrack * track, BKFUInt20 endTime)
{
	BKTrackUpdateUnit (track);

	return BKUnitRun (& track -> unit, endTime);
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
			track -> flags |= (BKPanningEnabledFlag);

		track -> flags |= BKTrackAttrUpdateFlagVolume;
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

	track -> flags |= BKTrackAttrUpdateFlagVolume;
}

static void BKTrackSetPanning (BKTrack * track, BKInt panning)
{
	panning = BKClamp (panning, -BK_MAX_VOLUME, +BK_MAX_VOLUME);

	BKSlideStateSetValue (& track -> panning, panning);

	track -> flags |= BKTrackAttrUpdateFlagVolume;
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

	BKTrackInstrumentUpdateFlags (track);
}

static void BKTrackInstrumentRelease (BKTrack * track)
{
	BKInstrumentStateSetPhase (& track -> instrState, BK_SEQUENCE_PHASE_RELEASE);
	BKTrackInstrumentUpdateFlags (track);
}

static void BKTrackInstrumentMute (BKTrack * track)
{
	BKInstrumentStateSetPhase (& track -> instrState, BK_SEQUENCE_PHASE_MUTE);
	BKTrackInstrumentUpdateFlags (track);
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

		if (track -> waveform == BK_SAMPLE) {
			if (track -> curNote == -1 || track -> unit.sample.repeat == 0)
				BKUnitSetAttr (& track -> unit, BK_PHASE, 0);
		}

		track -> curNote         = note;
		track -> arpeggio.offset = 0;

		BKUnitSetAttr (& track -> unit, BK_MUTE, 0);
	}
	else if (note == BK_NOTE_RELEASE) {
		track -> curNote = -1;

		if (track -> flags & BKInstrumentFlag) {
			BKTrackInstrumentRelease (track);
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

	track -> flags |= BKTrackAttrUpdateFlagNote;
}

static void BKTrackSetInstrument (BKTrack * track, BKInstrument * instrument)
{
	if (instrument != track -> instrState.instrument) {
		BKInstrumentStateSetInstrument (& track -> instrState, instrument);

		if (instrument) {
			track -> flags |= BKInstrumentFlag;
			track -> instrDivider.counter = 0;

			BKTrackSetInstrumentInitValues (track);

			if (track -> curNote == -1) {
				BKTrackInstrumentMute (track);
			}
			else {
				BKTrackInstrumentAttack (track);
			}
		}
		else {
			track -> flags &= ~BKInstrumentFlag;

			if (track -> curNote == -1)
				BKTrackSetNote (track, BK_NOTE_MUTE);
		}

		BKTrackInstrumentUpdateFlags (track);
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
			track -> flags |= BKTrackAttrUpdateFlagVolume;
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

			track -> flags |= BKTrackAttrUpdateFlagDutyCycle;

			break;
		}
		case BK_NOTE: {
			BKTrackSetNote (track, value);
			break;
		}
		case BK_PITCH: {
			track -> pitch = BKClamp (value, BK_MIN_SAMPLE_TONE << BK_FINT20_SHIFT, BK_MAX_SAMPLE_TONE << BK_FINT20_SHIFT);
			break;
		}
		case BK_SAMPLE_PITCH: {
			track -> samplePitch = BKClamp (value, BK_MIN_SAMPLE_TONE << BK_FINT20_SHIFT, BK_MAX_SAMPLE_TONE << BK_FINT20_SHIFT);
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
			}
			else {
				track -> flags &= ~BKTriangleIgnoresVolumeFlag;
			}

			BKTrackUpdateIgnoreVolume (track);

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
			value = (track -> volume.endValue >> track -> volume.valueShift);
			break;
		}
		case BK_PANNING: {
			value = (track -> panning.endValue >> track -> panning.valueShift);
			break;
		}
		case BK_NOTE: {
			value = track -> curNote;
			break;
		}
		case BK_SAMPLE_PITCH: {
			value = track -> samplePitch;
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

BKInt BKTrackSetEffect (BKTrack * track, BKEnum effect, void const * inValues, BKUInt size)
{
	BKInt flag;
	BKInt steps, delta, slideSteps;
	BKInt values [3];

	if (inValues) {
		size = BKMin (size, sizeof (values));
		memcpy (values, inValues, size);
	}
	else {
		size = sizeof (values);
		memset (values, 0, size);
	}

	memset ((void *) values + size, 0, sizeof (values) - size);

	switch (effect) {
		case BK_EFFECT_VOLUME_SLIDE: {
			steps = BKClamp (values [0], 0, BK_TRACK_EFFECT_MAX_STEPS);

			BKSlideStateSetSteps (& track -> volume, steps);

			track -> flags |= BKTrackAttrUpdateFlagVolume;
			break;
		}
		case BK_EFFECT_PORTAMENTO: {
			steps = BKClamp (values [0], 0, BK_TRACK_EFFECT_MAX_STEPS);

			BKSlideStateSetSteps (& track -> note, steps);

			track -> flags |= BKTrackAttrUpdateFlagNote;
			break;
		}
		case BK_EFFECT_PANNING_SLIDE: {
			steps = BKClamp (values [0], 0, BK_TRACK_EFFECT_MAX_STEPS);

			BKSlideStateSetSteps (& track -> panning, steps);

			track -> flags |= BKTrackAttrUpdateFlagVolume;
			break;
		}
		case BK_EFFECT_TREMOLO: {
			steps      = BKClamp (values [0], 0, BK_TRACK_EFFECT_MAX_STEPS);
			delta      = BKClamp (values [1], 0, BK_MAX_VOLUME);
			slideSteps = BKClamp (values [2], 0, BK_TRACK_EFFECT_MAX_STEPS);

			BKSlideStateSetValueAndSteps (& track -> tremoloDelta, delta, slideSteps);
			BKSlideStateSetValueAndSteps (& track -> tremoloSteps, steps, slideSteps);

			if (slideSteps == 0)
				BKIntervalStateSetDeltaAndSteps (& track -> tremolo, delta, steps);

			track -> flags |= BKTrackAttrUpdateFlagVolume;
			break;
		}
		case BK_EFFECT_VIBRATO: {
			steps      = BKClamp (values [0], 0, BK_TRACK_EFFECT_MAX_STEPS);
			delta      = values [1];
			slideSteps = BKClamp (values [2], 0, BK_TRACK_EFFECT_MAX_STEPS);

			BKSlideStateSetValueAndSteps (& track -> vibratoDelta, delta, slideSteps);
			BKSlideStateSetValueAndSteps (& track -> vibratoSteps, steps, slideSteps);

			if (slideSteps == 0)
				BKIntervalStateSetDeltaAndSteps (& track -> vibrato, delta, steps);

			track -> flags |= BKTrackAttrUpdateFlagNote;
			break;
		}
		default: {
			return BK_INVALID_ATTRIBUTE;
			break;
		}
	}

	flag = (1 << (effect + BK_EFFECT_FLAG_SHIFT));

	if (values [0]) {
		track -> flags |= flag;
	}
	else {
		track -> flags &= ~flag;
	}

	return 0;
}

BKInt BKTrackGetEffect (BKTrack const * track, BKEnum effect, void * outValues, BKUInt size)
{
	BKInt outSize;
	BKInt values [3];

	memset (values, 0, sizeof (values));

	switch (effect) {
		case BK_EFFECT_VOLUME_SLIDE: {
			values [0] = track -> volume.steps;
			break;
		}
		case BK_EFFECT_PORTAMENTO: {
			values [0] = track -> note.steps;
			break;
		}
		case BK_EFFECT_PANNING_SLIDE: {
			values [0] = track -> panning.steps;
			break;
		}
		case BK_EFFECT_TREMOLO: {
			values [0] = track -> tremoloSteps.endValue;
			values [1] = track -> tremoloDelta.endValue;
			values [2] = track -> tremoloDelta.steps;
			break;
		}
		case BK_EFFECT_VIBRATO: {
			values [0] = track -> vibratoSteps.endValue;
			values [1] = track -> vibratoDelta.endValue;
			values [2] = track -> vibratoDelta.steps;
			break;
		}
		default: {
			return BK_INVALID_ATTRIBUTE;
			break;
		}
	}

	outSize = BKMin (size, sizeof (values));
	// copy values
	memcpy (outValues, values, outSize);
	// empty trailing data
	memset ((void *) outValues + outSize, 0, BKMax (0, (BKInt) size - outSize));

	return 0;
}

BKInt BKTrackSetPtr (BKTrack * track, BKEnum attr, void * ptr)
{
	BKInt res;

	switch (attr & BK_ATTR_TYPE_MASK) {
		case BK_EFFECT_TYPE: {
			BKInt effectValues [3];

			memset (effectValues, 0, sizeof (effectValues));

			if (ptr)
				memcpy (effectValues, ptr, sizeof (BKInt [2]));

			return BKTrackSetEffect (track, attr, effectValues, sizeof (BKInt [2]));
			break;
		}
		default: {
			switch (attr) {
				case BK_WAVEFORM:
				case BK_SAMPLE: {
					res = BKUnitSetPtr (& track -> unit, attr, ptr);

					if (res != 0)
						return res;

					BKUnitGetAttr (& track -> unit, BK_WAVEFORM, & track -> waveform);
					BKTrackUpdateIgnoreVolume (track);

					if (track -> unit.sample.dataState.data) {
						track -> samplePitch = track -> unit.sample.dataState.data -> samplePitch;
					}
					else {
						track -> samplePitch = 0;
					}

					break;
				}
				case BK_INSTRUMENT: {
					BKTrackSetInstrument (track, ptr);
					break;
				}
				case BK_ARPEGGIO: {
					BKInt * arpeggio = ptr;
					BKUInt  count    = 0;

					if (arpeggio)
						count = BKMin (arpeggio [0], BK_MAX_ARPEGGIO);

					BKTrackArpeggioSetNotes (track, & arpeggio [1], count);

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
	BKInt res = 0;
	void ** ptrRef = outPtr;

	switch (attr & BK_ATTR_TYPE_MASK) {
		case BK_EFFECT_TYPE: {
			BKInt effectValues [3];

			res = BKTrackGetEffect (track, attr, effectValues, sizeof (BKInt [2]));

			if (res != 0)
				return res;

			memcpy (outPtr, effectValues, sizeof (BKInt [2]));
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
