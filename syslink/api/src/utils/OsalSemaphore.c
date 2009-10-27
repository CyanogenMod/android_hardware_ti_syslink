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
/*============================================================================
 *  @file   OsalSemaphore.c
 *
 *  @brief  Semaphore interface implementation.
 *
 *  ============================================================================
 */

/* Standard headers */
#include <Std.h>

/* OSAL and utils headers */
#include <OsalSemaphore.h>
#include <Trace.h>
#include <Memory.h>

/* Linux headers */
#include <semaphore.h>
#include <errno.h>
#if defined(OSALSEMAPHORE_USE_TIMEDWAIT)
#include <time.h>
#endif

#if defined (__cplusplus)
extern "C" {
#endif


#define MAX_SCHEDULE_TIMEOUT 10000

/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*!
 *  @brief   Defines object to encapsulate the Semaphore.
 *           The definition is OS/platform specific.
 */
typedef struct OsalSemaphore_Object_tag {
    OsalSemaphore_Type  semType;
    /*!< Indicates the type of the semaphore (binary or counting). */
    UInt32 value;
    /*!< Current status of semaphore (0,1) - binary and (0-n) counting. */
    sem_t lock;
    /*!< lock on which this Semaphore is based. */
} OsalSemaphore_Object;


/* =============================================================================
 * APIs
 * =============================================================================
 */
/*!
 *  @brief      Creates an instance of Mutex object.
 *
 *  @param      semType Type of semaphore. This parameter is a mask of semaphore
 *                      type and interruptability type.
 *
 *  @sa         OsalSemaphore_delete
 */
OsalSemaphore_Handle OsalSemaphore_create(UInt32 semType, UInt32 semValue)
{
    Int status = OSALSEMAPHORE_SUCCESS;
    OsalSemaphore_Object * semObj = NULL;
    int osStatus = 0;

    GT_1trace (curTrace, GT_ENTER, "OsalSemaphore_create", semType);

    /* Check for semaphore type (binary/counting) */
    GT_assert (curTrace,
               ((OSALSEMAPHORE_TYPE_VALUE(semType))
                <   OsalSemaphore_Type_EndValue));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (OSALSEMAPHORE_TYPE_VALUE(semType) >= OsalSemaphore_Type_EndValue) {
        /*! @retVal NULL Invalid semaphore type (OsalSemaphore_Type) provided */
        GT_setFailureReason (curTrace,
                        GT_4CLASS,
                        "OsalSemaphore_create",
                        (unsigned int)OSALSEMAPHORE_E_INVALIDARG,
                        "Invalid semaphore type (OsalSemaphore_Type) provided");
    }
    else {
#endif /* #if !defined(SYSLINK_BUILD_OPTIMIZE) */
        semObj = Memory_calloc (NULL, sizeof (OsalSemaphore_Object), 0);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (semObj == NULL) {
            /*! @retVal NULL Failed to allocate memory for semaphore object. */
            GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "OsalSemaphore_create",
                             (unsigned int)OSALSEMAPHORE_E_MEMORY,
                             "Failed to allocate memory for semaphore object.");
        }
        else {
#endif /* #if !defined(SYSLINK_BUILD_OPTIMIZE) */
            semObj->semType = semType;
            semObj->value = semValue;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if ((OSALSEMAPHORE_TYPE_VALUE (semObj->semType)
                ==  OsalSemaphore_Type_Binary) && (semValue > 1)){
                /*! @retVal NULL Invalid semaphore value. */
                status = OSALSEMAPHORE_E_INVALIDARG;
                GT_setFailureReason (curTrace,
                            GT_4CLASS,
                            "OsalSemaphore_create",
                            status,
                            "Invalid semaphore value");
            }
#endif /* #if !defined(SYSLINK_BUILD_OPTIMIZE) */
            osStatus = sem_init(&(semObj->lock), 0, semValue);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (osStatus < 0) {
                /*! @retVal NULL Failed to initialize semaphore. */
                status = OSALSEMAPHORE_E_RESOURCE;
                GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "OsalSemaphore_create",
                             status,
                             "Failed to initialize semaphore");
            }
#endif /* #if !defined(SYSLINK_BUILD_OPTIMIZE) */
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* #if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "OsalSemaphore_create", semObj);

    /*! @retVal Semaphore-handle Operation successfully completed. */
    return (OsalSemaphore_Handle) semObj;
}


/*!
 *  @brief      Deletes an instance of Semaphore object.
 *
 *  @param      mutexHandle   Semaphore object handle which needs to be deleted.
 *
 *  @sa         OsalSemaphore_create
 */
Int
OsalSemaphore_delete(OsalSemaphore_Handle *semHandle)
{
    Int status = OSALSEMAPHORE_SUCCESS;
    OsalSemaphore_Object **semObj = (OsalSemaphore_Object **)semHandle;
    int osStatus = 0;

    GT_1trace (curTrace, GT_ENTER, "OsalSemaphore_delete", semHandle);

    GT_assert (curTrace, (semHandle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (semHandle == NULL) {
        /*! @retVal OSALSEMAPHORE_E_INVALIDARG NULL provided for argument
                                           semHandle.*/
        status = OSALSEMAPHORE_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "OsalSemaphore_delete",
                             status,
                             "NULL provided for argument semHandle");
    }
    else if (*semHandle == NULL) {
        /*! @retVal OSALSEMAPHORE_E_HANDLE NULL Semaphore handle provided. */
        status = OSALSEMAPHORE_E_HANDLE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "OsalSemaphore_delete",
                             status,
                             "NULL Semaphore handle provided.");
    }
    else {
#endif /* #if !defined(SYSLINK_BUILD_OPTIMIZE) */
            osStatus = sem_destroy(&((*semObj)->lock));
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (osStatus < 0) {
                   /*! @retVal NULL Failed to destroy semaphore. */
                   status = OSALSEMAPHORE_E_HANDLE;
                GT_setFailureReason (curTrace,
                            GT_4CLASS,
                            "OsalSemaphore_delete",
                            status,
                            "Failed to destroy semaphore");
               }
#endif /* #if !defined(SYSLINK_BUILD_OPTIMIZE) */
        Memory_free(NULL, *semHandle, sizeof (OsalSemaphore_Object));
        *semHandle = NULL;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* #if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "OsalSemaphore_delete", status);

    /*! @retVal OSALSEMAPHORE_SUCCESS Operation successfully completed. */
    return status;
}


/*!
 *  @brief      Wait on the Semaphore in the kernel thread context
 *
 *  @param      semHandle   Semaphore object handle
 *  @param      timeout     Timeout (in msec). Special values are provided for
 *                          no-wait and infinite-wait.
 *
 *  @sa         OsalSemaphore_post
 */
Int
OsalSemaphore_pend(OsalSemaphore_Handle semHandle, UInt32 timeout)
{
    Int                     status      = OSALSEMAPHORE_SUCCESS;
    OsalSemaphore_Object *  semObj      = (OsalSemaphore_Object *) semHandle;
    int                     osStatus    = 0;
#if defined(OSALSEMAPHORE_USE_TIMEDWAIT)
    struct timespec         absTimeout;
#endif /* #if defined(OSALSEMAPHORE_USE_TIMEDWAIT) */

    GT_2trace (curTrace, GT_ENTER, "OsalSemaphore_pend", semHandle, timeout);

    GT_assert (curTrace, (semHandle != NULL));
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (semHandle == NULL) {
        /*! @retVal OSALSEMAPHORE_E_HANDLE NULL Semaphore handle provided. */
        status = OSALSEMAPHORE_E_HANDLE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "OsalSemaphore_pend",
                             status,
                             "NULL Semaphore handle provided.");
    }
    else {
#endif /* #if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Different handling for no-timeout case. */
        if (timeout == OSALSEMAPHORE_WAIT_NONE) {
            osStatus = sem_trywait(&(semObj->lock));
            GT_assert (curTrace, (osStatus == 0));
            if (osStatus != 0) {
                if (errno == EAGAIN) {
                    /*! @retVal semaphore was not available. */
                    status = OSALSEMAPHORE_E_WAITNONE;
                    GT_setFailureReason (curTrace,
                                         GT_4CLASS,
                                         "OsalSemaphore_pend",
                                         status,
                                         "WAIT_NONE timeout value was provided,"
                                         " semaphore was not available.");
                }
                else {
                    status = OSALSEMAPHORE_E_RESOURCE;
                    GT_setFailureReason (curTrace,
                                  GT_4CLASS,
                                  "OsalSemaphore_pend",
                                  status,
                                  "Failure in sem_trywait()");
                }
            }
            else if(semObj->value > 0) {
                if (OSALSEMAPHORE_TYPE_VALUE (semObj->semType)
                        ==  OsalSemaphore_Type_Binary) {
                    semObj->value = 0;
                }
                else {
                    semObj->value--;
                }
            }
        }
        /* Finite and infinite timeout cases */
        else {
#if defined(OSALSEMAPHORE_USE_TIMEDWAIT)
            /* Temporarily disabled as sem_timedwait is reflecting issues */
            /* Get timeout value in OS-recognizable format. */
            if (timeout == OSALSEMAPHORE_WAIT_FOREVER) {
                clock_gettime(CLOCK_MONOTONIC, &absTimeout);
                absTimeout.tv_sec += (MAX_SCHEDULE_TIMEOUT/1000);
                absTimeout.tv_nsec += (MAX_SCHEDULE_TIMEOUT % 1000) * 1000000;
            }
            else {
                clock_gettime(CLOCK_MONOTONIC, &absTimeout);
                absTimeout.tv_sec += (timeout/1000);
                absTimeout.tv_nsec += (timeout % 1000) * 1000000;
            }

            osStatus = sem_timedwait(&(semObj->lock), &absTimeout);
            GT_assert (curTrace, (osStatus == 0));
            if (osStatus != 0) {
                if (errno == ETIMEDOUT) {
                    status = OSALSEMAPHORE_E_WAITNONE;
                    GT_1trace (curTrace,
                               GT_LEAVE,
                               "sem_timedwait() timed out",
                               osStatus);
                }
                else {
                    status = OSALSEMAPHORE_E_RESOURCE;
                    GT_setFailureReason (curTrace,
                                  GT_4CLASS,
                                  "OsalSemaphore_pend",
                                  status,
                                  "Failure in sem_timedwait()");
                }
            }
#else
            osStatus = sem_wait(&(semObj->lock));
            GT_assert (curTrace, (osStatus == 0));
            if (osStatus != 0) {
                status = OSALSEMAPHORE_E_RESOURCE;
                GT_setFailureReason (curTrace,
                              GT_4CLASS,
                              "OsalSemaphore_pend",
                              status,
                              "Failure in sem_wait()");
            }
#endif /* #if defined(OSALSEMAPHORE_USE_TIMEDWAIT) */
            else if(semObj->value > 0) {
                if (OSALSEMAPHORE_TYPE_VALUE (semObj->semType)
                        ==  OsalSemaphore_Type_Binary) {
                    semObj->value = 0;
                }
                else {
                    semObj->value--;
                }
            }
        }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* #if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "OsalSemaphore_pend", status);

    /*! @retVal OSALSEMAPHORE_SUCCESS Operation successfully completed. */
    return status;
}

/*!
 *  @brief      Signals the semaphore and makes it available for other
 *              threads.
 *
 *  @param      semHandle Semaphore object handle
 *
 *  @sa         OsalSemaphore_pend
 */
Int
OsalSemaphore_post (OsalSemaphore_Handle semHandle)
{
    Int                     status      = OSALSEMAPHORE_SUCCESS;
    OsalSemaphore_Object *  semObj      = (OsalSemaphore_Object *) semHandle;
    int                     osStatus    = 0;

    GT_1trace (curTrace, GT_ENTER, "OsalSemaphore_post", semHandle);

    GT_assert (curTrace, (semHandle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (semHandle == NULL) {
        /*! @retVal OSALSEMAPHORE_E_HANDLE NULL Semaphore handle provided. */
        status = OSALSEMAPHORE_E_HANDLE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "OsalSemaphore_post",
                             status,
                             "NULL Semaphore handle provided.");
    }
    else {
#endif /* #if !defined(SYSLINK_BUILD_OPTIMIZE) */
        if (OSALSEMAPHORE_TYPE_VALUE (semObj->semType)
                ==  OsalSemaphore_Type_Binary) {
            semObj->value = 1;
        }
        else {
            semObj->value++;
        }
        osStatus = sem_post(&(semObj->lock));
        GT_assert (curTrace, (osStatus == 0));
        if (osStatus != 0) {
            status = OSALSEMAPHORE_E_RESOURCE;
            GT_1trace (curTrace,
                       GT_LEAVE,
                       "Error in sem_post",
                       osStatus);
        }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* #if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "OsalSemaphore_post", status);

    /*! @retVal OSALSEMAPHORE_SUCCESS Operation successfully completed. */
    return status;
}

#if defined (__cplusplus)
}
#endif /* defined (_cplusplus)*/
