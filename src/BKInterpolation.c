#include "BKInterpolation.h"

static BKUInt BKGetMaxValueShift(BKInt maxValue) {
	BKInt shift = 0;

	if (maxValue) {
		while ((1 << shift) < maxValue && shift < 30) {
			shift++;
		}

		shift = 30 - shift;
	}

	return shift;
}

void BKSlideStateInit(BKSlideState* state, BKInt maxValue) {
	memset(state, 0, sizeof(*state));

	state->valueShift = BKGetMaxValueShift(maxValue);
}

void BKSlideStateSetValueAndSteps(BKSlideState* state, BKInt endValue, BKInt steps) {
	BKInt roundBias = 0;
	BKInt const stepShift = BK_STATE_STEP_FRAC_SHIFT;

	steps = BKClamp(steps, 0, BK_STATE_MAX_STEPS);

	if (steps) {
		endValue <<= state->valueShift;
		BKInt deltaValue = endValue - state->value;
		BKInt stepDelta = deltaValue / steps;

		// prevent division by 0
		if ((deltaValue >> state->valueShift)) {
			roundBias = ((steps << stepShift) / (deltaValue >> state->valueShift) / 2);
			roundBias = (roundBias * (stepDelta >> stepShift));
		}

		state->endValue = endValue;
		state->stepDelta = stepDelta;
		state->steps = steps;
		state->step = steps;
		state->roundBias = roundBias;
	}
	else {
		state->value = (endValue << state->valueShift);
		state->endValue = state->value;
		state->steps = 0;
		state->stepDelta = 0;
		state->step = 0;
	}
}

void BKIntervalStateInit(BKIntervalState* state, BKInt maxValue) {
	memset(state, 0, sizeof(*state));

	BKInt valueShift = BKGetMaxValueShift(maxValue);
	// 1 additional bit is require for multiplication when recalculatating steps
	// so remove this 1 bit from the shift. Otherwise the sign is inverted by
	// bit overflow.
	valueShift = BKClamp(valueShift - 1, 0, 30);

	state->valueShift = valueShift;
}

void BKIntervalStateSetDeltaAndSteps(BKIntervalState* state, BKInt delta, BKInt steps) {
	BKInt const stepShift = BK_STATE_STEP_FRAC_SHIFT;

	steps = BKClamp(steps, 0, BK_STATE_MAX_STEPS);

	if (steps) {
		delta <<= state->valueShift;

		if (steps != state->steps || delta != state->delta) {
			BKInt step = 0;

			if (state->steps) {
				step = state->step;
				step <<= stepShift;
				// round up division by adding half of step fraction
				// this will make calculation with small values more accurate
				step += (1 << (stepShift - 1));
				BKInt stepFrac = step / state->steps;
				step = (stepFrac * steps) >> stepShift;
			}

			BKInt stepDelta = delta / steps;
			BKInt roundBias = 0;

			// prevent division by 0
			if (delta >> state->valueShift) {
				roundBias = ((steps << stepShift) / (delta >> state->valueShift) / 2);
				roundBias = (roundBias * (stepDelta >> stepShift));
			}

			BKInt value = 0;

			switch (state->phase) {
				//  / ̅  raise from zero
				case 0: {
					value = stepDelta * step;
					break;
				}
				//  \_  lower from top value
				case 1: {
					value = stepDelta * (steps - step);
					stepDelta = -stepDelta;
					break;
				}
				//   ̅\  lower from zero
				case 2: {
					value = -stepDelta * step;
					stepDelta = -stepDelta;
					break;
				}
				//  / ̅  raise from bottom value
				case 3: {
					value = -stepDelta * (steps - step);
					break;
				}
			}

			state->delta = delta;
			state->steps = steps;
			state->value = value;
			state->stepDelta = stepDelta;
			state->roundBias = roundBias;
			state->step = step;
		}
	}
	else {
		state->delta = 0;
		state->steps = 0;
		state->value = 0;
		state->stepDelta = 0;
		state->phase = 0;
		state->step = 0;
	}
}

void BKIntervalStateStep(BKIntervalState* state) {
	if (state->steps) {
		if (state->step < state->steps - 1) {
			state->value += state->stepDelta;
			state->step++;
		}
		else {
			BKInt value = state->value;
			BKInt stepDelta = state->stepDelta;

			// cycle phase from 0 to 3
			state->phase = (state->phase + 1) & 0x3; // % 4

			switch (state->phase) {
				//  / ̅  raise from zero
				case 0: {
					value = 0;
					break;
				}
				//  \_  lower from top value
				case 1: {
					value = state->delta;
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
					value = -state->delta;
					stepDelta = -stepDelta;
					break;
				}
			}

			state->value = value;
			state->stepDelta = stepDelta;
			state->step = 0;
		}
	}
}
