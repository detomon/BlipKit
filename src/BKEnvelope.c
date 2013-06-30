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

#include "BKEnvelope.h"

static void BKEnvelopeAddState (BKEnvelope * envelope, BKEnvelopeState * state)
{
	if (state -> envelope == NULL) {
		state -> envelope   = envelope;
		state -> prevState = NULL;
		state -> nextState = envelope -> states;
		
		if (envelope -> states)
			envelope -> states -> prevState = state;
		
		envelope -> states = state;
	}
}

static void BKEnvelopeStateRemove (BKEnvelopeState * state)
{
	BKEnvelope * envelope = state -> envelope;
	
	if (envelope) {
		if (state -> prevState)
			state -> prevState -> nextState = state -> nextState;
		
		if (state -> nextState)
			state -> nextState -> prevState = state -> prevState;
		
		if (envelope -> states == state)
			envelope -> states = state -> nextState;
		
		state -> envelope   = NULL;
		state -> prevState = NULL;
		state -> nextState = NULL;
	}
}

static BKInt BKEnvelopeGetMaxFracShift (BKEnvelopeValue const * phases, BKInt length)
{
	BKInt shift    = 0;
	BKInt maxValue = 0;

	for (BKInt i = 0; i < length; i ++)
		maxValue = BKMax (maxValue, phases [i].value);

	while ((1 << shift) < maxValue && shift < 31)
		shift ++;

	shift = 31 - shift;

	return shift;
}

BKInt BKEnvelopeInit (BKEnvelope * envelope, BKEnvelopeValue const * phases, BKInt length, BKInt sustainOffset, BKInt sustainLength)
{
	BKEnvelopeValue * copyPhases;

	if (length <= 0)
		return BK_INVALID_VALUE;
	
	copyPhases = malloc (length * sizeof (BKEnvelopeValue));

	if (copyPhases == NULL)
		return BK_ALLOCATION_ERROR;

	if (phases) {
		memset (envelope, 0, sizeof (BKEnvelope));
		
		envelope -> sustainOffset = BKClamp (sustainOffset, 0, length);
		envelope -> sustainLength = BKClamp (sustainLength, 0, length - envelope -> sustainOffset);
		envelope -> fracShift     = BKEnvelopeGetMaxFracShift (phases, length);
		envelope -> length        = length;
		envelope -> phases        = copyPhases;

		for (BKInt i = 0; i < length; i ++) {
			envelope -> phases [i].steps = phases [i].steps;
			envelope -> phases [i].value = phases [i].value;
		}
	}

	return 0;
}

BKInt BKEnvelopeInitADSR (BKEnvelope * envelope, BKInt attack, BKInt attackValue, BKInt decay, BKInt sustain, BKInt release)
{
	BKEnvelopeValue phases [4] = {
		{attack, attackValue},
		{decay, sustain},
		{1, sustain},
		{release, 0},
	};

	return BKEnvelopeInit (envelope, phases, 4, 2, 1);
}

void BKEnvelopeDispose (BKEnvelope * envelope)
{
	BKEnvelopeState * nextState;
	
	for (BKEnvelopeState * state = envelope -> states; state; state = nextState) {
		nextState = state -> nextState;
		BKEnvelopeStateRemove (state);
	}

	if (envelope -> phases)
		free (envelope -> phases);
}

BKInt BKEnvelopeStateInit (BKEnvelopeState * state, BKEnvelope * envelope)
{
	memset (state, 0, sizeof (* state));

	BKEnvelopeAddState (envelope, state);
	BKEnvelopeStateSetPhase (state, BK_ENVELOP_MUTE);

	return 0;
}

void BKEnvelopeStateDispose (BKEnvelopeState * state)
{
	BKEnvelopeStateRemove (state);
	memset (state, 0, sizeof (* state));
}

BKInt BKEnvelopeStateSetPhase (BKEnvelopeState * state, BKEnum phase)
{
	BKInt phaseOffset;
	BKEnvelope * envelope = state -> envelope;

	switch (phase) {
		case BK_ENVELOP_ATTACK: {
			phaseOffset = 0;
			break;
		}
		case BK_ENVELOP_RELEASE: {
			phaseOffset = envelope -> sustainOffset + envelope -> sustainLength;
			break;
		}
		case BK_ENVELOP_MUTE: {
			phaseOffset = envelope -> length;
			state -> value = 0;
			state -> delta = 0;
			break;
		}
		default: {
			return -1;
			break;
		}
	}

	state -> phase       = phase;
	state -> phaseOffset = phaseOffset;
	state -> step        = 0;

	if (state -> phaseOffset >= envelope -> length) {
		state -> value = 0;
		state -> delta = 0;
	}
	
	if (phase == BK_ENVELOP_RELEASE)
		BKEnvelopeStateStep (state);

	return 0;
}

BKInt BKEnvelopeStateStep (BKEnvelopeState * state)
{
	BKInt value, steps;
	BKEnvelopeValue * phase;
	BKEnvelope * envelope = state -> envelope;

	do {
		if (state -> step == 0) {
			if (state -> phase == BK_ENVELOP_ATTACK) {			
				if (state -> phaseOffset >= envelope -> sustainOffset + envelope -> sustainLength) {
					if (envelope -> sustainLength)
						state -> phaseOffset = envelope -> sustainOffset;
				}
			}

			if (state -> phaseOffset < envelope -> length) {
				phase = & envelope -> phases [state -> phaseOffset];
				steps = phase -> steps;
				
				if (steps) {
					value = (phase -> value << envelope -> fracShift);
					state -> delta = (value - state -> value) / steps;
					state -> endValue = value;
					state -> step = steps;
				}
				else {
					state -> value = (phase -> value << envelope -> fracShift);
					state -> step = 0;
				}
				
				state -> phaseOffset ++;
			}
			else {
				return 1;
			}
		}
		else {			
			state -> step --;
			
			if (state -> step) {
				state -> value += state -> delta;
			}
			else {
				state -> value = state -> endValue;
			}
		}
	}
	while (state -> step == 0);

	return 0;
}

BKInt BKEnvelopeStateGetValue (BKEnvelopeState const * state)
{
	BKEnvelope * envelope = state -> envelope;

	return (state -> value >> envelope -> fracShift);
}
