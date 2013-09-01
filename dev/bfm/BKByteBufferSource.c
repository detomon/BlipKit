#include "BKByteBuffer.h"

#define FILE_READ_BUFFER 0x4000

struct BKByteBufferFileSource
{
	BKByteBufferSourceReadHandle    read;
	BKByteBufferSourceDestroyHandle destroy;
	int                                fildes;
};

static BKSize BKByteBufferFileSourceRead (BKByteBuffer * buffer, struct BKByteBufferFileSource * source)
{
	BKSize readSize;
	char    bytes [FILE_READ_BUFFER];
	
	readSize = read (source -> fildes, bytes, FILE_READ_BUFFER);

	if (readSize > 0) {
		readSize = BKByteBufferWriteBytes (buffer, bytes, readSize);
		
		//if (readSize < 0)
		//	moErrorRaiseAppend ("BKByteBufferFileSourceRead", "Couldn't write bytes into buffer");
	}
	else if (readSize < 0) {
		//moErrorRaise ("BKByteBufferFileSourceRead", "Couldn't read file");
		readSize = -1;
	}

	return readSize;
}

static void BKByteBufferFileSourceDestroy (struct BKByteBufferFileSource * source)
{
	close (source -> fildes);
}

bool BKByteBufferFileSourceInit (BKByteBufferSource * source, int fildes)
{
	struct BKByteBufferFileSource * stdSource = (struct BKByteBufferFileSource *) source;

	stdSource -> read    = (BKByteBufferSourceReadHandle)    BKByteBufferFileSourceRead;
	stdSource -> destroy = (BKByteBufferSourceDestroyHandle) BKByteBufferFileSourceDestroy;	
	stdSource -> fildes  = fildes;

	return true;
}

bool BKByteBufferFileSourceInitWithFilename (BKByteBufferSource * source, const char * path)
{
	int fildes = open (path, O_RDONLY);

	if (fildes != -1) {
		if (BKByteBufferFileSourceInit (source, fildes)) {
			return true;
		}
		else {
			//moErrorRaise ("BKByteBufferFileSourceInitWithFilename", "Couldn't initialize buffer source");
			close (fildes);
		}
	}
	else {
		//moErrorRaise ("BKByteBufferFileSourceInitWithFilename", "Couldn't open file '%s'", path);
	}

	return false;
}

void BKByteBufferSourceDestroy (BKByteBufferSource * source)
{
	if (source -> destroy)
		source -> destroy (source);

	memset (source, 0, sizeof (BKByteBufferSource));
}
