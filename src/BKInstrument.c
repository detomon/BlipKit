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

/**
 * Reset states which has set this instrument
 */
static void BKInstrumentResetStates (BKInstrument * instr, BKEnum event)
{
	BKInt res;
	BKInt dispose;
	BKInstrumentState * state;
	BKInstrumentState * nextState, * prevState = NULL;

	instr -> flags |= BK_INSTR_FLAG_STATE_LIST_LOCK;
	
	for (state = instr -> stateList; state; state = nextState) {
		dispose = 0;
		nextState = state -> nextState;
		state -> sequences [BK_SEQUENCE_VOLUME].value = BK_MAX_VOLUME;

		// set state again in case sequences have changed
		if (event != BK_INSTR_STATE_EVENT_DISPOSE)
			BKInstrumentStateSet (state, state -> state);
		
		if (state -> callback) {
			res = state -> callback (event, state -> callbackUserInfo);
			
			// remove from list if failed
			if (res < 0)
				dispose = 1;
		}

		if (event == BK_INSTR_STATE_EVENT_DISPOSE)
			dispose = 1;

		if (dispose)
			BKInstrumentStateRemoveFromInstrument (state);

		prevState = state;
	}

	instr -> flags &= ~BK_INSTR_FLAG_STATE_LIST_LOCK;
}

BKInt BKInstrumentInit (BKInstrument * instr)
{
	memset (instr, 0, sizeof (BKInstrument));

	return 0;
}

void BKInstrumentDispose (BKInstrument * instr)
{
	BKInstrumentSequence * sequence;

	BKInstrumentResetStates (instr, BK_INSTR_STATE_EVENT_DISPOSE);

	for (BKInt i = 0; i < BK_MAX_SEQUENCES; i ++) {
		sequence = instr -> sequences[i].sequence;

		if (sequence)
			free (sequence);
	}

	memset (instr, 0, sizeof (BKInstrument));
}

static BKInt BKEnvelopeGetMaxFracShift (BKEnvelopeValue const * phases, BKInt length)
{
	BKInt shift    = 0;
	BKInt maxValue = 0;
	
	for (BKInt i = 0; i < length; i ++)
		maxValue = BKMax (maxValue, phases [i].value);
	
	while ((1 << shift) < maxValue && shift < 30)
		shift ++;
	
	shift = 30 - shift;
	
	return shift;
}

static BKInt BKInstrumentSetValues (BKInstrument * instr, BKEnum slot, BKEnum type, void const * values, BKUInt length, BKInt sustainOffset, BKInt sustainLength)
{
	BKInt numSequences = 0;
	BKInstrumentSequence * sequence;
	BKInt itemSize = sizeof (BKInt);

	if (slot < BK_MAX_SEQUENCES) {
		sequence = instr -> sequences[slot].sequence;
		
		switch (type) {
			case BK_INSTR_SEQ_TYPE_SEQ: {
				itemSize = sizeof (BKInt);
				break;
			}
			case BK_INSTR_SEQ_TYPE_ENVELOP: {
				itemSize = sizeof (BKEnvelopeValue);
				break;
			}
		}
		
		if (values && length) {
			sequence = realloc (sequence, sizeof (BKInstrumentSequence) + length * itemSize);
			
			if (sequence) {
				memset (sequence, 0, sizeof (BKInstrumentSequence));
				
				// normalize sustain range
				sustainOffset = BKClamp (sustainOffset, 0, length);
				sustainLength = BKClamp (sustainLength, 0, length - sequence -> sustainOffset);
				
				// does not contain sustain
				if (sustainLength == 0)
					sustainOffset = length;
				
				sequence -> length        = length;
				sequence -> sustainOffset = sustainOffset;
				sequence -> sustainLength = sustainLength;
				
				if (type == BK_INSTR_SEQ_TYPE_ENVELOP)
					sequence -> fracShift = BKEnvelopeGetMaxFracShift (values, length);
				
				memcpy (sequence -> values, values, length * itemSize);
			}
			else {
				return BK_ALLOCATION_ERROR;
			}
		}
		else {
			if (sequence)
				free (sequence);
			
			type = 0;
			sequence = NULL;
		}
		
		instr -> sequences[slot].type     = type;
		instr -> sequences[slot].sequence = sequence;
		
		// get number of used sequences
		for (BKInt i = 0; i < BK_MAX_SEQUENCES; i ++) {
			if (instr -> sequences[i].sequence)
				numSequences = BKMax (numSequences, i + 1);
		}
		
		instr -> numSequences = numSequences;
		
		BKInstrumentResetStates (instr, BK_INSTR_STATE_EVENT_RESET);
	}
	else {
		return BK_INVALID_VALUE;
	}
	
	return 0;
}

