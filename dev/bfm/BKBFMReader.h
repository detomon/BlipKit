#include "BKByteBuffer.h"

typedef struct BKBFMReader BKBFMReader;
typedef struct BKBFMToken  BKBFMToken;

/**
 *
 */
enum BKBFMTokenType
{
	BKBFMTokenTypeEnd        = 0x00,
	BKBFMTokenTypeGroupBegin = 0x01,
	BKBFMTokenTypeGroupEnd   = 0x02,
	BKBFMTokenTypeInteger    = 0x04,
	BKBFMTokenTypeString     = 0x06,
	BKBFMTokenTypeData       = 0x07,
};

/**
 *
 */
struct BKBFMReader
{
	BKUInt         flags;
	BKInt          valueCapacity;
	BKInt          valueSize;
	void         * value;
	BKByteBuffer * buffer;
};

/**
 *
 */
struct BKBFMToken
{
	BKEnum type;
	BKInt  len;
	BKInt  ival;
	void * sval;
};

/**
 *
 */
extern BKInt BKBFMReaderInit (BKBFMReader * reader);

/**
 *
 */
extern void BKBFMReaderDispose (BKBFMReader * reader);

/**
 *
 */
extern BKInt BKBFMReaderNextToken (BKBFMReader * reader, BKBFMToken * outToken);
