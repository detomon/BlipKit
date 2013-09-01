#include "BKByteBuffer.h"

#define OPTIONS_MASK        0x00FF
#define MIN_SEGMENT_SIZE    0x4000
#define MORE_STORAGE_FACTOR 1.25

static BKByteBufferSegment * BKByteBufferSegmentAlloc (BKUSize capacity)
{
	BKByteBufferSegment * segment = malloc (sizeof (BKByteBufferSegment) + capacity);

	if (segment) {
		memset (segment, 0, sizeof (BKByteBufferSegment));
		segment -> capacity = capacity;
	}

	return segment;
}

static void BKByteBufferFreeSegment (BKByteBufferSegment * segment, bool freeList)
{
	// Free all linked segments
	if (freeList) {
		for (BKByteBufferSegment * nextSegment = NULL; segment; segment = nextSegment) {
			nextSegment = segment -> nextSegment;
			free (segment);
		}
	}
	// Free single segment
	else if (segment) {
		free (segment);
	}
}

static bool BKByteBufferPushStorage (BKByteBuffer * buffer, BKUSize preferredSize)
{
	BKByteBufferSegment * segment = NULL;

	//#warning TEST!
	//moErrorRaise ("BKByteBufferPushStorage", "Allocation error");
	//return false;
	
	// Single segment
	if (buffer -> info & BKByteBufferOptionContinuousStorage) {
		BKUSize usedSize   = 0;
		BKUSize capacity   = 0;
		BKSize  readOffset = 0;

		segment = buffer -> readSegment;

		if (segment) {
			if (buffer -> info & BKByteBufferOptionKeepBytes) {
				usedSize   = buffer -> writeDataEnd - segment -> data;
				readOffset = buffer -> readCursor - segment -> data;
			} else {
				usedSize   = buffer -> writeDataEnd - buffer -> readCursor;
			}

			capacity = buffer -> readSegment -> capacity;
		}

		preferredSize = (usedSize + preferredSize) * MORE_STORAGE_FACTOR;
		preferredSize = MAX (preferredSize, MIN_SEGMENT_SIZE);

		if ((buffer -> info & BKByteBufferOptionKeepBytes) == false)
			memmove (segment -> data, buffer -> readCursor, usedSize);

		if (preferredSize > capacity) {
			BKByteBufferSegment * newSegment = realloc (segment, sizeof (BKByteBufferSegment) + preferredSize);

			if (newSegment) {
				newSegment -> capacity = preferredSize;
				buffer -> capacity     = preferredSize;
				buffer -> firstSegment = newSegment;
				buffer -> writeSegment = newSegment;
				buffer -> writeCursor  = & newSegment -> data [usedSize];
				buffer -> writeDataEnd = & newSegment -> data [preferredSize];
				buffer -> readSegment  = newSegment;
				buffer -> readCursor   = & newSegment -> data [readOffset];
				buffer -> readDataEnd  = buffer -> writeCursor;
			}
			else {
				//moErrorRaise ("BKByteBufferPushStorage", "Allocation error");
				return false;
			}
		}
		
		return true;
	}
	// Multiple segments
	else {
		// Reuse segments 
		if (buffer -> freeSegments) {
			segment = buffer -> freeSegments;
			buffer -> freeSegments = segment -> nextSegment;
		}
		// Allocate new segment
		else {
			preferredSize *= MORE_STORAGE_FACTOR;
			preferredSize = MAX (preferredSize, MIN_SEGMENT_SIZE);
			segment = BKByteBufferSegmentAlloc (preferredSize);
		}

		if (segment) {
			segment -> previousSegment = buffer -> writeSegment;
			segment -> nextSegment     = NULL;

			if (buffer -> writeSegment) {
				buffer -> writeSegment -> nextSegment = segment;
			}
			else {
				buffer -> firstSegment = segment;
				buffer -> readSegment  = segment;
				buffer -> readCursor   = segment -> data;
				buffer -> readDataEnd  = segment -> data;
			}

			buffer -> capacity += segment -> capacity;
			buffer -> writeSegment = segment;
			buffer -> writeCursor  = segment -> data;
			buffer -> writeDataEnd = & segment -> data [segment -> capacity];

			return true;
		}
		else {
			//moErrorRaise ("BKByteBufferPushStorage", "Allocation error");
			return false;
		}
	}

	return false;
}

