/**
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

#ifndef _BK_BASE_H_
#define _BK_BASE_H_

#define __USE_POSIX

#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Version
 */
#define BK_VERSION "0.13.1"
extern char const * const BKVersion;

/**
 * Settings
 */
#ifndef BK_USE_64_BIT
#define BK_USE_64_BIT 1
#endif

/**
 * Integers and fixed point numbers
 */
#define BK_INT_SHIFT    32
#define BK_FRAME_SHIFT  16
#define BK_VOLUME_SHIFT 15

typedef int32_t  BKInt;
typedef uint32_t BKUInt;
typedef int16_t  BKFrame;

typedef long          BKSize;
typedef unsigned long BKUSize;

#define BK_FINT20_SHIFT 20
#define BK_FINT20_UNIT (1 << BK_FINT20_SHIFT)
#define BK_FINT20_FRAC (BK_FINT20_UNIT - 1)

typedef int32_t  BKFInt20;   // 12.20 fixed point signed
typedef uint32_t BKFUInt20;  // 12.20 fixed point

/**
 * Limits
 */
#define BK_INT_MAX ((1U << (BK_INT_SHIFT - 1)) - 1)
#define BK_FRAME_MAX ((1U << (BK_FRAME_SHIFT - 1)) - 1)

#define BK_MAX_CHANNELS 8

#define BK_MIN_PERIOD (BK_FINT20_SHIFT / BK_TRIANGLE_PHASES)
#define BK_MAX_PERIOD (1 << (BK_FINT20_SHIFT + 4))
#define BK_MAX_VOLUME ((1 << BK_VOLUME_SHIFT) - 1)

#define BK_MIN_DUTY_CYCLE 1
#define BK_MAX_DUTY_CYCLE (BK_SQUARE_PHASES - 1)
#define BK_MAX_ARPEGGIO 8

#define BK_MIN_SAMPLE_RATE 16000
#define BK_MAX_SAMPLE_RATE 96000

#define BK_MIN_SAMPLE_PERIOD (1 << (BK_FINT20_SHIFT - 8))
#define BK_MAX_SAMPLE_PERIOD (1 << (BK_FINT20_SHIFT + 8))

#define BK_MAX_GENERATE_SAMPLES ((1 << (BK_INT_SHIFT - BK_FINT20_SHIFT)) / 4)

/**
 * Wave phases
 */
#define BK_SQUARE_PHASES   16
#define BK_TRIANGLE_PHASES 32
#define BK_NOISE_PHASES     8
#define BK_SAWTOOTH_PHASES  7
#define BK_SINE_PHASES     32

/**
 * Default values
 */
#define BK_DEFAULT_SAMPLE_RATE    44100
#define BK_DEFAULT_CLOCK_RATE       240
#define BK_DEFAULT_ARPEGGIO_DIVIDER   4
#define BK_DEFAULT_INSTR_DIVIDER      4
#define BK_DEFAULT_EFFECT_DIVIDER     1
#define BK_DEFAULT_DUTY_CYCLE         2

/**
 * Misc
 */
#define BK_FIRST_ELEMENT_PTR ((void *) -1)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * Macro functions
 */
#define BKMin(a, b) ((a) < (b) ? (a) : (b))
#define BKMax(a, b) ((a) > (b) ? (a) : (b))
#define BKClamp(a, l, h) ((a) < (l) ? (l) : ((a) > (h) ? (h) : (a)))
#define BKAbs(a) ((a) < 0 ? -(a) : (a))
#define BKCmp(a, b) ((a) < (b) ? -1 : ((a) > (b) ? 1 : 0))

#define BKBitSet(var, mask) ((var) |= (mask))
#define BKBitUnset(var, mask) ((var) &= ~(mask))

/**
 * Contitionally set or clear bit mask
 *
 * `cond` must be 0 or 1
 * http://graphics.stanford.edu/~seander/bithacks.html#ConditionalSetOrClearBitsWithoutBranching
 */
#define BKBitSetCond(var, mask, cond) ((var) ^= (-(cond) ^ (var)) & (mask))

/**
 * Set mask if `cond` is 1
 */
#define BKBitCond(mask, cond) (~((cond) - 1) & mask)

/**
 * Set mask if cond is != 0
 */
#define BKBitCond2(mask, cond) (~(((cond) != 0) - 1) & mask)

/**
 * Enum type
 */
typedef unsigned BKEnum;

/**
 * Define offsetof if needed
 */
#ifndef offsetof
#define offsetof(s, f) ((BKSize) & ((s *) NULL) -> f)
#endif

/**
 * Time struct
 */
#if BK_USE_64_BIT

typedef int64_t BKTime;

#else

typedef struct
{
	BKInt     time;
	BKFUInt20 frac;
} BKTime;

#endif

/**
 * Some global types
 */
typedef struct BKContext BKContext;

typedef struct BKCallbackInfo BKCallbackInfo;
typedef struct BKCallback     BKCallback;

typedef BKEnum (* BKCallbackFunc) (BKCallbackInfo * info, void * userInfo);

