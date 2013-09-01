#ifndef _BK_BYTE_BUFFER_SOURCE_H_
#define _BK_BYTE_BUFFER_SOURCE_H_

/**
 * Buffer source
 * Callback `read` is called every time the byte buffer runs empty
 * On success `read` should return a number greater than 0
 * If no more byte are available `read` should return 0
 * On error `read` should return -1
 */

typedef struct BKByteBufferSource BKByteBufferSource;
typedef BKSize (* BKByteBufferSourceReadHandle) (void * buffer, BKByteBufferSource * source);
typedef void (* BKByteBufferSourceDestroyHandle) (void * source);

/**
 * Field `private` can be used for private data 
 */
struct BKByteBufferSource
{
	BKByteBufferSourceReadHandle    read;
	BKByteBufferSourceDestroyHandle destroy;
	void                          * private [4];
};

/**
 * Initialize buffer source struct with file descriptor
 * File descriptor is not closed after calling `BKByteBufferSourceDestroy`
 * Returns `false` if an error occurred
 */
extern bool BKByteBufferFileSourceInit (BKByteBufferSource * source, int fildes);

/**
 * Initialize file buffer source struct with filename
 * Returns `false` if file couldn't be opened and raises an error
 */
extern bool BKByteBufferFileSourceInitWithFilename (BKByteBufferSource * source, const char * path);

/**
 * Destroy buffer source
 * Calls `destroy` of `source`
 */
extern void BKByteBufferSourceDestroy (BKByteBufferSource * source);

#endif /* ! _BK_BYTE_BUFFER_SOURCE_H_ */
