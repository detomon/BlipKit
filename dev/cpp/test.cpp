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

#ifndef _BK_TEST_CPP_H_
#define _BK_TEST_CPP_H_

extern "C" {
	#include "BlipKit.h"
}

#include <iostream>
#include "BlipKitCPP.h"

using namespace std;
using namespace BlipKit;

int main (int argc, char const * argv [])
{
	Context ctx   = Context (2, 44100);
	Track   track = Track (BK_SQUARE);

	track.attach(ctx);

	track.setAttr(BK_MASTER_VOLUME, 0.2 * BK_MAX_VOLUME);
	track.setAttr(BK_VOLUME, BK_MAX_VOLUME);

	BKInt tremolo [2] = {24, 0.5 * BK_MAX_VOLUME};

	track.setEffect(BK_EFFECT_TREMOLO, tremolo, sizeof (tremolo));
	track.getEffect(BK_EFFECT_TREMOLO, tremolo, sizeof (tremolo));

	return 0;
}

#endif /* ! _BK_TEST_CPP_H_ */