BKInt BKInstrumentInitCopy (BKInstrument * copy, BKInstrument const * original)
{
	BKInt res;
	BKInstrumentSequence * sequence;

	memset (copy, 0, sizeof (sizeof (BKInstrument)));

	copy -> flags = (original -> flags & BK_INSTR_FLAG_COPY_MASK);
	copy -> numSequences = original -> numSequences;

	for (BKInt i = 0; i < original -> numSequences; i ++) {
		sequence = original -> sequences[i].sequence;

		if (sequence)
			res = BKInstrumentSetValues (copy, i, original -> sequences [i].type, sequence -> values,sequence -> length, sequence -> sustainOffset, sequence -> sustainLength);

		if (res < 0)
			return res;
	}

	return 0;
}

BKInstrumentSequence const * BKInstrumentGetSequence (BKInstrument const * instr, BKUInt slot)
{
	BKInstrumentSequence * sequence = NULL;

	if (slot < BK_MAX_SEQUENCES)
		sequence = instr -> sequences[slot].sequence;

	return sequence;
}

static BKInt BKEnvelopCheckPhases (BKEnvelopeValue const * phases, BKUInt length, BKInt sustainOffset, BKInt sustainLength)
{
	BKInt steps = 0;

	for (BKInt i = 0; i < length; i ++)
		steps += phases [i].steps;

	if (steps == 0)
		return BK_INVALID_VALUE;

	return 0;
}

BKInt BKInstrumentSetSequence (BKInstrument * instr, BKEnum sequence, BKInt const * values, BKUInt length, BKInt sustainOffset, BKInt sustainLength)
{
	return BKInstrumentSetValues (instr, sequence, BK_INSTR_SEQ_TYPE_SEQ, values, length, sustainOffset, sustainLength);
}

BKInt BKInstrumentSetEnvelope (BKInstrument * instr, BKEnum sequence, BKEnvelopeValue const * phases, BKUInt length, BKInt sustainOffset, BKInt sustainLength)
{
	BKInt check = BKEnvelopCheckPhases (phases, length, sustainOffset, sustainLength);
	
	if (check != 0)
		return check;

	return BKInstrumentSetValues (instr, sequence, BK_INSTR_SEQ_TYPE_ENVELOP, phases, length, sustainOffset, sustainLength);
}

