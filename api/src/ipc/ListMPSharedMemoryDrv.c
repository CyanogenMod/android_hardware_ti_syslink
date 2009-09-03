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
 *  @file   ListMPSharedMemoryDrv.c
 *
 *  @brief      OS-specific implementation of ListMPSharedMemory driver
 *              for Linux
 *
 *  ============================================================================
 */


/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <Trace.h>

/* Module specific header files */
#include <_ListMP.h>
#include <ListMP.h>
#include <ListMPSharedMemory.h>
#include <ListMPSharedMemoryDrvDefs.h>

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
 *  @brief  Driver name for ListMPSharedMemory.
 */
#define LISTMPSHAREDMEMORY_DRIVER_NAME     "/dev/syslinkipc/ListMPSharedMemory"


/** ============================================================================
 *  Globals
 *  ============================================================================
 */
/*!
 *  @brief  Driver handle for ListMPSharedMemory in this process.
 */
static Int32 ListMPSharedMemoryDrv_handle = 0;

/*!
 *  @brief  Reference count for the driver handle.
 */
static UInt32 ListMPSharedMemoryDrv_refCount = 0;


/** ============================================================================
 *  Functions
 *  ============================================================================
 */
/*!
 *  @brief  Function to open the ListMPSharedMemory driver.
 *
 *  @sa     ListMPSharedMemoryDrv_close
 */
Int
ListMPSharedMemoryDrv_open (Void)
{
    Int status      = LISTMPSHAREDMEMORY_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "ListMPSharedMemoryDrv_open");

    if (ListMPSharedMemoryDrv_refCount == 0) {
        ListMPSharedMemoryDrv_handle = open (LISTMPSHAREDMEMORY_DRIVER_NAME,
                                             O_SYNC | O_RDWR);
        if (ListMPSharedMemoryDrv_handle < 0) {
            perror (LISTMPSHAREDMEMORY_DRIVER_NAME);
            /*! @retval LISTMPSHAREDMEMORY_E_OSFAILURE
             *          Failed to open ListMPSharedMemory driver with OS
             */
            status = LISTMPSHAREDMEMORY_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ListMPSharedMemoryDrv_open",
                                 status,
                                 "Failed to open ListMPSharedMemory driver"
                                 " with OS!");
        }
        else {
            osStatus = fcntl (ListMPSharedMemoryDrv_handle,
                              F_SETFD,
                              FD_CLOEXEC);
            if (osStatus != 0) {
                /*! @retval LISTMPSHAREDMEMORY_E_OSFAILURE
                 *          Failed to set file descriptor flags
                 */
                status = LISTMPSHAREDMEMORY_E_OSFAILURE;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "ListMPSharedMemoryDrv_open",
                                     status,
                                     "Failed to set file descriptor flags!");
            }
        }
    }

    if (status == LISTMPSHAREDMEMORY_SUCCESS) {
        /* TBD: Protection for refCount. */
        ListMPSharedMemoryDrv_refCount++;
    }

    GT_1trace (curTrace, GT_LEAVE, "ListMPSharedMemoryDrv_open", status);

    /*! @retval LISTMPSHAREDMEMORY_SUCCESS Operation successfully completed. */
    return status;
}


/*!
 *  @brief  Function to close the ListMPSharedMemory driver.
 *
 *  @sa     ListMPSharedMemoryDrv_open
 */
Int
ListMPSharedMemoryDrv_close (Void)
{
    Int status      = LISTMPSHAREDMEMORY_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "ListMPSharedMemoryDrv_close");

    /* TBD: Protection for refCount. */
    ListMPSharedMemoryDrv_refCount--;
    if (ListMPSharedMemoryDrv_refCount == 0) {
        osStatus = close (ListMPSharedMemoryDrv_handle);
        if (osStatus != 0) {
            perror ("ListMPSharedMemory driver close: ");
            /*! @retval LISTMPSHAREDMEMORY_E_OSFAILURE Failed to open
             *          ListMPSharedMemory driver with OS
             */
            status = LISTMPSHAREDMEMORY_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ListMPSharedMemoryDrv_close",
                                 status,
                                 "Failed to close ListMPSharedMemory driver"
                                 " with OS!");
        }
        else {
            ListMPSharedMemoryDrv_handle = 0;
        }
    }

    GT_1trace (curTrace, GT_LEAVE, "ListMPSharedMemoryDrv_close", status);

    /*! @retval LISTMPSHAREDMEMORY_SUCCESS Operation successfully completed. */
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
ListMPSharedMemoryDrv_ioctl (UInt32 cmd, Ptr args)
{
    Int status      = LISTMPSHAREDMEMORY_SUCCESS;
    int osStatus    = 0;

    GT_2trace (curTrace, GT_ENTER, "ListMPSharedMemoryDrv_ioctl", cmd, args);

    GT_assert (curTrace, (ListMPSharedMemoryDrv_refCount > 0));

    osStatus = ioctl (ListMPSharedMemoryDrv_handle, cmd, args);
    if (osStatus < 0) {
    /*! @retval LISTMPSHAREDMEMORY_E_OSFAILURE Driver ioctl failed */
        status = LISTMPSHAREDMEMORY_E_OSFAILURE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemoryDrv_ioctl",
                             status,
                             "Driver ioctl failed!");
    }
    else {
        /* First field in the structure is the API status. */
        status = ((ListMPSharedMemoryDrv_CmdArgs *) args)->apiStatus;
    }

    GT_1trace (curTrace, GT_LEAVE, "ListMPSharedMemoryDrv_ioctl", status);

    /*! @retval LISTMPSHAREDMEMORY_SUCCESS Operation successfully completed. */
    return status;
}



#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
