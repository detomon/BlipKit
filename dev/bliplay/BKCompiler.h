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

#ifndef _BK_COMPILER_H_
#define _BK_COMPILER_H_

#include "BKBlipReader.h"
#include "BKInterpreter.h"

typedef struct BKCompiler BKCompiler;

struct BKCompiler
{
	BKUInt          flags;
	BKInt         * cmds;
	BKInt         * groupCmds;
	BKInt         * groupOffsets;
	BKInt        ** activeCmdList;
	BKUInt          groupLevel;
	BKInterpreter * interpreter;
};

/**
 * Initialize compiler
 */
extern BKInt BKCompilerInit (BKCompiler * compiler);

/**
 * Dispose compiler
 */
extern void BKCompilerDispose (BKCompiler * compiler);

/**
 * Push command
 */
extern BKInt BKCompilerPushCommand (BKCompiler * compiler, BKBlipCommand * item);

/**
 * Compile commands and initialize interpreter
 */
extern BKInt BKCompilerTerminate (BKCompiler * compiler, BKInterpreter * interpreter);

/**
 * Compile all commands from parser and terminate compiler
 */
extern BKInt BKCompilerCompile (BKCompiler * compiler, BKInterpreter * interpreter, BKBlipReader * parser);

#endif /* ! _BK_COMPILER_H_ */
