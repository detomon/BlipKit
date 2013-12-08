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
%InstructionsEnumItems%
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
