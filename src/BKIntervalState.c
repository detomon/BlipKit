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
	BKInt  stepDelta;
	BKInt  roundBias;
	BKUInt valueShift;
	BKInt  step;
};

struct BKIntervalState
{
	BKInt  delta;
	BKInt  steps;
	BKInt  value;
	BKInt  stepDelta;
	BKInt  roundBias;
	BKUInt valueShift;
	BKUInt phase;
	BKInt  step;
};

static BKUInt BKGetMaxValueShift (BKInt maxValue)
{
	BKInt shift = 0;

	if (maxValue) {
		while ((1 << shift) < maxValue && shift < 30)
			shift ++;

		shift = 30 - shift;
	}

	return shift;
}

/**
 * Initialize a slide state struct
 *
 * This struct is used to interpolate from value to another. Due to the usage of
 * variable length fixed point numbers even very small values can be used.
 *
 * `maxValue` defines the absolute positive or negative value which will be used
 * This value is used to determine the fraction length to interpolate the values
 */
static void BKSlideStateInit (BKSlideState * state, BKInt maxValue)
{
	memset (state, 0, sizeof (* state));

	state -> valueShift = BKGetMaxValueShift (maxValue);
}

/**
 * Set the `endValue` which should be slided to after `steps` steps
 *
 * When changing the values before the current slide has finished, the new slide
 * starts from the current interpolated value.
 *
 * `steps` should be a value between 0 and BK_STATE_MAX_STEPS. If it is 0 then
 * `endValue` is set immediately without sliding.
 *
 * The absolute value of `endValue` should not exceed `maxValue` given at
 * initialization or else calculation errors can occur.
 */
static void BKSlideStateSetValue (BKSlideState * state, BKInt endValue, BKInt steps)
{
	BKInt stepDelta, deltaValue;
	BKInt roundBias = 0;
	BKInt const stepShift = BK_STATE_STEP_FRAC_SHIFT;

	steps = BKClamp (steps, 0, BK_STATE_MAX_STEPS);

	if (steps) {
		endValue <<= state -> valueShift;
		deltaValue = endValue - state -> value;
		stepDelta  = deltaValue / steps;

		if ((deltaValue >> (state -> valueShift))) {
			roundBias = ((steps << stepShift) / (deltaValue >> state -> valueShift) / 2);
			roundBias = (roundBias * (stepDelta >> stepShift));
		}

		state -> endValue   = endValue;
		state -> stepDelta = stepDelta;
		state -> steps      = steps;
		state -> step       = steps;
		state -> roundBias  = roundBias;
	}
	else {
		state -> value        = (endValue << state -> valueShift);
		state -> endValue     = state -> value;
		state -> steps        = 0;
		state -> stepDelta   = 0;
		state -> step         = 0;
	}
}

/**
 * Make slide step
 *
 * Every call slides the value by one step until the end value is reached.
 */
static void BKSlideStateTick (BKSlideState * state)
{
	if (state -> step > 0) {
		state -> value += state -> stepDelta;
		state -> step --;

		if (state -> step <= 0)
			state -> value = state -> endValue;
	}
}

/**
 * Get the current interpolated value
 *
 * The `rounded` option can be used to add a better behaviour when using small
 * values. When set to 1 the value is rounded to the nearest value otherwise it
 * is always rounded down. In either case the option has no real impact on the
 * performance.
 *
 * For example, the step values for a slide from 0 to 1 in 10 steps without the
 * round option (0) look like this:
 *   0 0 0 0 0 0 0 0 0 1
 * With the round option on (1) they look like this:
 *   0 0 0 0 0 1 1 1 1 1
 * This also works with negative values.
 */
static BKInt BKSlideStateGetValue (BKSlideState * state, BKInt rounded)
{
	BKInt value = state -> value;

	if (rounded)
		value += state -> roundBias;

	value >>= state -> valueShift;

	return value;
}

/**
 * Initialize a interval state struct
 *
 * This struct is used to create a periodic oscillation between a maximum value
 * and its negative counterpart starting from 0.
 */
static void BKIntervalStateInit (BKIntervalState * state, BKInt maxValue)
{
	BKInt valueShift;

	memset (state, 0, sizeof (* state));

	valueShift = BKGetMaxValueShift (maxValue);
	// 1 additional bit is require for multiplication when recalculatating steps
	// so remove this 1 bit from the shift. Otherwise the sign is inverted by
	// bit overflow.
	valueShift = BKClamp (valueShift - 1, 0, 30);

	state -> valueShift = valueShift;
}

