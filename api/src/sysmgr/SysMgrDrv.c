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
 *  @file   SysMgrDrv.c
 *
 *  @brief      OS-specific implementation of SysMgr driver for Linux
 *
 *  ============================================================================
 */


/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <Trace.h>

/* Module specific header files */
#include <Gate.h>
#include <SysMgr.h>
#include <SysMgrDrvDefs.h>

/* Linux specific header files */
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <IPCManager.h>


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/** ============================================================================
 *  Macros and types
 *  ============================================================================
 */
/*!
 *  @brief  Driver name for SysMgr.
 */
#define SYSMGR_DRIVER_NAME     "/dev/syslinkipc/SysMgr"


/** ============================================================================
 *  Globals
 *  ============================================================================
 */
/*!
 *  @brief  Driver handle for SysMgr in this process.
 */
static Int32 SysMgrDrv_handle = 0;

/*!
 *  @brief  Reference count for the driver handle.
 */
static UInt32 SysMgrDrv_refCount = 0;


/** ============================================================================
 *  Functions
 *  ============================================================================
 */
/*!
 *  @brief  Function to open the SysMgr driver.
 *
 *  @sa     SysMgrDrv_close
 */
Int
SysMgrDrv_open (Void)
{
    Int status      = SYSMGR_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "SysMgrDrv_open");

    if (SysMgrDrv_refCount == 0) {

        SysMgrDrv_handle = open (SYSMGR_DRIVER_NAME,
                                       O_SYNC | O_RDWR);
        if (SysMgrDrv_handle < 0) {
            perror (SYSMGR_DRIVER_NAME);
/*! @retval SYSMGR_E_OSFAILURE Failed to open SysMgr driver with OS
 */
            status = SYSMGR_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SysMgrDrv_open",
                                 status,
                                 "Failed to open SysMgr driver with OS!");
        }
        else {
            osStatus = fcntl (SysMgrDrv_handle, F_SETFD, FD_CLOEXEC);
            if (osStatus != 0) {
/*! @retval SYSMGR_E_OSFAILURE Failed to set file descriptor flags */
                status = SYSMGR_E_OSFAILURE;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "SysMgrDrv_open",
                                     status,
                                     "Failed to set file descriptor flags!");
            }
            else {
                /* TBD: Protection for refCount. */
                SysMgrDrv_refCount++;
            }
        }
    }

    GT_1trace (curTrace, GT_LEAVE, "SysMgrDrv_open", status);

/*! @retval SYSMGR_SUCCESS Operation successfully completed. */
    return status;
}


/*!
 *  @brief  Function to close the SysMgr driver.
 *
 *  @sa     SysMgrDrv_open
 */
Int
SysMgrDrv_close (Void)
{
    Int status      = SYSMGR_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "SysMgrDrv_close");

    /* TBD: Protection for refCount. */
    SysMgrDrv_refCount--;
    if (SysMgrDrv_refCount == 0) {
        osStatus = close (SysMgrDrv_handle);
        if (osStatus != 0) {
            perror ("SysMgr driver close: ");
/*! @retval SYSMGR_E_OSFAILURE Failed to open SysMgr driver with OS */
            status = SYSMGR_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SysMgrDrv_close",
                                 status,
                                 "Failed to close SysMgr driver with OS!");
        }
        else {
            SysMgrDrv_handle = 0;
        }
    }

    GT_1trace (curTrace, GT_LEAVE, "SysMgrDrv_close", status);

/*! @retval SYSMGR_SUCCESS Operation successfully completed. */
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
SysMgrDrv_ioctl (UInt32 cmd, Ptr args)
{
    Int status      = SYSMGR_SUCCESS;
    int osStatus    = 0;

    GT_2trace (curTrace, GT_ENTER, "SysMgrDrv_ioctl", cmd, args);

    GT_assert (curTrace, (SysMgrDrv_refCount > 0));

    osStatus = ioctl (SysMgrDrv_handle, cmd, args);
    if (osStatus < 0) {
    /*! @retval SYSMGR_E_OSFAILURE Driver ioctl failed */
        status = SYSMGR_E_OSFAILURE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "SysMgrDrv_ioctl",
                             status,
                             "Driver ioctl failed!");
    }
    else {
        /* First field in the structure is the API status. */
        status = ((SysMgrDrv_CmdArgs *) args)->apiStatus;
    }

    GT_1trace (curTrace, GT_LEAVE, "SysMgrDrv_ioctl", status);

/*! @retval SYSMGR_SUCCESS Operation successfully completed. */
    return status;
}



#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
