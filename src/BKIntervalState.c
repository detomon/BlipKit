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
				step <<= stepShift;
				// round up division by adding half of step fraction
				// this will make calculation with small values more accurate
				step += (1 << (stepShift - 1));
				stepFrac = step / state -> steps;
				step = (stepFrac * steps) >> stepShift;
			}

			deltaValue = delta / steps;
			
			switch (state -> phase) {
				//  / ̅  raise from zero
				case 0: {
					value = deltaValue * step;
					break;
				}
				//  \_  lower from top value
				case 1: {
					value      = deltaValue * (steps - step);
					deltaValue = -deltaValue;
					break;
				}
				//   ̅\  lower from zero
				case 2: {
					value      = -deltaValue * step;
					deltaValue = -deltaValue;
					break;
				}
				//  / ̅  raise from bottom value
				case 3: {
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

			// cycle phase from 0 to 3
			state -> phase = (state -> phase + 1) & 0x3;  // % 4
			
			switch (state -> phase) {
				//  / ̅  raise from zero
				case 0: {
					value = 0;
					break;
				}
				//  \_  lower from top value
				case 1: {
					value      = state -> delta;
					deltaValue = -deltaValue;
					break;
				}
				//   ̅\  lower from zero
				case 2: {
					value = 0;
					break;
				}
				//  / ̅  raise from bottom value
				case 3: {
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
			BKIntervalStateSetValues (& state, si, 20);
			
			if (si > 0)
				si -= 10;
		}

		BKIntervalStateTick (& state);
	}
	
	return 0;
}
