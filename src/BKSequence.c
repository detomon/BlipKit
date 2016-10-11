#include "BKSequence.h"

static BKInt BKSequenceFuncSimpleCreate (BKSequence ** outSequence, BKSequenceFuncs const * funcs, void const * values, BKUInt length, BKUInt sustainOffset, BKUInt sustainLength)
{
	BKInt        size;
	BKSequence * sequence;

	sustainOffset = BKClamp (sustainOffset, 0, length);
	sustainLength = BKClamp (sustainLength, 0, length - sustainOffset);

	size = sizeof (BKInt) * length;
	sequence = malloc (sizeof (* sequence) + size);

	if (sequence) {
		memset (sequence, 0, sizeof ( * sequence));

		sequence -> values = (void *) sequence + sizeof (* sequence);

		memcpy (sequence -> values, values, size);

		sequence -> funcs         = funcs;
		sequence -> length        = length;
		sequence -> sustainOffset = sustainOffset;
		sequence -> sustainLength = sustainLength;
		sequence -> fracShift     = 0;

		* outSequence = sequence;

		return 0;
	}

	return BK_ALLOCATION_ERROR;
}

static BKInt BKSequenceFuncSimpleSetPhase (BKSequenceState * state, BKEnum phase)
{
	BKInt        result   = BK_SEQUENCE_RETURN_NONE;
	BKSequence * sequence = state -> sequence;
	BKInt      * values   = sequence -> values;

	switch (phase) {
		case BK_SEQUENCE_PHASE_MUTE: {
			state -> offset = sequence -> length;
			break;
		}
		case BK_SEQUENCE_PHASE_ATTACK: {
			state -> offset = 0;
			break;
		}
		case BK_SEQUENCE_PHASE_RELEASE: {
			state -> offset = sequence -> sustainOffset + sequence -> sustainLength;
			break;
		}
		default: {
			return BK_INVALID_ATTRIBUTE;
			break;
		}
	}

	if (state -> offset < sequence -> length) {
		state -> value = values [state -> offset];
	}
	else {
		//state -> value  = sequence -> defaultValue;
		state -> offset = BK_INT_MAX;
		result = BK_SEQUENCE_RETURN_FINISH;
	}

	state -> phase = phase;

	return result;
}

static BKEnum BKSequenceFuncSimpleStep (BKSequenceState * state, BKEnum level)
{
	if (level < BK_SEQUENCE_STEP_DIVIDED)
		return BK_SEQUENCE_RETURN_NONE;

	if (state -> offset == BK_INT_MAX)
		return BK_SEQUENCE_RETURN_NONE;

	BKInt        result     = BK_SEQUENCE_RETURN_STEP;
	BKSequence * sequence   = state -> sequence;
	BKInt        sustainEnd = sequence -> sustainOffset + sequence -> sustainLength;
	BKInt      * phases     = sequence -> values;

	// reset sustain phase
	if (state -> phase == BK_SEQUENCE_PHASE_ATTACK) {
		if (state -> offset >= sustainEnd) {
			state -> offset = sequence -> sustainOffset;
			result = BK_SEQUENCE_RETURN_REPEAT;
		}
	}

	if (state -> offset < sequence -> length) {
		state -> value = phases [state -> offset];
		state -> offset ++;
	}
	else {
		state -> offset = BK_INT_MAX;
		result = BK_SEQUENCE_RETURN_FINISH;
	}

	return result;
}

static BKInt BKSequenceFuncSimpleSetValue (BKSequenceState * state, BKInt value)
{
	state -> value = value;

	return 0;
}

static BKInt BKSequenceFuncSimpleCopy (BKSequence ** outCopy, BKSequence const * sequence)
{
	BKInt        size;
	BKSequence * copy;
	BKInt      * values;

	size = sizeof (* values) * sequence -> length;
	copy = malloc (sizeof (* copy) + size);

	if (copy) {
		values = (void *) copy + sizeof (* copy);
		copy -> values = values;

		memcpy (copy, sequence, sizeof (* copy));
		memcpy (copy -> values, sequence -> values, size);

		* outCopy = copy;

		return 0;
	}

	return BK_ALLOCATION_ERROR;
}

BKSequenceFuncs const BKSequenceFuncsSimple =
{
	.create   = BKSequenceFuncSimpleCreate,
	.step     = BKSequenceFuncSimpleStep,
	.setValue = BKSequenceFuncSimpleSetValue,
	.setPhase = BKSequenceFuncSimpleSetPhase,
	.copy     = BKSequenceFuncSimpleCopy,
};

/**
 * Get the maximum absolute value of a sequence
 */
static BKUInt BKSequencePhaseGetMaxAbsValue (BKSequencePhase const * phases, BKUInt length)
{
	BKInt maxValue = 0;

	for (BKInt i = 0; i < length; i ++)
		maxValue = BKMax (maxValue, BKAbs (phases [i].value));

	return maxValue;
}

/**
 * This will calculate the maximum shift value for a sequence to fit in a 32 bit integer
 * The shift value will be used to increase the precision when interpolate between two phases
 */
