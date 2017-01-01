/* Copyright (C) 2016 by Jacob Alexander
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


// This file adds various functions that clang doesn't link properly
// AFAIK, clang doesn't have an elegant solution for this, so this is what we gotta do...

// ----- Includes -----

// Compiler Includes
#include <string.h>

// Not necessary when doing host compilation on macOS
#if !defined(_APPLE_)

void __aeabi_memcpy( void *dest, const void *src, size_t n )
{
	(void)memcpy(dest, src, n);
}

void __aeabi_memcpy4( void *dest, const void *src, size_t n )
{
	memcpy(dest, src, n);
}

void __aeabi_memclr( void *dest, size_t n )
{
	memset(dest, 0, n);
}

void __aeabi_memclr4( void *dest, size_t n )
{
	memset(dest, 0, n);
}

void __aeabi_memmove( void *dest, const void *src, size_t n )
{
	(void)memmove(dest, src, n);
}

void __aeabi_memset( void *s, size_t n, int c )
{
	(void)memset(s, c, n);
}

#endif

