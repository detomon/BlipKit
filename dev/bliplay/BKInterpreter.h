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

#ifndef _BK_INTERPRETER_H_
#define _BK_INTERPRETER_H_

#include "BKBase.h"
#include "BKTrack.h"
#include "item_list.h"

#define BK_CUSTOM_WAVEFOMR_FLAG (1 << 24)

typedef struct BKInterpreter BKInterpreter;

enum
{
	BKIntrAttack,
	BKIntrArpeggio,
	BKIntrArpeggioSpeed,
	BKIntrRelease,
	BKIntrMute,
	BKIntrMuteTicks,
	BKIntrVolume,
	BKIntrPanning,
	BKIntrMasterVolume,
	BKIntrStep,
	BKIntrEffect,
	BKIntrDutyCycle,
	BkIntrPhaseWrap,
	BKIntrInstrument,
	BKIntrWaveform,
	BKIntrReturn,
	BKIntrGroup,
	BKIntrCall,
	BKIntrJump,
	BKIntrEnd,
	BKIntrStepTicks,
};

enum
{
	BKIntrReleaseFlag = 1 << 0,
};

struct BKInterpreter
{
	BKUInt          flags;
	BKInt         * opcode;
	BKInt         * opcodePtr;
	BKInt         * stack [64];
	BKInt        ** stackPtr;
	BKInt        ** stackEnd;
	BKInstrument ** instruments;
	BKData       ** waveforms;
	BKUInt          stepTickCount;
	BKUInt          noteStepTickCount;
	BKUInt          muteTickCount;
};

/**
 * Apply commands to track and return steps to next event
 */
extern BKInt BKInterpreterTrackApplyNextStep (BKInterpreter * interpreter, BKTrack * track);

/**
 * Dispose interpreter
 */
extern void BKInterpreterDispose (BKInterpreter * interpreter);

/**
 * reset interpreter
 */
extern void BKInterpreterReset (BKInterpreter * interpreter);

#endif /* !_BK_INTERPRETER_H_  */
