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
 *  @file   SharedRegionDrv.c
 *
 *  @brief  OS-specific implementation of SharedRegion driver for Linux
 *
 *  ============================================================================
 */


/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <Trace.h>
#include <Memory.h>
#include <MultiProc.h>

/* Module specific header files */
#include <Gate.h>
#include <SharedRegion.h>
#include <SharedRegionDrvDefs.h>

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
 *  @brief  Driver name for SharedRegion.
 */
#define SHAREDREGION_DRIVER_NAME     "/dev/syslinkipc/SharedRegion"


/** ============================================================================
 *  Globals
 *  ============================================================================
 */
/*!
 *  @brief  Driver handle for SharedRegion in this process.
 */
static Int32 SharedRegionDrv_handle = 0;

/*!
 *  @brief  Reference count for the driver handle.
 */
static UInt32 SharedRegionDrv_refCount = 0;


/** ============================================================================
 *  Functions
 *  ============================================================================
 */
/*!
 *  @brief  Function to open the SharedRegion driver.
 *
 *  @sa     SharedRegionDrv_close
 */
Int
SharedRegionDrv_open (Void)
{
    Int status      = SHAREDREGION_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "SharedRegionDrv_open");

    if (SharedRegionDrv_refCount == 0) {
        SharedRegionDrv_handle = open (SHAREDREGION_DRIVER_NAME,
                                       O_SYNC | O_RDWR);
        if (SharedRegionDrv_handle < 0) {
            perror ("SharedRegion driver open: ");
            /*! @retval SHAREDREGION_E_OSFAILURE Failed to open SharedRegion
             *          driver with OS
             */
            status = SHAREDREGION_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SharedRegionDrv_open",
                                 status,
                                 "Failed to open SharedRegion driver with OS!");
        }
        else {
            osStatus = fcntl (SharedRegionDrv_handle, F_SETFD, FD_CLOEXEC);
            if (osStatus != 0) {
                /*! @retval SHAREDREGION_E_OSFAILURE Failed to set file
                 *          descriptor flags
                 */
                status = SHAREDREGION_E_OSFAILURE;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "SharedRegionDrv_open",
                                     status,
                                     "Failed to set file descriptor flags!");
            }
            else{
                /* TBD: Protection for refCount. */
                SharedRegionDrv_refCount++;
            }
        }
    }
    else {
        SharedRegionDrv_refCount++;
    }

    GT_1trace (curTrace, GT_LEAVE, "SharedRegionDrv_open", status);

    /*! @retval SHAREDREGION_SUCCESS Operation successfully completed. */
    return status;
}


/*!
 *  @brief  Function to close the SharedRegion driver.
 *
 *  @sa     SharedRegionDrv_open
 */
Int
SharedRegionDrv_close (Void)
{
    Int status      = SHAREDREGION_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "SharedRegionDrv_close");

    /* TBD: Protection for refCount. */
    SharedRegionDrv_refCount--;
    if (SharedRegionDrv_refCount == 0) {
        osStatus = close (SharedRegionDrv_handle);
        if (osStatus != 0) {
            perror ("SharedRegion driver close: ");
            /*! @retval SHAREDREGION_E_OSFAILURE Failed to open SharedRegion
             *          driver with OS
             */
            status = SHAREDREGION_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SharedRegionDrv_close",
                                 status,
                                 "Failed to close SharedRegion driver with OS!");
        }
        else {
            SharedRegionDrv_handle = 0;
        }
    }

    GT_1trace (curTrace, GT_LEAVE, "SharedRegionDrv_close", status);

    /*! @retval SHAREDREGION_SUCCESS Operation successfully completed. */
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
SharedRegionDrv_ioctl (UInt32 cmd, Ptr args)
{
    Int                       status   = SHAREDREGION_SUCCESS;
    int                       osStatus = 0;
    SharedRegion_Info *       info     = NULL;
    SharedRegionDrv_CmdArgs * cargs    = (SharedRegionDrv_CmdArgs *) args;
    SharedRegion_Config       config;
    Memory_MapInfo            mapInfo;
    UInt16                    i;
    UInt16                    j;

    GT_2trace (curTrace, GT_ENTER, "SharedRegionDrv_ioctl", cmd, args);

    GT_assert (curTrace, (SharedRegionDrv_refCount > 0));

    if (cmd == CMD_SHAREDREGION_SETUP) {
        cargs->args.setup.defaultCfg = &config;
        /* cargs->args.setup.cfg = &config; */
        MemoryOS_copy(&config, cargs->args.setup.config,
            sizeof(SharedRegion_Config));
    }

    osStatus = ioctl (SharedRegionDrv_handle, cmd, args);
    if (osStatus < 0) {
        /*! @retval SHAREDREGION_E_OSFAILURE Driver ioctl failed */
        status = SHAREDREGION_E_OSFAILURE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "SharedRegionDrv_ioctl",
                             status,
                             "Driver ioctl failed!");
    }
    else {
        /* First field in the structure is the API status. */
        status = ((SharedRegionDrv_CmdArgs *) args)->apiStatus;

        /* Convert the base address to user virtual address */
        if (cmd == CMD_SHAREDREGION_SETUP) {
            for (i = 0u; (   (i < config.maxRegions) && (status >= 0)); i++) {
                for (j = 0u; j < (MultiProc_getMaxProcessors() + 1u); j++) {
                    info = (  cargs->args.setup.table
                            + (j * config.maxRegions)
                            + i);
                    if (info->isValid == TRUE) {
                        mapInfo.src  = (UInt32) info->base;
                        mapInfo.size = info->len;
                        status = Memory_map (&mapInfo);
                        if (status < 0) {
                            GT_setFailureReason (curTrace,
                                                 GT_4CLASS,
                                                 "SharedRegionDrv_ioctl",
                                                 status,
                                                 "Memory_map failed!");
                        }
                        else {
                            info->base = (Ptr) mapInfo.dst;
                        }
                    }
                }
            }
        }
    }

    GT_1trace (curTrace, GT_LEAVE, "SharedRegionDrv_ioctl", status);

    /*! @retval SHAREDREGION_SUCCESS Operation successfully completed. */
    return status;
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
