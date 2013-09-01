#include "BKBFMWriter.h"

enum BKWriterFlag
{
	BKWriterFlagMagicWritten = 1 << 0,
	BKWriterFlagArgWritten   = 1 << 1,
};

BKInt BKBFMWriterInit (BKBFMWriter * writer, BKEnum format)
{
	memset (writer, 0, sizeof (* writer));

	if (BKByteBufferInit (& writer -> & buffer, 0, 0) != 0)
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

static BKBFMWriterTokenTypeIsArgument (BKEnum tokenType)
{
	switch (tokenType) {
		case BKBFMTokenTypeInteger:
		case BKBFMTokenTypeString:
		case BKBFMTokenTypeData: {
			return 1;
			break;
		}
	}

	return 0;
}

static BKInt BKBFMWriterPutMagicToken (BKBFMWriter * writer)
{
	BKBFMToken token;

	token.type = BKBFMTokenTypeGroupBegin;

	if (BKBFMWriterPutToken (writer, & token) != 0)
		return -1;

	token.type = BKBFMTokenTypeString;
	token.sval = "BFM1";
	token.len  = 4;

	if (BKBFMWriterPutToken (writer, & token) != 0)
		return -1;

	token.type = BKBFMTokenTypeString;
	token.sval = "blip";
	token.len  = 4;

	if (BKBFMWriterPutToken (writer, & token) != 0)
		return -1;

	token.type = BKBFMTokenTypeInteger;
	token.ival = 1;

	if (BKBFMWriterPutToken (writer, & token) != 0)
		return -1;

	return 0;
}

static BKInt BKBFMWriterPutGroupBeginToken (BKBFMWriter * writer)
{
	switch (writer -> format) {
		case BKWriterFormatBinary: {
			if (BKByteBufferWriteByte (& writer -> buffer, BKBFMTokenTypeGroupBegin) != 0)
				return -1;

			break;
		}
		case BKWriterFormatText: {
			if (BKByteBufferWriteByte (& writer -> buffer, '[') != 0)
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
			if (BKByteBufferWriteByte (& writer -> buffer, BKBFMTokenTypeGroupEnd) != 0)
				return -1;

			break;
		}
		case BKWriterFormatText: {
			if (BKByteBufferWriteByte (& writer -> buffer, ']') != 0)
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
			// ...

			break;
		}
		case BKWriterFormatText: {
			// ...

			break;
		}
	}

	return 0;
}

static BKInt BKBFMWriterPutStringToken (BKBFMWriter * writer, char const * value, BKSize length)
{
	switch (writer -> format) {
		case BKWriterFormatBinary: {
			// ...

			break;
		}
		case BKWriterFormatText: {
			// ...

			break;
		}
	}

	return 0;
}

static BKInt BKBFMWriterPutDataToken (BKBFMWriter * writer, void const * value, BKSize length)
{
	switch (writer -> format) {
		case BKWriterFormatBinary: {
			// ...

			break;
		}
		case BKWriterFormatText: {
			// ...

			break;
		}
	}

	return 0;
}

static BKInt BKBFMWriterPutOtherCommand (BKBFMWriter * writer, BKEnum otherCommand)
{
	switch (writer -> format) {
		case BKWriterFormatBinary: {
			// ...

			break;
		}
		case BKWriterFormatText: {
			// ...

			break;
		}
	}

	return 0;
}

static BKInt BKBFMWriterWriteCommandEnd (BKBFMWriter * writer)
{
	writer -> flags &= ~BKWriterFlagArgWritten;

	if (writer -> format == BKWriterFormatText) {
		if (BKByteBufferWriteByte (& writer -> buffer, ']') != 0)
			return -1;
	}

	return 0;
}

BKInt BKBFMWriterPutToken (BKBFMWriter * writer, BKBFMToken const * inToken)
{
	if ((writer -> flags & BKWriterFlagMagicWritten) == 0) {
		writer -> flags |= BKWriterFlagMagicWritten;

		if (BKBFMWriterPutMagicToken (writer) != 0)
			return -1;
	}

	if (BKBFMWriterTokenTypeIsArgument (inToken -> type)) {
		writer -> flags |= BKWriterFlagArgWritten;
	}
	else if (writer -> flags & BKWriterFlagArgWritten) {
		if (BKBFMWriterWriteCommandEnd (writer) != 0)
			return -1;
	}

	switch (inToken -> type) {
		default: {
			if (BKBFMWriterPutOtherCommand (writer, inToken -> type) != 0)
				return -1;

			break;
		}
		case BKBFMTokenTypeEnd: {
			if (BKBFMWriterPutGroupEndToken (writer) != 0)
				return -1;

			break;
		}
		case BKBFMTokenTypeGroupBegin: {
			if (BKBFMWriterPutGroupBeginToken (writer) != 0)
				return -1;

			break;
		}
		case BKBFMTokenTypeGroupEnd: {
			if (BKBFMWriterPutGroupEndToken (writer) != 0)
				return -1;

			break;
		}
		case BKBFMTokenTypeInteger: {
			if (BKBFMWriterPutIntegerToken (writer, inToken -> ival) != 0)
				return -1;

			break;
		}
		case BKBFMTokenTypeString: {
			if (BKBFMWriterPutStringToken (writer, inToken -> sval, inToken -> len) != 0)
				return -1;

			break;
		}
		case BKBFMTokenTypeData: {
			if (BKBFMWriterPutDataToken (writer, inToken -> sval, inToken -> len) != 0)
				return -1;

			break;
		}
	}

	return 0;
}
