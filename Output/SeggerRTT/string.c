/*
 * Copyright (c) 2001, 2002 Red Hat, Inc.
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

/* Extracted from newlib-nano (needed by SEGGER_RTT implementation) */

#include <stddef.h>

char *strcpy(char *dst, const char *src)
{
	char *s = dst;

	while ( (*dst++ = *src++) )
		;

	return s;
}

size_t strlen(const char *str)
{
	const char *start = str;

	while (*str++ != '\0')
		;

	return str - start - 1;
}
