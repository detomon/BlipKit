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

#include <stdio.h>
#include "BKTone.h"
#include "BKCompiler.h"
#include "item_list.h"

enum
{
	BKCompilerArpeggioFlag = 1 << 0,
};

/**
 * Used for lookup tables to assign a string to a value
 */
typedef struct
{
	char * name;
	BKInt  value;
} const strval;

/**
 * Note lookup table used for `bsearch`
 */
static strval notes [] =
{
	{"a",  9},
	{"a#", 10},
	{"b",  11},
	{"c",  0},
	{"c#", 1},
	{"d",  2},
	{"d#", 3},
	{"e",  4},
	{"f",  5},
	{"f#", 6},
	{"g",  7},
	{"g#", 8},
	{"h",  11},
};

#define NUM_NOTES (sizeof (notes) / sizeof (strval))

/**
 * Command lookup table used for `bsearch`
 */
static strval commands [] =
{
	{"a",  BKIntrAttack},
	{"as", BKIntrArpeggioSpeed},
	{"at", BKIntrAttackTicks},
	{"d",  BKIntrSample},
	{"dc", BKIntrDutyCycle},
	{"dr", BKIntrSampleRepeat},
	{"e",  BKIntrEffect},
	{"g",  BKIntrGroup},
	{"i",  BKIntrInstrument},
	{"m",  BKIntrMute},
	{"mt", BKIntrMuteTicks},
	{"p",  BKIntrPanning},
	{"pt", BKIntrPitch},
	{"pw", BKIntrPhaseWrap},
	{"r",  BKIntrRelease},
	{"rt", BKIntrReleaseTicks},
	{"s",  BKIntrStep},
	{"st", BKIntrStepTicks},
	{"t",  BKIntrTicks},
	{"v",  BKIntrVolume},
	{"vm", BKIntrMasterVolume},
	{"w",  BKIntrWaveform},
	{"x",  BKIntrEnd},
};

#define NUM_COMMANDS (sizeof (commands) / sizeof (strval))

/**
 * Effect lookup table used for `bsearch`
 */
static strval effects [] =
{
	{"pr", BK_EFFECT_PORTAMENTO},
	{"ps", BK_EFFECT_PANNING_SLIDE},
	{"tr", BK_EFFECT_TREMOLO},
	{"vb", BK_EFFECT_VIBRATO},
	{"vs", BK_EFFECT_VOLUME_SLIDE},
};

#define NUM_EFFECTS (sizeof (effects) / sizeof (strval))

/**
 * Waveform lookup table used for `bsearch`
 */
static strval waveforms [] =
{
	{"noise",    BK_NOISE},
	{"sawtooth", BK_SAWTOOTH},
	{"square",   BK_SQUARE},
	{"triangle", BK_TRIANGLE},
};

#define NUM_WAVEFORMS (sizeof (waveforms) / sizeof (strval))
/**
 * Compare name of command with string
 * Used as callback for `bsearch`
 */
static int cmdcmp (char const * name, strval const * item)
{
	return strcmp (name, item -> name);
}

/**
 * Convert string to signed integer like `atoi`
 * If string is NULL the alternative value is returned
 */
static int atoix (char const * str, int alt)
{
	return str ? atoi (str) : alt;
}

/**
 * Compare two strings like `strcmp`
 * If one of the strings is NULL -1 is returned
 */
static int strcmpx (char const * a, char const * b)
{
	return (a && b) ? strcmp (a, b) : -1;
}

BKInt BKCompilerInit (BKCompiler * compiler)
{
	memset (compiler, 0, sizeof (BKCompiler));

	compiler -> flags &= ~BKCompilerArpeggioFlag;
	compiler -> activeCmdList = & compiler -> cmds;

	return 0;
}

void BKCompilerDispose (BKCompiler * compiler)
{
	item_list_free (& compiler -> cmds);
	item_list_free (& compiler -> groupCmds);
	item_list_free (& compiler -> groupOffsets);

	memset (compiler, 0, sizeof (BKCompiler));
}

/**
 * Lookup index from name
 */
static BKInt BKCompilerLookupValue (strval table [], size_t size, char const * name)
{
	strval * item;
	BKUInt   index = -1;

	item = bsearch (name, table, size, sizeof (strval), (void *) cmdcmp);

	if (item)
		index = item -> value;

	return index;
}