static BKSize BKByteBufferShiftStorage (BKByteBuffer * buffer)
{
	BKSize shiftSize = 0;

	// Single segment
	if (buffer -> info & BKByteBufferOptionContinuousStorage) {
		// Asking source for new data
		if (buffer -> source) {
			BKSize writeSize = buffer -> source -> read (buffer, buffer -> source);

			if (writeSize > 0) {
				shiftSize = writeSize;
			}
			else if (writeSize < 0) {
				//moErrorRaiseAppend ("BKByteBufferShiftStorage", "Couldn't read from buffer source");
				shiftSize = -1;
			}
		}
	}
	// Multiple segments
	else {
		// Shift to next segment
		if (buffer -> readSegment && buffer -> readSegment -> nextSegment) {
			BKByteBufferSegment * nextSegment = buffer -> readSegment -> nextSegment;

			// Add segment to free segments
			if ((buffer -> info & BKByteBufferOptionKeepBytes) == false) {
				buffer -> readSegment -> nextSegment = buffer -> freeSegments;
				buffer -> freeSegments = buffer -> readSegment;
				buffer -> firstSegment = nextSegment;
				nextSegment -> previousSegment = NULL;
			}
			else {
				buffer -> readSize += buffer -> readSegment -> capacity;
			}

			buffer -> capacity -= buffer -> readSegment -> capacity;
			buffer -> readSegment = nextSegment;
			buffer -> readCursor  = nextSegment -> data;
			
			if (buffer -> readSegment == buffer -> writeSegment) {
				buffer -> readDataEnd = buffer -> writeDataEnd;
			} else {
				buffer -> readDataEnd = & nextSegment -> data [nextSegment -> capacity];
			}
	
			shiftSize = buffer -> readSegment -> capacity;
		}
		// Asking source for new data
		else if (buffer -> source) {
			BKSize writeSize = buffer -> source -> read (buffer, buffer -> source);

			if (writeSize >= 0) {
				shiftSize = writeSize;
			}
			else {
				//moErrorRaiseAppend ("BKByteBufferShiftStorage", "Couldn't read from buffer source");
				shiftSize = -1;
			}
		}
	}

	return shiftSize;
}

bool BKByteBufferInit (BKByteBuffer * buffer, BKUSize initSize, unsigned options)
{
	memset (buffer, 0, sizeof (BKByteBuffer));
	buffer -> info |= (options & OPTIONS_MASK);

	if (initSize) {
		if (BKByteBufferPushStorage (buffer, initSize) == false) {
			//moErrorRaise ("BKByteBufferInit", "Allocation error");
			return false;
		}
	}

	return true;
}
void BKByteBufferDestroy (BKByteBuffer * buffer)
{
	BKByteBufferFreeSegment (buffer -> firstSegment, true);
	BKByteBufferFreeSegment (buffer -> freeSegments, true);

	memset (buffer, 0, sizeof (BKByteBuffer));
}

void BKByteBufferSetSource (BKByteBuffer * buffer, BKByteBufferSource * source)
{
	buffer -> source = source;
}

BKSize BKByteBufferReadBytes (BKByteBuffer * buffer, void * bytes, BKUSize size)
{
	BKUSize readSize       = 0;
	BKUSize remainaingSize = buffer -> readDataEnd - buffer -> readCursor;

	do {
		if (size <= remainaingSize) {
			if (bytes)
				memcpy (bytes, buffer -> readCursor, size);

			buffer -> readCursor += size;
			readSize += size;
			break;
		}
		else {
			if (bytes) {
				memcpy (bytes, buffer -> readCursor, remainaingSize);
				bytes += remainaingSize;
			}

			buffer -> readCursor += remainaingSize;
			size -= remainaingSize;
			readSize += remainaingSize;

			BKSize shiftSize = BKByteBufferShiftStorage (buffer);
			
			if (shiftSize > 0) {
				remainaingSize = buffer -> readDataEnd - buffer -> readCursor;
			}
			else if (shiftSize == 0) {
				// No more bytes to read
				break;
			}
			else {
				//moErrorRaiseAppend ("BKByteBufferReadBytes", "Couldn't read bytes");
				return -1;
			}
		}
	}
	while (size);

	return readSize;
}

