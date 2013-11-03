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

/**
 * Created with Instructions/Generate.pl
 * Run script to recreate the header and source file
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
	BK_INSTR_End               = 0,
	BK_INSTR_GroupBegin        = 1,
	BK_INSTR_GroupEnd          = 2,
	BK_INSTR_Integer           = 3,
	BK_INSTR_String            = 4,
	BK_INSTR_Data              = 5,
	BK_INSTR_Attack            = 21,
	BK_INSTR_Arpeggio          = 22,
	BK_INSTR_ArpeggioSpeed     = 23,
	BK_INSTR_Release           = 24,
	BK_INSTR_StepTicks         = 25,
	BK_INSTR_Mute              = 26,
	BK_INSTR_MuteTicks         = 27,
	BK_INSTR_Volume            = 28,
	BK_INSTR_Panning           = 29,
	BK_INSTR_Pitch             = 30,
	BK_INSTR_MasterVolume      = 31,
	BK_INSTR_Step              = 32,
	BK_INSTR_Effect            = 33,
	BK_INSTR_DutyCycle         = 34,
	BK_INSTR_PhaseWrap         = 35,
	BK_INSTR_Instrument        = 36,
	BK_INSTR_Waveform          = 37,
	BK_INSTR_Group             = 38,
	BK_INSTR_InstrumentGroup   = 39,
	BK_INSTR_WaveformGroup     = 40,
	BK_INSTR_TrackGroup        = 41,
	BK_INSTR_SequenceVolume    = 42,
	BK_INSTR_SequencePanning   = 43,
	BK_INSTR_SequenceArpeggio  = 44,
	BK_INSTR_SequenceDutyCycle = 45,
	BK_INSTR_Jump              = 60,
	BK_INSTR_Call              = 61,
	BK_INSTR_Return            = 62,
	BK_INSTR_Exit              = 63,
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
