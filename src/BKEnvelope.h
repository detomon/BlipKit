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

#ifndef _BK_ENVELOP_H_
#define _BK_ENVELOP_H_

#include "BKBase.h"

enum
{
	BK_ENVELOP_MUTE,
	BK_ENVELOP_ATTACK,
	BK_ENVELOP_RELEASE,
};

typedef struct BKEnvelopeValue BKEnvelopeValue;
typedef struct BKEnvelope      BKEnvelope;
typedef struct BKEnvelopeState BKEnvelopeState;

struct BKEnvelopeValue
{
	BKInt steps;
	BKInt value;
};

struct BKEnvelope
{
	BKInt             length;
	BKInt             sustainOffset;
	BKInt             sustainLength;
	BKInt             fracShift;
	BKEnvelopeState * states;
	BKEnvelopeValue * phases;
};

struct BKEnvelopeState
{
	BKEnvelope      * envelope;
	BKEnvelopeState * prevState;
	BKEnvelopeState * nextState;
	BKInt             delta;
	BKInt             value;
	BKInt             endValue;
	BKInt             phase;
	BKInt             phaseOffset;
	BKInt             step;
};

/**
 *
 */
extern BKInt BKEnvelopeInit (BKEnvelope * envelope, BKEnvelopeValue const * phases, BKInt numPhases, BKInt sustainOffset, BKInt sustainLength);

/**
 *
 */
extern BKInt BKEnvelopeInitADSR (BKEnvelope * envelope, BKInt attack, BKInt attackValue, BKInt decay, BKInt sustain, BKInt release);

/**
 *
 */
extern void BKEnvelopeDispose (BKEnvelope * envelope);

/**
 *
 */
extern BKInt BKEnvelopeStateInit (BKEnvelopeState * state, BKEnvelope * envelope);

/**
 *
 */
extern void BKEnvelopeStateDispose (BKEnvelopeState * state);

/**
 *
 */
extern BKInt BKEnvelopeStateSetPhase (BKEnvelopeState * state, BKEnum phase);

/**
 *
 */
extern BKInt BKEnvelopeStateStep (BKEnvelopeState * state);

/**
 *
 */
extern BKInt BKEnvelopeStateGetValue (BKEnvelopeState const * state);

#endif /* ! _BK_ENVELOP_H_ */
