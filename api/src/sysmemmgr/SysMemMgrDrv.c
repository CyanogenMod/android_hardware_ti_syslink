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
 *  @file   SysMemMgrDrv.c
 *
 *  @brief      OS-specific implementation of SysMgr driver for Linux
 *
 *  ============================================================================
 */


/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <Trace.h>
#include <Bitops.h>
#include <Atomic_Ops.h>

/* Module specific header files */
#include <Gate.h>
#include <Memory.h>
#include <SysMemMgr.h>
#include <SysMemMgrDrvDefs.h>

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
#define SYSMGR_DRIVER_NAME     "/dev/syslinkipc/SysMemMgr"

/* Macro to make a correct module magic number with refCount */
#define SYSMEMMGR_MAKE_MAGICSTAMP(x) ((SYSMEMMGR_MODULEID << 12u) | (x))


/** ============================================================================
 *  Globals
 *  ============================================================================
 */
/*!
 *  @brief  Driver handle for SysMgr in this process.
 */
static Int32 SysMemMgrDrv_handle = 0;

/*!
 *  @brief  Reference count for the driver handle.
 */
static Atomic SysMemMgrDrv_refCount;


/** ============================================================================
 *  Functions
 *  ============================================================================
 */
/*!
 *  @brief  Function to open the SysMgr driver.
 *
 *  @sa     SysMemMgrDrv_close
 */
Int
SysMemMgrDrv_open (Void)
{
    Int status      = SYSMEMMGR_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "SysMemMgrDrv_open");

    /* This sets the refCount variable is not initialized, upper 16 bits is
     * written with module Id to ensure correctness of refCount variable.
     */
    Atomic_cmpmask_and_set (&SysMemMgrDrv_refCount,
                            SYSMEMMGR_MAKE_MAGICSTAMP(0),
                            SYSMEMMGR_MAKE_MAGICSTAMP(0));

    if (   Atomic_inc_return (&SysMemMgrDrv_refCount)
        != SYSMEMMGR_MAKE_MAGICSTAMP(1u)) {
        status = SYSMEMMGR_S_DRVALREADYOPENED;
        GT_0trace (curTrace,
                   GT_2CLASS,
                   "SysMemMgrDrv Module already opened!");
    }
    else {
        SysMemMgrDrv_handle = open (SYSMGR_DRIVER_NAME, O_SYNC | O_RDWR);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (SysMemMgrDrv_handle < 0) {
            Atomic_set (&SysMemMgrDrv_refCount, SYSMEMMGR_MAKE_MAGICSTAMP(0));
            perror (SYSMGR_DRIVER_NAME);
            /*! @retval SYSMEMMGR_E_OSFAILURE Failed to open SysMgr driver with
             * OS
             */
            status = SYSMEMMGR_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SysMemMgrDrv_open",
                                 status,
                                 "Failed to open SysMgr driver with OS!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            osStatus = fcntl (SysMemMgrDrv_handle, F_SETFD, FD_CLOEXEC);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (osStatus != 0) {
                Atomic_set (&SysMemMgrDrv_refCount,
                            SYSMEMMGR_MAKE_MAGICSTAMP(0));
                /*! @retval SYSMEMMGR_E_OSFAILURE Failed to set file descriptor
                 * flags */
                status = SYSMEMMGR_E_OSFAILURE;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "SysMemMgrDrv_open",
                                     status,
                                     "Failed to set file descriptor flags!");
            }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        }
    }

    GT_1trace (curTrace, GT_LEAVE, "SysMemMgrDrv_open", status);

/*! @retval SYSMGR_SUCCESS Operation successfully completed. */
    return status;
}


/*!
 *  @brief  Function to close the SysMgr driver.
 *
 *  @sa     SysMemMgrDrv_open
 */
