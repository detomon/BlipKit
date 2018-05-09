/*
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

#ifndef _BK_INTERPOLATION_H_
#define _BK_INTERPOLATION_H_

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

/**
 * Initialize a slide state struct.
 *
 * This struct is used to interpolate from value to another. Due to the usage of
 * variable length fixed point numbers even very small values can be used.
 *
 * @p maxValue defines the absolute positive or negative value which will be used.
 * This value is used to determine the fraction length to interpolate the values.
 *
 * @param state The state to initialize.
 * @param maxValue The maximum value used.
 */
extern void BKSlideStateInit (BKSlideState * state, BKInt maxValue);

/**
 * Set the @p endValue which should be slided to after @p steps steps.
 *
 * When changing the values before the current slide has finished, the new slide
 * starts from the current interpolated value.
 *
 * @p steps should be a value between 0 and BK_STATE_MAX_STEPS. If it is 0 then
 * @p endValue is set immediately without sliding.
 *
 * The absolute value of @p endValue should not exceed @p maxValue given at
 * initialization or else calculation errors can occur.
 *
 * @param state The state to update.
 * @param endValue The value to slide to.
 * @param steps The number of steps in which the value should be reached.
 */
extern void BKSlideStateSetValueAndSteps (BKSlideState * state, BKInt endValue, BKInt steps);

/**
 * Set only the @p endValue which should be slided to.
 *
 * This is the same as calling BKSlideStateSetValueAndSteps without changing
 * the @steps.
 *
 * @param state The state to update.
 * @param endValue The new value to slide to.
 */
BK_INLINE void BKSlideStateSetValue (BKSlideState * state, BKInt endValue);

/**
 * Set only the `steps` to reach the @endValue.
 *
 * This is the same as calling BKSlideStateSetValueAndSteps without changing
 * the @p endValue.
 *
 * @param state The state to update.
 * @param steps The new number of steps.
 */
BK_INLINE void BKSlideStateSetSteps (BKSlideState * state, BKInt steps);

/**
 * Halt slide.
 *
 * Following calls to BKSlideStateStep will not change the state value
 * anymore.
 *
 * If @p setEndValue is 1 the state value is set immediately to @p endValue
 * otherwise the interpolated value keeps its current value.
 *
 * @param state The state to halt.
 * @param setEndValue 0 if the current value should be kept; 1 if the end value
 *   should be set.
 */
BK_INLINE void BKSlideStateHalt (BKSlideState * state, BKInt setEndValue);

/**
 * Make slide step.
 *
 * Every call slides the value by one step until the end value is reached.
 *
 * @param state The state to advance.
 */
BK_INLINE void BKSlideStateStep (BKSlideState * state);

/**
 * Get the current interpolated value.
 *
 * @param state The state to get the value from.
 * @return The state value.
 */
BK_INLINE BKInt BKSlideStateGetValue (BKSlideState const * state);

/**
 * Initialize a interval state struct.
 *
 * This struct is used to create a periodic oscillation between a maximum value
 * and its negative counterpart starting from 0.
 *
 * @p maxValue defines the absolute @p delta value used
 * This value is used to determine the fraction length to interpolate the values
 *
 * @param state The state to initialize.
 * @param maxValue The maximum value used.
 */
extern void BKIntervalStateInit (BKIntervalState * state, BKInt maxValue);

/**
 * Set the maximum and mimimum @p delta value and @p steps for each phase.
 *
 * The oscillation has 4 phases and each phase has a duration of @p steps:
 *
 * 1. Beginning from 0 the value raises to the maxiumum @p delta value
 * 2. The value lowers to 0
 * 3. The value lowers to the negative @p delta value
 * 4. The value raises back to 0
 *
 * This phases are repeated until the values are changed. When changing the
 * values the current value and phase offset is recalculated for the new values.
 *
 * @param state The state to update.
 * @param delta The delta value.
 * @param steps The number of steps.
 */
extern void BKIntervalStateSetDeltaAndSteps (BKIntervalState * state, BKInt delta, BKInt steps);

/**
 * Set only the maximum and mimimum @p delta value.
 *
 * This is the same as calling BKIntervalStateSetDeltaAndSteps withou  changing
 * the @p steps.
 *
 * @param state The state to update.
 * @param delta The delta value.
 */
BK_INLINE void BKIntervalStateSetDelta (BKIntervalState * state, BKInt delta);

/**
 * Set only @p steps for each phase.
 *
 * This is the same as calling BKIntervalStateSetDeltaAndSteps without changing
 the @p delta value.
 *
 * @param state The state to update.
 * @param steps The number of steps.
 */
BK_INLINE void BKIntervalStateSetSteps (BKIntervalState * state, BKInt steps);

/**
 * Make oscillation step.
 *
 * Every call sets the next oscillation step.
 *
 * @param state The state to advance.
 */
extern void BKIntervalStateStep (BKIntervalState * state);

/**
 * Get the current oscillated value.
 */
BK_INLINE BKInt BKIntervalStateGetValue (BKIntervalState const * state);


BK_INLINE void BKSlideStateSetValue (BKSlideState * state, BKInt endValue)
{
	BKSlideStateSetValueAndSteps (state, endValue, state -> steps);
}

BK_INLINE  void BKSlideStateSetSteps (BKSlideState * state, BKInt steps)
{
	BKSlideStateSetValueAndSteps (state, (state -> endValue >> state -> valueShift), steps);
}

BK_INLINE  void BKSlideStateHalt (BKSlideState * state, BKInt setEndValue)
{
	state -> step = 0;

	if (setEndValue) {
		state -> value = state -> endValue;
	}
}

BK_INLINE  void BKSlideStateStep (BKSlideState * state)
{
	if (state -> step > 0) {
		state -> value += state -> stepDelta;
		state -> step --;

		if (state -> step <= 0) {
			state -> value = state -> endValue;
		}
	}
}

BK_INLINE  BKInt BKSlideStateGetValue (BKSlideState const * state)
{
	BKInt value = state -> value;

	// Without round bias the step values from 0 to 1 in 10 steps
	// would look like this:
	//   0 0 0 0 0 0 0 0 0 1
	// With round bias they look like this:
	//   0 0 0 0 0 1 1 1 1 1
	// This also works with negative values

	value += state -> roundBias;
	value >>= state -> valueShift;

	return value;
}

BK_INLINE void BKIntervalStateSetDelta (BKIntervalState * state, BKInt delta)
{
	BKIntervalStateSetDeltaAndSteps (state, delta, state -> steps);
}

BK_INLINE void BKIntervalStateSetSteps (BKIntervalState * state, BKInt steps)
{
	BKIntervalStateSetDeltaAndSteps (state, (state -> delta >> state -> valueShift), steps);
}

BK_INLINE BKInt BKIntervalStateGetValue (BKIntervalState const * state)
{
	BKInt value = state -> value;

	// Without round bias the step values from 0 to 1 in 10 steps
	// would look like this:
	//   0 0 0 0 0 0 0 0 0 1
	// With round bias they look like this:
	//   0 0 0 0 0 1 1 1 1 1
	// This also works with negative values

	value += state -> roundBias;
	value >>= state -> valueShift;

	return value;
}

#endif /* ! _BK_INTERPOLATION_H_ */
