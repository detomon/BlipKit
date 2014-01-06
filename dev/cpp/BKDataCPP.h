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

#ifndef _BK_DATA_CPP_H_
#define _BK_DATA_CPP_H_

#include "BlipKit.h"

namespace BlipKit
{
	class Data;
}

class BlipKit::Data
{

public:
	BKData data;

	Data (const Data & other)
	{
		if (BKDataInitCopy (& data, & other.data) < 0)
			throw std::bad_alloc ();
	}

	Data (BKFrame const * frames, BKUInt numFrames, BKUInt numChannels)
	{
		if (BKDataInitWithFrames (& data, frames, numFrames, numChannels, true) < 0)
			throw std::bad_alloc ();
	}

	Data (char const * path, BKEnum bits, BKUInt numChannels, BKEnum endian)
	{
		if (BKDataInitAndLoadRawAudio (& data, path, bits, numChannels, endian) < 0)
			throw std::bad_alloc ();
	}

	~Data () { BKDataDispose (& data); }

	BKInt setAttr (BKEnum attr, BKInt value) { return BKDataSetAttr (& data, attr, value); }
	BKInt getAttr (BKEnum attr, BKInt * outValue) const { return BKDataGetAttr (& data, attr, outValue); }

	BKInt setPtr (BKEnum attr, void * ptr) { return BKDataSetPtr (& data, attr, ptr); }
	BKInt getPtr (BKEnum attr, void * outPtr) const { return BKDataGetPtr (& data, attr, outPtr); }

	BKInt setFrames (BKFrame const * frames, BKUInt numFrames, BKUInt numChannels, BKInt copy) { return BKDataSetFrames (& data, frames, numFrames, numChannels, copy); }

	BKInt normalize (void) { return BKDataNormalize (& data); }

};

#endif /* ! _BK_DATA_CPP_H_ */
