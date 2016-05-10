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

#ifndef _BK_COMPLEX_H_
#define _BK_COMPLEX_H_

/**
 * The complex component type
 */
typedef double BKComplexComp;

/**
 * The complex type
 */
typedef struct {
	BKComplexComp re;
	BKComplexComp im;
} BKComplex;

/**
 * Make complex number with real and imaginary part
 */
BK_INLINE BKComplex BKComplexMake (BKComplexComp re, BKComplexComp im)
{
	return (BKComplex) {re, im};
}

/**
 * Get real part
 */
BK_INLINE BKComplexComp BKComplexReal (BKComplex c)
{
	return c.re;
}

/**
 * Get imaginary part
 */
BK_INLINE BKComplexComp BKComplexImag (BKComplex c)
{
	return c.im;
}

/**
 * Add two complex numbers
 */
BK_INLINE BKComplex BKComplexAdd (BKComplex a, BKComplex b)
{
	return (BKComplex) {
		a.re + b.re,
		a.im + b.im
	};
}

/**
 * Subtract two complex numbers
 */
BK_INLINE BKComplex BKComplexSub (BKComplex a, BKComplex b)
{
	return (BKComplex) {
		a.re - b.re,
		a.im - b.im
	};
}

/**
 * Multiply two complex numbers
 */
BK_INLINE BKComplex BKComplexMult (BKComplex a, BKComplex b)
{
	return (BKComplex) {
		a.re * b.re + a.im * -b.im,
		a.re * b.im + a.im *  b.re
	};
}

/**
 * Divide two complex numbers
 */
BK_INLINE BKComplex BKComplexDiv (BKComplex a, BKComplex b)
{
	return (BKComplex) {
		(a.re * b.re + a.im * b.im) / (b.re * b.re + b.im * b.im),
		(a.im * b.re - a.re * b.im) / (b.re * b.re + b.im * b.im)
	};
}

/**
 * Conjugate complex number
 */
BK_INLINE BKComplex BKComplexConj (BKComplex a)
{
	return (BKComplex) {a.re, -a.im};
}

#endif /* ! _BK_COMPLEX_H_ */