Int
SysMemMgrDrv_close (Void)
{
    Int status      = SYSMEMMGR_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "SysMemMgrDrv_close");

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (   Atomic_cmpmask_and_lt (&(SysMemMgrDrv_refCount),
                                  SYSMEMMGR_MAKE_MAGICSTAMP(0),
                                  SYSMEMMGR_MAKE_MAGICSTAMP(1))
        == TRUE) {
        /*! @retval SYSMEMMGR_E_INVALIDSTATE Module was not initialized */
        status = SYSMEMMGR_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "SysMemMgrDrv_close",
                             status,
                             "Module was not initialized!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        if (   Atomic_dec_return (&SysMemMgrDrv_refCount)
            == SYSMEMMGR_MAKE_MAGICSTAMP(0)) {
            osStatus = close (SysMemMgrDrv_handle);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (osStatus != 0) {
                perror ("SysMgr driver close: ");
                /*! @retval SYSMEMMGR_E_OSFAILURE Failed to open SysMgr driver
                 * with OS */
                status = SYSMEMMGR_E_OSFAILURE;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "SysMemMgrDrv_close",
                                     status,
                                     "Failed to close SysMgr driver with OS!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                SysMemMgrDrv_handle = 0;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        }
    }

    GT_1trace (curTrace, GT_LEAVE, "SysMemMgrDrv_close", status);

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
SysMemMgrDrv_ioctl (UInt32 cmd, Ptr args)
{
    Int                    status      = SYSMEMMGR_SUCCESS;
    int                    osStatus    = 0;
    SysMemMgrDrv_CmdArgs * cargs = (SysMemMgrDrv_CmdArgs *) args;
    Memory_MapInfo         info;
    SysMemMgrDrv_CmdArgs   cmdArgs;

    GT_2trace (curTrace, GT_ENTER, "SysMemMgrDrv_ioctl", cmd, args);

    GT_assert (curTrace, (SysMemMgrDrv_refCount > 0));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (   Atomic_cmpmask_and_lt (&(SysMemMgrDrv_refCount),
                                  SYSMEMMGR_MAKE_MAGICSTAMP(0),
                                  SYSMEMMGR_MAKE_MAGICSTAMP(1))
        == TRUE) {
        /*! @retval SYSMEMMGR_E_INVALIDSTATE Module was not initialized */
        status = SYSMEMMGR_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "SysMemMgrDrv_ioctl",
                             status,
                             "Module was not initialized!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        osStatus = ioctl (SysMemMgrDrv_handle, cmd, args);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (osStatus < 0) {
            perror ("ioctl");
        /*! @retval SYSMEMMGR_E_OSFAILURE Driver ioctl failed */
            status = SYSMEMMGR_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SysMemMgrDrv_ioctl",
                                 status,
                                 "Driver ioctl failed!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            /* First field in the structure is the API status. */
            status = ((SysMemMgrDrv_CmdArgs *) args)->apiStatus;

            if (cmd == CMD_SYSMEMMGR_ALLOC) {
                /* Get the user virtual address of the buffer */
                info.src  = (UInt32) cargs->args.alloc.phys;
                info.size = cargs->args.alloc.size;
                status = Memory_map (&info);
                cargs->args.alloc.buf = (Ptr) info.dst;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
                if (status < 0) {
                    /* free the allocated buffer */
                    cmdArgs.args.free.kbuf  = cargs->args.alloc.kbuf;
                    cmdArgs.args.free.buf   = cargs->args.alloc.buf;
                    cmdArgs.args.free.phys  = cargs->args.alloc.phys;
                    cmdArgs.args.free.size  = cargs->args.alloc.size;
                    cmdArgs.args.free.flags = cargs->args.alloc.flags;
                    ioctl (SysMemMgrDrv_handle, CMD_SYSMEMMGR_FREE, &cmdArgs);
                    cargs->args.alloc.buf = NULL;
                    GT_setFailureReason (curTrace,
                                         GT_4CLASS,
                                         "SysMemMgrDrv_ioctl",
                                         status,
                                         "Memory_map failed!");

                }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */


    GT_1trace (curTrace, GT_LEAVE, "SysMemMgrDrv_ioctl", status);

/*! @retval SYSMGR_SUCCESS Operation successfully completed. */
    return status;
}



#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
