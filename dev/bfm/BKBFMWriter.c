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
#include "BKBFMWriter.h"

typedef struct BKTokenDef BKTokenDef;

enum BKWriterFlag
{
	BKWriterFlagMagicWritten = 1 << 0,
	BKWriterFlagArgWritten   = 1 << 1,
	BKWriterFlagCmdWritten   = 1 << 2,
};

struct BKTokenDef
{
	char const * name;
	BKEnum       value;
};

static char const * BKBFMWriterBase64Table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

BKInt BKBFMWriterInit (BKBFMWriter * writer, BKEnum format)
{
	memset (writer, 0, sizeof (* writer));

	if (BKByteBufferInit (& writer -> buffer, 0, 0) != 0)
		return -1;

	if (format == 0)
		format = BKWriterFormatBinary;

	switch (format) {
		case BKWriterFormatBinary:
		case BKWriterFormatText: {
			writer -> format = format;
			break;
		}
		default: {
			return -1;
			break;
		}
	}

	return 0;
}

void BKBFMWriterDispose (BKBFMWriter * writer)
{
	BKByteBufferDispose (& writer -> buffer);

	memset (writer, 0, sizeof (* writer));
}

static BKInt BKBFMWriterTokenTypeIsArgument (BKEnum tokenType)
{
	switch (tokenType) {
		case BK_INSTR_Integer:
		case BK_INSTR_String:
		case BK_INSTR_Data: {
			return 1;
			break;
		}
	}

	return 0;
}

static BKInt BKBFMWriterWriteVarInt (BKBFMWriter * writer, BKInt value)
{
	unsigned char bytes [5];
	unsigned char * s = bytes;
	BKInt size;
	BKInt n = value;

	if (n >= 0) {
		n <<= 1;
	}
	else {
		n = -n;
		n = (n << 1) | 1;
	}

	if (n <= 0x7F) {
		(* s ++) = n;
	}
	else if (n <= 0x3FFF) {
		(* s ++) = ((n >> 7) & 0x7F) | 0x80;
		(* s ++) = (n & 0x7F);
	}
	else if (n <= 0x1FFFFF) {
		(* s ++) = ((n >> 14) & 0x7F) | 0x80;
		(* s ++) = ((n >>  7) & 0x7F) | 0x80;
		(* s ++) = (n & 0x7F);
	}
	else if (n <= 0xFFFFFFF) {
		(* s ++) = ((n >> 21) & 0x7F) | 0x80;
		(* s ++) = ((n >> 14) & 0x7F) | 0x80;
		(* s ++) = ((n >>  7) & 0x7F) | 0x80;
		(* s ++) = (n & 0x7F);
	}
	else {
		(* s ++) = ((n >> 28) & 0x7F) | 0x80;
		(* s ++) = ((n >> 21) & 0x7F) | 0x80;
		(* s ++) = ((n >> 14) & 0x7F) | 0x80;
		(* s ++) = ((n >>  7) & 0x7F) | 0x80;
		(* s ++) = (n & 0x7F);
	}

	size = s - bytes;

	if (BKByteBufferWriteBytes (& writer -> buffer, bytes, size) < 0)
		return -1;

	return 0;
}

static BKInt BKBFMWriterWriteCharEscape (BKBFMWriter * writer, char c)
{
	switch (c) {
		case '"':
		case ':':
		case ';':
		case '!':
		case '\\': {
			if (BKByteBufferWriteByte (& writer -> buffer, '\\') <= 0)
				return -1;
			break;
		}
	}

	return BKByteBufferWriteByte (& writer -> buffer, c);
}

static BKInt BKBFMWriterWriteCharsEscape (BKBFMWriter * writer, char const * chars, BKSize size)
{
	if (size == 0 && chars)
		size = strlen (chars);

	for (BKInt i = 0; i < size; i ++) {
		if (BKBFMWriterWriteCharEscape (writer, chars [i]) < 0)
			return -1;
	}

	return 0;
}

static BKInt BKBFMWriterWriteInteger (BKBFMWriter * writer, BKInt value)
{
	BKInt length;
	char string [30];

	length = snprintf (string, sizeof (string), "%d", value);

	if (BKByteBufferWriteBytes (& writer -> buffer, string, length) < 0)
		return -1;

	return 0;
}

