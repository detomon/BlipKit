/**
 * Copyright (c) 2012-2014 Simon Schoenenberger
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

#ifndef _BK_CONTEXT_CPP_H_
#define _BK_CONTEXT_CPP_H_

#include "BlipKit.h"
#include "BKClockCPP.h"

namespace BlipKit
{
	class Context;
}

class BlipKit::Context
{

public:
	BKContext ctx;

	Context (BKUInt numChannels, BKUInt sampleRate)
	{
		if (BKContextInit (& ctx, numChannels, sampleRate) < 0) {
			throw std::bad_alloc ();
		}
	}

	~Context () { BKContextDispose (& ctx); }

	BKInt setAttr (BKEnum attr, BKInt value) { return BKContextSetAttr (& ctx, attr, value); }
	BKInt getAttr (BKEnum attr, BKInt * outValue) const { return BKContextGetAttr (& ctx, attr, outValue); }

	BKInt setPtr (BKEnum attr, void * ptr) { return BKContextSetPtr (& ctx, attr, ptr); }
	BKInt getPtr (BKEnum attr, void * outPtr) const { return BKContextGetPtr (& ctx, attr, outPtr); }

	BKInt generate (BKFrame outFrames [], BKUInt size) { return BKContextGenerate (& ctx, outFrames, size); }
	void run (BKFUInt20 endTime) { BKContextRun (& ctx, endTime); }
	void end (BKFUInt20 endTime) { BKContextEnd (& ctx, endTime); }

	BKInt size (void) const { return BKContextSize (& ctx); }
	BKInt read (BKFrame outFrames [], BKUInt size) { return BKContextRead (& ctx, outFrames, size); }

	void reset (void) { BKContextReset (& ctx); }

	BKInt attachClock (BlipKit::Clock * clock) { return BKClockAttach (& clock -> clock, & ctx, NULL); }

	BKTime timeFromSeconds (double seconds) { return BKTimeFromSeconds (& ctx, seconds); }

	friend class Unit;
	friend class Track;

};

#endif /* ! _BK_CONTEXT_CPP_H_ */
