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

#ifndef _BK_INSTRUMENT_INTERNAL_H_
#define _BK_INSTRUMENT_INTERNAL_H_

#include "BKInstrument.h"

/**
 * Initialize instrument state.
 *
 * @param state The instrument to initialize.
 * @retval BK_SUCCESS
 */
extern BKInt BKInstrumentStateInit (BKInstrumentState * state);

/**
 * Set instrument to state.
 *
 * @param state The state to set the instrument to.
 * @param instr The instrument to set.
 * @retval BK_SUCCESS
 */
extern BKInt BKInstrumentStateSetInstrument (BKInstrumentState * state, BKInstrument * instr);

/**
 * Get sequence value at offset.
 *
 * @param state The state to get the value from.
 * @param slot The sequence type.
 * @param offset The sequence offset.
 * @return The value at the given sequence offset.
 */
extern BKInt BKInstrumentStateGetSequenceValueAtOffset (BKInstrumentState const * state, BKEnum slot, BKInt offset);

/**
 * Advance state to the next sequence values.
 *
 * @param state The state to advance.
 * @param level 0 for ticks; 1 for instrument steps.
 */
extern void BKInstrumentStateTick (BKInstrumentState * state, BKInt level);

/**
 * Set sequence phase.
 *
 * @param state The state to set the sequence phase.
 * @param phase BK_SEQUENCE_PHASE_ATTACK, BK_SEQUENCE_PHASE_RELEASE or BK_SEQUENCE_PHASE_MUTE
 */
void BKInstrumentStateSetPhase (BKInstrumentState * state, BKEnum phase);

#endif /* ! _BK_INSTRUMENT_INTERNAL_H_ */
