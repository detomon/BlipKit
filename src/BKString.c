#include <string.h>
#include "BKString.h"

static uint8_t const escapeChars [256] =
{
	[0x00] = 1,   [0x01] = 1,   [0x02] = 1,   [0x03] = 1,
	[0x04] = 1,   [0x05] = 1,   [0x06] = 1,   [0x07] = 'a',
	[0x08] = 'b', [0x09] = 't', [0x0A] = 'n', [0x0B] = 'v',
	[0x0C] = 'f', [0x0D] = 'r', [0x0E] = 1,   [0x0F] = 1,
	[0x10] = 1,   [0x11] = 1,   [0x12] = 1,   [0x13] = 1,
	[0x14] = 1,   [0x15] = 1,   [0x16] = 1,   [0x17] = 1,
	[0x18] = 1,   [0x19] = 1,   [0x1A] = 1,   [0x1B] = 1,
	[0x1C] = 1,   [0x1D] = 1,   [0x1E] = 1,   [0x1F] = 1,
	[0x80] = 1,
};

BKInt BKStringInit (BKString * str, char const * chars)
{
	*str = BK_STRING_INIT;

	return BKStringAppend (str, (char *) chars);
}

void BKStringDispose (BKString * str)
{
	if (str -> cap) {
		free (str -> str);
	}

	*str = BK_STRING_INIT;
}

BKInt BKStringReserve (BKString * str, BKUSize size)
{
	uint8_t * chars;
	BKUSize cap;

	cap = str -> len + size;
	cap = (cap + 8ULL) & ~(8ULL - 1);

	if (cap >= str -> cap) {
		// str == ""
		if (!str -> cap) {
			str -> str = NULL;
		}

		cap += cap >> 1; // +1/2
		chars = realloc (str -> str, cap);

		if (!chars) {
			return BK_ALLOCATION_ERROR;
		}

		str -> str = chars;
		str -> cap = cap;
		str -> str [str -> len] = '\0';
	}

	return 0;
}

BKInt BKStringAppend (BKString * str, char const * chars)
{
	return BKStringAppendLen (str, (char *) chars, strlen ((void *) chars));
}

BKInt BKStringAppendLen (BKString * str, char const * chars, BKUSize len)
{
	if (str -> len + len >= str -> cap) {
		if (BKStringReserve (str, len) != 0) {
			return BK_ALLOCATION_ERROR;
		}
	}

	memcpy (&str -> str [str -> len], chars, len);
	str -> len += len;
	str -> str [str -> len] = '\0';

	return 0;
}

BKInt BKStringAppendString (BKString * str, BKString const * other)
{
	return BKStringAppendLen (str, (char *) other -> str, other -> len);
}

BKInt BKStringAppendFormat (BKString * str, char const * format, ...)
{
	BKInt res;
	va_list args;

	va_start (args, format);

	res = BKStringAppendFormatArgs (str, format, args);

	return res;
}

BKInt BKStringAppendFormatArgs (BKString * str, char const * format, va_list args)
{
	int res = 0;
	int length;
	uint8_t * buf;
	BKUSize cap;
	va_list args2;

	va_copy (args2, args);

	if ((res = BKStringReserve (str, 64)) != 0) {
		goto error;
	}

	buf = str -> str + str -> len;
	cap = str -> cap - str -> len;
	length = vsnprintf ((char *) buf, cap, (char *) format, args2);

	if (length < 0) {
		res = -1;
		goto error;
	}
	else if (length >= cap) {
		if ((res = BKStringReserve (str, length)) != 0) {
			goto error;
		}

		va_copy (args2, args);

		buf = str -> str + str -> len;
		cap = str -> cap - str -> len;
		length = vsnprintf ((char *) buf, cap, (char *) format, args2);

		if (length < 0) {
			res = -1;
			goto error;
		}
	}

	str -> len += length;

error:
	va_end (args2);

	return res;
}

