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
 *  @file   Atomic_Ops.h
 *
 *  @brief      Atomic operations abstraction
 *
 *
 *  @ver        2.00.00.06
 *
 *  ============================================================================
 */


#ifndef  ATOMIC_OPS_H
#define  ATOMIC_OPS_H

/* Standard libc headers */
#include <stdio.h>
#include <pthread.h>


/* =============================================================================
 * Typedef
 * =============================================================================
 */
/*! @brief Typedef for atomic variable */
typedef UInt32 Atomic;


/* =============================================================================
 * Global
 * =============================================================================
 */
/* Lock used by atomic operations to create psuedo atomicity */
static pthread_mutex_t Atomic_lock = PTHREAD_MUTEX_INITIALIZER;


/* =============================================================================
 * APIs & Macros
 * =============================================================================
 */
/*!
 *   @brief Function to read an variable atomically
 *
 *   @param var Pointer to atomic variable
 */
static inline UInt32 Atomic_read (Atomic * var)
{
    UInt32 ret;

    pthread_mutex_lock (&Atomic_lock);
    ret = *var;
    pthread_mutex_unlock (&Atomic_lock);

    /*! @retval value   Current value of the atomic variable */
    return ret;
}


/*!
 *   @brief Function to set an variable atomically
 *
 *   @param var Pointer to atomic variable
 *   @param val Value to be set
 */
static inline void Atomic_set (Atomic * var, UInt32 val)
{
    pthread_mutex_lock (&Atomic_lock);
    *var = val;
    pthread_mutex_unlock (&Atomic_lock);
}


/*!
 *   @brief Function to increment an variable atomically
 *
 *   @param var Pointer to atomic variable
 *   @param val Value to be set
 */
static inline UInt32 Atomic_inc_return (Atomic * var)
{
    UInt32 ret;

    pthread_mutex_lock (&Atomic_lock);
    *var = *var + 1u;
    ret = *var;
    pthread_mutex_unlock (&Atomic_lock);

    /*! @retval value   Current value of the atomic variable */
    return ret;
}


/*!
 *   @brief Function to decrement an variable atomically
 *
 *   @param var Pointer to atomic variable
 *   @param val Value to be set
 */
static inline UInt32 Atomic_dec_return (Atomic * var)
{
    UInt32 ret;

    pthread_mutex_lock (&Atomic_lock);
    *var = *var - 1u;
    ret = *var;
    pthread_mutex_unlock (&Atomic_lock);

    /*! @retval value   Current value of the atomic variable */
    return ret;
}


/*!
 * @brief Function to compare a mask and set if not equal
 *
 * @params v    Pointer to atomic variable
 * @params mask Mask to compare with
 * @params val  Value to be set if mask does not match.
 */
static inline void Atomic_cmpmask_and_set(Atomic * var, UInt32 mask, UInt32 val)
{
    UInt32  ret;

    pthread_mutex_lock (&Atomic_lock);
    ret = *var;
    if ((ret & mask) != mask) {
        *var = val;
    }
    pthread_mutex_unlock (&Atomic_lock);
}

/*!
 * @brief Function to compare a mask and then check current value less than
 *        provided value.
 *
 * @params v    Pointer to atomic variable
 * @params mask Mask to compare with
 * @params val  Value to be set if mask does not match.
 */
static inline Bool Atomic_cmpmask_and_lt(Atomic * var, UInt32 mask, UInt32 val)
{
    Bool   ret = TRUE;
    UInt32 cur = 0;

    pthread_mutex_lock (&Atomic_lock);
    cur = *var;
    if ((cur & mask) == mask) {
        if (cur >= val) {
            ret = FALSE;
        }
    }
    pthread_mutex_unlock (&Atomic_lock);

    /*! @retval TRUE  if mask matches and current value is less than given
     *  value */
    /*! @retval FALSE either mask doesnot matches or current value is not less
     *  than given value */
    return ret;
}

/*!
 * @brief Function to compare a mask and then check current value greater than
 *        provided value.
 *
 * @params v    Pointer to atomic variable
 * @params mask Mask to compare with
 * @params val  Value to be set if mask does not match.
 */
static inline Bool Atomic_cmpmask_and_gt(Atomic * var, UInt32 mask, UInt32 val)
{
    Bool   ret = TRUE;
    UInt32 cur = 0;

    pthread_mutex_lock (&Atomic_lock);
    cur = *var;
    if ((cur & mask) == mask) {
        if (cur < val) {
            ret = FALSE;
        }
    }
    pthread_mutex_unlock (&Atomic_lock);

    /*! @retval TRUE  if mask matches and current value is less than given
     *  value */
    /*! @retval FALSE either mask doesnot matches or current value is not
     *  greater than given value */
    return ret;
}

#endif /* if !defined(ATOMIC_OPS_H) */
