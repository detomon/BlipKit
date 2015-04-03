/**
 * Copyright (c) 2012-2015 Simon Schoenenberger
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

#include "BKByteBuffer.h"

#define OPTIONS_MASK        0x00FF
#define MIN_SEGMENT_SIZE    0x4000
#define MORE_STORAGE_FACTOR 1.25

static BKByteBufferSegment * BKByteBufferSegmentAlloc (BKUSize capacity)
{
	BKByteBufferSegment * segment = malloc (sizeof (BKByteBufferSegment) + capacity);

	if (segment) {
		memset (segment, 0, sizeof (* segment));
		segment -> capacity = capacity;
	}

	return segment;
}

static void BKByteBufferFreeSegment (BKByteBufferSegment * segment, BKInt freeList)
{
	// free all linked segments
	if (freeList) {
		for (BKByteBufferSegment * nextSegment = NULL; segment; segment = nextSegment) {
			nextSegment = segment -> nextSegment;
			free (segment);
		}
	}
	// free single segment
	else if (segment) {
		free (segment);
	}
}

static BKInt BKByteBufferPushStorage (BKByteBuffer * buffer, BKUSize preferredSize)
{
	BKByteBufferSegment * segment = NULL;

	// single segment
	if (buffer -> info & BKByteBufferOptionContinuousStorage) {
		BKUSize usedSize   = 0;
		BKUSize capacity   = 0;
		BKSize  readOffset = 0;

		segment = buffer -> readSegment;

		if (segment) {
			if (buffer -> info & BKByteBufferOptionKeepBytes) {
				usedSize   = buffer -> writeDataEnd - segment -> data;
				readOffset = buffer -> readCursor - segment -> data;
			}
			else {
				usedSize   = buffer -> writeDataEnd - buffer -> readCursor;
			}

			capacity = buffer -> readSegment -> capacity;
		}

		preferredSize = (usedSize + preferredSize) * MORE_STORAGE_FACTOR;
		preferredSize = BKMax (preferredSize, MIN_SEGMENT_SIZE);

		if ((buffer -> info & BKByteBufferOptionKeepBytes) != 0) {
			memmove (segment -> data, buffer -> readCursor, usedSize);
		}

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
				return -1;
			}
		}

		return 0;
	}
	// multiple segments
	else {
		// reuse segments
		if (buffer -> freeSegments) {
			segment = buffer -> freeSegments;
			buffer -> freeSegments = segment -> nextSegment;
		}
		// allocate new segment
		else {
			preferredSize *= MORE_STORAGE_FACTOR;
			preferredSize = BKMax (preferredSize, MIN_SEGMENT_SIZE);
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

			return 0;
		}
		else {
			return -1;
		}
	}

	return -1;
}

static BKSize BKByteBufferShiftStorage (BKByteBuffer * buffer)
{
	BKSize shiftSize = 0;

	// single segment
	if (buffer -> info & BKByteBufferOptionContinuousStorage) {
		// asking source for new data
		if (buffer -> source) {
			BKSize writeSize = buffer -> source -> read (buffer, buffer -> source);

			if (writeSize > 0) {
				shiftSize = writeSize;
			}
			else if (writeSize != 0) { // error
				shiftSize = -1;
			}
		}
	}
	// multiple segments
	else {
		// shift to next segment
		if (buffer -> readSegment && buffer -> readSegment -> nextSegment) {
			BKByteBufferSegment * nextSegment = buffer -> readSegment -> nextSegment;

			// add segment to free segments
			if ((buffer -> info & BKByteBufferOptionKeepBytes) != 0) {
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
			}
			else {
				buffer -> readDataEnd = & nextSegment -> data [nextSegment -> capacity];
			}

			shiftSize = buffer -> readSegment -> capacity;
		}
		// asking source for new data
		else if (buffer -> source) {
			BKSize writeSize = buffer -> source -> read (buffer, buffer -> source);

			if (writeSize >= 0) {
				shiftSize = writeSize;
			}
			else {
				shiftSize = -1;
			}
		}
	}

	return shiftSize;
}

BKInt BKByteBufferInit (BKByteBuffer * buffer, BKUSize initSize, unsigned options)
{
	memset (buffer, 0, sizeof (* buffer));
	buffer -> info |= (options & OPTIONS_MASK);

	if (initSize) {
		if (BKByteBufferPushStorage (buffer, initSize) != 0) {
			return -1;
		}
	}

	return 0;
}
void BKByteBufferDispose (BKByteBuffer * buffer)
{
	BKByteBufferFreeSegment (buffer -> firstSegment, 1);
	BKByteBufferFreeSegment (buffer -> freeSegments, 1);

	memset (buffer, 0, sizeof (* buffer));
}

void BKByteBufferSetSource (BKByteBuffer * buffer, BKByteBufferSource * source)
{
	buffer -> source = source;
}

BKSize BKByteBufferReadBytes (BKByteBuffer * buffer, void * bytes, BKUSize size)
{
	BKSize  shiftSize;
	BKUSize readSize       = 0;
	BKUSize remainaingSize = buffer -> readDataEnd - buffer -> readCursor;

	do {
		if (size <= remainaingSize) {
			if (bytes) {
				memcpy (bytes, buffer -> readCursor, size);
			}

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

			shiftSize = BKByteBufferShiftStorage (buffer);

			if (shiftSize > 0) {
				remainaingSize = buffer -> readDataEnd - buffer -> readCursor;
			}
			else if (shiftSize == 0) {
				// mo more bytes to read
				break;
			}
			else {
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
				return -1;
			}
		}
	}

	return (* buffer -> readCursor ++);
}

void * BKByteBufferGetBytes (BKByteBuffer * buffer)
{
	if (buffer -> info & BKByteBufferOptionContinuousStorage) {
		if (buffer -> readSegment) {
			return buffer -> readSegment -> data;
		}
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

			if (buffer -> readSegment == buffer -> writeSegment) {
				buffer -> readDataEnd = buffer -> writeCursor;
			}

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

			if (BKByteBufferPushStorage (buffer, size) == 0) {
				remainaingSize = buffer -> writeDataEnd - buffer -> writeCursor;
			}
			else {
				return -1;
			}
		}
	}
	while (size);

	return writtenSize;
}

BKSize BKByteBufferWriteByte (BKByteBuffer * buffer, unsigned char byte)
{
	if (buffer -> writeCursor >= buffer -> writeDataEnd) {
		if (BKByteBufferPushStorage (buffer, 1) != 0) {
			return -1;
		}
	}

	(* buffer -> writeCursor ++) = byte;

	if (buffer -> readSegment == buffer -> writeSegment) {
		buffer -> readDataEnd = buffer -> writeCursor;
	}

	return 1;
}

BKUSize BKByteBufferGetSize (BKByteBuffer * buffer)
{
	BKUSize size = buffer -> capacity;

	// subtract end of current write segment
	size -= buffer -> writeDataEnd - buffer -> writeCursor;
	// subtract beginning of current read segment
	size -= buffer -> readCursor - buffer -> readSegment -> data;

	return size;
}

BKSize BKByteBufferGetOffset (BKByteBuffer * buffer)
{
	BKSize offset = -1;

	if (buffer -> info & BKByteBufferOptionKeepBytes) {
		offset = buffer -> readSize;

		// add number of read bytes of current segment
		if (buffer -> readSegment) {
			offset += buffer -> readCursor - buffer -> readSegment -> data;
		}
	}

	return offset;
}

static BKUSize BKByteBufferRestoreBytes (BKByteBuffer * buffer, BKUSize size)
{
	BKUSize restoredSize  = 0;
	BKUSize beginningSize = buffer -> readCursor - buffer -> readSegment -> data;

	do {
		if (size <= beginningSize) {
			buffer -> readCursor -= size;
			restoredSize += size;
			break;
		}
		else {
			buffer -> readCursor -= beginningSize;
			restoredSize += beginningSize;
			size -= beginningSize;

			if (buffer -> readSegment -> previousSegment) {
				buffer -> readSegment = buffer -> readSegment -> previousSegment;
				buffer -> readDataEnd = & buffer -> readSegment -> data [buffer -> readSegment -> capacity];
				buffer -> readCursor  = buffer -> readDataEnd;
				buffer -> capacity   += buffer -> readSegment -> capacity;
				buffer -> readSize   -= buffer -> readSegment -> capacity;
				beginningSize = buffer -> readCursor - buffer -> readSegment -> data;
			}
			else {
				// no more bytes to restore
				break;
			}
		}
	}
	while (size);

	return restoredSize;
}

static BKInt BKByteBufferSeekToOffset (BKByteBuffer * buffer, BKSize offset)
{
	BKByteBufferSegment * segment = buffer -> firstSegment;

	if (segment) {
		BKUSize readSize = 0;
		BKUSize capacity = 0;

		while (offset > 0) {
			if (offset < segment -> capacity) {
				break;
			}

			offset -= segment -> capacity;
			segment = segment -> nextSegment;

			if (segment == NULL) {
				return -1;
			}

			readSize += segment -> capacity;
		}

		if (segment == buffer -> writeSegment) {
			// can't seek beyond write cursor
			if (& segment -> data [offset] > buffer -> writeCursor) {
				return -1;
			}

			buffer -> readDataEnd = buffer -> writeCursor;
		}
		else {
			buffer -> readDataEnd = & segment -> data [segment -> capacity];
		}

		buffer -> readSegment = segment;
		buffer -> readCursor  = & segment -> data [offset];
		buffer -> readSize    = readSize;

		// calculate remaining capacity
		for (; segment; segment = segment -> nextSegment) {
			capacity += segment -> capacity;
		}

		buffer -> capacity = capacity;

		return 0;
	}

	return -1;
}

BKSize BKByteBufferSeek (BKByteBuffer * buffer, BKSize offset, unsigned options)
{
	if (options & BKByteBufferSeekRestore) {
		if (offset > 0) {
			return BKByteBufferRestoreBytes (buffer, offset);
		}
	}
	else if (options & BKByteBufferSeekSet) {
		if (offset >= 0 && (buffer -> info & BKByteBufferOptionKeepBytes)) {
			return BKByteBufferSeekToOffset (buffer, offset);
		}
	}

	return -1;
}

void BKByteBufferClear (BKByteBuffer * buffer, unsigned options)
{
	if (options & BKByteBufferOptionDiscardReaded) {
		if ((buffer -> info & BKByteBufferOptionContinuousStorage) != 0) {
			BKByteBufferSegment * segment = buffer -> readSegment;

			if (segment) {
				BKByteBufferSegment * previousSegment;

				for (segment = segment -> previousSegment; segment; segment = previousSegment) {
					previousSegment = segment -> previousSegment;
					buffer -> capacity -= segment -> capacity;
					buffer -> readSize -= segment -> capacity;

					// add segment to free segments
					if (options & BKByteBufferOptionReuseStorage) {
						segment -> nextSegment = buffer -> freeSegments;
						buffer -> freeSegments = segment;
					}
					// free segment
					else {
						BKByteBufferFreeSegment (segment, 0);
					}
				}

				buffer -> readSegment -> previousSegment = NULL;
				buffer -> firstSegment = buffer -> readSegment;
			}
		}
	}
	else {
		// add segments to free segments
		if (options & BKByteBufferOptionReuseStorage) {
			if (buffer -> writeSegment) {
				buffer -> writeSegment -> nextSegment = buffer -> freeSegments;
				buffer -> freeSegments = buffer -> writeSegment;
			}
		}
		// free segments
		else {
			BKByteBufferFreeSegment (buffer -> firstSegment, 1);
			BKByteBufferFreeSegment (buffer -> freeSegments, 1);
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
