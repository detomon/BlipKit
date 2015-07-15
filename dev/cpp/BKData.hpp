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
		BKDataInit (& data);

		if (BKDataSetFrames (& data, frames, numFrames, numChannels, true) != 0) {
			throw std::bad_alloc ();
		}
	}

	Data (char const * path, FILE * file, BKUInt numChannels, BKEnum params)
	{
		BKDataInit (& data);

		if (BKDataLoadRaw (& data, file, numChannels, params) != 0) {
			throw std::bad_alloc ();
		}
	}

	~Data () { BKDispose (& data); }

	BKInt setAttr (BKEnum attr, BKInt value) { return BKSetAttr (& data, attr, value); }
	BKInt getAttr (BKEnum attr, BKInt * outValue) const { return BKGetAttr (& data, attr, outValue); }

	BKInt setPtr (BKEnum attr, void * ptr, size_t size) { return BKSetPtr (& data, attr, ptr, size); }
	BKInt getPtr (BKEnum attr, void * outPtr, size_t size) const { return BKGetPtr (& data, attr, outPtr, size); }

	BKInt setFrames (BKFrame const * frames, BKUInt numFrames, BKUInt numChannels, BKInt copy) { return BKDataSetFrames (& data, frames, numFrames, numChannels, copy); }

	BKInt normalize (void) { return BKDataNormalize (& data); }

};

#endif /* ! _BK_DATA_CPP_H_ */
