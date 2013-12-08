/**
 * Created with Instructions/Generate.pl
 * Run script to recreate the header and source file
 */

#include "BKInstruction.h"

#define BK_INSTRUCTIONS_COUNT %InstructionsCount%

static BKInstructionDef const BKInstructionNames [BK_INSTRUCTIONS_COUNT] =
{
%InstructionsNamesList%
};

static BKInstructionDef const BKInstructionValues [BK_INSTRUCTIONS_COUNT] =
{
%InstructionsValueList%
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
