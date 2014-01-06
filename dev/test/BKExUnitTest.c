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

#include "BKExUnitTest.h"

static BKInt BKExUnitTestRun (BKExUnitTest * unit, BKFUInt20 endTime);
static void BKExUnitTestEnd (BKExUnitTest * unit, BKFUInt20 time);
static void BKExUnitTestReset (BKExUnitTest * unit);

static BKUnitFuncs const exUnitTestFuncs =
{
	.run   = (void *) BKExUnitTestRun,
	.end   = (void *) BKExUnitTestEnd,
	.reset = (void *) BKExUnitTestReset,
};

static BKInt BKExUnitTestRun (BKExUnitTest * unit, BKFUInt20 endTime)
{
	BKFUInt20 time;
	
	for (time = unit -> time; time < endTime; time += BK_FINT20_UNIT) {
		//BKBufferUpdateSample (& unit -> ctx -> channels [0], time, BK_MAX_VOLUME * 0.04);
		//BKBufferUpdateSample (& unit -> ctx -> channels [1], time, BK_MAX_VOLUME * -0.04);
	}

	unit -> time = time;

	return 0;
}

static void BKExUnitTestEnd (BKExUnitTest * unit, BKFUInt20 time)
{
	unit -> time -= time;
}

static void BKExUnitTestReset (BKExUnitTest * unit)
{
	//printf ("reset\n");
}

BKInt BKExUnitTestInit (BKExUnitTest * unit)
{
	memset (unit, 0, sizeof (BKExUnitTest));
	
	unit -> funcs = (BKUnitFuncs *) & exUnitTestFuncs;
	
	return 0;
}
