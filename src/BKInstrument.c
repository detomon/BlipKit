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

#include "BKInstrument_internal.h"

enum
{
	BK_INSTR_FLAG_STATE_LIST_LOCK = 1 << 0,
	BK_INSTR_FLAG_COPY_MASK       = 0,
};

BKInt const sequenceDefaultValue [BK_MAX_SEQUENCES] =
{
	[BK_SEQUENCE_VOLUME]     = BK_MAX_VOLUME,
	[BK_SEQUENCE_PANNING]    = 0,
	[BK_SEQUENCE_ARPEGGIO]   = 0,
	[BK_SEQUENCE_DUTY_CYCLE] = 0,
};

static void BKInstrumentStateAddToInstrument (BKInstrumentState * state, BKInstrument * instr);
static void BKInstrumentStateRemoveFromInstrument (BKInstrumentState * state);

static void BKInstrumentStateSetDefaultValues (BKInstrumentState * state)
{
	BKSequence * sequence;

	for (BKInt i = 0; i < BK_MAX_SEQUENCES; i ++) {
		sequence = state -> states [i].sequence;

		if (sequence) {
			BKSequenceStateSetValue (& state -> states [i], 0);
		}
		else {
			BKSequenceStateSetValue (& state -> states [i], sequenceDefaultValue [i]);
		}
	}
}

/**
 * Reset states in which this instrument is set
 */
static void BKInstrumentResetStates (BKInstrument * instr, BKEnum event)
{
	BKInstrumentState * state, * nextState;

	for (state = instr -> stateList; state; state = nextState) {
		if (state -> callback) {
			state -> callback (event, state -> callbackUserInfo);
		}

		nextState = state -> nextState;

		if (event == BK_INSTR_STATE_EVENT_DISPOSE)
			BKInstrumentStateSetInstrument (state, NULL);
	}
}

BKInt BKInstrumentInit (BKInstrument * instr)
{
	memset (instr, 0, sizeof (BKInstrument));

	return 0;
}

void BKInstrumentDispose (BKInstrument * instr)
{
	BKSequence * sequence;

	BKInstrumentResetStates (instr, BK_INSTR_STATE_EVENT_DISPOSE);

	for (BKInt i = 0; i < BK_MAX_SEQUENCES; i ++) {
		sequence = instr -> sequences [i];

		if (sequence)
			BKSequenceDispose (sequence);
	}

	memset (instr, 0, sizeof (BKInstrument));
}

BKInt BKInstrumentInitCopy (BKInstrument * copy, BKInstrument const * original)
{
	BKInt res = 0;
	BKSequence * sequence;

	memset (copy, 0, sizeof (sizeof (BKInstrument)));

	copy -> flags = (original -> flags & BK_INSTR_FLAG_COPY_MASK);
	copy -> numSequences = original -> numSequences;

	for (BKInt i = 0; i < original -> numSequences; i ++) {
		sequence = original -> sequences [i];

		if (sequence) {
			res = BKSequenceCopy (& copy -> sequences [i], sequence);

			if (res < 0)
				return res;
		}
	}

	return 0;
}

BKSequence const * BKInstrumentGetSequence (BKInstrument const * instr, BKEnum slot)
{
	BKSequence * sequence = NULL;

	if (slot < BK_MAX_SEQUENCES)
		sequence = instr -> sequences [slot];

	return sequence;
}

static void BKInstrumentUpdateNumSequences (BKInstrument * instr)
{
	BKInt numSequences = 0;

	for (BKInt i = 0; i < BK_MAX_SEQUENCES; i ++) {
		if (instr -> sequences [i])
			numSequences = BKMax (numSequences, i + 1);
	}

	instr -> numSequences = numSequences;
}

static void BKInstrumentStateSetSequence (BKInstrument * instr, BKEnum slot, BKSequence * sequence)
{
	instr -> sequences [slot] = sequence;

	for (BKInstrumentState * state = instr -> stateList; state; state = state -> nextState)
		state -> states [slot].sequence = sequence;
}

static BKInt BKInstrumentSetSequenceValues (BKInstrument * instr, BKSequenceFuncs const * funcs, BKEnum slot, void const * values, BKUInt length, BKInt sustainOffset, BKInt sustainLength)
{
	BKInt error = 0;
	BKSequence * sequence;

	if (slot < BK_MAX_SEQUENCES) {
		sequence = instr -> sequences [slot];

		if (sequence) {
			BKSequenceDispose (sequence);
			sequence = NULL;
		}

		if (values && length) {
			error = BKSequenceCreate (& sequence, funcs, values, length, sustainOffset, sustainLength);

			if (error != 0)
				return error;
		}

		BKInstrumentStateSetSequence (instr, slot, sequence);

		BKInstrumentUpdateNumSequences (instr);

		BKInstrumentResetStates (instr, BK_INSTR_STATE_EVENT_RESET);
	}
	else {
		error = BK_INVALID_ATTRIBUTE;
	}

	return error;
}

BKInt BKInstrumentSetSequence (BKInstrument * instr, BKEnum slot, BKInt const * values, BKUInt length, BKInt sustainOffset, BKInt sustainLength)
{
	return BKInstrumentSetSequenceValues (instr, & BKSequenceFuncsSimple, slot, values, length, sustainOffset, sustainLength);
}

BKInt BKInstrumentSetEnvelope (BKInstrument * instr, BKEnum slot, BKSequencePhase const * phases, BKUInt length, BKInt sustainOffset, BKInt sustainLength)
{
	return BKInstrumentSetSequenceValues (instr, & BKSequenceFuncsEnvelope, slot, phases, length, sustainOffset, sustainLength);
}

