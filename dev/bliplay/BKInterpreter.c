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

#include "BKInterpreter.h"
#include "BKTone.h"

BKInt BKInterpreterTrackApplyNextStep (BKInterpreter * interpreter, BKTrack * track)
{
	BKInt   command;
	BKInt   value0;
	BKInt   numSteps = 0;
	BKInt   run = 1;
	BKInt * opcode = interpreter -> opcodePtr;
	
	if (interpreter -> noteStepTickCount) {
		numSteps = interpreter -> noteStepTickCount;
		interpreter -> noteStepTickCount = 0;

		if (interpreter -> flags & BKIntrReleaseFlag) {
			BKTrackSetAttr (track, BK_NOTE, BK_NOTE_RELEASE);
			interpreter -> flags &= ~BKIntrReleaseFlag;
		}
		
		return numSteps;
	}
	
	do {
		command = * (opcode ++);
		
		switch (command) {
			case BKIntrAttack: {
				value0 = * (opcode ++);
				value0 += interpreter -> pitch;
				BKTrackSetAttr (track, BK_NOTE, value0);
				break;
			}
			case BKIntrArpeggio: {
				value0 = * (opcode);
				BKTrackSetPtr (track, BK_ARPEGGIO, opcode);
				opcode += value0 + 1;
				break;
			}
			case BKIntrArpeggioSpeed: {
				value0 = * (opcode ++);
				BKTrackSetAttr (track, BK_ARPEGGIO_DIVIDER, value0);
				break;
			}
			case BKIntrRelease: {
				interpreter -> muteTickCount = 0;
				BKTrackSetAttr (track, BK_NOTE, BK_NOTE_RELEASE);
				break;
			}
			case BKIntrMute: {
				interpreter -> muteTickCount = 0;
				BKTrackSetAttr (track, BK_NOTE, BK_NOTE_MUTE);
				break;
			}
			case BKIntrMuteTicks: {
				value0 = * (opcode ++);
				interpreter -> muteTickCount = value0;
				break;
			}
			case BKIntrVolume: {
				value0 = * (opcode ++);
				BKTrackSetAttr (track, BK_VOLUME, value0);
				break;
			}
			case BKIntrMasterVolume: {
				value0 = * (opcode ++);
				BKTrackSetAttr (track, BK_MASTER_VOLUME, value0);
				break;
			}
			case BKIntrPanning: {
				value0 = * (opcode ++);
				BKTrackSetAttr (track, BK_PANNING, value0);
				break;
			}
			case BKIntrPitch: {
				value0 = * (opcode ++);
				interpreter -> pitch = value0;
				break;
			}
			case BKIntrStep: {
				value0 = * (opcode ++);
				numSteps = value0 * interpreter -> stepTickCount;
				interpreter -> noteStepTickCount = value0 * interpreter -> stepTickCount;
				run = 0;
				break;
			}
			case BKIntrStepTicks: {
				value0 = * (opcode ++);
				interpreter -> stepTickCount = value0;
				break;
			}
			case BKIntrEffect: {
				value0 = * (opcode ++);
				BKTrackSetEffect (track, value0, opcode, sizeof (BKInt [3]));
				opcode += 3;
				break;
			}
			case BKIntrDutyCycle: {
				value0 = * (opcode ++);
				BKTrackSetAttr (track, BK_DUTY_CYCLE, value0);
				break;
			}
			case BKIntrPhaseWrap: {
				value0 = * (opcode ++);
				BKTrackSetAttr (track, BK_PHASE_WRAP, value0);
				break;
			}
			case BKIntrInstrument: {
				value0 = * (opcode ++);
				BKTrackSetPtr (track, BK_INSTRUMENT, value0 > -1 ? interpreter -> instruments [value0] : NULL);
				break;
			}
			case BKIntrWaveform: {
				value0 = * (opcode ++);
				if (value0 & BK_CUSTOM_WAVEFOMR_FLAG) {
					value0 &= ~BK_CUSTOM_WAVEFOMR_FLAG;
					BKTrackSetPtr (track, BK_WAVEFORM, value0 > -1 ? interpreter -> waveforms [value0] : NULL);
				} else {
					BKTrackSetAttr (track, BK_WAVEFORM, value0);
				}
				break;
			}
			case BKIntrReturn: {
				if (interpreter -> stackPtr > interpreter -> stack) {
					opcode = * (-- interpreter -> stackPtr);
				}
				break;
			}
			case BKIntrCall: {
				value0 = * (opcode ++);
				
				if (interpreter -> stackPtr < interpreter -> stackEnd) {
					* (interpreter -> stackPtr ++) = opcode;
					opcode = & interpreter -> opcode [value0];
				}
				break;
			}
			case BKIntrJump: {
				value0 = * (opcode ++);
				opcode = & interpreter -> opcode [value0];
				break;
			}
			case BKIntrEnd: {
				BKTrackSetAttr (track, BK_MUTE, 1);
				numSteps = 256;
				opcode --; // Repeat command forever
				run = 0;
				break;
			}
		}
	}
	while (run);

	if (interpreter -> muteTickCount) {
		if (interpreter -> muteTickCount > interpreter -> noteStepTickCount)
			interpreter -> muteTickCount = interpreter -> noteStepTickCount;

		numSteps = interpreter -> muteTickCount;
		interpreter -> noteStepTickCount -= interpreter -> muteTickCount;
		interpreter -> muteTickCount = 0;
		interpreter -> flags |= BKIntrReleaseFlag;
	}
	else {
		interpreter -> noteStepTickCount = 0;
		interpreter -> flags &= ~BKIntrReleaseFlag;
	}

	interpreter -> opcodePtr = opcode;

	return numSteps;
}

void BKInterpreterDispose (BKInterpreter * interpreter)
{
	item_list_free (& interpreter -> opcode);

	memset (interpreter, 0, sizeof (BKInterpreter));
}

void BKInterpreterReset (BKInterpreter * interpreter)
{
	interpreter -> flags             = 0;
	interpreter -> pitch             = 0;
	interpreter -> muteTickCount     = 0;
	interpreter -> opcodePtr         = interpreter -> opcode;
	interpreter -> stackPtr          = interpreter -> stack;
}
