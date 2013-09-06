#include "BKInstruction.h"

#define BK_INSTRUCTIONS_COUNT 24

static BKInstructionDef const BKInstructionNames [BK_INSTRUCTIONS_COUNT] =
{
	{"a",    BK_INSTR_Attack},
	{"as",   BK_INSTR_ArpeggioSpeed},
	{"dc",   BK_INSTR_DutyCycle},
	{"e",    BK_INSTR_Effect},
	{"g",    BK_INSTR_Group},
	{"i",    BK_INSTR_Instrument},
	{"inst", BK_INSTR_InstrumentGroup},
	{"m",    BK_INSTR_Mute},
	{"ms",   BK_INSTR_MasterVolume},
	{"mt",   BK_INSTR_MuteTicks},
	{"p",    BK_INSTR_Panning},
	{"pt",   BK_INSTR_Pitch},
	{"pw",   BK_INSTR_PhaseWrap},
	{"r",    BK_INSTR_Release},
	{"s",    BK_INSTR_Step},
	{"seqa", BK_INSTR_SequenceArpeggio},
	{"seqd", BK_INSTR_SequenceDutyCycle},
	{"seqp", BK_INSTR_SequencePanning},
	{"seqv", BK_INSTR_SequenceVolume},
	{"st",   BK_INSTR_StepTicks},
	{"trck", BK_INSTR_TrackGroup},
	{"v",    BK_INSTR_Volume},
	{"w",    BK_INSTR_Waveform},
	{"wave", BK_INSTR_WaveformGroup},
};

static BKInstructionDef const BKInstructionValues [BK_INSTRUCTIONS_COUNT] =
{
	{"a",    BK_INSTR_Attack},
	{"as",   BK_INSTR_ArpeggioSpeed},
	{"r",    BK_INSTR_Release},
	{"st",   BK_INSTR_StepTicks},
	{"m",    BK_INSTR_Mute},
	{"mt",   BK_INSTR_MuteTicks},
	{"v",    BK_INSTR_Volume},
	{"p",    BK_INSTR_Panning},
	{"pt",   BK_INSTR_Pitch},
	{"ms",   BK_INSTR_MasterVolume},
	{"s",    BK_INSTR_Step},
	{"e",    BK_INSTR_Effect},
	{"dc",   BK_INSTR_DutyCycle},
	{"pw",   BK_INSTR_PhaseWrap},
	{"i",    BK_INSTR_Instrument},
	{"w",    BK_INSTR_Waveform},
	{"g",    BK_INSTR_Group},
	{"inst", BK_INSTR_InstrumentGroup},
	{"wave", BK_INSTR_WaveformGroup},
	{"trck", BK_INSTR_TrackGroup},
	{"seqv", BK_INSTR_SequenceVolume},
	{"seqp", BK_INSTR_SequencePanning},
	{"seqa", BK_INSTR_SequenceArpeggio},
	{"seqd", BK_INSTR_SequenceDutyCycle},
};

static int BKInstructionCompareName (char const * name, BKInstructionDef const * item)
{
	return strcmp (name, item -> name);
}

static int BKInstructionCompareValue (BKInt const * value, BKInstructionDef const * item)
{
	return BKCmp (* value, item -> value);
}

BKInstructionDef const * BKInstructionLookupByName (char const * name)
{
	BKInstructionDef const * item;

	item = bsearch (
		name,
		BKInstructionNames,
		BK_INSTRUCTIONS_COUNT,
		sizeof (BKInstructionDef),
		(void *) BKInstructionCompareName
	);

	return item;
}

BKInstructionDef const * BKInstructionLookupByValue (BKInt value)
{
	BKInstructionDef const * item;

	item = bsearch (
		& value,
		BKInstructionValues,
		BK_INSTRUCTIONS_COUNT,
		sizeof (BKInstructionDef),
		(void *) BKInstructionCompareValue
	);

	return item;
}
