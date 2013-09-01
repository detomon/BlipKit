#include "BKBFMReader.h"

typedef struct BKBFMWriter BKBFMWriter;

enum BKWriterFormat
{
	BKWriterFormatDefault = 0,
	BKWriterFormatText    = 1,
	BKWriterFormatBinary  = 2,
};

/**
 *
 */
struct BKBFMWriter
{
	BKUInt         flags;
	BKEnum         format;
	BKByteBuffer * buffer;
};

/**
 *
 */
extern BKInt BKBFMWriterInit (BKBFMWriter * writer, BKEnum format);

/**
 *
 */
extern void BKBFMWriterDispose (BKBFMWriter * writer);

/**
 *
 */
extern BKInt BKBFMWriterPutToken (BKBFMWriter * writer, BKBFMToken const * inToken);
