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
 *  @file   Gate.c
 *
 *  @brief      gate wrapper implementation
 *
 *  ============================================================================
 */



/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <Trace.h>

/* Module level headers */
#include <Gate.h>


#if defined (__cplusplus)
extern "C" {
#endif



/*! @brief Function to enter a Gate
 *
 *  @params gHandle handle to a gate instance
 *
 *  @sa Gate_leave
 */
UInt32
Gate_enter (Gate_Handle gHandle)
{
    UInt32 key = 0;

    GT_1trace (curTrace, GT_ENTER, "Gate_enter", gHandle);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (gHandle == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "Gate_enter",
                             GATE_E_INVALIDARG,
                             "Handle passed is invalid!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        key = gHandle->enter (gHandle);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "Gate_enter", key);

    /*! @retval Flags Operation successful */
    return key;
}


/*! @brief Function to leave a Gate
 *
 *  @params gHandle handle to a gate instance
 *  @params key     Key received in Gate_enter
 *
 *  @sa Gate_enter
 */
Void
Gate_leave (Gate_Handle gHandle, UInt32 key)
{
    GT_2trace (curTrace, GT_ENTER, "Gate_leave", gHandle, key);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (gHandle == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "Gate_leave",
                             GATE_E_INVALIDARG,
                             "Handle passed is invalid!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        gHandle->leave (gHandle, key);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "Gate_leave");
}


/*!
 *  @brief Function to get the kernel object pointer embedded in userspace gate.
 *         Some Gate implementations return the kernel object handle.
 *         Gates which do not have kernel object pointer embedded return NULL.
 *
 *  @params gHandle handle to a gate instance
 *
 *  @sa
 */
Void *
Gate_getKnlHandle (Gate_Handle gHandle)
{
    Gate_Object * gateObject = (Gate_Object *) gHandle;
    Ptr           knlHandle = NULL;

    GT_1trace (curTrace, GT_ENTER, "Gate_getKnlHandle", gHandle);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (gHandle == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "Gate_getKnlHandle",
                             GATE_E_INVALIDARG,
                             "Handle passed is invalid!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
       if (gateObject->getKnlHandle != NULL) {
           knlHandle = gateObject->getKnlHandle (gHandle);
       }

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "Gate_getKnlHandle", knlHandle);

    return (knlHandle);
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