static BKInt BKBFMWriterWriteBase64 (BKBFMWriter * writer, unsigned char const * chars, BKSize size)
{
	unsigned c;
	unsigned char chunk [4];

	for (; size >= 3; size -= 3) {
		c = * chars ++;
		c = (c << 8) + (* chars ++);
		c = (c << 8) + (* chars ++);

		chunk [0] = BKBFMWriterBase64Table [(c & 0xFC0000) >> 18];
		chunk [1] = BKBFMWriterBase64Table [(c & 0x03F000) >> 12];
		chunk [2] = BKBFMWriterBase64Table [(c & 0x000FC0) >>  6];
		chunk [3] = BKBFMWriterBase64Table [(c & 0x00003F) >>  0];

		if (BKByteBufferWriteBytes (& writer -> buffer, chunk, 4) < 0)
			return -1;
	}

	if (size) {
		if (size == 2) {
			c = * chars ++;
			c = (c << 8) + (* chars ++);
			c <<= 8;

			chunk [0] =  BKBFMWriterBase64Table [(c & 0xFC0000) >> 18];
			chunk [1] =  BKBFMWriterBase64Table [(c & 0x03F000) >> 12];
			chunk [2] =  BKBFMWriterBase64Table [(c & 0x000FC0) >>  6];
			chunk [3] =  '=';

			if (BKByteBufferWriteBytes (& writer -> buffer, chunk, 4) < 0)
				return -1;
		}
		else if (size == 1) {
			c = * chars ++;
			c <<= 16;

			chunk [0] =  BKBFMWriterBase64Table [(c & 0xFC0000) >> 18];
			chunk [1] =  BKBFMWriterBase64Table [(c & 0x03F000) >> 12];
			chunk [2] =  '=';
			chunk [3] =  '=';

			if (BKByteBufferWriteBytes (& writer -> buffer, chunk, 4) < 0)
				return -1;
		}
	}

	return 0;
}

static BKInt BKBFMWriterPutMagicToken (BKBFMWriter * writer)
{
	BKBFMToken token;

	token.type = BK_INSTR_GroupBegin;

	if (BKBFMWriterPutToken (writer, & token) < 0)
		return -1;

	token.type = BK_INSTR_String;
	token.sval = "bfm";
	token.len  = 3;

	if (BKBFMWriterPutToken (writer, & token) < 0)
		return -1;

	token.type = BK_INSTR_String;
	token.sval = "blip";
	token.len  = 4;

	if (BKBFMWriterPutToken (writer, & token) < 0)
		return -1;

	token.type = BK_INSTR_Integer;
	token.ival = 1;

	if (BKBFMWriterPutToken (writer, & token) < 0)
		return -1;

	return 0;
}

static BKInt BKBFMWriterPutGroupBeginToken (BKBFMWriter * writer)
{
	switch (writer -> format) {
		case BKWriterFormatBinary: {
			if (BKBFMWriterWriteVarInt (writer, BK_INSTR_GroupBegin) < 0)
				return -1;

			break;
		}
		case BKWriterFormatText: {
			if (BKByteBufferWriteByte (& writer -> buffer, '[') < 0)
				return -1;

			break;
		}
	}

	return 0;
}

static BKInt BKBFMWriterPutGroupEndToken (BKBFMWriter * writer)
{
	switch (writer -> format) {
		case BKWriterFormatBinary: {
			if (BKBFMWriterWriteVarInt (writer, BK_INSTR_GroupEnd) < 0)
				return -1;

			break;
		}
		case BKWriterFormatText: {
			if (BKByteBufferWriteByte (& writer -> buffer, ']') < 0)
				return -1;

			break;
		}
	}

	return 0;
}

static BKInt BKBFMWriterPutIntegerToken (BKBFMWriter * writer, BKInt value)
{
	switch (writer -> format) {
		case BKWriterFormatBinary: {
			if (BKBFMWriterWriteVarInt (writer, BK_INSTR_Integer) < 0)
				return -1;

			if (BKBFMWriterWriteVarInt (writer, value) < 0)
				return -1;

			break;
		}
		case BKWriterFormatText: {
			if (BKBFMWriterWriteInteger (writer, value) < 0)
				return -1;

			break;
		}
	}

	return 0;
}

static BKInt BKBFMWriterPutStringToken (BKBFMWriter * writer, char const * value, BKSize length)
{
	switch (writer -> format) {
		case BKWriterFormatBinary: {
			if (BKBFMWriterWriteVarInt (writer, BK_INSTR_String) < 0)
				return -1;

			if (BKBFMWriterWriteVarInt (writer, length) < 0)
				return -1;

			if (BKByteBufferWriteBytes (& writer -> buffer, value, length) < 0)
				return -1;

			break;
		}
		case BKWriterFormatText: {
			if (BKBFMWriterWriteCharsEscape (writer, value, length) < 0)
				return -1;

			break;
		}
	}

	return 0;
}

