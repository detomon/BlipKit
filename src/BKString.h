/**
 * Copyright (c) 2016 Simon Schoenenberger
 * http://blipkit.audio
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

#ifndef _BK_STRING_H_
#define _BK_STRING_H_

#include "BKBase.h"

typedef struct BKString BKString;

struct BKString
{
	uint8_t * str;
	BKUSize   len;
	BKUSize   cap;
};

/**
 * Initialize string struct
 */
#define BK_STRING_INIT ((BKString) {(uint8_t *) "", 0, 0})

/**
 * Initialize string with chars
 */
extern BKInt BKStringInit (BKString * str, char const * chars);

/**
 * Dispose string
 */
extern void BKStringDispose (BKString * str);

/**
 * Reserve space for `size` characters
 */
extern BKInt BKStringReserve (BKString * str, BKUSize size);

/**
 * Append single character
 */
BK_INLINE BKInt BKStringAppendChar (BKString * str, uint8_t c);

/**
 * Append NUL-terminated string
 */
extern BKInt BKStringAppend (BKString * str, char const * chars);

/**
 * Append characters with `size`
 */
extern BKInt BKStringAppendLen (BKString * str, char const * chars, BKUSize len);

/**
 * Append string
 */
extern BKInt BKStringAppendString (BKString * str, BKString const * other);

/**
 * Append formatted string
 */
extern BKInt BKStringAppendFormat (BKString * str, char const * format, ...);

/**
 * Append formatted string
 */
extern BKInt BKStringAppendFormatArgs (BKString * str, char const * format, va_list args);

/**
 * Compare string
 */
extern BKInt BKStringCompare (BKString const * str, char const * chars);

/**
 * Compare string
 */
extern BKInt BKStringCompareLen (BKString const * str, char const * chars, BKUSize len);

/**
 * Compare string
 */
extern BKInt BKStringCompareString (BKString const * str, BKString const * other);

/**
 * Get substring
 */
extern BKInt BKStringSubstring (BKString const * str, BKString * substr, BKUSize offset, BKUSize length);

/**
 * Get substring
 */
extern BKInt BKStringSubstring (BKString const * str, BKString * substr, BKUSize offset, BKUSize length);

/**
 * Replace chars in range
 */
extern BKInt BKStringReplaceInRange (BKString * str, BKString const * substr, BKUSize offset, BKUSize length);

/**
 * Get directory name
 */
extern BKInt BKStringDirname (BKString const * str, BKString * dirname);

/**
 * Append path segment
 */
extern BKInt BKStringAppendPathSegment (BKString * str, BKString const * segment);

/**
 * Escape and write string `str` into `buffer` to be safely printable to console
 */
extern BKString * BKStringEscape (BKString * buffer, char const * str);

/**
 * Escape and write string `str` into `buffer` to be safely printable to console
 */
BK_INLINE BKString * BKStringEscapeString (BKString * buffer, BKString const * str);

/**
 * Empty string and keep capacity
 */
BK_INLINE void BKStringEmpty (BKString * str);


// --- Inline implementations

BK_INLINE BKInt BKStringAppendChar (BKString * str, uint8_t c)
{
	if (str -> len + 1 >= str -> cap) {
		if (BKStringReserve (str, 1) != 0) {
			return -1;
		}
	}

	str -> str [str -> len ++] = c;
	str -> str [str -> len] = '\0';

	return 0;
}

BK_INLINE BKString * BKStringEscapeString (BKString * buffer, BKString const * str)
{
	return BKStringEscape (buffer, (char *) str -> str);
}

BK_INLINE void BKStringEmpty (BKString * str)
{
	if (str -> cap) {
		str -> str [0] = '\0';
	}

	str -> len = 0;
}

#endif /* ! _BK_STRING_H_  */
