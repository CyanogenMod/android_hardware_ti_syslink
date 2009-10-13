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
 *  @file   MultiProcDrv.c
 *
 *  @brief     Driver for MultiProc on HLOS side
 *
 *  ============================================================================
 */



/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <Trace.h>

/* Module specific header files */
#include <MultiProc.h>
#include <MultiProcDrvDefs.h>

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
 *  @brief  Driver name for MultiProc.
 */
#define MULTIPROC_DRIVER_NAME     "/dev/syslinkipc/MultiProc"


/** ============================================================================
 *  Globals
 *  ============================================================================
 */
/*!
 *  @brief  Driver handle for MultiProc in this process.
 */
static Int32 MultiProcDrv_handle = 0;

/*!
 *  @brief  Reference count for the driver handle.
 */
static UInt32 MultiProcDrv_refCount = 0;



/** ============================================================================
 *  Functions
 *  ============================================================================
 */
/*!
 *  @brief  Function to open the MultiProc driver.
 *
 *  @sa     MultiProcDrv_close
 */
Int
MultiProcDrv_open (Void)
{
    Int status      = MULTIPROC_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "MultiProcDrv_open");

    if (MultiProcDrv_refCount == 0) {
        MultiProcDrv_handle = open (MULTIPROC_DRIVER_NAME,
                                             O_SYNC | O_RDWR);
        if (MultiProcDrv_handle < 0) {
            perror (MULTIPROC_DRIVER_NAME);
            /*! @retval MULTIPROC_E_OSFAILURE
             *          Failed to open MultiProc driver with OS
             */
            status = MULTIPROC_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MultiProcDrv_open",
                                 status,
                                 "Failed to open MultiProc driver"
                                 " with OS!");
        }
        else {
            osStatus = fcntl (MultiProcDrv_handle,
                              F_SETFD,
                              FD_CLOEXEC);
            if (osStatus != 0) {
                /*! @retval MULTIPROC_E_OSFAILURE
                 *          Failed to set file descriptor flags
                 */
                status = MULTIPROC_E_OSFAILURE;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "MultiProcDrv_open",
                                     status,
                                     "Failed to set file descriptor flags!");
            }
            else{
                /* TBD: Protection for refCount. */
                MultiProcDrv_refCount++;
            }
        }
    }
    else {
        MultiProcDrv_refCount++;
    }

    GT_1trace (curTrace, GT_LEAVE, "MultiProcDrv_open", status);

    /*! @retval MULTIPROC_SUCCESS Operation successfully completed. */
    return status;
}


/*!
 *  @brief  Function to close the MultiProc driver.
 *
 *  @sa     MultiProcDrv_open
 */
Int
MultiProcDrv_close (Void)
{
    Int status      = MULTIPROC_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "MultiProcDrv_close");

    /* TBD: Protection for refCount. */
    MultiProcDrv_refCount--;
    if (MultiProcDrv_refCount == 0) {
        osStatus = close (MultiProcDrv_handle);
        if (osStatus != 0) {
            perror ("MultiProc driver close: ");
            /*! @retval MULTIPROC_E_OSFAILURE Failed to open
             *          MultiProc driver with OS
             */
            status = MULTIPROC_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MultiProcDrv_close",
                                 status,
                                 "Failed to close MultiProc driver"
                                 " with OS!");
        }
        else {
            MultiProcDrv_handle = 0;
        }
    }

    GT_1trace (curTrace, GT_LEAVE, "MultiProcDrv_close", status);

    /*! @retval MULTIPROC_SUCCESS Operation successfully completed. */
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
MultiProcDrv_ioctl (UInt32 cmd, Ptr args)
{
    Int status      = MULTIPROC_SUCCESS;
    int osStatus    = 0;

    GT_2trace (curTrace, GT_ENTER, "MultiProcDrv_ioctl", cmd, args);

    osStatus = ioctl (MultiProcDrv_handle, cmd, args);
    if (osStatus < 0) {
    /*! @retval MULTIPROC_E_OSFAILURE Driver ioctl failed */
        status = MULTIPROC_E_OSFAILURE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MultiProcDrv_ioctl",
                             status,
                             "Driver ioctl failed!");
    }
    else {
        /* First field in the structure is the API status. */
        status = ((MultiProcDrv_CmdArgs *) args)->apiStatus;
    }

    GT_1trace (curTrace, GT_LEAVE, "MultiProcDrv_ioctl", status);

    /*! @retval MULTIPROC_SUCCESS Operation successfully completed. */
    return status;
}



#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
