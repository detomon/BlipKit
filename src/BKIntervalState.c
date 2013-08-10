#include <stdio.h>
#include "BKBase.h"

#define BK_STATE_STEP_FRAC_SHIFT 8
#define BK_STATE_MAX_STEPS (1 << (32 - BK_STATE_STEP_FRAC_SHIFT - 1))

typedef struct BKSlideState    BKSlideState;
typedef struct BKIntervalState BKIntervalState;

struct BKSlideState
{
	BKInt  endValue;
	BKInt  steps;
	BKInt  value;
	BKInt  deltaValue;
	BKInt  roundBias;
	BKUInt valueShift;
	BKInt  step;
};

struct BKIntervalState
{
	BKInt  delta;
	BKInt  steps;
	BKInt  value;
	BKInt  deltaValue;
	BKUInt phase;
	BKInt  step;
};

static void BKSlideStateInit (BKSlideState * state, BKInt maxValue)
{
	BKInt shift = 0;

	memset (state, 0, sizeof (* state));

	if (maxValue) {
		while ((1 << shift) < maxValue && shift < 30)
			shift ++;

		shift = 30 - shift;
	}

	state -> valueShift = shift;
}

static void BKSlideStateSetValue (BKSlideState * state, BKInt endValue, BKInt steps)
{
	BKInt deltaValue;
	BKInt roundBias;
	BKInt const stepShift = BK_STATE_STEP_FRAC_SHIFT;

	steps = BKClamp (steps, 0, BK_STATE_MAX_STEPS);

	if (steps) {
		endValue <<= state -> valueShift;
		deltaValue = (endValue - state -> value) / steps;

		roundBias = ((steps << stepShift) / ((endValue - state -> value) >> (state -> valueShift)) / 2);
		roundBias = (roundBias * (deltaValue >> stepShift));

		state -> endValue   = endValue;
		state -> deltaValue = deltaValue;
		state -> steps      = steps;
		state -> step       = steps;
		state -> roundBias  = roundBias;
	}
	else {
		state -> value        = endValue >> state -> valueShift;
		state -> endValue     = state -> value;
		state -> steps        = 0;
		state -> deltaValue   = 0;
		state -> step         = 0;
	}
}

static void BKSlideStateTick (BKSlideState * state)
{
	if (state -> steps) {
		if (state -> step > 0) {
			state -> value += state -> deltaValue;
			state -> step --;
		}
		else {
			state -> value = state -> endValue;
		}
	}
}

static BKInt BKSlideStateGetValue (BKSlideState * state, BKInt rounded)
{
	BKInt value = state -> value;

	if (rounded)
		value += state -> roundBias;

	value >>= state -> valueShift;

	return value;
}

static void BKIntervalStateInit (BKIntervalState * state)
{
	memset (state, 0, sizeof (* state));
}

static void BKIntervalStateSetValues (BKIntervalState * state, BKInt delta, BKInt steps)
{
	BKInt stepFrac, step;
	BKInt deltaValue, value;
	BKInt const stepShift = BK_STATE_STEP_FRAC_SHIFT;
	
	steps = BKClamp (steps, 0, BK_STATE_MAX_STEPS);
	
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

static void BKSlideStateTest (void)
{
	BKSlideState state;

	memset (& state, 0, sizeof (state));

	BKSlideStateInit (& state, 1000);

	BKSlideStateSetValue (& state, 1, 10);

	for (int i = 0; i < 100; i ++) {
		if (i == 50) {
			BKSlideStateSetValue (& state, 10, 50);
		}

		BKSlideStateTick (& state);
		printf ("%4d  %+5d  %+d\n", i, BKSlideStateGetValue (& state, 1), state.deltaValue);
	}
}

static void BKIntervalStateTest (void)
{
	BKInt si = 10000;
	
	BKIntervalState state;
	
	memset (& state, 0, sizeof (state));

	BKIntervalStateSetValues (& state, 10000, 20);

	for (BKInt i = 0; i < 1001; i ++) {
		printf ("%4d  %+5d  %+d\n", i, state.value, state.deltaValue);

		if (i >= 0) {
			BKIntervalStateSetValues (& state, si, 20);

			if (si > 0)
				si -= 10;
		}

		BKIntervalStateTick (& state);
	}
}

int main (int argc, char const * argv [])
{
	printf ("*** Slide Test ***\n\n");

	BKSlideStateTest ();

	printf ("\n*** Interval Test ***\n\n");

	BKIntervalStateTest ();

	return 0;
}
