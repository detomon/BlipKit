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

#include "BKInterpreter.h"
#include "BKTone.h"

enum
{
	BKInterpreterFlagHasAttackEvent = 1 << 0,
	BKInterpreterFlagHasArpeggio    = 1 << 1,
};

enum {
	BKIntrEventStep    = 1 << 0,
	BKIntrEventAttack  = 1 << 1,
	BKIntrEventRelease = 1 << 2,
	BKIntrEventMute    = 1 << 3,
};

static BKTickEvent * BKInterpreterEventGet (BKInterpreter * interpreter, BKInt eventsMaks)
{
	BKTickEvent * tickEvent;

	for (BKInt i = 0; i < interpreter -> numEvents; i ++) {
		tickEvent = & interpreter -> events [i];

		if (tickEvent -> event & eventsMaks)
			return tickEvent;
	}

	return NULL;
}

static void BKInterpreterEventsUnset (BKInterpreter * interpreter, BKInt eventMask)
{
	BKInt i;
	BKSize size;
	BKTickEvent * tickEvent;

	if (eventMask & BKIntrEventAttack)
		interpreter -> flags &= ~BKInterpreterFlagHasAttackEvent;

	// remove events
	for (BKInt i = 0; i < interpreter -> numEvents;) {
		tickEvent = & interpreter -> events [i];

		if (tickEvent -> event & eventMask) {
			size = sizeof (BKTickEvent) * (interpreter -> numEvents - i - 1);
			memmove (tickEvent,	& tickEvent [1], size);
			interpreter -> numEvents --;
		}
		else {
			i ++;
		}
	}
}

static BKInt BKInterpreterEventSet (BKInterpreter * interpreter, BKInt event, BKInt ticks)
{
	BKTickEvent * tickEvent;

	if (ticks == 0) {
		BKInterpreterEventsUnset (interpreter, event);
		return 0;
	}

	tickEvent = BKInterpreterEventGet (interpreter, event);

	if (tickEvent == NULL) {
		if (interpreter -> numEvents < BK_INTR_MAX_EVENTS) {
			tickEvent = & interpreter -> events [interpreter -> numEvents];
			interpreter -> numEvents ++;
		}
		else {
			return -1;
		}
	}

	switch (event) {
		case BKIntrEventAttack: {
			interpreter -> flags |= BKInterpreterFlagHasAttackEvent;
			break;
		}
		case BKIntrEventStep: {
			// other events can't happen after step event
			for (BKInt i = 0; i < interpreter -> numEvents; i ++) {
				tickEvent = & interpreter -> events [i];

				if (tickEvent -> ticks > ticks)
					tickEvent -> ticks = ticks;
			}
			break;
		}
	}

	tickEvent -> event = event;
	tickEvent -> ticks = ticks;

	return 0;
}

static BKTickEvent * BKInterpreterEventGetNext (BKInterpreter * interpreter)
{
	BKInt i;
	BKSize size;
	BKInt ticks = BK_INT_MAX;
	BKTickEvent * tickEvent, * nextEvent = NULL;

	for (BKInt i = 0; i < interpreter -> numEvents; i ++) {
		tickEvent = & interpreter -> events [i];

		if (tickEvent -> ticks < ticks) {
			ticks     = tickEvent -> ticks;
			nextEvent = tickEvent;
		}
	}

	return nextEvent;
}

static void BKInterpreterEventsAdvance (BKInterpreter * interpreter, BKInt ticks)
{
	BKSize size;
	BKTickEvent * tickEvent;

	for (BKInt i = 0; i < interpreter -> numEvents; i ++) {
		tickEvent = & interpreter -> events [i];

		if (tickEvent -> ticks > 0)
			tickEvent -> ticks -= ticks;
	}
}