struct BKCallback
{
	BKCallbackFunc func;
	void         * userInfo;
};

struct BKCallbackInfo
{
	void * object;
	BKEnum event;
	BKTime nextTime;
	BKUInt divider;
};

/**
 * Attributes
 */
#define BK_ATTR_TYPE_SHIFT 12
#define BK_ATTR_TYPE_MASK (~((1 << BK_ATTR_TYPE_SHIFT) - 1))

/**
 * Inline keyword
 */
#ifndef _MSC_VER
#	define BK_INLINE static inline
#else
#	define BK_INLINE static __inline
#endif

/**
 * Context attributes
 */
enum
{
	BK_CONTEXT_ATTR_TYPE = (1 << BK_ATTR_TYPE_SHIFT),
	BK_NUM_CHANNELS,
	BK_SAMPLE_RATE,
	BK_TIME,
};

/**
 * Track attributes
 */
enum
{
	BK_TRACK_ATTR_TYPE = (2 << BK_ATTR_TYPE_SHIFT),
	BK_WAVEFORM,
	BK_DUTY_CYCLE,
	BK_PERIOD,
	BK_PHASE,
	BK_PHASE_WRAP,
	BK_NUM_PHASES,
	BK_MASTER_VOLUME,
	BK_VOLUME,
	BK_VOLUME_0,
	BK_VOLUME_1,
	BK_VOLUME_2,
	BK_VOLUME_3,
	BK_VOLUME_4,
	BK_VOLUME_5,
	BK_VOLUME_6,
	BK_VOLUME_7,
	BK_MUTE,
	BK_PITCH,
	BK_SAMPLE_RANGE,         // full range to play
	BK_SAMPLE_REPEAT,        // repeat mode BK_NO_REPEAT, BK_REPEAT, BK_PALINDROME
	BK_SAMPLE_SUSTAIN_RANGE, // sustain repeat range
	BK_SAMPLE_IMMED_RELEASE, // should immediately jump to sustain end when releasing
	BK_SAMPLE_PERIOD,
	BK_SAMPLE_PITCH,
	BK_SAMPLE_CALLBACK,
	BK_FLAG_RELEASE,         // Set release phase
	BK_NOTE,
	BK_ARPEGGIO,
	BK_PANNING,
	BK_INSTRUMENT,
	BK_CLOCK_PERIOD,
	BK_ARPEGGIO_DIVIDER,
	BK_EFFECT_DIVIDER,
	BK_INSTRUMENT_DIVIDER,
	BK_TRIANGLE_IGNORES_VOLUME,
};

/**
 * Data attributes
 */
enum
{
	BK_DATA_ATTR_TYPE = (3 << BK_ATTR_TYPE_SHIFT),
	BK_NUM_FRAMES,
};

/**
 * Waveforms
 */
enum
{
	BK_WAVEFORM_TYPE = (4 << BK_ATTR_TYPE_SHIFT),
	BK_SQUARE,
	BK_TRIANGLE,
	BK_NOISE,
	BK_SAWTOOTH,
	BK_SINE,
	BK_CUSTOM,
	BK_SAMPLE,
};

/**
 * Effects
 */
enum
{
	BK_EFFECT_TYPE = (5 << BK_ATTR_TYPE_SHIFT),
	BK_EFFECT_VOLUME_SLIDE,
	BK_EFFECT_PANNING_SLIDE,
	BK_EFFECT_PORTAMENTO,
	BK_EFFECT_TREMOLO,
	BK_EFFECT_VIBRATO,
};

/**
 * Events in callback info
 */
enum
{
	BK_EVENT_TYPE = (6 << BK_ATTR_TYPE_SHIFT),
	BK_EVENT_CLOCK,
	BK_EVENT_DIVIDER,
	BK_EVENT_SAMPLE_BEGIN,
	BK_EVENT_SAMPLE_RESET,
};

/**
 * Repeat options
 */
enum
{
	BK_NO_REPEAT,
	BK_REPEAT,
	BK_PALINDROME,
};

/**
 * Return codes
 */
enum
{
	BK_RETURN_TYPE = (-7 << BK_ATTR_TYPE_SHIFT),
	BK_ALLOCATION_ERROR,
	BK_INVALID_ATTRIBUTE,
	BK_INVALID_VALUE,
	BK_INVALID_STATE,
	BK_INVALID_NUM_CHANNELS,
	BK_INVALID_NUM_FRAMES,
	BK_INVALID_NUM_BITS,
	BK_INVALID_RETURN_VALUE,
	BK_FILE_ERROR,
	BK_FILE_NOT_READABLE_ERROR,
	BK_FILE_NOT_WRITABLE_ERROR,
	BK_FILE_NOT_SEEKABLE_ERROR,
	BK_OTHER_ERROR,
};

/**
 * Get name of error
 */
extern char const * BKStatusGetName (BKEnum status);

#if __GNUC__
#define BK_DEPRECATED_FUNC(msg) __attribute__((deprecated(msg)))
#endif

#endif /* !_BK_BASE_H_  */
