#include <stdio.h>
#include "BKInstruction.h"

int main (int argc, char const * argv [])
{
	BKInstructionDef const * def;

	def = BKInstructionLookupByValue (17);

	if (def)
		printf ("%s %d\n", def -> name, def -> value);

	return 0;
}
