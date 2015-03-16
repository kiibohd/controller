/* Copyright (c) 2011,2012 Simon Schubert <2@0x2c.org>.
 * Modifications by Jacob Alexander 2014 <haata@kiibohd.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MCHCK_CDEFS_H
#define _MCHCK_CDEFS_H

// ----- Compiler Includes -----

#include <sys/param.h>



// ----- Defines & Macros -----


#define _CONCAT(x,y) _CONCAT1(x,y)
#define _CONCAT1(x,y) x ## y
#define _STR(a) #a

typedef __CHAR16_TYPE__ char16_t;

#define __packed __attribute__((__packed__))

/* From FreeBSD: compile-time asserts */
#define CTASSERT(x)             _Static_assert(x, _STR(x))

#define CTASSERT_SIZE_BYTE(t, s)     CTASSERT(sizeof(t) == (s))
#define CTASSERT_SIZE_BIT(t, s)     CTASSERT(sizeof(t) * 8 == (s))

#define UNION_STRUCT_START(size)                                \
	union {                                                 \
	_CONCAT(_CONCAT(uint, size), _t) raw;                 \
	struct {                                                \
	/* just to swallow the following semicolon */           \
	struct _CONCAT(_CONCAT(__dummy_, __COUNTER__), _t) {}

#define UNION_STRUCT_END                        \
	}; /* struct */                         \
	}; /* union */


/**
 * From <news:dqgm2f$ije$1@sunnews.cern.ch>,
 * <https://groups.google.com/forum/#!topic/comp.std.c/d-6Mj5Lko_s>
 */
#define __PP_NARG(...)                          \
	__PP_NARG_(__0, ## __VA_ARGS__, __PP_RSEQ_N())
#define __PP_NARG_(...)                         \
	__PP_ARG_N(__VA_ARGS__)
#define __PP_ARG_N(                                     \
	_1, _2, _3, _4, _5, _6, _7, _8, _9,_10,         \
	_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,        \
	_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,        \
	_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,        \
	_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,        \
	_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,        \
	_61,_62,_63,N,...) N
#define __PP_RSEQ_N()                           \
	62,61,60,                               \
		59,58,57,56,55,54,53,52,51,50,  \
		49,48,47,46,45,44,43,42,41,40,  \
		39,38,37,36,35,34,33,32,31,30,  \
		29,28,27,26,25,24,23,22,21,20,  \
		19,18,17,16,15,14,13,12,11,10,  \
		9,8,7,6,5,4,3,2,1,0

/**
 * From <https://github.com/pfultz2/Cloak/wiki/C-Preprocessor-tricks,-tips,-and-idioms>
 */
#define __CAT(a, ...) __PRIMITIVE_CAT(a, __VA_ARGS__)
#define __PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__

#define __IIF(c) __PRIMITIVE_CAT(__IIF_, c)
#define __IIF_0(t, ...) __VA_ARGS__
#define __IIF_1(t, ...) t

#define __COMPL(b) __PRIMITIVE_CAT(__COMPL_, b)
#define __COMPL_0 1
#define __COMPL_1 0

#define __CHECK_N(x, n, ...) n
#define __CHECK(...) __CHECK_N(__VA_ARGS__, 0,)
#define __PROBE(x) x, 1,

#define __NOT(x) __CHECK(__PRIMITIVE_CAT(__NOT_, x))
#define __NOT_0 __PROBE(~)

#define __BOOL(x) __COMPL(__NOT(x))
#define __IF(c) __IIF(__BOOL(c))

#define __EAT(...)
#define __EXPAND(...) __VA_ARGS__
#define __WHEN(c) __IF(c)(__EXPAND, __EAT)

#define __HEAD(h, ...) h
#define __TAIL(h, ...) __VA_ARGS__

#define __EVAL(...)  __EVAL1(__EVAL1(__EVAL1(__VA_ARGS__)))
#define __EVAL1(...) __EVAL2(__EVAL2(__EVAL2(__VA_ARGS__)))
#define __EVAL2(...) __EVAL3(__EVAL3(__EVAL3(__VA_ARGS__)))
#define __EVAL3(...) __EVAL4(__EVAL4(__EVAL4(__VA_ARGS__)))
#define __EVAL4(...) __EVAL5(__EVAL5(__EVAL5(__VA_ARGS__)))
#define __EVAL5(...) __EVAL6(__EVAL6(__EVAL6(__VA_ARGS__)))
#define __EVAL6(...) __VA_ARGS__

#define __EMPTY()
#define __DEFER(id) id __EMPTY()
#define __OBSTRUCT(...) __VA_ARGS__ __DEFER(__EMPTY)()
#define __CAT_ARG(f, a) __OBSTRUCT(f) a

#define __REPEAT(...) __EVAL(__REPEAT_(__VA_ARGS__))
#define __REPEAT_INNER(...) __OBSTRUCT(__REPEAT_INDIRECT) () (__VA_ARGS__)
#define __REPEAT_INDIRECT() __REPEAT_
#define __REPEAT_(iter, itermacro, macro, a, ...)                       \
	__OBSTRUCT(macro)(iter, a)                                      \
	__WHEN(__PP_NARG(__VA_ARGS__))                                  \
		(                                                       \
			__OBSTRUCT(__REPEAT_INDIRECT) () (              \
				itermacro(iter, a), itermacro, macro, __VA_ARGS__ \
				)                                       \
			)

#endif