int BKByteBufferReadByte (BKByteBuffer * buffer)
{
	if (buffer -> readCursor >= buffer -> readDataEnd) {
		BKSize shiftSize = BKByteBufferShiftStorage (buffer);

		if (shiftSize <= 0) {
			if (shiftSize == 0) {
				return -1;
			}
			else {
				//moErrorRaiseAppend ("BKByteBufferReadByte", "Couldn't read byte");
				return -1;
			}
		}
	}

	return (* buffer -> readCursor ++);
}

void * BKByteBufferGetBytes (BKByteBuffer * buffer)
{
	if (buffer -> info & BKByteBufferOptionContinuousStorage) {
		if (buffer -> readSegment)
			return buffer -> readSegment -> data;
	}

	return NULL;
}

BKSize BKByteBufferWriteBytes (BKByteBuffer * buffer, const void * bytes, BKUSize size)
{
	BKUSize writtenSize    = 0;
	BKUSize remainaingSize = buffer -> writeDataEnd - buffer -> writeCursor;

	do {
		if (size <= remainaingSize) {
			memcpy (buffer -> writeCursor, bytes, size);
			buffer -> writeCursor += size;
			writtenSize += size;
	
			if (buffer -> readSegment == buffer -> writeSegment)
				buffer -> readDataEnd = buffer -> writeCursor;

			break;
		}
		else {
			memcpy (buffer -> writeCursor, bytes, remainaingSize);
			buffer -> writeCursor += remainaingSize;
			bytes += remainaingSize;
			size -= remainaingSize;
			writtenSize += remainaingSize;

			if (buffer -> readSegment == buffer -> writeSegment)
				buffer -> readDataEnd = buffer -> writeCursor;

			if (BKByteBufferPushStorage (buffer, size)) {
				remainaingSize = buffer -> writeDataEnd - buffer -> writeCursor;
			}
			else {
				//moErrorRaiseAppend ("BKByteBufferWriteBytes", "Couldn't write bytes");
				return -1;
			}
		}
	}
	while (size);

	return writtenSize;
}

bool BKByteBufferWriteByte (BKByteBuffer * buffer, unsigned char byte)
{
	if (buffer -> writeCursor >= buffer -> writeDataEnd) {
		if (BKByteBufferPushStorage (buffer, 1) == false) {
			//moErrorRaiseAppend ("BKByteBufferWriteByte", "Couldn't write bytes");
			return false;
		}
	}

	(* buffer -> writeCursor ++) = byte;

	return true;
}

BKUSize BKByteBufferGetSize (BKByteBuffer * buffer)
{
	BKUSize size = buffer -> capacity;

	// Subtract end of current write segment
	size -= buffer -> writeDataEnd - buffer -> writeCursor;
	// Subtract beginning of current read segment
	size -= buffer -> readCursor - buffer -> readSegment -> data;

	return size;
}

BKSize BKByteBufferGetOffset (BKByteBuffer * buffer)
{
	BKSize offset = -1;

	if (buffer -> info & BKByteBufferOptionKeepBytes) {
		offset = buffer -> readSize;

		// Add number of read bytes of current segment
		if (buffer -> readSegment)
			offset += buffer -> readCursor - buffer -> readSegment -> data;
	}

	return offset;
}

static BKUSize BKByteBufferRestoreBytes (BKByteBuffer * buffer, BKUSize size)
{
	BKUSize restoredSize  = 0;
	BKUSize remainingSize = buffer -> readCursor - buffer -> readSegment -> data;

	do {
		if (size <= remainingSize) {
			buffer -> readCursor -= size;
			restoredSize += size;
			break;
		}
		else {
			buffer -> readCursor -= remainingSize;
			restoredSize += remainingSize;
			size -= remainingSize;

			if (buffer -> readSegment -> previousSegment) {
				buffer -> readSegment = buffer -> readSegment -> previousSegment;
				buffer -> readDataEnd = & buffer -> readSegment -> data [buffer -> readSegment -> capacity];				
				buffer -> readCursor  = buffer -> readDataEnd;
				buffer -> capacity   += buffer -> readSegment -> capacity;
				buffer -> readSize   -= buffer -> readSegment -> capacity;
				remainingSize = buffer -> readCursor - buffer -> readSegment -> data;
			}
			else {
				// No more bytes to restore
				break;
			}
		}
	}
	while (size);

	return restoredSize;
}

