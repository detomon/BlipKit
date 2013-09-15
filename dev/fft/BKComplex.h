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
 * Use built-in complex type by default
 */
#ifndef BK_USE_COMPLEX
#define BK_USE_COMPLEX 1
#endif

/**
 * The complex component type
 */
typedef double BKComplexComp;

/**
 * Define macros to allow using fallbacks when working with the complex type on
 * systems which do not support it
 */
#if BK_USE_COMPLEX

#include <complex.h>

#define BKComplexMake(r, i) ((r) + (i) * I)
#define BKComplexReal(a) (creal (a))
#define BKComplexImag(a) (cimag (a))
#define BKComplexAdd(a, b) ((a) + (b))
#define BKComplexSub(a, b) ((a) - (b))
#define BKComplexMult(a, b) ((a) * (b))
#define BKComplexDiv(a, b) ((a) / (b))

typedef double complex BKComplex; 

/**
 * Define fallback macros
 */
#else

#define BKComplexMake(r, i) ((BKComplex) {(r), (i)})
#define BKComplexReal(a) ((a).real)
#define BKComplexImag(a) ((a).imag)
#define BKComplexAdd(a, b) ((BKComplex) {((a).real + (b).real), (b).real + (b).real})
#define BKComplexSub(a, b) ((BKComplex) {((a).real - (b).real), (b).real - (b).real})
#define BKComplexMult(a, b) ((BKComplex) {((a).real * (b).real - (a).imag * (b).imag), \
                                       ((a).real * (b).imag + (a).imag * (b).real)})
#define BKComplexDiv(a, b) ((a) / (b))
#error Define BKComplexDiv!

typedef struct {
	BKComplexComp real;
	BKComplexComp imag;
} BKComplex;  

#endif

#endif /* ! _BK_COMPLEX_H_ */
