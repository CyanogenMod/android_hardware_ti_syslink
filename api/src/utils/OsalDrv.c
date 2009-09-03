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
 *  @file   OsalDrv.c
 *
 *  @brief      User-side OS-specific implementation of OsalDrv driver for Linux
 *
 *  ============================================================================
 */


/* Linux specific header files */
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>

/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <Trace.h>
#include <Memory.h>
#include <OsalDrv.h>


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/** ============================================================================
 *  Macros and types
 *  ============================================================================
 */
/*!
 *  @brief  Driver name for OsalDrv.
 */
#define OSALDRV_DRIVER_NAME         "/dev/syslink-procmgr"


/** ============================================================================
 *  Globals
 *  ============================================================================
 */
/*!
 *  @brief  Driver handle for OsalDrv in this process.
 */
static Int32 OsalDrv_handle = -1;

/*!
 *  @brief  Reference count for the driver handle.
 */
static UInt32 OsalDrv_refCount = 0;


/** ============================================================================
 *  Functions
 *  ============================================================================
 */
/*!
 *  @brief  Function to open the OsalDrv driver.
 *
 *  @sa     OsalDrv_close
 */
Int
OsalDrv_open (Void)
{
    Int status      = OSALDRV_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "OsalDrv_open");

    if (OsalDrv_refCount == 0) {
        /* TBD: Protection for refCount. */
        OsalDrv_refCount++;

        OsalDrv_handle = open (OSALDRV_DRIVER_NAME, O_SYNC | O_RDWR);
        if (OsalDrv_handle < 0) {
            perror ("OsalDrv driver open: " OSALDRV_DRIVER_NAME);
            /*! @retval OSALDRV_E_OSFAILURE Failed to open OsalDrv driver with
                        OS */
            status = OSALDRV_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "OsalDrv_open",
                                 status,
                                 "Failed to open OsalDrv driver with OS!");
        }
        else {
            osStatus = fcntl (OsalDrv_handle, F_SETFD, FD_CLOEXEC);
            if (osStatus != 0) {
                /*! @retval OSALDRV_E_OSFAILURE Failed to set file descriptor
                                                flags */
                status = OSALDRV_E_OSFAILURE;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "OsalDrv_open",
                                     status,
                                     "Failed to set file descriptor flags!");
            }
        }
    }
    else {
        OsalDrv_refCount++;
    }

    GT_1trace (curTrace, GT_LEAVE, "OsalDrv_open", status);

    /*! @retval OSALDRV_SUCCESS Operation successfully completed. */
    return status;
}


/*!
 *  @brief  Function to close the OsalDrv driver.
 *
 *  @sa     OsalDrv_open
 */
Int
OsalDrv_close (Void)
{
    Int status      = OSALDRV_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "OsalDrv_close");

    /* TBD: Protection for refCount. */
    OsalDrv_refCount--;
    if (OsalDrv_refCount == 0) {
        osStatus = close (OsalDrv_handle);
        if (osStatus != 0) {
            perror ("OsalDrv driver close: " OSALDRV_DRIVER_NAME);
            /*! @retval OSALDRV_E_OSFAILURE Failed to open OsalDrv driver
                                            with OS */
            status = OSALDRV_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "OsalDrv_close",
                                 status,
                                 "Failed to close OsalDrv driver with OS!");
        }
        else {
            OsalDrv_handle = 0;
        }
    }

    GT_1trace (curTrace, GT_LEAVE, "OsalDrv_close", status);

    /*! @retval OSALDRV_SUCCESS Operation successfully completed. */
    return status;
}


/*!
 *  @brief  Function to map a memory region specific to the driver.
 *
 *  @sa     OsalDrv_close,OsalDrv_open
 */
UInt32
OsalDrv_map (UInt32 addr, UInt32 size)
{
    UInt32 pageSize = getpagesize ();
    UInt32 userAddr = (UInt32) NULL;
    UInt32 taddr;
    UInt32 tsize;

    GT_0trace (curTrace, GT_ENTER, "OsalDrv_map");

    if (OsalDrv_refCount > 0) {
        taddr = addr;
        tsize = size;
        /* Align the physical address to page boundary */
        tsize = tsize + (taddr % pageSize);
        taddr = taddr - (taddr % pageSize);

        userAddr = (UInt32) mmap (NULL,
                                  tsize,
                                  PROT_READ | PROT_WRITE,
                                  MAP_SHARED,
                                  OsalDrv_handle,
                                  (off_t) taddr);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (userAddr == (UInt32) MAP_FAILED) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "OsalDrv_map",
                                 OSALDRV_E_MAP,
                                 "Failed to map memory to user space!");
        }
        else {
#endif /* #if !defined(SYSLINK_BUILD_OPTIMIZE) */
            /* Change the user address to reflect the actual user address of
             * memory block mapped. This is done since during mmap memory block
             * was shifted (+-) so that it is aligned to page boundary.
             */
            userAddr = userAddr + (addr % pageSize);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
#endif /* #if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

    GT_1trace (curTrace, GT_LEAVE, "OsalDrv_map", userAddr);

    /*! @retval NULL Operation was successful. */
    /*! @retval valid-address Operation successfully completed. */
    return userAddr;
}


/*!
 *  @brief  Function to unmap a memory region specific to the driver.
 *
 *  @sa     OsalDrv_close,OsalDrv_open,OsalDrv_map
 */
Void
OsalDrv_unmap (UInt32 addr, UInt32 size)
{
    UInt32 pageSize = getpagesize ();
    UInt32 taddr;
    UInt32 tsize;

    GT_0trace (curTrace, GT_ENTER, "OsalDrv_unmap");

    if (OsalDrv_refCount > 0) {
        taddr = addr;
        tsize = size;

        /* Get the actual user address and size. Since these are modified at the
         * time of mmaping, to have memory block page boundary aligned.
         */
        tsize = tsize + (taddr % pageSize);
        taddr = taddr - (taddr % pageSize);
        munmap ((Void *) taddr, tsize);
    }

    GT_0trace (curTrace, GT_LEAVE, "OsalDrv_unmap");
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
