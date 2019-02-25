/*
  Basic UTF-8 manipulation routines
  by Jeff Bezanson
  placed in the public domain Fall 2005
  Modifications by Jacob Alexander 2019

  This code is designed to provide the utilities you need to manipulate
  UTF-8 as an internal string encoding. These functions do not perform the
  error checking normally needed when handling UTF-8 data, so if you happen
  to be from the Unicode Consortium you will want to flay me alive.
  I do this because error checking can be performed at the boundaries (I/O),
  with these routines reserved for higher performance on data known to be
  valid.
*/

// ----- System Includes -----

#include <stdint.h>
/*
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
*/



// ----- Local Includes -----

#include "utf8.h"

static const uint32_t offsetsFromUTF8[6] = {
	0x00000000UL, 0x00003080UL, 0x000E2080UL,
	0x03C82080UL, 0xFA082080UL, 0x82082080UL
};



// ----- Defines -----

#define NULL 0



// ----- Constants -----

static const char trailingBytesForUTF8[256] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};



// ----- Functions -----

/* returns length of next utf-8 sequence */
int u8_seqlen(char *s)
{
	return trailingBytesForUTF8[(unsigned int)(unsigned char)s[0]] + 1;
}

int u8_wc_toutf8(char *dest, uint32_t ch)
{
	if (ch < 0x80) {
		dest[0] = (char)ch;
		return 1;
	}
	if (ch < 0x800) {
		dest[0] = (ch>>6) | 0xC0;
		dest[1] = (ch & 0x3F) | 0x80;
		return 2;
	}
	if (ch < 0x10000) {
		dest[0] = (ch>>12) | 0xE0;
		dest[1] = ((ch>>6) & 0x3F) | 0x80;
		dest[2] = (ch & 0x3F) | 0x80;
		return 3;
	}
	if (ch < 0x110000) {
		dest[0] = (ch>>18) | 0xF0;
		dest[1] = ((ch>>12) & 0x3F) | 0x80;
		dest[2] = ((ch>>6) & 0x3F) | 0x80;
		dest[3] = (ch & 0x3F) | 0x80;
		return 4;
	}
	return 0;
}

/* charnum => byte offset */
int u8_offset(char *str, int charnum)
{
	int offs=0;

	while (charnum > 0 && str[offs]) {
		(void)(isutf(str[++offs]) || isutf(str[++offs]) ||
			   isutf(str[++offs]) || ++offs);
		charnum--;
	}
	return offs;
}

/* byte offset => charnum */
int u8_charnum(char *s, int offset)
{
	int charnum = 0, offs=0;

	while (offs < offset && s[offs]) {
		(void)(isutf(s[++offs]) || isutf(s[++offs]) ||
			   isutf(s[++offs]) || ++offs);
		charnum++;
	}
	return charnum;
}

/* number of characters */
int u8_strlen(char *s)
{
	int count = 0;
	int i = 0;

	while (u8_nextchar(s, &i) != 0)
		count++;

	return count;
}

/* reads the next utf-8 sequence out of a string, updating an index */
uint32_t u8_nextchar(char *s, int *i)
{
	uint32_t ch = 0;
	int sz = 0;

	do {
		ch <<= 6;
		ch += (unsigned char)s[(*i)++];
		sz++;
	} while (s[*i] && !isutf(s[*i]));
	ch -= offsetsFromUTF8[sz-1];

	return ch;
}

void u8_inc(char *s, int *i)
{
	(void)(isutf(s[++(*i)]) || isutf(s[++(*i)]) ||
		   isutf(s[++(*i)]) || ++(*i));
}

void u8_dec(char *s, int *i)
{
	(void)(isutf(s[--(*i)]) || isutf(s[--(*i)]) ||
		   isutf(s[--(*i)]) || --(*i));
}

char *u8_strchr(char *s, uint32_t ch, int *charn)
{
	int i = 0, lasti=0;
	uint32_t c;

	*charn = 0;
	while (s[i]) {
		c = u8_nextchar(s, &i);
		if (c == ch) {
			return &s[lasti];
		}
		lasti = i;
		(*charn)++;
	}
	return NULL;
}

char *u8_memchr(char *s, uint32_t ch, uint32_t sz, int *charn)
{
	uint32_t i = 0, lasti=0;
	uint32_t c;
	int csz;

	*charn = 0;
	while (i < sz) {
		c = csz = 0;
		do {
			c <<= 6;
			c += (unsigned char)s[i++];
			csz++;
		} while (i < sz && !isutf(s[i]));
		c -= offsetsFromUTF8[csz-1];

		if (c == ch) {
			return &s[lasti];
		}
		lasti = i;
		(*charn)++;
	}
	return NULL;
}

