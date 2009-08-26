/*
 * Syslink-IPC for TI OMAP Processors
 *
 * Copyright (C) 2009 Texas Instruments, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation version 2.1 of the License.
 *
 * This program is distributed .as is. WITHOUT ANY WARRANTY of any kind,
 * whether express or implied; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */
/*****************************************************************************/
/* util.h                                                                    */
/*****************************************************************************/
#ifndef UTIL_H
#define UTIL_H

#include <ctype.h>

#if !defined(__linux)

/*****************************************************************************/
/* STRCASECMP() - Case-insensitive strcmp.                                   */
/*****************************************************************************/
static int strcasecmp(const char* s1, const char* s2)
{
   char c1, c2;
   do { c1 = *s1++; c2 = *s2++; }
   while (c1 && c2 && (tolower(c1) == tolower(c2)));

   return tolower(c1) - tolower(c2);
}

/*****************************************************************************/
/* STRNCASECMP() - Case-insensitive strncmp.                                 */
/*****************************************************************************/
static int strncasecmp(const char* s1, const char* s2, size_t n)
{
   char c1, c2;

   if (!n) return 0;
   
   do { c1 = *s1++; c2 = *s2++; }
   while (--n && c1 && c2 && (tolower(c1) == tolower(c2)));

   return tolower(c1) - tolower(c2);
}

#endif

/*****************************************************************************/
/* Define MIN and MAX macros.                                                */
/*****************************************************************************/
#define MIN(x,y)	(((x) > (y)) ? (y) : (x))
#define MAX(x,y)	(((x) >= (y)) ? (x) : (y))

/*****************************************************************************/
/* C implementation of 'bool' type.                                          */
/*****************************************************************************/
typedef int BOOL;
#define TRUE    1
#define FALSE   0

#endif
