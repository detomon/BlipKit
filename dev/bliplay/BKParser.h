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

#ifndef _BK_PARSER_H_
#define _BK_PARSER_H_

#include "BKBase.h"

#define BK_INS_BUFFER_SIZE 32768
#define BK_ARG_BUFFER_SIZE 2048

typedef struct BKParser     BKParser;
typedef struct BKParserItem BKParserItem;

struct BKParser
{
	size_t       bytesLength;
	char const * bytes;
	char const * bytesPtr;
	char         buffer [BK_INS_BUFFER_SIZE];
	char       * bufferPtr;
	size_t       argCount;
	char       * args [BK_ARG_BUFFER_SIZE];
	size_t       argLengths [BK_ARG_BUFFER_SIZE];
};

struct BKParserItem
{
	size_t        nameLength;
	char const  * name;
	size_t        argCount;
	size_t      * argLengths;
	char const ** args;
};

/**
 * Initialize parser with bytes to parse
 */
extern BKInt BKParserInit (BKParser * parser, char const * bytes, size_t size);

/**
 * Dispose parser
 */
extern void BKParserDispose (BKParser * parser);

/**
 * Parse next instruction
 */
extern BKInt BKParserNextItem (BKParser * parser, BKParserItem * outItem);

#endif /* ! _BK_PARSER_H_ */