static BKInt BKBFMWriterPutDataToken (BKBFMWriter * writer, void const * value, BKSize length)
{
	switch (writer -> format) {
		case BKWriterFormatBinary: {
			if (BKBFMWriterWriteVarInt (writer, BK_INSTR_Data) < 0)
				return -1;

			if (BKBFMWriterWriteVarInt (writer, length) < 0)
				return -1;

			if (BKByteBufferWriteBytes (& writer -> buffer, value, length) < 0)
				return -1;

			break;
		}
		case BKWriterFormatText: {
			if (BKByteBufferWriteByte (& writer -> buffer, '!') < 0)
				return -1;

			if (BKBFMWriterWriteBase64 (writer, value, length))
				return -1;

			break;
		}
	}

	return 0;
}

static BKInt BKBFMWriterPutOtherCommand (BKBFMWriter * writer, BKEnum otherCommand)
{
	switch (writer -> format) {
		case BKWriterFormatBinary: {
			if (BKBFMWriterWriteVarInt (writer, otherCommand) < 0)
				return -1;

			break;
		}
		case BKWriterFormatText: {
			BKInstructionDef const * token;

			token = BKInstructionLookupByValue (otherCommand);

			if (token == NULL)
				return -1;

			if (BKByteBufferWriteBytes (& writer -> buffer, token -> name, strlen (token -> name)) < 0)
				return -1;

			break;
		}
	}

	return 0;
}

static BKInt BKBFMWriterWriteArgSeparator (BKBFMWriter * writer)
{
	writer -> flags &= ~BKWriterFlagArgWritten;

	if (writer -> format == BKWriterFormatText) {
		if (BKByteBufferWriteByte (& writer -> buffer, ':') < 0)
			return -1;
	}

	return 0;
}

static BKInt BKBFMWriterWriteCommandEnd (BKBFMWriter * writer)
{
	writer -> flags &= ~(BKWriterFlagArgWritten | BKWriterFlagCmdWritten);

	if (writer -> format == BKWriterFormatText) {
		if (BKByteBufferWriteByte (& writer -> buffer, ';') < 0)
			return -1;
	}

	return 0;
}

BKInt BKBFMWriterPutToken (BKBFMWriter * writer, BKBFMToken const * inToken)
{
	if ((writer -> flags & BKWriterFlagMagicWritten) == 0) {
		writer -> flags |= BKWriterFlagMagicWritten;

		if (BKBFMWriterPutMagicToken (writer) < 0)
			return -1;
	}

	if (BKBFMWriterTokenTypeIsArgument (inToken -> type)) {
		if (writer -> flags & (BKWriterFlagArgWritten | BKWriterFlagCmdWritten)) {
			if (BKBFMWriterWriteArgSeparator (writer) < 0)
				return -1;
		}

		writer -> flags |= BKWriterFlagArgWritten;
	}
	else if (writer -> flags & (BKWriterFlagArgWritten | BKWriterFlagCmdWritten)) {
		if (BKBFMWriterWriteCommandEnd (writer) < 0)
			return -1;
	}

	switch (inToken -> type) {
		default: {
			writer -> flags |= BKWriterFlagCmdWritten;

			if (BKBFMWriterPutOtherCommand (writer, inToken -> type) < 0)
				return -1;

			break;
		}
		case BK_INSTR_End: {
			if (BKBFMWriterPutGroupEndToken (writer) < 0)
				return -1;

			break;
		}
		case BK_INSTR_GroupBegin: {
			if (BKBFMWriterPutGroupBeginToken (writer) < 0)
				return -1;

			break;
		}
		case BK_INSTR_GroupEnd: {
			if (BKBFMWriterPutGroupEndToken (writer) < 0)
				return -1;

			break;
		}
		case BK_INSTR_Integer: {
			if (BKBFMWriterPutIntegerToken (writer, inToken -> ival) < 0)
				return -1;

			break;
		}
		case BK_INSTR_String: {
			if (BKBFMWriterPutStringToken (writer, inToken -> sval, inToken -> len) < 0)
				return -1;

			break;
		}
		case BK_INSTR_Data: {
			if (BKBFMWriterPutDataToken (writer, inToken -> sval, inToken -> len) < 0)
				return -1;

			break;
		}
	}

	return 0;
}
