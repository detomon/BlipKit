/**
 * Copyright (c) 2012-2014 Simon Schoenenberger
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

#ifndef _BK_INTERPRETER_H_
#define _BK_INTERPRETER_H_

#include "BKBase.h"
#include "BKTrack.h"
#include "item_list.h"

#define BK_INTR_CUSTOM_WAVEFOMR_FLAG (1 << 24)
#define BK_INTR_STACK_SIZE 64
#define BK_INTR_MAX_EVENTS 4

typedef struct BKInterpreter BKInterpreter;
typedef struct BKTickEvent   BKTickEvent;

enum
{
	BKIntrAttack,
	BKIntrArpeggio,
	BKIntrArpeggioSpeed,
	BKIntrAttackTicks,
	BKIntrRelease,
	BKIntrReleaseTicks,
	BKIntrMute,
	BKIntrMuteTicks,
	BKIntrVolume,
	BKIntrPanning,
	BKIntrPitch,
	BKIntrMasterVolume,
	BKIntrStep,
	BKIntrTicks,
	BKIntrEffect,
	BKIntrDutyCycle,
	BKIntrPhaseWrap,
	BKIntrInstrument,
	BKIntrWaveform,
	BKIntrSample,
	BKIntrSampleRepeat,
	BKIntrReturn,
	BKIntrGroup,
	BKIntrCall,
	BKIntrJump,
	BKIntrEnd,
	BKIntrStepTicks,
};

struct BKTickEvent
{
	BKInt event;
	BKInt ticks;
};

struct BKInterpreter
{
	BKUInt          flags;
	BKInt         * opcode;
	BKInt         * opcodePtr;
	BKInt           stack [BK_INTR_STACK_SIZE];
	BKInt         * stackPtr;
	BKInt         * stackEnd;
	BKInstrument ** instruments;
	BKData       ** waveforms;
	BKData       ** samples;
	BKUInt          stepTickCount;
	BKUInt          numSteps;
	BKInt           nextNote;
	BKInt           nextArpeggio [1 + BK_MAX_ARPEGGIO];
	BKInt           numEvents;
	BKTickEvent     events [BK_INTR_MAX_EVENTS];
};

/**
 * Apply commands to track and return steps to next event
 */
extern BKInt BKInterpreterTrackAdvance (BKInterpreter * interpreter, BKTrack * track);

/**
 * Dispose interpreter
 */
extern void BKInterpreterDispose (BKInterpreter * interpreter);

/**
 * reset interpreter
 */
extern void BKInterpreterReset (BKInterpreter * interpreter);

#endif /* !_BK_INTERPRETER_H_  */