/**
 * Set the maximum and mimimum `delta` value and `steps` for each phase
 *
 * The oscillation has 4 phases and each phase has a duration of `steps` steps:
 *
 * 1. Beginning from 0 the value raises to the maxiumum `delta` value
 * 2. The value lowers to 0
 * 3. The value lowers to the negative `delta` value
 * 4. The value raises back to 0
 *
 * This phases are repeated until the values are changed. When changing the
 * values the current value and phase offset is recalculated for the new values.
 */
static void BKIntervalStateSetValues (BKIntervalState * state, BKInt delta, BKInt steps)
{
	BKInt stepFrac, step;
	BKInt stepDelta, value;
	BKInt roundBias = 0;
	BKInt const stepShift = BK_STATE_STEP_FRAC_SHIFT;
	
	steps = BKClamp (steps, 0, BK_STATE_MAX_STEPS);
	
	if (steps) {
		delta <<= state -> valueShift;

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

			stepDelta = delta / steps;

			if (delta >> state -> valueShift) {
				roundBias = ((steps << stepShift) / (delta >> state -> valueShift) / 2);
				roundBias = (roundBias * (stepDelta >> stepShift));
			}

			switch (state -> phase) {
				//  / ̅  raise from zero
				case 0: {
					value = stepDelta * step;
					break;
				}
				//  \_  lower from top value
				case 1: {
					value      = stepDelta * (steps - step);
					stepDelta = -stepDelta;
					break;
				}
				//   ̅\  lower from zero
				case 2: {
					value      = -stepDelta * step;
					stepDelta = -stepDelta;
					break;
				}
				//  / ̅  raise from bottom value
				case 3: {
					value = -stepDelta * (steps - step);
					break;
				}
			}
			
			state -> delta      = delta;
			state -> steps      = steps;
			state -> value      = value;
			state -> stepDelta = stepDelta;
			state -> roundBias  = roundBias;
			state -> step       = step;
		}
	}
	else {
		state -> delta      = 0;
		state -> steps      = 0;
		state -> value      = 0;
		state -> stepDelta = 0;
		state -> phase      = 0;
		state -> step       = 0;
	}
}

/**
 * Make oscillation step
 *
 * Every call sets the next oscillation step
 */
static void BKIntervalStateTick (BKIntervalState * state)
{
	BKInt value, stepDelta;

	if (state -> steps) {
		if (state -> step < state -> steps - 1) {
			state -> value += state -> stepDelta;
			state -> step ++;
		}
		else {
			value      = state -> value;
			stepDelta = state -> stepDelta;

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
					stepDelta = -stepDelta;
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
					stepDelta = -stepDelta;
					break;
				}
			}
			
			state -> value      = value;
			state -> stepDelta = stepDelta;
			state -> step       = 0;
		}
	}
}

static BKInt BKIntervalStateGetValue (BKIntervalState * state)
{
	BKInt value = state -> value;

	value += state -> roundBias;
	value >>= state -> valueShift;

	return value;
}

static void BKSlideStateTest (void)
{
	BKSlideState state;

	memset (& state, 0, sizeof (state));

	BKSlideStateInit (& state, 1000);

	BKSlideStateSetValue (& state, -1, 10);

	for (int i = 0; i < 100; i ++) {
		if (i == 50) {
			BKSlideStateSetValue (& state, 10, 50);
		}

		BKSlideStateTick (& state);
		printf ("%4d  %+5d  %+d\n", i, BKSlideStateGetValue (& state, 1), state.stepDelta);
	}
}

static void BKIntervalStateTest (void)
{
	BKInt si = 1000;
	
	BKIntervalState state;

	BKIntervalStateInit (& state, 1000);

	BKIntervalStateSetValues (& state, 1000, 20);

	for (BKInt i = 0; i < 1001; i ++) {
		printf ("%4d  %+5d  %+d\n", i, BKIntervalStateGetValue (& state), state.stepDelta);

		if (i >= 0) {
			BKIntervalStateSetValues (& state, si, 20);

			if (si > 0)
				si -= 1;
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
