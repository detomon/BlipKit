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

#ifndef _BK_SEQUENCE_H_
#define _BK_SEQUENCE_H_

#include "BKBase.h"

typedef struct BKSequence BKSequence;
typedef struct BKSequenceState BKSequenceState;
typedef struct BKSequenceFuncs BKSequenceFuncs;
typedef struct BKSequencePhase BKSequencePhase;

/**
 * Sequence phases.
 */
enum {
	BK_SEQUENCE_PHASE_MUTE,	   ///< Aequence state is muted (default).
	BK_SEQUENCE_PHASE_ATTACK,  ///< Sequence state is in attack phase.
	BK_SEQUENCE_PHASE_RELEASE, ///< Sequence state is in release phase.
};

/**
 * Step return values.
 */
enum {
	BK_SEQUENCE_RETURN_NONE = 0,		///< No step.
	BK_SEQUENCE_RETURN_STEP = 1 << 0,	///< Normal step.
	BK_SEQUENCE_RETURN_REPEAT = 1 << 1, ///< Sustain sequence was restarted.
	BK_SEQUENCE_RETURN_FINISH = 1 << 2, ///< Sequence has finished.
	/** Mask for active sequence */
	BK_SEQUENCE_RETURN_ACTIVE_MASK = BK_SEQUENCE_RETURN_STEP | BK_SEQUENCE_RETURN_REPEAT,
};

/**
 * Step levels.
 */
enum {
	BK_SEQUENCE_STEP_MAX,	  ///< Envelope step.
	BK_SEQUENCE_STEP_DIVIDED, ///< Sequence step.
};

/**
 * Defines function prototypes of sequence types.
 */
struct BKSequenceFuncs {
	/** Create a sequence or envelope. */
	BKInt (*create)(BKSequence** outSequence, BKSequenceFuncs const* funcs, void const* values, BKUInt length, BKUInt sustainOffset, BKUInt sustainLength);

	/** Advance state by one step. `level` indicates the step level. */
	BKEnum (*step)(BKSequenceState* state, BKEnum level);

	/** Set current value of sequence, e.g. before attacking duty cycle envelope to slide from current value. */
	BKInt (*setValue)(BKSequenceState* state, BKInt value);

	/** Set phase of sequence state. */
	BKInt (*setPhase)(BKSequenceState* state, BKEnum phase);

	/** Copy sequence. */
	BKInt (*copy)(BKSequence** outCopy, BKSequence const* sequence);
};

/**
 * Defines phase used in envelope sequence.
 */
struct BKSequencePhase {
	BKUInt steps;
	BKInt value;
};

/**
 * Defines a sequence. This can be a simple array of values or an envelope.
 */
struct BKSequence {
	BKSequenceFuncs const* funcs;
	BKSequenceState* stateList;
	BKInt length;
	BKInt sustainOffset;
	BKInt sustainLength;
	BKInt fracShift;
	// BKInt                   defaultValue;
	BKEnum state;
	void* values;
};

/**
 * Holds the state of a sequence.
 */
struct BKSequenceState {
	BKSequence* sequence;
	BKSequenceState* prevState;
	BKSequenceState* nextState;
	BKEnum phase;
	BKInt steps;
	BKInt delta;
	BKInt offset;
	BKInt value;
	BKInt shiftedValue;
	BKInt endValue;
};

/**
 * Sequence prototype functions
 */
extern BKSequenceFuncs const BKSequenceFuncsSimple;
extern BKSequenceFuncs const BKSequenceFuncsEnvelope;

/**
 * Create a sequence or envelope.
 *
 * @param outSequence A pointer to a sequence pointer.
 * @param funcs The sequence or envelope functions.
 * @param values The sequence values.
 * @param length The number of sequence values.
 * @param sustainOffset The beginning of the sustain range.
 * @param sustainLength The length of the susutain range.
 * @retval BK_SUCCESS
 * @retval BK_ALLOCATION_ERROR
 */
extern BKInt BKSequenceCreate(BKSequence** outSequence, BKSequenceFuncs const* funcs, void const* values, BKUInt length, BKUInt sustainOffset, BKUInt sustainLength);

/**
 * Create a copy of a sequence.
 *
 * @param outSequence A pointer to a sequence pointer.
 * @param sequence The sequence to copy.
 * @retval BK_SUCCESS
 * @retval BK_ALLOCATION_ERROR
 */
extern BKInt BKSequenceCopy(BKSequence** outSequence, BKSequence* sequence);

/**
 * Dispose sequence.
 *
 * @param sequence The sequence to dispose.
 */
extern void BKSequenceDispose(BKSequence* sequence);

/**
 * Set new sequence.
 *
 * @param state The state to set the sequence to.
 * @param sequence The sequence to set.
 * @retval BK_SUCCESS
 */
extern BKInt BKSequenceStateSetSequence(BKSequenceState* state, BKSequence* sequence);

/**
 * Set state phase.
 *
 * @param state The state to set the phase.
 * @param phase The phase to set.
 * @retval BK_SEQUENCE_RETURN_NONE
 * @retval BK_INVALID_ATTRIBUTE
 * @retval BK_SEQUENCE_RETURN_FINISH
 */
extern BKInt BKSequenceStateSetPhase(BKSequenceState* state, BKEnum phase);

/**
 * Call step function.
 *
 * @param state The state to advance.
 * @param level 0 for ticks; 1 for instrument steps.
 * @retval BK_SEQUENCE_RETURN_NONE
 * @retval BK_SEQUENCE_RETURN_STEP
 * @retval BK_SEQUENCE_RETURN_REPEAT
 * @retval BK_SEQUENCE_RETURN_FINISH
 */
extern BKInt BKSequenceStateStep(BKSequenceState* state, BKEnum level);

/**
 * Call setValue function.
 *
 * @param state The state.
 * @param value The value to set.
 * @retval BK_SUCCESS
 */
extern BKInt BKSequenceStateSetValue(BKSequenceState* state, BKInt value);

#endif /* ! _BK_SEQUENCE_H_ */
