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

#ifndef _BK_CLOCK_CPP_H_
#define _BK_CLOCK_CPP_H_

#include "BlipKit.h"

namespace BlipKit
{
	class Clock;
	class Divider;
}

class BlipKit::Clock
{

public:
	BKClock clock;

	Clock (BKTime period, BKCallback * callback) { BKClockInit (& clock, period, callback); }

	void detach () { return BKClockDetach (& clock); }

	void reset () { BKClockReset (& clock); }

	friend class Divider;
	friend class Context;

};

class BlipKit::Divider
{

private:
	BKDivider divider;
	
public:
	Divider (BKUInt count, BKCallback * callback) { BKDividerInit (& divider, count, callback); }
	
	void attach (BlipKit::Clock * clock) { BKDividerAttachToClock (& divider, & clock -> clock); }
	void detach (void) { BKDividerDetach (& divider); }
	
	void reset (void) { BKDividerReset (& divider); }

};

#endif /* ! _BK_CLOCK_CPP_H_ */
