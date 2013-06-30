#ifndef _TRACK_H_
#define _TRACK_H_

#include <SDL/SDL.h>
#include "BlipKit.h"
#include "BKCompiler.h"

typedef struct {
	BKTrack       * track;
	BKInterpreter * interpreter;
} BKSDLUserData;

typedef struct {
	BKTrack       track;
	BKDivider     divider;
	BKInterpreter interpreter;
	BKSDLUserData userData;
} BKSDLTrack;

typedef struct {
	BKContext      ctx;
	BKUInt         speed;
	BKInstrument * instruments [256];
	BKUInt         numInstruments;
	BKData       * waveforms [256];
	BKUInt         numWaveforms;
	BKSDLTrack   * tracks [256];
	BKUInt         numTracks;
} BKSDLContext;

/**
 *
 */
extern BKInt BKSDLContextInit (BKSDLContext * ctx, BKUInt numChannels, BKUInt sampleRate);

/**
 *
 */
extern void BKSDLContextDispose (BKSDLContext * ctx);

/**
 *
 */
extern BKInt BKSDLContextLoadData (BKSDLContext * ctx, void const * data, size_t size);

/**
 *
 */
extern BKInt BKSDLContextLoadFile (BKSDLContext * ctx, char const * filename);

/**
 *
 */
extern void BKSDLContextUnloadData (BKSDLContext * ctx);

#endif /* ! _TRACK_H_ */