BKInt BKStringCompare (BKString const * str, char const * chars)
{
	return BKStringCompareLen (str, chars, strlen (chars));
}

BKInt BKStringCompareLen (BKString const * str, char const * chars, BKUSize len)
{
	int cmp;

	len = str -> len < len ? str -> len : len;
	cmp = memcmp ((void *) str -> str, chars, len);

	if (cmp != 0) {
		return cmp;
	}

	return str -> len < len ? -1 : str -> len != len;
}

BKInt BKStringCompareString (BKString const * str, BKString const * other)
{
	return BKStringCompareLen (str, (char *) other -> str, other -> len);
}

BKInt BKStringSubstring (BKString const * str, BKString * substr, BKUSize offset, BKUSize length)
{
	BKStringEmpty (substr);

	if (offset > str -> len) {
		offset = str -> len;
	}

	if (offset + length > str -> len) {
		length = str -> len - offset;
	}

	return BKStringAppendLen (substr, (char *) str -> str + offset, length);
}

BKInt BKStringReplaceInRange (BKString * str, BKString const * substr, BKUSize offset, BKUSize length)
{
	BKInt res = 0;
	BKSize lenDiff;

	offset = BKMin (offset, str -> len);
	length = BKMin (length, str -> len - offset);
	lenDiff = substr -> len - length;

	if (lenDiff > 0) {
		if ((res = BKStringReserve (str, lenDiff)) != 0) {
			return res;
		}
	}

	memmove (&str -> str [offset + substr -> len], &str -> str [offset + length], str -> len - (offset + length));
	memcpy (&str -> str [offset], substr -> str, substr -> len);

	str -> len += lenDiff;
	str -> str [str -> len] = '\0';

	return res;
}

BKInt BKStringDirname (BKString const * str, BKString * dirname)
{
	BKInt res = 0;
	uint8_t const * c;
	BKUSize size;

	if (!str -> len) {
		BKStringEmpty (dirname);

		if ((res = BKStringAppend (dirname, ".")) != 0) {
			return res;
		}

		return 0;
	}

	c = str -> str + str -> len - 1;

	// trim '/' from right
	while (c > str -> str && *c == '/') {
		c --;
	}

	// trim everything except '/' from right
	while (c > str -> str && *c != '/') {
		c --;
	}

	// trim '/' from right again
	while (c > str -> str && *c == '/') {
		c --;
	}

	size = c - str -> str;
	BKStringEmpty (dirname);

	if (size) {
		BKStringAppendLen (dirname, (char *) str -> str, size + 1);
	}
	// dirname is empty
	else {
		BKStringAppendChar (dirname, (str -> str [0] == '/') ? '/' : '.');
	}

	return 0;
}

BKInt BKStringAppendPathSegment (BKString * str, BKString const * segment)
{
	if (str -> len) {
		if (str -> str [str -> len - 1] != '/') {
			if (BKStringAppendChar (str, '/') != 0) {
				return BK_ALLOCATION_ERROR;
			}
		}
	}

	return BKStringAppendString (str, segment);
}

BKInt BKStringEscape (BKString * buffer, char const * str)
{
	BKUSize len = strlen ((char *) str);

	BKStringEmpty (buffer);
	BKStringReserve (buffer, len + (len >> 2));

	for (BKUSize i = 0; i < len; i ++) {
		uint8_t c = str [i];
		int e = escapeChars [c];

		if (e) {
			if (e > 1) {
				BKStringAppendFormat (buffer, "\\%c", e);
			}
			else {
				BKStringAppendFormat (buffer, "\\x%02x", c);
			}
		}
		else {
			BKStringAppendChar (buffer, c);
		}
	}

	return 0;
}

char * BKStrdup (char const * str)
{
	char * newStr = NULL;

	if (str) {
		size_t len = strlen (str);

		if ((newStr = malloc (len + 1))) {
			strncpy (newStr, str, len + 1);
		}
	}

	return newStr;
}
