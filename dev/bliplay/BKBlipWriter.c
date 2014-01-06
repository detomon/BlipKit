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
#include "BKBlipWriter.h"

static char const * BKWriterBase64Table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define INIT_BUFFER_SIZE 0x4000

/*
static void base64Table (void)
{
	char table [256];
	memset (table, -1, sizeof (table));

	for (int i = 0; i < 64; i ++)
		table [BKWriterBase64Table [i]] = i;

	table ['-'] = 62;
	table ['_'] = 63;

	table [':'] = -2;
	table [';'] = -2;
	table ['='] = -2;

	for (int i = 0; i < 256; i ++) {
		if (i && i % 16 == 0)
			printf ("\n");

		printf ("%2d, ", table [i]);
	}
}
*/

BKInt BKBlipWriterInit (BKBlipWriter * writer, BKBlipWriterCallback write, void * userInfo)
{
	memset (writer, 0, sizeof (BKBlipWriter));

	writer -> buffer = malloc (INIT_BUFFER_SIZE);

	if (writer -> buffer) {
		writer -> bufferPtr      = writer -> buffer;
		writer -> bufferCapacity = INIT_BUFFER_SIZE;
		writer -> write          = write;
		writer -> userInfo       = userInfo;
	}
	else {
		return -1;
	}

	//base64Table ();

	return 0;
}

void BKBlipWriterDispose (BKBlipWriter * writer)
{
	if (writer -> buffer)
		free (writer -> buffer);

	memset (writer, 0, sizeof (BKBlipWriter));
}

static BKInt BKBlipWriterFlushBuffer (BKBlipWriter * writer)
{
	if (writer -> bufferPtr > writer -> buffer) {
		if (writer -> write ((void *) writer -> buffer, writer -> bufferPtr - writer -> buffer, writer -> userInfo) < 0)
			return -1;

		writer -> bufferPtr = writer -> buffer;
	}

	return 0;
}

static BKInt BKBlipWriterWriteChar (BKBlipWriter * writer, char c)
{
	if (writer -> bufferPtr >= & writer -> buffer [writer -> bufferCapacity]) {
		if (BKBlipWriterFlushBuffer (writer) < 0)
			return -1;
	}

	* writer -> bufferPtr ++ = c;

	return 0;
}

static BKInt BKBlipWriterWriteCharEscape (BKBlipWriter * writer, char c)
{
	switch (c) {
		case ':':
		case ';':
		case '!':
		case '\\':
			if (BKBlipWriterWriteChar (writer, '\\') < 0)
				return -1;
			break;
	}

	return BKBlipWriterWriteChar (writer, c);
}

static BKInt BKBlipWriterWriteDataCharsBase64 (BKBlipWriter * writer, unsigned char const * chars, size_t size)
{
	unsigned c;

	for (; size >= 3; size -= 3) {
		c = * chars ++;
		c = (c << 8) + (* chars ++);
		c = (c << 8) + (* chars ++);

		if (writer -> bufferPtr > & writer -> buffer [writer -> bufferCapacity - 4]) {
			if (BKBlipWriterFlushBuffer (writer) < 0)
				return -1;
		}

		* writer -> bufferPtr ++ = BKWriterBase64Table [(c & 0xFC0000) >> 18];
		* writer -> bufferPtr ++ = BKWriterBase64Table [(c & 0x03F000) >> 12];
		* writer -> bufferPtr ++ = BKWriterBase64Table [(c & 0x000FC0) >>  6];
		* writer -> bufferPtr ++ = BKWriterBase64Table [(c & 0x00003F) >>  0];
	}

	if (size) {
		if (writer -> bufferPtr > & writer -> buffer [writer -> bufferCapacity - 4]) {
			if (BKBlipWriterFlushBuffer (writer) < 0)
				return -1;
		}

		if (size == 2) {
			c = * chars ++;
			c = (c << 8) + (* chars ++);
			c <<= 8;

			* writer -> bufferPtr ++ = BKWriterBase64Table [(c & 0xFC0000) >> 18];
			* writer -> bufferPtr ++ = BKWriterBase64Table [(c & 0x03F000) >> 12];
			* writer -> bufferPtr ++ = BKWriterBase64Table [(c & 0x03F000) >>  6];
			* writer -> bufferPtr ++ = '=';
		}
		else if (size == 1) {
			c = * chars ++;
			c <<= 16;

			* writer -> bufferPtr ++ = BKWriterBase64Table [(c & 0xFC0000) >> 18];
			* writer -> bufferPtr ++ = BKWriterBase64Table [(c & 0x03F000) >> 12];
			* writer -> bufferPtr ++ = '=';
			* writer -> bufferPtr ++ = '=';
		}
	}

	return 0;
}

static BKInt BKBlipWriterWriteCharsEscape (BKBlipWriter * writer, char const * chars, size_t size)
{
	char const * charsEnd;

	if (size == 0 && chars)
		size = strlen (chars);

	for (charsEnd = & chars [size]; chars < charsEnd; chars ++) {
		if (BKBlipWriterWriteCharEscape (writer, * chars) < 0)
			return -1;
	}

	return 0;
}

static BKInt BKBlipWriterWriteChars (BKBlipWriter * writer, char const * chars, size_t size)
{
	char const * charsEnd;

	if (size == 0 && chars)
		size = strlen (chars);

	for (charsEnd = & chars [size]; chars < charsEnd; chars ++) {
		if (BKBlipWriterWriteChar (writer, * chars) < 0)
			return -1;
	}

	return 0;
}

static BKInt BKBlipWriterAddArg (BKBlipWriter * writer, char const * arg, size_t size)
{
	if (writer -> argCount > 0) {
		if (BKBlipWriterWriteChar (writer, ':') < 0)
			return -1;
	}

	if (BKBlipWriterWriteCharsEscape (writer, arg, size))
		return -1;

	writer -> argCount ++;

	return 0;
}

BKInt BKBlipWriterBeginCommand (BKBlipWriter * writer, char const * name)
{
	if (writer -> argCount > 0)
		return -1;

	if (BKBlipWriterAddArg (writer, name, 0))
		return -1;

	return 0;
}

BKInt BKBlipWriterAddIntArg (BKBlipWriter * writer, long value)
{
	char buffer [64];

	snprintf (buffer, 64, "%ld", value);

	return BKBlipWriterAddArg (writer, buffer, 0);
}

BKInt BKBlipWriterAddFloatArg (BKBlipWriter * writer, double value)
{
	char buffer [64];

	snprintf (buffer, 64, "%lg", value);

	return BKBlipWriterAddArg (writer, buffer, 0);
}

BKInt BKBlipWriterAddStringArg (BKBlipWriter * writer, char const * string, size_t size)
{
	if (size == 0 && string)
		size = strlen (string);

	return BKBlipWriterAddArg (writer, string, size);
}

BKInt BKBlipWriterAddDataArg (BKBlipWriter * writer, void const * data, size_t size)
{
	// add separator
	if (BKBlipWriterAddArg (writer, "", 0) < 0)
		return -1;

	if (BKBlipWriterWriteChar (writer, '!') < 0)
		return -1;

	if (BKBlipWriterWriteDataCharsBase64 (writer, data, size) < 0)
		return -1;

	return 0;
}

BKInt BKBlipWriterEndCommand (BKBlipWriter * writer)
{
	if (writer -> argCount > 0) {
		if (BKBlipWriterWriteChar (writer, ';') < 0)
			return -1;

		if (BKBlipWriterFlushBuffer (writer) < 0)
			return -1;

		writer -> argCount = 0;

		return 0;
	}

	return -1;
}
