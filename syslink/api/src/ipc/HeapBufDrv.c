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
 *  @file   HeapBufDrv.c
 *
 *  @brief      OS-specific implementation of HeapBuf driver for Linux
 *
 *  ============================================================================
 */


/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <Trace.h>

/* Module specific header files */
#include <Heap.h>
#include <HeapBuf.h>
#include <HeapBufDrvDefs.h>

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
 *  @brief  Driver name for HeapBuf.
 */
#define HEAPBUF_DRIVER_NAME     "/dev/syslinkipc/HeapBuf"


/** ============================================================================
 *  Globals
 *  ============================================================================
 */
/*!
 *  @brief  Driver handle for HeapBuf in this process.
 */
static Int32 HeapBufDrv_handle = 0;

/*!
 *  @brief  Reference count for the driver handle.
 */
static UInt32 HeapBufDrv_refCount = 0;


/** ============================================================================
 *  Functions
 *  ============================================================================
 */
/*!
 *  @brief  Function to open the HeapBuf driver.
 *
 *  @sa     HeapBufDrv_close
 */
Int
HeapBufDrv_open (Void)
{
    Int status      = HEAPBUF_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "HeapBufDrv_open");

    if (HeapBufDrv_refCount == 0) {
        HeapBufDrv_handle = open (HEAPBUF_DRIVER_NAME,
                                  O_SYNC | O_RDWR);
        if (HeapBufDrv_handle < 0) {
            perror (HEAPBUF_DRIVER_NAME);
            /*! @retval HEAPBUF_E_OSFAILURE Failed to open
             *          HeapBuf driver with OS
             */
            status = HEAPBUF_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "HeapBufDrv_open",
                                 status,
                                 "Failed to open HeapBuf driver with OS!");
        }
        else {
            osStatus = fcntl (HeapBufDrv_handle, F_SETFD, FD_CLOEXEC);
            if (osStatus != 0) {
                /*! @retval HEAPBUF_E_OSFAILURE
                 *          Failed to set file descriptor flags
                 */
                status = HEAPBUF_E_OSFAILURE;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "HeapBufDrv_open",
                                     status,
                                     "Failed to set file descriptor flags!");
            }
            else {
                /* TBD: Protection for refCount. */
                HeapBufDrv_refCount++;
            }
        }
    }
    else {
        HeapBufDrv_refCount++;
    }

    GT_1trace (curTrace, GT_LEAVE, "HeapBufDrv_open", status);

    /*! @retval HEAPBUF_SUCCESS Operation successfully completed. */
    return status;
}


/*!
 *  @brief  Function to close the HeapBuf driver.
 *
 *  @sa     HeapBufDrv_open
 */
Int
HeapBufDrv_close (Void)
{
    Int status      = HEAPBUF_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "HeapBufDrv_close");

    /* TBD: Protection for refCount. */
    HeapBufDrv_refCount--;
    if (HeapBufDrv_refCount == 0) {
        osStatus = close (HeapBufDrv_handle);
        if (osStatus != 0) {
            perror ("HeapBuf driver close: ");
            /*! @retval HEAPBUF_E_OSFAILURE
             *          Failed to open HeapBuf driver with OS
             */
            status = HEAPBUF_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "HeapBufDrv_close",
                                 status,
                                 "Failed to close HeapBuf driver with OS!");
        }
        else {
            HeapBufDrv_handle = 0;
        }
    }

    GT_1trace (curTrace, GT_LEAVE, "HeapBufDrv_close", status);

    /*! @retval HEAPBUF_SUCCESS Operation successfully completed. */
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
HeapBufDrv_ioctl (UInt32 cmd, Ptr args)
{
    Int status      = HEAPBUF_SUCCESS;
    int osStatus    = 0;

    GT_2trace (curTrace, GT_ENTER, "HeapBufDrv_ioctl", cmd, args);

    GT_assert (curTrace, (HeapBufDrv_refCount > 0));

    do {
        osStatus = ioctl (HeapBufDrv_handle, cmd, args);
    } while( (osStatus < 0) && (errno == EINTR) );

    if (osStatus < 0) {
        /*! @retval HEAPBUF_E_OSFAILURE Driver ioctl failed */
        status = HEAPBUF_E_OSFAILURE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBufDrv_ioctl",
                             status,
                             "Driver ioctl failed!");
    }
    else {
        /* First field in the structure is the API status. */
        status = ((HeapBufDrv_CmdArgs *) args)->apiStatus;
    }

    GT_1trace (curTrace, GT_LEAVE, "HeapBufDrv_ioctl", status);

    /*! @retval HEAPBUF_SUCCESS Operation successfully completed. */
    return status;
}



#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
