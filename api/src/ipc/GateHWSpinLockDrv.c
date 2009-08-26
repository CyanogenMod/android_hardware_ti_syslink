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
 *  @file   GateHWSpinLockDrv.c
 *
 *  @brief      OS-specific implementation of GateHWSpinLock driver for Linux
 *
 *
 *  ============================================================================
 */


/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <Trace.h>

/* Module specific header files */
#include <Gate.h>
#include <GateHWSpinLock.h>
#include <GateHWSpinLockDrvDefs.h>

/* Linux specific header files */
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/** ============================================================================
 *  Macros and types
 *  ============================================================================
 */
/*!
 *  @brief  Driver name for GateHWSpinLock.
 */
#define GATEHWSPINLOCK_DRIVER_NAME     "/dev/syslinkipc/GateHWSpinLock"


/** ============================================================================
 *  Globals
 *  ============================================================================
 */
/*!
 *  @brief  Driver handle for GateHWSpinLock in this process.
 */
static Int32 GateHWSpinLockDrv_handle = 0;

/*!
 *  @brief  Reference count for the driver handle.
 */
static UInt32 GateHWSpinLockDrv_refCount = 0;


/** ============================================================================
 *  Functions
 *  ============================================================================
 */
/*!
 *  @brief  Function to open the GateHWSpinLock driver.
 *
 *  @sa     GateHWSpinLockDrv_close
 */
Int
GateHWSpinLockDrv_open (Void)
{
    Int status      = GATEHWSPINLOCK_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "GateHWSpinLockDrv_open");

    if (GateHWSpinLockDrv_refCount == 0) {

        GateHWSpinLockDrv_handle = open (GATEHWSPINLOCK_DRIVER_NAME,
                                       O_SYNC | O_RDWR);
        if (GateHWSpinLockDrv_handle < 0) {
            perror (GATEHWSPINLOCK_DRIVER_NAME);
            /*! @retval GATEHWSPINLOCK_E_OSFAILURE Failed to open GateHWSpinLock
             *          driver with OS
             */
            status = GATEHWSPINLOCK_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "GateHWSpinLockDrv_open",
                                 status,
                                 "Failed to open GateHWSpinLock driver with OS!");
        }
        else {
            osStatus = fcntl (GateHWSpinLockDrv_handle, F_SETFD, FD_CLOEXEC);
            if (osStatus != 0) {
                /*! @retval GATEHWSPINLOCK_E_OSFAILURE Failed to set file
                 *          descriptor flags.
                 */
                status = GATEHWSPINLOCK_E_OSFAILURE;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "GateHWSpinLockDrv_open",
                                     status,
                                     "Failed to set file descriptor flags!");
            }
            else {
                /* TBD: Protection for refCount. */
                GateHWSpinLockDrv_refCount++;
            }
        }
    }

    GT_1trace (curTrace, GT_LEAVE, "GateHWSpinLockDrv_open", status);

    /*! @retval GATEHWSPINLOCK_SUCCESS Operation successfully completed. */
    return status;
}


/*!
 *  @brief  Function to close the GateHWSpinLock driver.
 *
 *  @sa     GateHWSpinLockDrv_open
 */
Int
GateHWSpinLockDrv_close (Void)
{
    Int status      = GATEHWSPINLOCK_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "GateHWSpinLockDrv_close");

    /* TBD: Protection for refCount. */
    GateHWSpinLockDrv_refCount--;
    if (GateHWSpinLockDrv_refCount == 0) {
        osStatus = close (GateHWSpinLockDrv_handle);
        if (osStatus != 0) {
            perror ("GateHWSpinLock driver close: ");
            /*! @retval GATEHWSPINLOCK_E_OSFAILURE Failed to open GateHWSpinLock
             *          driver with OS 
             */
            status = GATEHWSPINLOCK_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "GateHWSpinLockDrv_close",
                                 status,
                                 "Failed to close GateHWSpinLock driver with OS!");
        }
        else {
            GateHWSpinLockDrv_handle = 0;
        }
    }

    GT_1trace (curTrace, GT_LEAVE, "GateHWSpinLockDrv_close", status);

    /*! @retval GATEHWSPINLOCK_SUCCESS Operation successfully completed. */
    return status;
}


/*!
 *  @brief  Function to invoke the APIs through ioctl.
 *
 *  @param  cmd     Command for driver ioctl
 *  @param  args    Arguments for the ioctl command
 *
 *  @sa
 */
Int
GateHWSpinLockDrv_ioctl (UInt32 cmd, Ptr args)
{
    Int status      = GATEHWSPINLOCK_SUCCESS;
    int osStatus    = 0;

    GT_2trace (curTrace, GT_ENTER, "GateHWSpinLockDrv_ioctl", cmd, args);

    GT_assert (curTrace, (GateHWSpinLockDrv_refCount > 0));

    osStatus = ioctl (GateHWSpinLockDrv_handle, cmd, args);
    if (osStatus < 0) {
        /*! @retval GATEHWSPINLOCK_E_OSFAILURE Driver ioctl failed */
        status = GATEHWSPINLOCK_E_OSFAILURE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateHWSpinLockDrv_ioctl",
                             status,
                             "Driver ioctl failed!");
    }
    else {
        /* First field in the structure is the API status. */
        status = ((GateHWSpinLockDrv_CmdArgs *) args)->apiStatus;
    }

    GT_1trace (curTrace, GT_LEAVE, "GateHWSpinLockDrv_ioctl", status);

    /*! @retval GATEHWSPINLOCK_SUCCESS Operation successfully completed. */
    return status;
}



#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
