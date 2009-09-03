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
 *  @file   GatePetersonDrv.c
 *
 *  @brief      OS-specific implementation of GatePeterson driver for Linux
 *
 *  ============================================================================
 */


/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <Trace.h>

/* Module specific header files */
#include <Gate.h>
#include <GatePeterson.h>
#include <GatePetersonDrvDefs.h>

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
 *  @brief  Driver name for GatePeterson.
 */
#define GATEPETERSON_DRIVER_NAME     "/dev/syslinkipc/GatePeterson"


/** ============================================================================
 *  Globals
 *  ============================================================================
 */
/*!
 *  @brief  Driver handle for GatePeterson in this process.
 */
static Int32 GatePetersonDrv_handle = 0;

/*!
 *  @brief  Reference count for the driver handle.
 */
static UInt32 GatePetersonDrv_refCount = 0;


/** ============================================================================
 *  Functions
 *  ============================================================================
 */
/*!
 *  @brief  Function to open the GatePeterson driver.
 *
 *  @sa     GatePetersonDrv_close
 */
Int
GatePetersonDrv_open (Void)
{
    Int status      = GATEPETERSON_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "GatePetersonDrv_open");

    if (GatePetersonDrv_refCount == 0) {
        GatePetersonDrv_handle = open (GATEPETERSON_DRIVER_NAME,
                                       O_SYNC | O_RDWR);
        if (GatePetersonDrv_handle < 0) {
            perror (GATEPETERSON_DRIVER_NAME);
            /*! @retval GATEPETERSON_E_OSFAILURE Failed to open GatePeterson 
             *          driver with OS
             */
            status = GATEPETERSON_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "GatePetersonDrv_open",
                                 status,
                                 "Failed to open GatePeterson driver with OS!");
        }
        else {
            osStatus = fcntl (GatePetersonDrv_handle, F_SETFD, FD_CLOEXEC);
            if (osStatus != 0) {
                /*! @retval GATEPETERSON_E_OSFAILURE Failed to 
                 * set file descriptor flags
                 */
                status = GATEPETERSON_E_OSFAILURE;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "GatePetersonDrv_open",
                                     status,
                                     "Failed to set file descriptor flags!");
            }
        }
    }

    if (status == GATEPETERSON_SUCCESS) {
        /* TBD: Protection for refCount. */
        GatePetersonDrv_refCount++;
    }

    GT_1trace (curTrace, GT_LEAVE, "GatePetersonDrv_open", status);

    /*! @retval GATEPETERSON_SUCCESS Operation successfully completed. */
    return status;
}


/*!
 *  @brief  Function to close the GatePeterson driver.
 *
 *  @sa     GatePetersonDrv_open
 */
Int
GatePetersonDrv_close (Void)
{
    Int status      = GATEPETERSON_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "GatePetersonDrv_close");

    /* TBD: Protection for refCount. */
    GatePetersonDrv_refCount--;
    if (GatePetersonDrv_refCount == 0) {
        osStatus = close (GatePetersonDrv_handle);
        if (osStatus != 0) {
            perror ("GatePeterson driver close: ");
/*! @retval GATEPETERSON_E_OSFAILURE Failed to open GatePeterson driver with OS */
            status = GATEPETERSON_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "GatePetersonDrv_close",
                                 status,
                                 "Failed to close GatePeterson driver with OS!");
        }
        else {
            GatePetersonDrv_handle = 0;
        }
    }

    GT_1trace (curTrace, GT_LEAVE, "GatePetersonDrv_close", status);

/*! @retval GATEPETERSON_SUCCESS Operation successfully completed. */
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
GatePetersonDrv_ioctl (UInt32 cmd, Ptr args)
{
    Int status      = GATEPETERSON_SUCCESS;
    int osStatus    = 0;

    GT_2trace (curTrace, GT_ENTER, "GatePetersonDrv_ioctl", cmd, args);

    GT_assert (curTrace, (GatePetersonDrv_refCount > 0));

    osStatus = ioctl (GatePetersonDrv_handle, cmd, args);
    if (osStatus < 0) {
    /*! @retval GATEPETERSON_E_OSFAILURE Driver ioctl failed */
        status = GATEPETERSON_E_OSFAILURE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GatePetersonDrv_ioctl",
                             status,
                             "Driver ioctl failed!");
    }
    else {
        /* First field in the structure is the API status. */
        status = ((GatePetersonDrv_CmdArgs *) args)->apiStatus;
    }

    GT_1trace (curTrace, GT_LEAVE, "GatePetersonDrv_ioctl", status);

/*! @retval GATEPETERSON_SUCCESS Operation successfully completed. */
    return status;
}



#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
