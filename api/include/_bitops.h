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
 *  @file   _bitops.h
 *
 *
 *  @desc   Defines generic macros for bitwise opeartions.
 *
 *  ============================================================================
 */


#if !defined (_BITOPS_H)
#define _BITOPS_H


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/** ============================================================================
 *  @macro  SET_BITS
 *
 *  @desc   Sets bits contained in given mask.
 *          This macro is independent of operand width. User must ensure
 *          correctness.
 *  ============================================================================
 */
#define SET_BITS(num,mask)          ((num) |= (mask))

/** ============================================================================
 *  @macro  CLEAR_BITS
 *
 *  @desc   Clears bits contained in given mask.
 *          This macro is independent of operand width. User must ensure
 *          correctness.
 *  ============================================================================
 */
#define CLEAR_BITS(num,mask)        ((num) &= ~(mask))

/** ============================================================================
 *  @macro  SET_BIT
 *
 *  @desc   Sets bit at given position.
 *          This macro is independent of operand width. User must ensure
 *          correctness.
 *  ============================================================================
 */
#define SET_BIT(num,pos)            ((num) |= (1u << (pos)))

/** ============================================================================
 *  @macro  CLEAR_BIT
 *
 *  @desc   Clears bit at given position.
 *          This macro is independent of operand width. User must ensure
 *          correctness.
 *  ============================================================================
 */
#define CLEAR_BIT(num,pos)          ((num) &= ~(1u << (pos)))


/** ============================================================================
 *  @macro  TEST_BIT
 *
 *  @desc   Tests if bit at given position is set.
 *          This macro is independent of operand width. User must ensure
 *          correctness.
 *  ============================================================================
 */
#define TEST_BIT(num,pos)           ((((num) & (1u << (pos))) >> (pos)) & 0x01)


/** ============================================================================
 *  @macro  GET_NBITS8
 *
 *  @desc   Gets numeric value of specified bits from an 8 bit value.
 *  ============================================================================
 */
#define GET_NBITS8(num,pos,width)   \
            (((num) & ((0xFF >> (8u - (width))) << (pos))) >> (pos))

/** ============================================================================
 *  @macro  GET_NBITS16
 *
 *  @desc   Gets numeric value of specified bits from an 16 bit value.
 *  ============================================================================
 */
#define GET_NBITS16(num,pos,width)  \
            (((num) & ((0xFFFF >> (16u - (width))) << (pos))) >>  (pos))

/** ============================================================================
 *  @macro  GET_BITS32
 *
 *  @desc   Gets numeric value of specified bits from an 32 bit value.
 *  ============================================================================
 */
#define GET_NBITS32(num,pos,width)  \
            (((num) & ((0xFFFFFFFF >> (32u - (width))) << (pos))) >>  (pos))

/** ============================================================================
 *  @macro  SET_NBITS8
 *
 *  @desc   Sets specified bits in a 8 bit entity to given value.
 *  ============================================================================
 */
#define SET_NBITS8(num,pos,width,value)     \
            (num) &= ~((0xFF >> (8u - (width))) << (pos)) ;                  \
            (num) |= (((value) & (0xFF >> (8u - (width)))) << (pos)) ;

/** ============================================================================
 *  @macro  SET_NBITS16
 *
 *  @desc   Sets specified bits in a 16 bit entity to given value.
 *  ============================================================================
 */
#define SET_NBITS16(num,pos,width,value)    \
            (num) &= ~((0xFFFF >> (16u - (width))) << (pos)) ;               \
            (num) |= (((value) & (0xFFFF >> (16u - (width)))) << (pos)) ;

/** ============================================================================
 *  @macro  SET_NBITS32
 *
 *  @desc   Sets specified bits in a 32 bit entity to given value.
 *  ============================================================================
 */
#define SET_NBITS32(num,pos,width,value)   \
            (num) &= ~((0xFFFFFFFF >> (32u - (width))) << (pos)) ;           \
            (num) |= (((value) & (0xFFFFFFFF >> (32u - (width)))) << (pos)) ;

/** ============================================================================
 *  @macro  BYTESWAP_WORD
 *
 *  @desc   Macro to swap bytes within a 16 bit word.
 *  ============================================================================
 */
#define BYTESWAP_WORD(x)   (Uint16) (( (Uint16) ((((Uint16)(x)) << 8u) & 0xFF00u) \
                                     | (Uint16)((((Uint16)(x)) >> 8u) & 0x00FFu)))

/** ============================================================================
 *  @macro  BYTESWAP_LONG
 *
 *  @desc   Macro to swap bytes within a 32 bit dword.
 *  ============================================================================
 */
#define BYTESWAP_LONG(x) (   (((x) << 24u) & 0xFF000000u)    \
                          |  (((x) <<  8u) & 0x00FF0000u)   \
                          |  (((x) >>  8u) & 0x0000FF00u)   \
                          |  (((x) >> 24u) & 0x000000FFu))

/** ============================================================================
 *  @macro  WORDSWAP_LONG
 *
 *  @desc   Macro to swap two 16-bit values within a 32 bit dword.
 *  ============================================================================
 */
#define WORDSWAP_LONG(x) (   (((x) << 16u) & 0xFFFF0000u)    \
                          |  (((x) >> 16u) & 0x0000FFFFu))

/** ============================================================================
 *  @macro  SWAP_LONG
 *
 *  @desc   Returns a word-swapped or non-swapped dword based on the parameter
 *          passed.
 *  ============================================================================
 */
#define SWAP_LONG(value, swap) (((swap) == FALSE) ?          \
                                (value) : WORDSWAP_LONG (value))


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif  /* !defined (_BITOPS_H) */
