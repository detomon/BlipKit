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
