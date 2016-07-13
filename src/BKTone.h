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

/**
 * @file
 *
 * Contains constants and functions for calculating note and sample pitches.
 */

#ifndef _BK_NOTE_H_
#define _BK_NOTE_H_

#include "BKBase.h"

#define BK_MIN_NOTE BK_C_0 ///< The minimum note.
#define BK_MAX_NOTE BK_C_8 ///< The maximum note.

#define BK_NOTE_RELEASE -1 ///< The attribute value for releasing the note.
#define BK_NOTE_MUTE    -2 ///< The attribute value for muting the note.

#define BK_MIN_PIANO_TONE (3 - 4 * 12 + BK_C_0)        ///< Minimum note offset for table generator script.
#define BK_MAX_PIANO_TONE (BK_MIN_PIANO_TONE + BK_C_8) ///< Maximum note offset for table generator script.

#define BK_MIN_SAMPLE_TONE (-4 * 12 + BK_C_0)            ///< Minimum sample pitch.
#define BK_MAX_SAMPLE_TONE (BK_MIN_SAMPLE_TONE + BK_C_8) ///< Maximum sample pitch.

#define BK_TONE_SHIFT 16                  ///< Bit shift used for calculating note.
#define BK_TONE_UNIT (1 << BK_TONE_SHIFT) ///< Unit factor used for calculating note.
#define BK_TONE_FRAC (BK_TONE_UNIT - 1)   ///< Fraction mask used for calculating note.

#define BK_TONE_SAMPLE_RATE_SHIFT 18 ///< Bit shift used for calculating sample pitch.

/**
 * Defines named notes.
 */
enum
{
	BK_C_0, BK_C_SH_0, BK_D_0, BK_D_SH_0, BK_E_0, BK_F_0, BK_F_SH_0,
	BK_G_0, BK_G_SH_0, BK_A_0, BK_A_SH_0, BK_B_0,
	BK_C_1, BK_C_SH_1, BK_D_1, BK_D_SH_1, BK_E_1, BK_F_1, BK_F_SH_1,
	BK_G_1, BK_G_SH_1, BK_A_1, BK_A_SH_1, BK_B_1,
	BK_C_2, BK_C_SH_2, BK_D_2, BK_D_SH_2, BK_E_2, BK_F_2, BK_F_SH_2,
	BK_G_2, BK_G_SH_2, BK_A_2, BK_A_SH_2, BK_B_2,
	BK_C_3, BK_C_SH_3, BK_D_3, BK_D_SH_3, BK_E_3, BK_F_3, BK_F_SH_3,
	BK_G_3, BK_G_SH_3, BK_A_3, BK_A_SH_3, BK_B_3,
	BK_C_4, BK_C_SH_4, BK_D_4, BK_D_SH_4, BK_E_4, BK_F_4, BK_F_SH_4,
	BK_G_4, BK_G_SH_4, BK_A_4, BK_A_SH_4, BK_B_4,
	BK_C_5, BK_C_SH_5, BK_D_5, BK_D_SH_5, BK_E_5, BK_F_5, BK_F_SH_5,
	BK_G_5, BK_G_SH_5, BK_A_5, BK_A_SH_5, BK_B_5,
	BK_C_6, BK_C_SH_6, BK_D_6, BK_D_SH_6, BK_E_6, BK_F_6, BK_F_SH_6,
	BK_G_6, BK_G_SH_6, BK_A_6, BK_A_SH_6, BK_B_6,
	BK_C_7, BK_C_SH_7, BK_D_7, BK_D_SH_7, BK_E_7, BK_F_7, BK_F_SH_7,
	BK_G_7, BK_G_SH_7, BK_A_7, BK_A_SH_7, BK_B_7,
	BK_C_8,
};

/**
 * Calculate sample period from a note using the sample rate.
 */
extern BKFUInt20 BKTonePeriodLookup (BKFInt20 tone, BKUInt sampleRate);

/**
 * Calculate log2 of a note used for sample pitches.
 */
extern BKFUInt20 BKLog2PeriodLookup (BKFInt20 tone);

#endif /* ! _BK_NOTE_H_ */