static BKUInt BKSequencePhaseGetFracShift (BKSequencePhase const * phases, BKUInt length)
{
	BKInt shift    = 0;
	BKInt maxValue = BKSequencePhaseGetMaxAbsValue (phases, length);

	// shift must be smaller than maximum value
	// reduce by one bit to allow enough delta to slide from maximum to minimum
	while ((1 << shift) <= maxValue && shift < 30)
		shift ++;

	shift = 30 - shift;

	return shift;
}

/**
 * Check step sum of envelope values
 */
static BKInt BKSequenceEnvelopeCheckValues (BKSequencePhase const * values, BKUInt length, BKUInt sustainOffset, BKUInt sustainLength)
{
	BKInt steps = 0;

	if (sustainOffset == length)
		return 0;

	// check sustain sequence step length
	for (BKInt i = sustainOffset; i < sustainOffset + sustainLength; i ++)
		steps += values [i].steps;

	if (steps == 0)
		return BK_INVALID_VALUE;

	return 0;
}

static BKInt BKSequenceFuncEnvelopeCreate (BKSequence ** outSequence, BKSequenceFuncs const * funcs, void const * values, BKUInt length, BKUInt sustainOffset, BKUInt sustainLength)
{
	sustainOffset = BKClamp (sustainOffset, 0, length);
	sustainLength = BKClamp (sustainLength, 0, length - sustainOffset);

	if (BKSequenceEnvelopeCheckValues (values, length, sustainOffset, sustainLength) != 0)
		return BK_INVALID_VALUE;

	BKInt        size     = sizeof (BKSequencePhase) * length;
	BKSequence * sequence = malloc (sizeof (* sequence) + size);

	if (sequence) {
		memset (sequence, 0, sizeof ( * sequence));

		sequence -> values = (void *) sequence + sizeof (* sequence);

		memcpy (sequence -> values, values, size);

		sequence -> funcs         = funcs;
		sequence -> length        = length;
		sequence -> sustainOffset = sustainOffset;
		sequence -> sustainLength = sustainLength;
		sequence -> fracShift     = BKSequencePhaseGetFracShift (values, length);

		* outSequence = sequence;

		return 0;
	}

	return BK_ALLOCATION_ERROR;
}

static BKEnum BKSequenceFuncEnvelopeStep (BKSequenceState * state, BKEnum level)
{
	if (level < BK_SEQUENCE_STEP_MAX)
		return BK_SEQUENCE_RETURN_NONE;

	if (state -> offset == BK_INT_MAX)
		return BK_SEQUENCE_RETURN_NONE;

	BKEnum            result      = BK_SEQUENCE_RETURN_STEP;
	BKSequence      * sequence   = state -> sequence;
	BKSequencePhase * phases     = sequence -> values;
	BKInt             sustainEnd = sequence -> sustainOffset + sequence -> sustainLength;

	do {
		if (state -> steps) {
			state -> steps --;

			if (state -> steps > 0) {
				state -> shiftedValue += state -> delta;
				state -> value = (state -> shiftedValue >> sequence -> fracShift);
				break;
			}
			else {
				state -> shiftedValue = state -> endValue;
				state -> value = (state -> shiftedValue >> sequence -> fracShift);
				state -> offset ++;
			}
		}

		// reset sustain phase
		if (state -> phase == BK_SEQUENCE_PHASE_ATTACK) {
			if (state -> offset >= sustainEnd) {
				state -> offset = sequence -> sustainOffset;
				result = BK_SEQUENCE_RETURN_REPEAT;
			}
		}

		if (state -> offset < sequence -> length) {
			BKSequencePhase * phase = & phases [state -> offset];

			if (phase -> steps) {
				state -> steps = phase -> steps;
				state -> endValue = phase -> value << sequence -> fracShift;
				state -> delta = (state -> endValue - state -> shiftedValue) / (BKInt) phase -> steps;
			}
			else {
				state -> shiftedValue = (phase -> value << sequence -> fracShift);
				state -> steps = 0;
				state -> offset ++;
			}

			state -> value = (state -> shiftedValue >> sequence -> fracShift);
		}
		else {
			state -> offset = BK_INT_MAX;
			result = BK_SEQUENCE_RETURN_FINISH;
			break;
		}
	}
	while (state -> steps == 0);

	return result;
}

