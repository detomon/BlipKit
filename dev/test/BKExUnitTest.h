#ifndef _BK_EXUNIT_TEST_
#define _BK_EXUNIT_TEST_

#include "BKUnit.h"

typedef struct BKExUnitTest BKExUnitTest;

struct BKExUnitTest
{
	BKContext   * ctx;
	BKUnitFuncs * funcs;

	BKUnit * prevUnit;
	BKUnit * nextUnit;
	
	BKFUInt20 time;
};

extern BKInt BKExUnitTestInit (BKExUnitTest * unit);

#endif /* ! _BK_EXUNIT_TEST_ */
