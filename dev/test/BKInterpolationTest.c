#include "BKInterpolation.h"
#include <stdio.h>

static void BKSlideStateTest(void) {
	BKSlideState state;

	memset(&state, 0, sizeof(state));

	BKSlideStateInit(&state, 1000);

	BKSlideStateSetValueAndSteps(&state, -1, 10);

	for (int i = 0; i < 100; i++) {
		if (i == 50) {
			BKSlideStateSetValueAndSteps(&state, 10, 50);
		}

		BKSlideStateStep(&state);
		printf("%4d  %+5d  %+d\n", i, BKSlideStateGetValue(&state), state.stepDelta);
	}
}

static void BKIntervalStateTest(void) {
	BKInt si = 1000;

	BKIntervalState state;

	BKIntervalStateInit(&state, 1000);

	BKIntervalStateSetDeltaAndSteps(&state, 1000, 20);

	for (BKInt i = 0; i < 1001; i++) {
		printf("%4d  %+5d  %+d\n", i, BKIntervalStateGetValue(&state), state.stepDelta);

		if (i >= 0) {
			BKIntervalStateSetDelta(&state, si);

			if (si > 0) {
				si -= 1;
			}
		}

		BKIntervalStateStep(&state);
	}
}

int main(int argc, char const* argv[]) {
	printf("*** Slide Test ***\n\n");

	BKSlideStateTest();

	printf("\n*** Interval Test ***\n\n");

	BKIntervalStateTest();

	return 0;
}