/**
 * Lookup note index from name
 * Index range from 0 to 11
 */
static BKInt BKCompilerLookupNote (char const * name)
{
	char     tone [3];
	BKUInt   octave = 0;
	BKUInt   value  = 0;
	strval * item;

	strcpy (tone, "");  // empty name
	sscanf (name, "%2[a-z#]%u", tone, & octave);  // scan string; d#3 => "d#", 3

	item = bsearch (tone, notes, NUM_NOTES, sizeof (strval), (void *) cmdcmp);

	if (item) {
		octave = BKClamp (octave, 0, 7);

		value = item -> value;
		value += octave * 12;
		value <<= BK_FINT20_SHIFT;
	}

	return value;
}

/**
 * Append commands from array
 */
static BKInt * BKCompilerCombineCmds (BKCompiler * compiler, BKInt * allCmds, BKInt * cmds)
{
	BKInt  cmd;
	BKUInt argCount, variable, value;
	BKUInt numCmd = item_list_length (cmds);

	for (BKInt * cmdPtr = cmds; cmdPtr < & cmds [numCmd];) {
		argCount = 0;
		variable = 0;

		cmd = * cmdPtr ++;

		switch (cmd) {
			case BKIntrAttack:        argCount = 1; break;
			case BKIntrAttackTicks:   argCount = 1; break;
			case BKIntrArpeggio:      variable = 1; break;
			case BKIntrArpeggioSpeed: argCount = 1; break;
			case BKIntrRelease:       argCount = 0; break;
			case BKIntrReleaseTicks:  argCount = 1; break;
			case BKIntrMute:          argCount = 0; break;
			case BKIntrMuteTicks:     argCount = 1; break;
			case BKIntrVolume:        argCount = 1; break;
			case BKIntrPanning:       argCount = 1; break;
			case BKIntrPitch:         argCount = 1; break;
			case BKIntrMasterVolume:  argCount = 1; break;
			case BKIntrStep:          argCount = 1; break;
			case BKIntrStepTicks:     argCount = 1; break;
			case BKIntrTicks:         argCount = 1; break;
			case BKIntrEffect:        argCount = 4; break;
			case BKIntrDutyCycle:     argCount = 1; break;
			case BKIntrPhaseWrap:     argCount = 1; break;
			case BKIntrInstrument:    argCount = 1; break;
			case BKIntrWaveform:      argCount = 1; break;
			case BKIntrSample:        argCount = 1; break;
			case BKIntrSampleRepeat:  argCount = 1; break;
			case BKIntrReturn:        argCount = 0; break;
			case BKIntrGroup:         argCount = 1; break;
			case BKIntrJump:          argCount = 1; break;
			case BKIntrEnd:           argCount = 0; break;
		}

		// command has variable number of arguments
		if (variable) {
			argCount = * cmdPtr ++;
			* allCmds ++ = cmd;
			* allCmds ++ = argCount;

			for (BKInt i = 0; i < argCount; i ++)
				* allCmds ++ = * cmdPtr ++;
		}
		// command is group jump
		else if (cmd == BKIntrGroup) {
			value = * cmdPtr ++;

			if (value < item_list_length (compiler -> groupOffsets)) {
				value = compiler -> groupOffsets [value] + item_list_length (compiler -> cmds);

				* allCmds ++ = BKIntrCall;
				* allCmds ++ = value;
			}
		}
		// other command
		else {
			* allCmds ++ = cmd;

			for (BKInt i = 0; i < argCount; i ++)
				* allCmds ++ = * cmdPtr ++;
		}
	}

	return allCmds;
}

/**
 * Combine commands in `cmds` and `groupCmds` into array `allCmds`
 */
static BKInt BKCompilerCombine (BKCompiler * compiler, BKInterpreter * interpreter)
{
	BKUInt  numCmds = 0;
	BKInt * cmds;

	compiler -> interpreter = interpreter;

	numCmds += item_list_length (compiler -> cmds);
	numCmds += item_list_length (compiler -> groupCmds);

	interpreter -> opcode = item_list_alloc (numCmds);

	if (interpreter -> opcode) {
		cmds = interpreter -> opcode;
		cmds = BKCompilerCombineCmds (compiler, cmds, compiler -> cmds);
		cmds = BKCompilerCombineCmds (compiler, cmds, compiler -> groupCmds);

		interpreter -> opcodePtr = interpreter -> opcode;
		interpreter -> stackPtr  = interpreter -> stack;
		interpreter -> stackEnd  = (void *) interpreter -> stack + sizeof (interpreter -> stack);

		return 0;
	}

	return -1;
}

