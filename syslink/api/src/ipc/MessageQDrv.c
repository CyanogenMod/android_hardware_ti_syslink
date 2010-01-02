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
 *  @file   MessageQDrv.c
 *
 *  @brief      OS-specific implementation of MessageQ driver for Linux
 *  ============================================================================
 */


/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <Trace.h>

/* Module specific header files */
#include <Heap.h>
#include <MessageQ.h>
#include <MessageQDrvDefs.h>

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
 *  @brief  Driver name for MessageQ.
 */
#define MESSAGEQ_DRIVER_NAME     "/dev/syslinkipc/MessageQ"


/** ============================================================================
 *  Globals
 *  ============================================================================
 */
/*!
 *  @brief  Driver handle for MessageQ in this process.
 */
static Int32 MessageQDrv_handle = 0;

/*!
 *  @brief  Reference count for the driver handle.
 */
static UInt32 MessageQDrv_refCount = 0;


/** ============================================================================
 *  Functions
 *  ============================================================================
 */
/*!
 *  @brief  Function to open the MessageQ driver.
 *
 *  @sa     MessageQDrv_close
 */
Int
MessageQDrv_open (Void)
{
    Int status      = MESSAGEQ_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "MessageQDrv_open");

    if (MessageQDrv_refCount == 0) {
        MessageQDrv_handle = open (MESSAGEQ_DRIVER_NAME,
                                  O_SYNC | O_RDWR);
        if (MessageQDrv_handle < 0) {
            perror (MESSAGEQ_DRIVER_NAME);
            /*! @retval MESSAGEQ_E_OSFAILURE Failed to open
             *          MessageQ driver with OS
             */
            status = MESSAGEQ_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MessageQDrv_open",
                                 status,
                                 "Failed to open MessageQ driver with OS!");
        }
        else {
            osStatus = fcntl (MessageQDrv_handle, F_SETFD, FD_CLOEXEC);
            if (osStatus != 0) {
                /*! @retval MESSAGEQ_E_OSFAILURE
                 *          Failed to set file descriptor flags
                 */
                status = MESSAGEQ_E_OSFAILURE;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "MessageQDrv_open",
                                     status,
                                     "Failed to set file descriptor flags!");
            }
            else{
                /* TBD: Protection for refCount. */
                MessageQDrv_refCount++;
            }
        }
    }
    else {
        MessageQDrv_refCount++;
    }

    GT_1trace (curTrace, GT_LEAVE, "MessageQDrv_open", status);

    /*! @retval MESSAGEQ_SUCCESS Operation successfully completed. */
    return status;
}


/*!
 *  @brief  Function to close the MessageQ driver.
 *
 *  @sa     MessageQDrv_open
 */
Int
MessageQDrv_close (Void)
{
    Int status      = MESSAGEQ_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "MessageQDrv_close");

    /* TBD: Protection for refCount. */
    MessageQDrv_refCount--;
    if (MessageQDrv_refCount == 0) {
        osStatus = close (MessageQDrv_handle);
        if (osStatus != 0) {
            perror ("MessageQ driver close: ");
            /*! @retval MESSAGEQ_E_OSFAILURE
             *          Failed to open MessageQ driver with OS
             */
            status = MESSAGEQ_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MessageQDrv_close",
                                 status,
                                 "Failed to close MessageQ driver with OS!");
        }
        else {
            MessageQDrv_handle = 0;
        }
    }

    GT_1trace (curTrace, GT_LEAVE, "MessageQDrv_close", status);

    /*! @retval MESSAGEQ_SUCCESS Operation successfully completed. */
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
MessageQDrv_ioctl (UInt32 cmd, Ptr args)
{
    Int status      = MESSAGEQ_SUCCESS;
    int osStatus    = 0;

    GT_2trace (curTrace, GT_ENTER, "MessageQDrv_ioctl", cmd, args);

    GT_assert (curTrace, (MessageQDrv_refCount > 0));

    do {
        osStatus = ioctl (MessageQDrv_handle, cmd, args);
    } while( (osStatus < 0) && (errno == EINTR) );

    if (osStatus < 0) {
        /*! @retval MESSAGEQ_E_OSFAILURE Driver ioctl failed */
        status = MESSAGEQ_E_OSFAILURE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQDrv_ioctl",
                             status,
                             "Driver ioctl failed!");
    }
    else {
        /* First field in the structure is the API status. */
        status = ((MessageQDrv_CmdArgs *) args)->apiStatus;
    }

    GT_1trace (curTrace, GT_LEAVE, "MessageQDrv_ioctl", status);

    /*! @retval MESSAGEQ_SUCCESS Operation successfully completed. */
    return status;
}



#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
