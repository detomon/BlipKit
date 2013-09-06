/**
 * Copyright (c) 2012-2013 Simon Schoenenberger
 * http://blipkit.monoxid.net/
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _BK_INSTRUCTION_H_
#define _BK_INSTRUCTION_H_

#include "BKBase.h"

typedef struct BKInstructionDef BKInstructionDef;

struct BKInstructionDef
{
	char const * name;
	BKInt value;
};

/**
 * instructions
 */
enum BKInstruction
{
	BK_INSTR_Attack            = 1,
	BK_INSTR_Arpeggio          = 2,
	BK_INSTR_ArpeggioSpeed     = 3,
	BK_INSTR_Release           = 4,
	BK_INSTR_StepTicks         = 5,
	BK_INSTR_Mute              = 6,
	BK_INSTR_MuteTicks         = 7,
	BK_INSTR_Volume            = 8,
	BK_INSTR_Panning           = 9,
	BK_INSTR_Pitch             = 10,
	BK_INSTR_MasterVolume      = 11,
	BK_INSTR_Step              = 12,
	BK_INSTR_Effect            = 13,
	BK_INSTR_DutyCycle         = 14,
	BK_INSTR_PhaseWrap         = 15,
	BK_INSTR_Instrument        = 16,
	BK_INSTR_Waveform          = 17,
	BK_INSTR_Group             = 18,
	BK_INSTR_InstrumentGroup   = 19,
	BK_INSTR_WaveformGroup     = 20,
	BK_INSTR_TrackGroup        = 21,
	BK_INSTR_SequenceVolume    = 22,
	BK_INSTR_SequencePanning   = 23,
	BK_INSTR_SequenceArpeggio  = 24,
	BK_INSTR_SequenceDutyCycle = 25,
	BK_INSTR_Jump              = 40,
	BK_INSTR_Call              = 41,
	BK_INSTR_Return            = 42,
	BK_INSTR_Exit              = 43,
};

/**
 * lookup instruction by name
 */
extern BKInstructionDef const * BKInstructionLookupByName (char const * name);

/**
 * lookup instruction by value
 */
extern BKInstructionDef const * BKInstructionLookupByValue (BKInt value);

#endif /* ! _BK_INSTRUCTION_H_ */
