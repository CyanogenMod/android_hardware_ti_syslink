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
/** ============================================================================
 *  @file   String.h
 *
 *  @brief      Defines for String manipulation library.
 *
 *  ============================================================================
 */


#ifndef UTILS_STRING_H_0XEFC8
#define UTILS_STRING_H_0XEFC8


#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 *  APIs
 * =============================================================================
 */
/* Function to find offset of string in the given string */
Int String_find (String as1, String as2);

/* Function to add a string at the end of another one */
String String_cat (String s1, String s2);

/* Function to return a pointer to the first occurrence of the character c in the string sp */
String String_chr (String sp, Int c);

/* Function to compare two string */
Int String_cmp (String s1, String s2);

/* Function to compare two string only first n characters */
Int String_ncmp (String s1, String s2, UInt32 n);

/* Function to copy strings */
String String_cpy (String s1, String s2);

/* Function to copy first n bytes from s2 to s1 */
String String_ncpy (String s1, String s2, UInt32 n);

/* Function to locate a substring */
String String_str (String haystack, String needle);

/* Function to returns the number of characters in s */
Int String_len (String s);

/* Function to get string representation of a hex number */
Int String_hexToStr (String s, UInt32 hex);

/* Function to calculate hash for a string */
UInt32 String_hash (String s);


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* STRING_H_0X5B4D */
