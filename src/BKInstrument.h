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

#ifndef _BK_INSTRUMENT_H_
#define _BK_INSTRUMENT_H_

#include "BKBase.h"
#include "BKContext.h"

enum
{
	BK_SEQUENCE_VOLUME,
	BK_SEQUENCE_PANNING,
	BK_SEQUENCE_ARPEGGIO,
	BK_SEQUENCE_DUTY_CYCLE,
	BK_MAX_SEQUENCES,
};

enum
{
	BK_INSTR_STATE_ATTACK,
	BK_INSTR_STATE_RELEASE,
	BK_INSTR_STATE_MUTE,
};

enum
{
	BK_INSTR_STATE_EVENT_RESET,
	BK_INSTR_STATE_EVENT_DISPOSE,
	BK_INSTR_STATE_EVENT_MUTE,
};

enum
{
	BK_INSTR_SEQ_TYPE_SEQ,
	BK_INSTR_SEQ_TYPE_ENVELOP,
};

typedef struct BKInstrumentState    BKInstrumentState;
typedef struct BKInstrument         BKInstrument;
typedef struct BKInstrumentSequence BKInstrumentSequence;
typedef struct BKInstrumentSeqState BKInstrumentSeqState;
typedef struct BKEnvelopeValue      BKEnvelopeValue;

typedef BKInt (* BKInstrumentStateCallback) (BKEnum event, void * userInfo);

struct BKInstrumentState
{
	BKInstrument            * instrument;
	BKInstrumentState       * prevState;
	BKInstrumentState       * nextState;
	BKUInt                    state;
	BKInt                     numActiveSequences;
	BKInstrumentStateCallback callback;  // called when disposing
	void                    * callbackUserInfo;
	struct BKInstrumentSeqState {
		BKInt step;
		BKInt delta;
		BKInt offset;
		BKInt value;
		BKInt fullValue;
		BKInt endValue;
	} sequences [BK_MAX_SEQUENCES];
};

struct BKInstrumentSequence
{
	BKUInt length;
	BKInt  sustainOffset;
	BKInt  sustainLength;
	BKInt  fracShift;
	BKInt  values [];
};

struct BKEnvelopeValue
{
	BKInt steps;
	BKInt value;
};

struct BKInstrument
{
	BKUInt              flags;
	BKUInt              numSequences;
	BKInstrumentState * stateList;
	struct {
		BKInt  type;
		void * sequence;
	} sequences [BK_MAX_SEQUENCES];
};

/**
 * Initalize instrument
 */
extern BKInt BKInstrumentInit (BKInstrument * instr);

/**
 * Dispose instrument
 */
extern void BKInstrumentDispose (BKInstrument * instr);

/**
 * Copy an instrument
 * The copy will not be attached to any track
 */
extern BKInt BKInstrumentInitCopy (BKInstrument * copy, BKInstrument const * original);

/**
 * Set sequence
 */
extern BKInt BKInstrumentSetSequence (BKInstrument * instr, BKEnum sequence, BKInt const * values, BKUInt length, BKInt sustainOffset, BKInt sustainLength);

/**
 * Set envelope sequence
 */
extern BKInt BKInstrumentSetEnvelope (BKInstrument * instr, BKEnum sequence, BKEnvelopeValue const * phases, BKUInt length, BKInt sustainOffset, BKInt sustainLength);

/**
 * Set ADSR envelope sequence in BK_SEQUENCE_VOLUME slot
 */
extern BKInt BKInstrumentSetEnvelopeADSR (BKInstrument * instr, BKUInt attack, BKUInt decay, BKInt sustain, BKUInt release);

/**
 * Get sequence
 */
extern BKInstrumentSequence const * BKInstrumentGetSequence (BKInstrument const * instr, BKUInt slot);

#endif /* ! _BK_INSTRUMENT_H_ */