static bool BKByteBufferSeekToOffset (BKByteBuffer * buffer, BKSize offset)
{
	BKByteBufferSegment * segment = buffer -> firstSegment;

	if (segment) {
		BKUSize readSize = 0;
		BKUSize capacity = 0;

		while (offset > 0) {
			if (offset > segment -> capacity) {
				offset   -= segment -> capacity;
				segment   = segment -> nextSegment;
				readSize += segment -> capacity;
			}
			else {
				break;
			}

			if (segment == NULL)
				return false;
		}

		if (segment == buffer -> writeSegment) {
			if (& segment -> data [offset] > buffer -> writeCursor)
				return false;

			buffer -> readDataEnd = buffer -> writeCursor;
		}
		else {
			buffer -> readDataEnd = & segment -> data [segment -> capacity];
		}

		buffer -> readSegment = segment;
		buffer -> readCursor  = & segment -> data [offset];
		buffer -> readSize    = readSize;

		// Calculate remaining capacity
		for (; segment; segment = segment -> nextSegment)
			capacity += segment -> capacity;

		buffer -> capacity = capacity;

		return true;
	}

	return false;
}

BKSize BKByteBufferSeek (BKByteBuffer * buffer, BKSize offset, unsigned options)
{
	if (options & BKByteBufferSeekRestore) {
		if (offset > 0)
			return BKByteBufferRestoreBytes (buffer, offset);
	}
	else if (options & BKByteBufferSeekSet) {
		if (offset >= 0 && (buffer -> info & BKByteBufferOptionKeepBytes))
			return BKByteBufferSeekToOffset (buffer, offset);
	}

	return -1;
}

void BKByteBufferClear (BKByteBuffer * buffer, unsigned options)
{
	if (options & BKByteBufferOptionDiscardReaded) {
		if ((buffer -> info & BKByteBufferOptionContinuousStorage) == false) {
			BKByteBufferSegment * segment = buffer -> readSegment;
			
			if (segment) {
				BKByteBufferSegment * previousSegment;
				
				for (segment = segment -> previousSegment; segment; segment = previousSegment) {
					previousSegment = segment -> previousSegment;
					buffer -> capacity -= segment -> capacity;
					buffer -> readSize -= segment -> capacity;
					
					// Add segment to free segments
					if (options & BKByteBufferOptionReuseStorage) {
						segment -> nextSegment = buffer -> freeSegments;
						buffer -> freeSegments = segment;
					}
					// Free segment
					else {
						BKByteBufferFreeSegment (segment, false);
					}
				}
				
				buffer -> readSegment -> previousSegment = NULL;
				buffer -> firstSegment = buffer -> readSegment;
			}
		}
	}
	else {
		// Add segments to free segments
		if (options & BKByteBufferOptionReuseStorage) {
			if (buffer -> writeSegment) {
				buffer -> writeSegment -> nextSegment = buffer -> freeSegments;
				buffer -> freeSegments = buffer -> writeSegment;
			}
		}
		// Free segments
		else {
			BKByteBufferFreeSegment (buffer -> firstSegment, true);
			BKByteBufferFreeSegment (buffer -> freeSegments, true);
			buffer -> freeSegments = NULL;
		}

		buffer -> capacity     = 0;
		buffer -> readSize     = 0;
		buffer -> firstSegment = NULL;
		buffer -> writeSegment = NULL;
		buffer -> writeCursor  = NULL;
		buffer -> writeDataEnd = NULL;
		buffer -> readSegment  = NULL;
		buffer -> readCursor   = NULL;
		buffer -> readDataEnd  = NULL;
	}
}