BKInt BKInstrumentSetEnvelopeADSR (BKInstrument * instr, BKUInt attack, BKUInt decay, BKInt sustain, BKUInt release)
{
	BKEnvelopeValue phases [4] = {
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

	for (BKInt i = 0; i < BK_MAX_SEQUENCES; i ++)
		state -> sequences [i].value = sequenceDefaultValue [i];

	return 0;
}

BKInt BKInstrumentStateSetInstrument (BKInstrumentState * state, BKInstrument * instr)
{
	BKInstrumentStateRemoveFromInstrument (state);
	BKInstrumentStateAddToInstrument (state, instr);

	if (instr)
		BKInstrumentStateSet (state, BK_INSTR_STATE_ATTACK);

	return 0;
}

BKInt BKInstrumentStateGetSequenceValueAtOffset (BKInstrumentState * state, BKEnum slot, BKInt offset)
{
	BKInt value = 0;
	BKInstrument * instrument = state -> instrument;
	BKInstrumentSequence * sequence;

	if (slot < BK_MAX_SEQUENCES && instrument) {
		sequence = instrument -> sequences [slot].sequence;
	
		if (sequence) {
			switch (instrument -> sequences [slot].type) {
				case BK_INSTR_SEQ_TYPE_SEQ: {
					BKInt * phases;
					
					phases = sequence -> values;
					value  = phases [offset];
					
					break;
				}
				case BK_INSTR_SEQ_TYPE_ENVELOP: {
					BKEnvelopeValue * phases;
					
					phases = (void *) sequence -> values;
					value  = phases [offset].value;
					
					break;
				}
			}
		}
	}
	
	return value;
}

static void BKInstrumentStateSequenceTick (BKInstrumentState * state, BKInstrumentSequence * sequence, BKInstrumentSeqState * seqState)
{
	BKInt value;
	BKInt offset;
	BKInt repeatEnd;

	value  = seqState -> value;
	offset = seqState -> offset;
	
	if (sequence) {
		repeatEnd = sequence -> sustainOffset + sequence -> sustainLength;
		
		if (offset != BK_INT_MAX) {
			switch (state -> state) {
				case BK_INSTR_STATE_ATTACK: {
					if (offset >= repeatEnd) {
						if (sequence -> sustainLength) {
							offset = sequence -> sustainOffset;
						}
						// no repeat sequence
						else {
							offset = BK_INT_MAX;
						}
					}
					
					break;
				}
				case BK_INSTR_STATE_RELEASE: {
					if (offset >= sequence -> length) {
						offset = BK_INT_MAX;
						state -> numActiveSequences --;
						
						if (state -> numActiveSequences <= 0)
							BKInstrumentStateSet (state, BK_INSTR_STATE_MUTE);
					}
					
					break;
				}
			}
			
			if (offset != BK_INT_MAX) {
				value = sequence -> values [offset];
				offset ++;
			}
			
			seqState -> offset = offset;
		}
	}
	
	seqState -> value = value;
}

static BKInt BKInstrumentStateEnvelopeTick (BKInstrumentState * state, BKInstrumentSequence * sequence, BKInstrumentSeqState * seqState)
{
	BKInt value, steps;
	BKEnvelopeValue * phase;
	
	do {
		if (seqState -> step == 0) {
			if (state -> state == BK_INSTR_STATE_ATTACK) {
				if (seqState -> offset >= sequence -> sustainOffset + sequence -> sustainLength) {
					if (sequence -> sustainLength)
						seqState -> offset = sequence -> sustainOffset;
				}
			}
			
			if (seqState -> offset < sequence -> length) {
				BKEnvelopeValue * phases = (BKEnvelopeValue *) & sequence -> values;

				phase = & phases [seqState -> offset];
				steps = phase -> steps;
				
				if (steps) {
					value = (phase -> value << sequence -> fracShift);
					seqState -> delta = (value - seqState -> fullValue) / steps;
					seqState -> endValue = value;
					seqState -> step = steps;
				}
				else {
					seqState -> fullValue = (phase -> value << sequence -> fracShift);
					seqState -> step = 0;
				}
				
				seqState -> offset ++;
			}
			else if (seqState -> offset != BK_INT_MAX) {				
				state -> numActiveSequences --;
				
				if (state -> numActiveSequences <= 0)
					BKInstrumentStateSet (state, BK_INSTR_STATE_MUTE);

				seqState -> offset = BK_INT_MAX;

				return 1;
			}
			else {
				return 1;
			}
		}
		else {
			seqState -> step --;
			
			if (seqState -> step) {
				seqState -> fullValue += seqState -> delta;
			}
			else {
				seqState -> fullValue = seqState -> endValue;
			}
		}

		seqState -> value = (seqState -> fullValue >> sequence -> fracShift);
	}
	while (seqState -> step == 0);
	
	return 0;
}

void BKInstrumentStateTick (BKInstrumentState * state, BKInt level)
{
	BKInstrumentSequence * sequence;
	BKInstrumentSeqState * seqState;

	if (state -> instrument) {
		for (BKInt i = 0; i < state -> instrument -> numSequences; i ++) {
			sequence = state -> instrument -> sequences[i].sequence;
			seqState = & state -> sequences [i];

			if (sequence) {
				switch (state -> instrument -> sequences[i].type) {
					case BK_INSTR_SEQ_TYPE_SEQ: {
						if (level == 1)
							BKInstrumentStateSequenceTick (state, sequence, seqState);
						break;
					}
					case BK_INSTR_SEQ_TYPE_ENVELOP: {
						BKInstrumentStateEnvelopeTick (state, sequence, seqState);
						break;
					}
				}
			}
		}
	}
}

void BKInstrumentStateSet (BKInstrumentState * state, BKEnum type)
{
	BKInt offset;
	BKInstrumentSequence * sequence;

	if (state -> instrument == NULL)
		return;

	state -> numActiveSequences = 0;

	switch (type) {
		case BK_INSTR_STATE_ATTACK: {
			for (BKInt i = 0; i < BK_MAX_SEQUENCES; i ++) {
				sequence = state -> instrument -> sequences[i].sequence;

				state -> sequences [i].offset = 0;

				if (sequence) {
					if (i == BK_SEQUENCE_DUTY_CYCLE) {
#warning Set current duty cycle value from track!
						state -> sequences [i].value = BKInstrumentStateGetSequenceValueAtOffset (state, i, 0);
					}
					else {
						state -> sequences [i].value = BKInstrumentStateGetSequenceValueAtOffset (state, i, 0);
					}

					state -> numActiveSequences ++;
				}
				else {
					state -> sequences [i].value = sequenceDefaultValue [i];
				}

				state -> sequences [i].step = 0;
			}

			state -> state = BK_INSTR_STATE_ATTACK;
			
			break;
		}
		case BK_INSTR_STATE_RELEASE: {
			for (BKInt i = 0; i < BK_MAX_SEQUENCES; i ++) {
				sequence = state -> instrument -> sequences[i].sequence;
				
				if (sequence) {
					offset = sequence -> sustainOffset + sequence -> sustainLength;
					
					if (offset >= sequence -> length)
						offset = BK_INT_MAX;
					
					state -> sequences [i].offset = offset;
					
					if (offset != BK_INT_MAX) {
						state -> sequences [i].value = BKInstrumentStateGetSequenceValueAtOffset (state, i, offset);
						state -> numActiveSequences ++;
					}
					else {
						state -> sequences [i].value = sequenceDefaultValue [i];
						state -> sequences [i].fullValue = 0;
					}
				}
				
				state -> sequences [i].step = 0;
			}
			
			if (state -> numActiveSequences == 0) {
				BKInstrumentStateSet (state, BK_INSTR_STATE_MUTE);
			}

			state -> state = BK_INSTR_STATE_RELEASE;
			
			break;
		}
		case BK_INSTR_STATE_MUTE: {			
			for (BKInt i = 0; i < BK_MAX_SEQUENCES; i ++) {
				state -> sequences [i].offset    = BK_INT_MAX;
				state -> sequences [i].fullValue = 0;
				state -> sequences [i].value     = 0;
				state -> sequences [i].step      = 0;
			}

			if (state -> state != BK_INSTR_STATE_MUTE) {
				state -> state = BK_INSTR_STATE_MUTE;
				
				if (state -> callback)
					state -> callback (BK_INSTR_STATE_EVENT_MUTE, state -> callbackUserInfo);
			}

			break;
		}
		default: {
			return;
			break;
		}
	}
}
