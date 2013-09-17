/**
 * Copyright (c) 2012-2013 Simon Schoenenberger
 * http://blipkit.monoxid.net/
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

typedef struct {
	BKComplexComp real;
	BKComplexComp imag;
} BKComplex;

/**
 * Make complex number with real and imaginary part
 */
static inline BKComplex BKComplexMake (BKComplexComp real, BKComplexComp imag)
{
	return (BKComplex) {real, imag};
}

/**
 * Get real part
 */
static inline BKComplexComp BKComplexReal (BKComplex c)
{
	return c.real;
}

/**
 * Get imaginary part
 */
static inline BKComplexComp BKComplexImag (BKComplex c)
{
	return c.imag;
}

/**
 * Add two complex numbers
 */
static inline BKComplex BKComplexAdd (BKComplex a, BKComplex b)
{
	return (BKComplex) {
		a.real + b.real,
		a.imag + b.imag
	};
}

/**
 * Subtract two complex numbers
 */
static inline BKComplex BKComplexSub (BKComplex a, BKComplex b)
{
	return (BKComplex) {
		a.real - b.real,
		a.imag - b.imag
	};
}

/**
 * Multiply two complex numbers
 */
static inline BKComplex BKComplexMult (BKComplex a, BKComplex b)
{
	return (BKComplex) {
		a.real * b.real + a.imag * -b.imag,
		a.real * b.imag + a.imag *  b.real
	};
}

/**
 * Divide two complex numbers
 */
static inline BKComplex BKComplexDiv (BKComplex a, BKComplex b)
{
	return (BKComplex) {
		(a.real * b.real + a.imag * b.imag) / (b.real * b.real + b.imag * b.imag),
		(a.imag * b.real - a.real * b.imag) / (b.real * b.real + b.imag * b.imag)
	};
}

#endif /* ! _BK_COMPLEX_H_ */