BKInt BKInstrumentSetEnvelopeADSR (BKInstrument * instr, BKUInt attack, BKUInt decay, BKInt sustain, BKUInt release)
{
	BKSequencePhase phases [4] = {
		{attack, BK_MAX_VOLUME},
		{decay, sustain},
		{1, sustain},
		{release, 0},
	};

	return BKInstrumentSetEnvelope (instr, BK_SEQUENCE_VOLUME, phases, 4, 2, 1);
}

/**
 * Add state to instrument state list
 */
static void BKInstrumentStateAddToInstrument (BKInstrumentState * state, BKInstrument * instr)
{
	if (state -> instrument == NULL && instr && (instr -> flags & BK_INSTR_FLAG_STATE_LIST_LOCK) == 0) {
		state -> instrument = instr;
		state -> prevState  = NULL;
		state -> nextState  = instr -> stateList;

		if (instr -> stateList)
			instr -> stateList -> prevState = state;

		instr -> stateList = state;

		BKInstrumentStateSetDefaultValues (state);
	}
}

/**
 * Remove state from instrument state list
 */
static void BKInstrumentStateRemoveFromInstrument (BKInstrumentState * state)
{
	BKInstrument * instr = state -> instrument;

	if (instr != NULL && (instr -> flags & BK_INSTR_FLAG_STATE_LIST_LOCK) == 0) {
		if (state -> prevState)
			state -> prevState -> nextState = state -> nextState;
		
		if (state -> nextState)
			state -> nextState -> prevState = state -> prevState;
		
		if (instr -> stateList == state)
			instr -> stateList = state -> nextState;

		state -> instrument = NULL;
		state -> prevState  = NULL;
		state -> nextState  = NULL;
	}
}

BKInt BKInstrumentStateInit (BKInstrumentState * state)
{
	memset (state, 0, sizeof (BKInstrumentState));

	BKInstrumentStateSetDefaultValues (state);

	return 0;
}

static void BKInstrumentUpdateState (BKInstrumentState * state)
{
	for (BKInt i = 0; i < BK_MAX_SEQUENCES; i ++)
		state -> states [i].sequence = state -> instrument -> sequences [i];
}

BKInt BKInstrumentStateSetInstrument (BKInstrumentState * state, BKInstrument * instr)
{
	BKInstrumentStateRemoveFromInstrument (state);
	BKInstrumentStateAddToInstrument (state, instr);

	if (instr) {
		BKInstrumentUpdateState (state);
		BKInstrumentStateSetDefaultValues (state);
		BKInstrumentStateSetPhase (state, BK_SEQUENCE_PHASE_ATTACK);
	}

	return 0;
}

BKInt BKInstrumentStateGetSequenceValueAtOffset (BKInstrumentState * state, BKEnum slot, BKInt offset)
{
	BKInt value = 0;
	BKSequenceState * sequenceState;

	if (slot < BK_MAX_SEQUENCES) {
		sequenceState = & state -> states [slot];

		if (sequenceState -> sequence) {
			value = state -> states [slot].value;
		}
		else {
			value = sequenceDefaultValue [slot];
		}
	}

	return value;
}

void BKInstrumentStateTick (BKInstrumentState * state, BKInt level)
{
	for (BKInt i = 0; i < state -> instrument -> numSequences; i ++) {
		BKSequenceState * sequenceState = & state -> states [i];

		// should only happen once per sequence
		if (BKSequenceStateStep (sequenceState, level) == BK_SEQUENCE_RETURN_FINISH) {
			state -> numActiveSequences --;

			if (state -> numActiveSequences == 0)
				BKInstrumentStateSetPhase (state, BK_SEQUENCE_PHASE_MUTE);
		}
	}
}

void BKInstrumentStateSetPhase (BKInstrumentState * state, BKEnum phase)
{
	BKSequence * sequence;
	BKInstrument * instr = state -> instrument;

	if (instr == NULL)
		return;

	state -> numActiveSequences = 0;
	
	switch (phase) {
		case BK_SEQUENCE_PHASE_ATTACK:
		case BK_SEQUENCE_PHASE_RELEASE: {
			for (BKInt i = 0; i < instr -> numSequences; i ++) {
				sequence = instr -> sequences [i];
				
				if (sequence) {
					//if (BKSequenceStateSetPhase (& state -> states [i], phase) == 0)
					//	state -> numActiveSequences ++;
					
					BKSequenceStateSetPhase (& state -> states [i], phase);
					state -> numActiveSequences ++;
				}
			}
			
			break;
		}
	}

#warning Set current duty cycle value from track!

	if (state -> numActiveSequences == 0)
		phase = BK_SEQUENCE_PHASE_MUTE;
		//BKInstrumentStateSetPhase (state, BK_SEQUENCE_PHASE_MUTE);

	switch (phase) {
		case BK_SEQUENCE_PHASE_ATTACK:
		case BK_SEQUENCE_PHASE_RELEASE: {
			state -> phase = phase;
			break;
		}
		case BK_SEQUENCE_PHASE_MUTE: {
			if (state -> phase != BK_SEQUENCE_PHASE_MUTE) {
				state -> phase = BK_SEQUENCE_PHASE_MUTE;
				
				if (state -> callback)
					state -> callback (BK_INSTR_STATE_EVENT_MUTE, state -> callbackUserInfo);
			}
		}
		default: {
			return;
			break;
		}
	}
}
