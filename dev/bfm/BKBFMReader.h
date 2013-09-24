/**
 * Copyright (c) 2012-2013 Simon Schoenenberger
 * http://blipkit.monoxid.net/
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

#ifndef _BK_BFM_READER_BUFFER_H_
#define _BK_BFM_READER_BUFFER_H_

#include "BKByteBuffer.h"
#include "BKInstruction.h"

typedef struct BKBFMReader BKBFMReader;
typedef struct BKBFMToken  BKBFMToken;

/**
 *
 */
 /*
enum BKBFMTokenType
{
	BKBFMTokenTypeEnd        = 0x00,
	BKBFMTokenTypeGroupBegin = 0x01,
	BKBFMTokenTypeGroupEnd   = 0x02,
	BKBFMTokenTypeInteger    = 0x04,
	BKBFMTokenTypeString     = 0x06,
	BKBFMTokenTypeData       = 0x07,

	BKIntrAttack = 0x40,
	BKIntrArpeggio,
	BKIntrArpeggioSpeed,
	BKIntrRelease,
	BKIntrMute,
	BKIntrMuteTicks,
	BKIntrVolume,
	BKIntrPanning,
	BKIntrPitch,
	BKIntrMasterVolume,
	BKIntrStep,
	BKIntrEffect,
	BKIntrDutyCycle,
	BkIntrPhaseWrap,
	BKIntrInstrument,
	BKIntrInstrumentGroup,
	BKIntrWaveform,
	BKIntrWaveformGroup,
	BKIntrReturn,
	BKIntrGroup,
	BKIntrCall,
	BKIntrJump,
	BKIntrEnd,
	BKIntrStepTicks,

	BKIntrTrackGroup,
	BKIntrSequenceVolume,
	BKIntrSequencePanning,
	BKIntrSequenceArpeggio,
	BKIntrSequenceDutyCycle,
};
*/
/**
 *
 */
struct BKBFMReader
{
	BKUInt       flags;
	BKInt        valueCapacity;
	BKInt        valueSize;
	void       * value;
	BKByteBuffer buffer;
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

#endif /* ! _BK_BFM_WRITER_BUFFER_H_ */