BKInt BKCompilerPushCommand (BKCompiler * compiler, BKBlipCommand * instr)
{
	BKInt    value0, value1, arg1, arg2, arg3;
	BKInt ** cmds = compiler -> activeCmdList;
	strval * item;

	item = bsearch (instr -> name, commands, NUM_COMMANDS, sizeof (strval), (void *) cmdcmp);

	if (item == NULL)
		return 0;

	char const * arg0 = instr -> args [0].arg;

	switch (item -> value) {
		case BKIntrGroup: {
			// begin group
			if (strcmpx (arg0, "begin") == 0) {
				if (compiler -> groupLevel == 0) {
					item_list_add (& compiler -> groupOffsets, item_list_length (compiler -> groupCmds));
					compiler -> activeCmdList = & compiler -> groupCmds;
					compiler -> groupLevel ++;
				}
				else {
					return -1;
				}
			}
			// end group
			else if (strcmpx (arg0, "end") == 0) {
				if (compiler -> groupLevel >= 1) {
					item_list_add (cmds, BKIntrReturn);
					compiler -> activeCmdList = & compiler -> cmds;
					compiler -> groupLevel --;
				}
				else {
					return -1;
				}

				return 0;
			}
			// play group
			else {
				item_list_add (cmds, item -> value);
				item_list_add (cmds, atoix (arg0, 0));
			}

			break;
		}
		case BKIntrAttack: {
			value0 = BKCompilerLookupNote (arg0);

			if (value0 > -1) {
				item_list_add (cmds, item -> value);
				item_list_add (cmds, value0);

				// set arpeggio
				if (instr -> argCount > 1) {
					instr -> argCount = BKMin (instr -> argCount, BK_MAX_ARPEGGIO);

					item_list_add (cmds, BKIntrArpeggio);
					item_list_add (cmds, (BKInt) instr -> argCount);

					for (BKInt j = 0; j < instr -> argCount; j ++) {
						value1 = BKCompilerLookupNote (instr -> args [j].arg);

						if (value1 < 0)
							value1 = 0;

						item_list_add (cmds, value1 - value0);
					}

					compiler -> flags |= BKCompilerArpeggioFlag;
				}
				// disable arpeggio
				else if (compiler -> flags & BKCompilerArpeggioFlag) {
					item_list_add (cmds, BKIntrArpeggio);
					item_list_add (cmds, 0);
					compiler -> flags &= ~BKCompilerArpeggioFlag;
				}
			}

			break;
		}
		case BKIntrRelease:
		case BKIntrMute: {
			item_list_add (cmds, item -> value);

			// deactivate arpeggio
			if (compiler -> flags & BKCompilerArpeggioFlag) {
				item_list_add (cmds, BKIntrArpeggio);
				item_list_add (cmds, 0);
				compiler -> flags &= ~BKCompilerArpeggioFlag;
			}

			break;
		}
		case BKIntrAttackTicks:
		case BKIntrReleaseTicks:
		case BKIntrMuteTicks: {
			value0 = atoix (arg0, 0);

			item_list_add (cmds, item -> value);
			item_list_add (cmds, value0);
			break;
		}
		case BKIntrVolume: {
			item_list_add (cmds, item -> value);
			item_list_add (cmds, atoix (arg0, 0) * (BK_MAX_VOLUME / 255));
			break;
		}
		case BKIntrMasterVolume: {
			item_list_add (cmds, item -> value);
			item_list_add (cmds, atoix (arg0, 0) * (BK_MAX_VOLUME / 255));
			break;
		}
		case BKIntrStep:
		case BKIntrTicks: {
			value0 = atoix (arg0, 0);

			if (value0 == 0)
				return 0;

			item_list_add (cmds, item -> value);
			item_list_add (cmds, value0);
			break;
		}
		case BKIntrStepTicks: {
			value0 = atoix (arg0, 0);

			item_list_add (cmds, item -> value);
			item_list_add (cmds, value0);
			break;
		}
		case BKIntrEffect: {
			value0 = BKCompilerLookupValue (effects, NUM_EFFECTS, arg0);

			if (value0 > -1) {
				arg1 = atoix (instr -> args [1].arg, 0);
				arg2 = atoix (instr -> args [2].arg, 0);
				arg3 = atoix (instr -> args [3].arg, 0);

				switch (value0) {
					case BK_EFFECT_TREMOLO: {
						arg2 *= (BK_MAX_VOLUME / 255);
						break;
					}
					case BK_EFFECT_VIBRATO: {
						arg2 *= (BK_FINT20_UNIT / 100);

						break;
					}
				}

				item_list_add (cmds, item -> value);
				item_list_add (cmds, value0);
				item_list_add (cmds, arg1);
				item_list_add (cmds, arg2);
				item_list_add (cmds, arg3);
			}

			break;
		}
		case BKIntrDutyCycle: {
			item_list_add (cmds, item -> value);
			item_list_add (cmds, atoix (arg0, 0));
			break;
		}
		case BKIntrPhaseWrap: {
			item_list_add (cmds, item -> value);
			item_list_add (cmds, atoix (arg0, 0));
			break;
		}
		case BKIntrPanning: {
			item_list_add (cmds, item -> value);
			item_list_add (cmds, atoix (arg0, 0) * (BK_MAX_VOLUME / 255));
			break;
		}
		case BKIntrPitch: {
			item_list_add (cmds, item -> value);
			item_list_add (cmds, atoix (arg0, 0) * (BK_FINT20_UNIT / 100));
			break;
		}
		case BKIntrInstrument: {
			item_list_add (cmds, item -> value);
			item_list_add (cmds, atoix (arg0, -1));
			break;
		}
		case BKIntrWaveform: {
			item_list_add (cmds, item -> value);

			item = bsearch (arg0, waveforms, NUM_WAVEFORMS, sizeof (strval), (void *) cmdcmp);

			// custom waveform
			if (item) {
				item_list_add (cmds, item -> value);
			}
			else {
				value0 = atoix (arg0, 0);
				value0 |= BK_INTR_CUSTOM_WAVEFOMR_FLAG;
				item_list_add (cmds, value0);
			}
			break;
		}
		case BKIntrSample: {
			item_list_add (cmds, item -> value);
			item_list_add (cmds, atoix (arg0, -1));
			break;
		}
		case BKIntrSampleRepeat: {
			item_list_add (cmds, item -> value);
			item_list_add (cmds, atoix (arg0, 0));
			break;
		}
		case BKIntrArpeggioSpeed: {
			item_list_add (cmds, item -> value);
			item_list_add (cmds, BKMax (atoix (arg0, 0), 1));
			break;
		}
		case BKIntrEnd: {
			item_list_add (cmds, item -> value);
			break;
		}
		default:
			return -1;
			break;
	}

	return 0;
}

BKInt BKCompilerTerminate (BKCompiler * compiler, BKInterpreter * interpreter)
{
	memset (interpreter, 0, sizeof (BKInterpreter));

	// Add return command if group is not terminated
	if (compiler -> groupLevel > 0)
		item_list_add (& compiler -> groupCmds, BKIntrReturn);

	// add repeat command
	item_list_add (& compiler -> cmds, BKIntrJump);
	item_list_add (& compiler -> cmds, 0);

	// combine commands and group commands into one array
	if (BKCompilerCombine (compiler, interpreter) < 0)
		return -1;

	return 0;
}

static BKInt BKCompilerReadCommands (BKCompiler * compiler, BKBlipReader * parser)
{
	BKBlipCommand item;

	while (BKBlipReaderNextCommand (parser, & item)) {
		if (BKCompilerPushCommand (compiler, & item) < 0)
			return -1;
	}

	return 0;
}

BKInt BKCompilerCompile (BKCompiler * compiler, BKInterpreter * interpreter, BKBlipReader * parser)
{
	if (BKCompilerReadCommands (compiler, parser) < 0)
		return -1;

	BKCompilerTerminate (compiler, interpreter);

	return 0;
}
