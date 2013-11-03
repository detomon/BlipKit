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

#include <stdio.h>
#include <unistd.h>
#include "BKBFMWriter.h"

static void printBuffer (BKByteBuffer * buffer)
{
	BKSize size;
	char data [4096];

	do {
		size = BKByteBufferReadBytes (buffer, data, sizeof (data));

		if (size < 0)
			return;

		write (STDOUT_FILENO, data, size);
	}
	while (size);

	return;
}

int main (int argc, const char * argv [])
{
	BKBFMWriter writer;
	BKBFMToken  token;

	BKBFMWriterInit (& writer, BKWriterFormatBinary);

	token.type = BK_INSTR_GroupBegin;
	BKBFMWriterPutToken (& writer, & token);

		token.type = BK_INSTR_WaveformGroup;
		BKBFMWriterPutToken (& writer, & token);

			token.type = BK_INSTR_SequenceVolume;
			BKBFMWriterPutToken (& writer, & token);

			for (int i = 0; i < 16; i ++) {
				token.type = BK_INSTR_Integer;
				token.ival = i;
				BKBFMWriterPutToken (& writer, & token);
			}

		token.type = BK_INSTR_GroupEnd;
		BKBFMWriterPutToken (& writer, & token);

		token.type = BK_INSTR_WaveformGroup;
		BKBFMWriterPutToken (& writer, & token);

			token.type = BK_INSTR_GroupBegin;
			BKBFMWriterPutToken (& writer, & token);

			token.type = BK_INSTR_String;
			token.sval = "square";
			token.len  = 6;
			BKBFMWriterPutToken (& writer, & token);

			token.type = BK_INSTR_Attack;
			BKBFMWriterPutToken (& writer, & token);

			token.type = BK_INSTR_Integer;
			token.ival = 15;
			BKBFMWriterPutToken (& writer, & token);

		token.type = BK_INSTR_GroupEnd;
		BKBFMWriterPutToken (& writer, & token);

	token.type = BK_INSTR_End;
	BKBFMWriterPutToken (& writer, & token);

	printBuffer (& writer.buffer);

	//printf ("%ld\n", BKByteBufferGetSize (& writer.buffer));

    return 0;
}
