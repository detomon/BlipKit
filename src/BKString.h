/*
 * Copyright (c) 2012-2016 Simon Schoenenberger
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

/**
 * @file
 *
 * The string object.
 */

#ifndef _BK_STRING_H_
#define _BK_STRING_H_

#include <stdarg.h>
#include "BKBase.h"

typedef struct BKString BKString;

/**
 * The string struct.
 */
struct BKString
{
	uint8_t * str; ///< The bytes including the terminating NUL-byte.
	BKUSize   len; ///< The number of bytes.
	BKUSize   cap; ///< The string capacity.
};

/**
 * Initialize string struct.
 */
#define BK_STRING_INIT (BKString) {(uint8_t *) "", 0, 0}

/**
 * Initialize string with chars.
 *
 * @param str The string to initialize.
 * @param chars A NUL-terminated string.
 * @retval BK_SUCCESS
 * @retval BK_ALLOCATION_ERROR
 */
extern BKInt BKStringInit (BKString * str, char const * chars);

/**
 * Dispose string.
 *
 * @param str The string to dispose.
 */
extern void BKStringDispose (BKString * str);

/**
 * Reserve space for characters.
 *
 * @param str The string to reserve capacity for.
 * @param size The number of characters to reserve.
 */
extern BKInt BKStringReserve (BKString * str, BKUSize size);

/**
 * Append single byte.
 *
 * @param str The string to append a byte to.
 * @param c The byte to append.
 * @return 0 on success.
 */
BK_INLINE BKInt BKStringAppendChar (BKString * str, uint8_t c);

/**
 * Append NUL-terminated bytes.
 *
 * @param str The string to append the characters to.
 * @param chars The NUL-terminated bytes to append.
 * @return 0 on success.
 */
extern BKInt BKStringAppend (BKString * str, char const * chars);

/**
 * Append bytes with given length.
 *
 * @param str The string to append the characters to.
 * @param chars The bytes to append.
 * @param size The number of characters to append.
 * @return 0 on success.
 */
extern BKInt BKStringAppendLen (BKString * str, char const * chars, BKUSize len);

/**
 * Append other string.
 *
 * @param str The string to append the string to.
 * @param other The string to append.
 * @return 0 on success.
 */
extern BKInt BKStringAppendString (BKString * str, BKString const * other);

/**
 * Append formatted string.
 *
 * @param str The string to append the format to.
 * @param format The format to append.
 * @return 0 on success.
 */
extern BKInt BKStringAppendFormat (BKString * str, char const * format, ...);

/**
 * Append formatted string.
 *
 * @param str The string to append the format to.
 * @param format The format to append.
 * @param The argument list.
 * @return 0 on success.
 */
extern BKInt BKStringAppendFormatArgs (BKString * str, char const * format, va_list args);

/**
 * Compare strings with bytes.
 *
 * @param str The string.
 * @param chars The NUL-terminating bytes to compare with the string.
 * @return 0 if the strings are equal.
 */
extern BKInt BKStringCompare (BKString const * str, char const * chars);

/**
 * Compare string with bytes.
 *
 * @param str The string.
 * @param chars The bytes to compare with the string.
 * @param len The length of the bytes.
 * @return 0 if the strings are equal.
 */
extern BKInt BKStringCompareLen (BKString const * str, char const * chars, BKUSize len);

/**
 * Compare string with other string.
 *
 * @param str The string.
 * @param other The other string to compare with.
 * @return 0 if the strings are equal.
 */
extern BKInt BKStringCompareString (BKString const * str, BKString const * other);

/**
 * Copy substring to given string by replacing its content.
 *
 * @param str The string to copy the substring from.
 * @param substr The string to copy the substring into.
 * @param offset The offset of the substring to copy. Will be clamped to a valid length.
 * @param The length of the substring to copy. Will be clamped to a valid length.
 * @return 0 on success.
 */
extern BKInt BKStringSubstring (BKString const * str, BKString * substr, BKUSize offset, BKUSize length);

/**
 * Replace characters in range.
 *
 * @param str The string to replace the characters in.
 * @param substr The string to replace in the given range.
 * @param offset The offset of the range to be replaced.
 * @param length The length of the range to be replaced.
 * @return 0 on success.
 */
extern BKInt BKStringReplaceInRange (BKString * str, BKString const * substr, BKUSize offset, BKUSize length);

/**
 * Copy directory name of given path.
 *
 * @param str The path to copy the directory name from.
 * @param dirname The string to be replaced with the directory name.
 * @return 0 on success.
 */
extern BKInt BKStringDirname (BKString const * str, BKString * dirname);

/**
 * Append path segment.
 *
 * @param str The string to append the path segment to.
 * @param segment The path segment to append.
 * @return 0 on success.
 */
extern BKInt BKStringAppendPathSegment (BKString * str, BKString const * segment);

/**
 * Escape and write string into given buffer to be safely printable to console.
 *
 * @param buffer The buffer to write th escpaed string to.
 * @param str The string to escape.
 * @return 0 on success
 */
extern BKInt BKStringEscape (BKString * buffer, char const * str);

/**
 * Escape and write string into given buffer to be safely printable to console.
 *
 * @param buffer The buffer to write th escpaed string to.
 * @param str The string to escape.
 * @return 0 on success
 */
BK_INLINE BKInt BKStringEscapeString (BKString * buffer, BKString const * str);

/**
 * Empty string and keep capacity.
 *
 * @param str The string to empty.
 */
BK_INLINE void BKStringEmpty (BKString * str);

/**
 * Duplicate NUL-terminated string.
 *
 * @param str The NUL-terminated string to copy.
 * @return A copy of the given string which can be released with `free`.
 */
extern char * BKStrdup (char const * str);

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

BK_INLINE BKInt BKStringEscapeString (BKString * buffer, BKString const * str)
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