BKInt BKInterpreterTrackAdvance (BKInterpreter * interpreter, BKTrack * track)
{
	BKInt         command;
	BKInt         value0;
	BKInt         numSteps = 1;
	BKInt         run = 1;
	BKInt       * opcode;
	BKTickEvent * tickEvent;

	opcode   = interpreter -> opcodePtr;
	numSteps = interpreter -> numSteps;

	if (numSteps) {
		BKInterpreterEventsAdvance (interpreter, numSteps);

		do {
			tickEvent = BKInterpreterEventGetNext (interpreter);

			if (tickEvent) {
				numSteps = tickEvent -> ticks;

				if (tickEvent -> ticks <= 0) {
					switch (tickEvent -> event) {
						case BKIntrEventStep: {
							// do nothing
							break;
						}
						case BKIntrEventAttack: {
							BKTrackSetAttr (track, BK_NOTE, interpreter -> nextNote);

							if (interpreter -> flags & BKInterpreterFlagHasArpeggio)
								BKTrackSetPtr (track, BK_ARPEGGIO, interpreter -> nextArpeggio);
							break;
						}
						case BKIntrEventRelease: {
							BKTrackSetAttr (track, BK_NOTE, BK_NOTE_RELEASE);
							BKTrackSetPtr (track, BK_ARPEGGIO, NULL);
							break;
						}
						case BKIntrEventMute: {
							BKTrackSetAttr (track, BK_NOTE, BK_NOTE_MUTE);
							break;
						}
					}

					BKInterpreterEventSet (interpreter, tickEvent -> event, 0);
				}
				else {
					break;
				}
			}
			else {
				numSteps = 0;
			}
		}
		while (tickEvent);

		if (numSteps) {
			interpreter -> numSteps = numSteps;
			return numSteps;
		}
	}

	do {
		command = * (opcode ++);

		switch (command) {
			case BKIntrAttack: {
				value0 = * (opcode ++);

				if (interpreter -> flags & BKInterpreterFlagHasAttackEvent) {
					interpreter -> nextNote = value0;
				}
				else {
					BKTrackSetAttr (track, BK_NOTE, value0);
				}

				break;
			}
			case BKIntrAttackTicks: {
				value0 = * (opcode ++);
				BKInterpreterEventSet (interpreter, BKIntrEventAttack, value0);
				break;
			}
			case BKIntrArpeggio: {
				value0 = * (opcode);

				if (value0) {
					interpreter -> flags |= BKInterpreterFlagHasArpeggio;
				} else {
					interpreter -> flags &= ~BKInterpreterFlagHasArpeggio;
				}

				if (interpreter -> flags & BKInterpreterFlagHasAttackEvent) {
					memcpy (interpreter -> nextArpeggio, opcode, sizeof (BKInt) * (1 + value0));
				}
				else {
					BKTrackSetPtr (track, BK_ARPEGGIO, opcode);
				}

				opcode += 1 + value0;
				break;
			}
			case BKIntrArpeggioSpeed: {
				value0 = * (opcode ++);
				BKTrackSetAttr (track, BK_ARPEGGIO_DIVIDER, value0);
				break;
			}
			case BKIntrRelease: {
				BKInterpreterEventSet (interpreter, BKIntrEventRelease | BKIntrEventMute, 0);
				BKTrackSetAttr (track, BK_NOTE, BK_NOTE_RELEASE);
				break;
			}
			case BKIntrReleaseTicks: {
				value0 = * (opcode ++);
				BKInterpreterEventSet (interpreter, BKIntrEventMute, 0);
				BKInterpreterEventSet (interpreter, BKIntrEventRelease, value0);
				break;
			}
			case BKIntrMute: {
				BKInterpreterEventSet (interpreter, BKIntrEventRelease | BKIntrEventMute, 0);
				BKTrackSetAttr (track, BK_NOTE, BK_NOTE_MUTE);
				break;
			}
			case BKIntrMuteTicks: {
				value0 = * (opcode ++);
				BKInterpreterEventSet (interpreter, BKIntrEventRelease, 0);
				BKInterpreterEventSet (interpreter, BKIntrEventMute, value0);
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
				BKTrackSetAttr (track, BK_PITCH, value0);
				break;
			}
			case BKIntrTicks: {
				value0 = * (opcode ++);
				BKInterpreterEventSet (interpreter, BKIntrEventStep, value0);
				run = 0;
				break;
			}
			case BKIntrStep: {
				value0 = * (opcode ++);
				BKInterpreterEventSet (interpreter, BKIntrEventStep, value0 * interpreter -> stepTickCount);
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
				if (value0 & BK_INTR_CUSTOM_WAVEFOMR_FLAG) {
					value0 &= ~BK_INTR_CUSTOM_WAVEFOMR_FLAG;
					BKTrackSetPtr (track, BK_WAVEFORM, value0 > -1 ? interpreter -> waveforms [value0] : NULL);
				} else {
					BKTrackSetAttr (track, BK_WAVEFORM, value0);
				}
				break;
			}
			case BKIntrSample: {
				value0 = * (opcode ++);
				BKTrackSetPtr (track, BK_SAMPLE, value0 > -1 ? interpreter -> samples [value0] : NULL);
				break;
			}
			case BKIntrSampleRepeat: {
				value0 = * (opcode ++);
				BKTrackSetAttr (track, BK_SAMPLE_REPEAT, value0);
				break;
			}
			case BKIntrReturn: {
				if (interpreter -> stackPtr > interpreter -> stack) {
					value0 = * (-- interpreter -> stackPtr);
					opcode = & interpreter -> opcode [value0];
				}
				break;
			}
			case BKIntrCall: {
				value0 = * (opcode ++);

				if (interpreter -> stackPtr < interpreter -> stackEnd) {
					* (interpreter -> stackPtr ++) = opcode - interpreter -> opcode;
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
				BKInterpreterEventSet (interpreter, BKIntrEventMute, BK_INT_MAX);
				opcode --; // Repeat command forever
				run = 0;
				break;
			}
		}
	}
	while (run);

	numSteps  = 1;  // default steps
	tickEvent = BKInterpreterEventGetNext (interpreter);

	if (tickEvent)
		numSteps = tickEvent -> ticks;

	interpreter -> numSteps  = numSteps;
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
	interpreter -> flags     = 0;
	interpreter -> numSteps  = 0;
	interpreter -> opcodePtr = interpreter -> opcode;
	interpreter -> stackPtr  = interpreter -> stack;
	interpreter -> numEvents = 0;
}