static BKInt BKSequenceFuncEnvelopeSetPhase (BKSequenceState * state, BKEnum phase)
{
	BKInt        result   = BK_SEQUENCE_RETURN_NONE;
	BKSequence * sequence = state -> sequence;

	switch (phase) {
		case BK_SEQUENCE_PHASE_MUTE: {
			state -> steps        = 0;
			state -> delta        = 0;
			state -> offset       = sequence -> length;
			state -> shiftedValue = (state -> value << sequence -> fracShift);
			state -> endValue     = 0;
			state -> phase        = BK_SEQUENCE_PHASE_MUTE;
			break;
		}
		case BK_SEQUENCE_PHASE_ATTACK: {
			state -> steps  = 0;
			state -> offset = 0;
			state -> phase  = BK_SEQUENCE_PHASE_ATTACK;

			result = BKSequenceFuncEnvelopeStep (state, 0);  // make first step

			break;
		}
		case BK_SEQUENCE_PHASE_RELEASE: {
			state -> steps  = 0;
			state -> offset = sequence -> sustainOffset + sequence -> sustainLength;
			state -> phase  = BK_SEQUENCE_PHASE_RELEASE;

			result = BKSequenceFuncEnvelopeStep (state, 0);  // make first step

			break;
		}
		default: {
			return BK_INVALID_ATTRIBUTE;
			break;
		}
	}

	if (result == BK_SEQUENCE_RETURN_NONE) {
		if (state -> offset >= sequence -> length) {
			state -> offset = sequence -> length;
			result = BK_SEQUENCE_RETURN_FINISH;
		}
	}

	return result;
}

static BKInt BKSequenceFuncEnvelopeSetValue (BKSequenceState * state, BKInt value)
{
	BKSequence * sequence = state -> sequence;

	state -> value        = value;
	state -> shiftedValue = (value << sequence -> fracShift);

	if (state -> steps)
		state -> delta = (state -> endValue - state -> shiftedValue) / state -> steps;

	return 0;
}

static BKInt BKSequenceFuncEnvelopeCopy (BKSequence ** outCopy, BKSequence const * sequence)
{
	BKInt              size;
	BKSequence       * copy;
	BKSequencePhase  * values;

	size = sizeof (* values) * sequence -> length;
	copy = malloc (sizeof (* copy) + size);

	if (copy) {
		values = (void *) copy + sizeof (* copy);
		copy -> values = values;

		memcpy (copy, sequence, sizeof (* copy));
		memcpy (copy -> values, sequence -> values, size);

		* outCopy = copy;

		return 0;
	}

	return BK_ALLOCATION_ERROR;
}

BKSequenceFuncs const BKSequenceFuncsEnvelope =
{
	.create   = BKSequenceFuncEnvelopeCreate,
	.step     = BKSequenceFuncEnvelopeStep,
	.setValue = BKSequenceFuncEnvelopeSetValue,
	.setPhase = BKSequenceFuncEnvelopeSetPhase,
	.copy     = BKSequenceFuncEnvelopeCopy,
};

BKInt BKSequenceCreate (BKSequence ** outSequence, BKSequenceFuncs const * funcs, void const * values, BKUInt length, BKUInt sustainOffset, BKUInt sustainLength)
{
	return funcs -> create (outSequence, funcs, values, length, sustainOffset, sustainLength);
}

static void BKSequenceStateAddToSequence (BKSequenceState * state, BKSequence * sequence)
{
	if (state -> sequence == NULL) {
		state -> sequence   = sequence;
		state -> prevState  = NULL;
		state -> nextState  = sequence -> stateList;

		if (sequence -> stateList)
			sequence -> stateList -> prevState = state;

		sequence -> stateList = state;
	}
}

static void BKSequenceStateRemoveFromSequence (BKSequenceState * state)
{
	BKSequence * sequence = state -> sequence;

	if (sequence != NULL) {
		if (state -> prevState)
			state -> prevState -> nextState = state -> nextState;

		if (state -> nextState)
			state -> nextState -> prevState = state -> prevState;

		if (sequence -> stateList == state)
			sequence -> stateList = state -> nextState;

		state -> sequence   = NULL;
		state -> prevState  = NULL;
		state -> nextState  = NULL;
	}
}

BKInt BKSequenceStateSetSequence (BKSequenceState * state, BKSequence * sequence)
{
	BKInt result = 0;

	if (state -> sequence != sequence) {
		if (state -> sequence)
			BKSequenceStateRemoveFromSequence (state);

		if (sequence)
			BKSequenceStateAddToSequence (state, sequence);

		state -> sequence = sequence;

		if (sequence)
			result = sequence -> funcs -> setPhase (state, state -> phase);
	}

	return result;
}

BKInt BKSequenceCopy (BKSequence ** outSequence, BKSequence * sequence)
{
	return sequence -> funcs -> copy (outSequence, sequence);
}

void BKSequenceDispose (BKSequence * sequence)
{
	free (sequence);
}

BKInt BKSequenceStateSetPhase (BKSequenceState * state, BKEnum phase)
{
	if (state -> sequence == NULL)
		return BK_INVALID_STATE;

	return state -> sequence -> funcs -> setPhase (state, phase);
}

BKInt BKSequenceStateStep (BKSequenceState * state, BKEnum level)
{
	if (state -> sequence == NULL)
		return BK_INVALID_STATE;

	return state -> sequence -> funcs -> step (state, level);
}

BKInt BKSequenceStateSetValue (BKSequenceState * state, BKInt value)
{
	if (state -> sequence)
		state -> sequence -> funcs -> setValue (state, value);

	state -> value = value;

	return 0;
}
