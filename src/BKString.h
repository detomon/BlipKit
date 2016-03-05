#ifndef _BK_STRING_H_
#define _BK_STRING_H_

#include "BKBase.h"

typedef struct BKString BKString;

struct BKString
{
	uint8_t * str;
	size_t    len;
	size_t    cap;
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
extern BKInt BKStringReserve (BKString * str, size_t size);

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
extern BKInt BKStringAppendLen (BKString * str, char const * chars, size_t len);

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
extern BKInt BKStringCompare (BKString const * str, BKString const * other);

/**
 * Get substring
 */
extern BKInt BKStringSubstring (BKString const * str, BKString * substr, size_t offset, size_t length);

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
