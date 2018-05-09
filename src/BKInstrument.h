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

#ifndef _BK_INSTRUMENT_H_
#define _BK_INSTRUMENT_H_

#include "BKBase.h"
#include "BKContext.h"
#include "BKSequence.h"

typedef struct BKInstrumentState BKInstrumentState;
typedef struct BKInstrument      BKInstrument;

typedef BKInt (* BKInstrumentStateCallback) (BKEnum event, void * userInfo);

enum
{
	BK_SEQUENCE_VOLUME,     ///< The volume sequence.
	BK_SEQUENCE_PANNING,    ///< The panning sequence.
	BK_SEQUENCE_PITCH,      ///< The pitch sequence.
	BK_SEQUENCE_DUTY_CYCLE, ///< The duty cycle sequence.
	BK_MAX_SEQUENCES,       ///< Used internally.
};

enum
{
	BK_INSTR_STATE_EVENT_RESET,
	BK_INSTR_STATE_EVENT_DISPOSE,
	BK_INSTR_STATE_EVENT_MUTE,
};

struct BKInstrumentState
{
	BKInstrument            * instrument;
	BKInstrumentState       * prevState;
	BKInstrumentState       * nextState;
	BKUInt                    phase;
	BKInt                     numActiveSequences;
	BKInstrumentStateCallback callback;  // called when disposing
	void                    * callbackUserInfo;
	BKSequenceState           states [BK_MAX_SEQUENCES];
};

struct BKInstrument
{
	BKObject            object;
	BKUInt              numSequences;
	BKInstrumentState * stateList;
	BKSequence        * sequences [BK_MAX_SEQUENCES];
};

/**
 * Initalize instrument.
 *
 * Disposing with BKDispose detaches the object from all tracks.
 *
 * @param instr The instrument to initialize.
 * @retval BK_SUCCESS
 * @retval BK_ALLOCATION_ERROR
 */
extern BKInt BKInstrumentInit (BKInstrument * instr);

/**
 * Allocate instrument and store in given pointer.
 *
 * @param outInstr A pointer to a instrument pointer.
 * @retval BK_SUCCESS
 * @retval BK_ALLOCATION_ERROR
 */
extern BKInt BKInstrumentAlloc (BKInstrument ** outInstr);

/**
 * Detach instrument from all tracks.
 *
 * @param instr The instrument to detach.
 */
extern void BKInstrumentDetach (BKInstrument * instr);

/**
 * Copy an instrument. The copy will not be attached to any track.
 *
 * @param copy The instrument struct to copy into; must not be initialized.
 * @param original The instrument to copy.
 * @retval BK_SUCCESS
 * @retval BK_ALLOCATION_ERROR
 */
extern BKInt BKInstrumentInitCopy (BKInstrument * copy, BKInstrument const * original);

/**
 * Set sequence values.
 *
 * @param instr The instrument to set the sequence to.
 * @param sequence The sequence type.
 * @param values The sequence values.
 * @param length The number of sequence values.
 * @param sustainOffset The beginning of the sustain range.
 * @param sustainLength The length of the susutain range.
 * @retval BK_SUCCESS
 * @retval BK_INVALID_ATTRIBUTE If the sequence type is invalid.
 * @retval BK_ALLOCATION_ERROR
 */
extern BKInt BKInstrumentSetSequence (BKInstrument * instr, BKEnum sequence, BKInt const * values, BKUInt length, BKInt sustainOffset, BKInt sustainLength);

/**
 * Set envelope values.
 *
 * @param instr The instrument to set the envelope to.
 * @param sequence The sequence type.
 * @param phases The envelope phases.
 * @param length The number of sequence values.
 * @param sustainOffset The beginning of the sustain range.
 * @param sustainLength The length of the susutain range.
 * @retval BK_SUCCESS
 * @retval BK_INVALID_ATTRIBUTE If the envelope type is invalid.
 * @retval BK_ALLOCATION_ERROR
 */
extern BKInt BKInstrumentSetEnvelope (BKInstrument * instr, BKEnum sequence, BKSequencePhase const * phases, BKUInt length, BKInt sustainOffset, BKInt sustainLength);

/**
 * Set ADSR envelope sequence in BK_SEQUENCE_VOLUME slot.
 *
 * @param instr The instrument
 * @param attack The number of attack ticks.
 * @param decay The number if decay ticks.
 * @param sustain The sustain volume (max. BK_MAX_VOLUME).
 * @param release The number of release ticks.
 * @retval BK_SUCCESS
 * @retval BK_INVALID_ATTRIBUTE If the envelope type is invalid.
 * @retval BK_ALLOCATION_ERROR
 */
extern BKInt BKInstrumentSetEnvelopeADSR (BKInstrument * instr, BKUInt attack, BKUInt decay, BKInt sustain, BKUInt release);

/**
 * Get sequence for given sequence type.
 *
 * @param instr The instrument to get the sequence from.
 * @param sequence The sequence type.
 * @return A pointer to the sequence or NULL if no sequence is defined.
 */
extern BKSequence const * BKInstrumentGetSequence (BKInstrument const * instr, BKEnum sequence);

#endif /* ! _BK_INSTRUMENT_H_ */
