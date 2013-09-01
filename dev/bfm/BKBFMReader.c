#include "BKBFMReader.h"

BKInt BKBFMReaderInit (BKBFMReader * reader)
{
	memset (reader, 0, sizeof (* reader));

	if (BKByteBufferInit (& reader -> & buffer, 0, 0) != 0)
		return -1;

	return 0;
}

void BKBFMReaderDispose (BKBFMReader * reader)
{
	BKByteBufferDispose (& reader -> buffer);

	memset (reader, 0, sizeof (* reader));
}

BKInt BKBFMReaderNextToken (BKBFMReader * reader, BKBFMToken * outToken)
{
	// ...

	return -1;
}
