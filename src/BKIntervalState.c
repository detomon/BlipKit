#include <stdio.h>
#include "BKBase.h"

typedef struct BKIntervalState BKIntervalState;

struct BKIntervalState
{
	BKInt  delta;
	BKInt  steps;
	BKInt  value;
	BKInt  deltaValue;
	BKUInt phase:2;
	BKUInt step:30;
};

static void BKIntervalStateSetValues (BKIntervalState * state, BKInt delta, BKInt steps)
{
	BKInt stepFrac, step;
	BKInt deltaValue, value;
	BKInt const stepShift = 8;
	
	if (steps < 0)
		steps = 0;
	
	if (steps) {
		if (steps != state -> steps || delta != state -> delta) {
			step = 0;

			if (state -> steps) {
				step = state -> step;
				// round up
				stepFrac = ((step << stepShift) + (1 << (stepShift - 1))) / state -> steps;
				step = (stepFrac * steps) >> stepShift;
			}
			
			deltaValue = delta / steps;

			//printf ("!!! %d\n", state -> phase);
			
			switch (state -> phase) {
				case 0: {  //  _/    |
					value = deltaValue * step;
					break;
				}
				case 1: {  //  \_    |
					value = deltaValue * (steps - step);
					deltaValue = -deltaValue;
					break;
				}
				case 2: {  //   ̅\    |
					value = -deltaValue * step;
					deltaValue = -deltaValue;
					break;
				}
				case 3: {  //  /̅    |
					value = -deltaValue * (steps - step);
					break;
				}
			}
			
			state -> delta      = delta;
			state -> steps      = steps;
			state -> value      = value;
			state -> deltaValue = deltaValue;
			state -> step       = step;
		}
	}
	else {
		state -> delta      = 0;
		state -> steps      = 0;
		state -> value      = 0;
		state -> deltaValue = 0;
		state -> phase      = 0;
		state -> step       = 0;
	}
}

static void BKIntervalStateTick (BKIntervalState * state)
{
	BKInt value, deltaValue;

	if (state -> steps) {
		if (state -> step < state -> steps - 1) {
			state -> value += state -> deltaValue;
			state -> step ++;
		}
		else {
			value      = state -> value;
			deltaValue = state -> deltaValue;

			state -> phase = (state -> phase + 1) & 0x3;  // % 4

			printf ("<->\n");
			
			switch (state -> phase) {
				case 0: {  //  _/    |
					value = 0;
					break;
				}
				case 1: {  //  \_    |
					value      = state -> delta;
					deltaValue = -deltaValue;
					break;
				}
				case 2: {  //   ̅\    |
					value = 0;
					break;
				}
				case 3: {  //  /̅    |
					value      = -state -> delta;
					deltaValue = -deltaValue;
					break;
				}
			}
			
			state -> value      = value;
			state -> deltaValue = deltaValue;
			state -> step       = 0;
		}
	}
}

int main (int argc, char const * argv [])
{
	int si = 10000;
	
	BKIntervalState state;
	
	memset (& state, 0, sizeof (state));

	BKIntervalStateSetValues (& state, 10000, 20);
	
	for (int i = 0; i < 1001; i ++) {
		printf ("%4d  %+5d  %+d\n", i, state.value, state.deltaValue);

		if (i >= 0) {
			//printf ("* %d\n", si);
			BKIntervalStateSetValues (& state, si, 20);
			
			if (si > 0)
				si -= 10;
		}

		BKIntervalStateTick (& state);
	}
	
	return 0;
}
