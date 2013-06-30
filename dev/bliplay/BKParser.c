/**
 * Copyright (c) 2012 Simon Schoenenberger
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

#include <ctype.h>
#include "BKParser.h"

#define BK_SEPARATOR_CHAR ':'
#define BK_TERMINATE_CHAR ';'

BKInt BKParserInit (BKParser * parser, char const * bytes, size_t size)
{
	memset (parser, 0, sizeof (BKParser));
	
	parser -> bytesPtr    = bytes;
	parser -> bytes       = bytes;
	parser -> bytesLength = size;

	return 0;
}

void BKParserDispose (BKParser * parser)
{
	memset (parser, 0, sizeof (BKParser));
}

/**
 * Get next char but do not advance cursor
 */
static inline BKInt BKParserLookahead (BKParser * parser)
{
	return parser -> bytesPtr < & parser -> bytes [parser -> bytesLength] ? (* parser -> bytesPtr) : -1;
}

/**
 * Get next char and advance cursor
 */
static inline BKInt BKParserNext (BKParser * parser)
{
	return parser -> bytesPtr < & parser -> bytes [parser -> bytesLength] ? (* parser -> bytesPtr ++) : -1;
}

/**
 * Parse single item to terminating char
 */
static size_t BKParserParseItem (BKParser * parser)
{
	int c = 0;
	size_t length;
	char * buffer, * bufferPtr;
	
	// reset buffer cursor
	parser -> bufferPtr = parser -> buffer;
	
	// empty previous argument list
	memset (parser -> args, 0, parser -> argCount * sizeof (const char *));
	parser -> argCount = 0;
	
	do {
		// ignore whitespace from current position but do not read line ending
		for (int lookahead; isspace (lookahead = BKParserLookahead (parser)) && lookahead != BK_TERMINATE_CHAR;)
			c = BKParserNext (parser);

		buffer    = parser -> bufferPtr;
		bufferPtr = parser -> bufferPtr;

		// read token
		for (int lookahead; (lookahead = BKParserLookahead (parser)) != BK_TERMINATE_CHAR && lookahead != -1;) {
			// Read next char
			c = BKParserNext (parser);
			
			if (lookahead == BK_SEPARATOR_CHAR)
				break;
			
			// leave space for terminating '\0'
			if (bufferPtr < & parser -> buffer [BK_INS_BUFFER_SIZE - 1])
				* bufferPtr ++ = c;
		}
		
		// length of token
		length = bufferPtr - buffer;
		
		// token not empty
		if (length) {
			// terminate token
			* bufferPtr ++ = '\0';

			// has space for token
			if (parser -> argCount < BK_ARG_BUFFER_SIZE) {
				parser -> bufferPtr = bufferPtr;
				parser -> args [parser -> argCount] = buffer;
				parser -> argLengths [parser -> argCount] = length;
				parser -> argCount ++;
			}
		}
		
		// current line finished
		if (BKParserLookahead (parser) == BK_TERMINATE_CHAR) {
			c = BKParserNext (parser);
			break;
		}
	}
	while (BKParserLookahead (parser) != -1);
	
	return parser -> argCount;
}

BKInt BKParserNextItem (BKParser * parser, BKParserItem * outItem)
{
	// clear struct
	memset (outItem, 0, sizeof (* outItem));

	// has instruction
	if (BKParserParseItem (parser)) {
		outItem -> name       = parser -> args [0];
		outItem -> nameLength = parser -> argLengths [0];
		outItem -> args       = (char const **) & parser -> args [1];
		outItem -> argCount   = parser -> argCount - 1;
		outItem -> argLengths = & parser -> argLengths [1];
		
		return 1;
	}

	return 0;
}
