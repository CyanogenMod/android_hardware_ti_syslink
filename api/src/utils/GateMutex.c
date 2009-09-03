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
 *  @file   GateMutex.c
 *
 *  @brief      Gate based on Mutex
 *
 *  ============================================================================
 */



/* Standard headers */
#include <Std.h>

/* Osal & Utility headers */
#include <OsalMutex.h>
#include <Memory.h>
#include <Trace.h>

/* Module level headers */
#include <Gate.h>
#include <GateMutex.h>


#if defined (__cplusplus)
extern "C" {
#endif


/* Forward declaration of function */
UInt32 GateMutex_enter (GateMutex_Handle gmhandle);


/* Forward declaration of function */
Void GateMutex_leave (GateMutex_Handle gmhandle, UInt32 key);


/*!
 *  @brief      Function to create a Gate based on Mutex.
 *
 *  @sa         GateMutex_delete
 */
GateMutex_Handle
GateMutex_create (Void)
{
    GateMutex_Handle handle = NULL;

    GT_0trace (curTrace, GT_ENTER, "GateMutex_create");

    /* Allocate memory for the gate object */
    handle = (GateMutex_Handle) Memory_alloc (NULL,
                                              sizeof (GateMutex_Object),
                                              0);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateMutex_create",
                             GATEMUTEX_E_MEMORY,
                             "Unable to allocate memory for the handle!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        handle->enter = (Lock_enter) GateMutex_enter;
        handle->leave = (Lock_leave) GateMutex_leave;
        handle->obj = OsalMutex_create (OsalMutex_Type_Interruptible);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (handle->obj == NULL) {
            Memory_free (NULL, handle, sizeof (GateMutex_Object));
            handle = NULL;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "GateMutex_create",
                                 GATEMUTEX_E_FAIL,
                                 "Unable to create Osal mutex object!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "GateMutex_create", handle);

    /*! @retval NULL   operation was not successful */
    /*! @retval Handle operation was successful */
    return handle;
}


/*!
 *  @brief      Function to delete a Gate based on Mutex.
 *
 *  @param      gmhandle  Handle to previously created gate mutex instance.
 *
 *  @sa         GateMutex_create
 */
Int
GateMutex_delete (GateMutex_Handle * gmHandle)
{
    Int              status = GATEMUTEX_SUCCESS;
    OsalMutex_Handle osalHandle;

    GT_1trace (curTrace, GT_ENTER, "GateMutex_delete", gmHandle);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (gmHandle == NULL) {
        /*! @retval GATEMUTEX_E_INVLIADARG gmHandle passed is invalid*/
        status = GATEMUTEX_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateMutex_delete",
                             status,
                             "gmHandle passed is invalid!");
    }
    else if (*gmHandle == NULL) {
        /*! @retval GATEMUTEX_E_INVLIADARG *gmHandle passed is invalid*/
        status = GATEMUTEX_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateMutex_delete",
                             status,
                             "*gmHandle passed is invalid!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        osalHandle = ((GateMutex_Object *) (*gmHandle))->obj;
        status = OsalMutex_delete (&osalHandle);
        ((GateMutex_Object *) (*gmHandle))->obj = NULL;
        GT_assert (curTrace, (status >= 0));

        Memory_free (NULL, (*gmHandle), sizeof (GateMutex_Object));
        (*gmHandle) = NULL;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "GateMutex_delete", status);

    /*! @retval GATEMUTEX_SUCCESS Operation successful */
    return status;
}


/*!
 *  @brief      Function to enter a Gate Mutex.
 *
 *  @param      gmhandle  Handle to previously created gate mutex instance.
 *
 *  @sa         GateMutex_leave
 */
UInt32
GateMutex_enter (GateMutex_Handle gmHandle)
{
    UInt32 key = 0x0;

    GT_1trace (curTrace, GT_ENTER, "GateMutex_enter", gmHandle);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (gmHandle == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateMutex_enter",
                             GATEMUTEX_E_INVALIDARG,
                             "Handle passed is invalid!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        key = OsalMutex_enter ((OsalMutex_Handle) gmHandle->obj);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "GateMutex_enter", key);

    /*! @retval Flags Operation successful */
    return key;
}


/*!
 *  @brief      Function to leave a Gate Mutex.
 *
 *  @param      gmhandle  Handle to previously created gate mutex instance.
 *  @param      key       Flags.
 *
 *  @sa         GateMutex_enter
 */
Void
GateMutex_leave (GateMutex_Handle gmHandle, UInt32 key)
{
    GT_1trace (curTrace, GT_ENTER, "GateMutex_leave", gmHandle);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (gmHandle == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateMutex_enter",
                             GATEMUTEX_E_INVALIDARG,
                             "Handle passed is invalid!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        OsalMutex_leave ((OsalMutex_Handle) gmHandle->obj, key);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "GateMutex_leave");
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
