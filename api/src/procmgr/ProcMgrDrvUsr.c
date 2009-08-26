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
 *  @file   ProcMgrDrvUsr.c
 *
 *  @brief      User-side OS-specific implementation of ProcMgr driver for Linux
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

/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <Trace.h>
#include <Memory.h>
#include <ProcMgrDrvUsr.h>

/* Module headers */
#include <ProcMgrDrvDefs.h>


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/** ============================================================================
 *  Macros and types
 *  ============================================================================
 */
/*!
 *  @brief  Driver name for ProcMgr.
 */
#define PROCMGR_DRIVER_NAME         "/dev/syslink-procmgr"


/** ============================================================================
 *  Globals
 *  ============================================================================
 */
/*!
 *  @brief  Driver handle for ProcMgr in this process.
 */
static Int32 ProcMgrDrvUsr_handle = -1;

/*!
 *  @brief  Reference count for the driver handle.
 */
static UInt32 ProcMgrDrvUsr_refCount = 0;


/** ============================================================================
 *  Forward declarations of internal functions
 *  ============================================================================
 */
/* Function to map the processor's memory regions to user space. */
static Int _ProcMgrDrvUsr_mapMemoryRegion (ProcMgr_ProcInfo * procInfo);

/* Function to unmap the processor's memory regions to user space. */
static Int _ProcMgrDrvUsr_unmapMemoryRegion (ProcMgr_ProcInfo * procInfo);


/** ============================================================================
 *  Functions
 *  ============================================================================
 */
/*!
 *  @brief  Function to open the ProcMgr driver.
 *
 *  @sa     ProcMgrDrvUsr_close
 */
Int
ProcMgrDrvUsr_open (Void)
{
    Int status      = PROCMGR_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "ProcMgrDrvUsr_open");

    if (ProcMgrDrvUsr_refCount == 0) {
        /* TBD: Protection for refCount. */
        ProcMgrDrvUsr_refCount++;

        ProcMgrDrvUsr_handle = open (PROCMGR_DRIVER_NAME, O_SYNC | O_RDWR);
        if (ProcMgrDrvUsr_handle < 0) {
            perror ("ProcMgr driver open: " PROCMGR_DRIVER_NAME);
            /*! @retval PROCMGR_E_OSFAILURE Failed to open ProcMgr driver with
                        OS */
            status = PROCMGR_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMgrDrvUsr_open",
                                 status,
                                 "Failed to open ProcMgr driver with OS!");
        }
        else {
            osStatus = fcntl (ProcMgrDrvUsr_handle, F_SETFD, FD_CLOEXEC);
            if (osStatus != 0) {
                /*! @retval PROCMGR_E_OSFAILURE Failed to set file descriptor
                                                flags */
                status = PROCMGR_E_OSFAILURE;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "ProcMgrDrvUsr_open",
                                     status,
                                     "Failed to set file descriptor flags!");
            }
        }
    }
    else {
        ProcMgrDrvUsr_refCount++;
    }

    GT_1trace (curTrace, GT_LEAVE, "ProcMgrDrvUsr_open", status);

    /*! @retval PROCMGR_SUCCESS Operation successfully completed. */
    return status;
}


/*!
 *  @brief  Function to close the ProcMgr driver.
 *
 *  @sa     ProcMgrDrvUsr_open
 */
Int
ProcMgrDrvUsr_close (Void)
{
    Int status      = PROCMGR_SUCCESS;
    int osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "ProcMgrDrvUsr_close");

    /* TBD: Protection for refCount. */
    ProcMgrDrvUsr_refCount--;
    if (ProcMgrDrvUsr_refCount == 0) {
        osStatus = close (ProcMgrDrvUsr_handle);
        if (osStatus != 0) {
            perror ("ProcMgr driver close: " PROCMGR_DRIVER_NAME);
            /*! @retval PROCMGR_E_OSFAILURE Failed to open ProcMgr driver
                                            with OS */
            status = PROCMGR_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMgrDrvUsr_close",
                                 status,
                                 "Failed to close ProcMgr driver with OS!");
        }
        else {
            ProcMgrDrvUsr_handle = 0;
        }
    }

    GT_1trace (curTrace, GT_LEAVE, "ProcMgrDrvUsr_close", status);

    /*! @retval PROCMGR_SUCCESS Operation successfully completed. */
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
ProcMgrDrvUsr_ioctl (UInt32 cmd, Ptr args)
{
    Int     status          = PROCMGR_SUCCESS;
    Int     tmpStatus       = PROCMGR_SUCCESS;
    int     osStatus        = 0;
    Bool    driverOpened    = FALSE;

    GT_2trace (curTrace, GT_ENTER, "ProcMgrDrvUsr_ioctl", cmd, args);

    if (ProcMgrDrvUsr_handle < 0) {
        /* Need to open the driver handle. It was not opened from this process.
         */
        driverOpened = TRUE;
        status = ProcMgrDrvUsr_open ();
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMgrDrvUsr_ioctl",
                                 status,
                                 "Failed to open OS driver handle!");
        }
    }

    GT_assert (curTrace, (ProcMgrDrvUsr_refCount > 0));

    switch (cmd) {
        case CMD_PROCMGR_ATTACH:
        {
            ProcMgr_CmdArgsAttach * srcArgs = (ProcMgr_CmdArgsAttach *) args;

            osStatus = ioctl (ProcMgrDrvUsr_handle, cmd, args);
            if (osStatus < 0) {
                /*! @retval PROCMGR_E_OSFAILURE Driver ioctl failed */
                status = PROCMGR_E_OSFAILURE;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "ProcMgrDrvUsr_ioctl",
                                     status,
                                     "Driver ioctl failed!");
            }
            else {
                status = _ProcMgrDrvUsr_mapMemoryRegion (&(srcArgs->procInfo));
                if (status < 0) {
                    GT_setFailureReason (curTrace,
                                         GT_4CLASS,
                                         "ProcMgrDrvUsr_ioctl",
                                         status,
                                         "Failed to map memory regions!");
                }
            }
        }
        break;

        case CMD_PROCMGR_OPEN:
        {
            ProcMgr_CmdArgsOpen * srcArgs = (ProcMgr_CmdArgsOpen *) args;

            osStatus = ioctl (ProcMgrDrvUsr_handle, cmd, args);
            if (osStatus < 0) {
                /*! @retval PROCMGR_E_OSFAILURE Driver ioctl failed */
                status = PROCMGR_E_OSFAILURE;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "ProcMgrDrvUsr_ioctl",
                                     status,
                                     "Driver ioctl failed!");
            }
            else {
                status = _ProcMgrDrvUsr_mapMemoryRegion (&(srcArgs->procInfo));
                if (status < 0) {
                    GT_setFailureReason (curTrace,
                                         GT_4CLASS,
                                         "ProcMgrDrvUsr_ioctl",
                                         status,
                                         "Failed to map memory regions!");
                }
            }
        }
        break;

        case CMD_PROCMGR_DETACH:
        {
            ProcMgr_CmdArgsDetach * srcArgs = (ProcMgr_CmdArgsDetach *) args;

            osStatus = ioctl (ProcMgrDrvUsr_handle, cmd, args);
            if (osStatus < 0) {
                /*! @retval PROCMGR_E_OSFAILURE Driver ioctl failed */
                status = PROCMGR_E_OSFAILURE;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "ProcMgrDrvUsr_ioctl",
                                     status,
                                     "Driver ioctl failed!");
            }
            else {
                status = _ProcMgrDrvUsr_unmapMemoryRegion(&(srcArgs->procInfo));
                if (status < 0) {
                    GT_setFailureReason (curTrace,
                                         GT_4CLASS,
                                         "ProcMgrDrvUsr_ioctl",
                                         status,
                                         "Failed to unmap memory regions!");
                }
            }
        }
        break;

        case CMD_PROCMGR_CLOSE:
        {
            ProcMgr_CmdArgsClose * srcArgs = (ProcMgr_CmdArgsClose *) args;

            osStatus = ioctl (ProcMgrDrvUsr_handle, cmd, args);
            if (osStatus < 0) {
                /*! @retval PROCMGR_E_OSFAILURE Driver ioctl failed */
                status = PROCMGR_E_OSFAILURE;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "ProcMgrDrvUsr_ioctl",
                                     status,
                                     "Driver ioctl failed!");
            }
            else {
                status = _ProcMgrDrvUsr_unmapMemoryRegion(&(srcArgs->procInfo));
                if (status < 0) {
                    GT_setFailureReason (curTrace,
                                         GT_4CLASS,
                                         "ProcMgrDrvUsr_ioctl",
                                         status,
                                         "Failed to unmap memory regions!");
                }
            }
        }
        break;

        default:
        {
            osStatus = ioctl (ProcMgrDrvUsr_handle, cmd, args);
            if (osStatus < 0) {
                /*! @retval PROCMGR_E_OSFAILURE Driver ioctl failed */
                status = PROCMGR_E_OSFAILURE;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "ProcMgrDrvUsr_ioctl",
                                     status,
                                     "Driver ioctl failed!");
            }
        }
        break ;

        if (osStatus >= 0) {
            /* First field in the structure is the API status. */
            status = ((ProcMgr_CmdArgs *) args)->apiStatus;
        }

        GT_1trace (curTrace,
                   GT_1CLASS,
                   "    ProcMgrDrvUsr_ioctl: API Status [0x%x]",
                   status);
    }

    if (driverOpened == TRUE) {
        /* If the driver was temporarily opened here, close it. */
        tmpStatus = ProcMgrDrvUsr_close ();
        if ((status > 0) && (tmpStatus < 0)) {
            status = tmpStatus;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMgrDrvUsr_ioctl",
                                 status,
                                 "Failed to close OS driver handle!");
        }
        ProcMgrDrvUsr_handle = -1;
    }

    GT_1trace (curTrace, GT_LEAVE, "ProcMgrDrvUsr_ioctl", status);

    /*! @retval PROCMGR_SUCCESS Operation successfully completed. */
    return status;
}


/** ============================================================================
 *  Internal functions
 *  ============================================================================
 */
/*!
 *  @brief  Function to map the processor's memory regions to user space.
 *
 *  @param  procInfo    Processor information structure
 *
 *  @sa     _ProcMgrDrvUsr_unmapMemoryRegion
 */
static
Int
_ProcMgrDrvUsr_mapMemoryRegion (ProcMgr_ProcInfo * procInfo)
{
    Int                status = PROCMGR_SUCCESS;
    ProcMgr_AddrInfo * memEntry;
    Memory_MapInfo     mapInfo;
    UInt16             i;

    GT_1trace (curTrace, GT_ENTER, "_ProcMgrDrvUsr_mapMemoryRegion", procInfo);

    GT_1trace (curTrace,
               GT_2CLASS,
               "    Number of memory entries: %d\n",
               procInfo->numMemEntries);

    for (i = 0 ; i < procInfo->numMemEntries ; i++) {
        GT_assert (curTrace, (i < PROCMGR_MAX_MEMORY_REGIONS));
        /* Map all memory regions to user-space. */
        memEntry = &(procInfo->memEntries [i]);

        /* Get virtual address for shared memory. */
        /* TBD: For now, assume that slave virt is same as physical
         * address.
         */
        mapInfo.src  = memEntry->addr [ProcMgr_AddrType_MasterUsrVirt];
        mapInfo.size = memEntry->size;
        mapInfo.isCached = FALSE;
        mapInfo.drvHandle = (Ptr) ProcMgrDrvUsr_handle;

        status = Memory_map (&mapInfo);
        if (status < 0) {
            /*! @retval PROCMGR_E_OSFAILURE Memory map to user space failed */
            status = PROCMGR_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMgrDrvUsr_ioctl",
                                 status,
                                 "Memory map to user space failed!");
        }
        else {
            GT_5trace (curTrace,
                       GT_2CLASS,
                       "    Memory region %d mapped:\n"
                       "        Region slave   base [0x%x]\n"
                       "        Region knlVirt base [0x%x]\n"
                       "        Region usrVirt base [0x%x]\n"
                       "        Region size         [0x%x]\n",
                       i,
                       memEntry->addr [ProcMgr_AddrType_SlaveVirt],
                       memEntry->addr [ProcMgr_AddrType_MasterKnlVirt],
                       mapInfo.dst,
                       mapInfo.size);
            memEntry->addr [ProcMgr_AddrType_MasterUsrVirt] = mapInfo.dst;
            memEntry->isInit = TRUE;
        }
    }

    GT_1trace (curTrace, GT_LEAVE, "_ProcMgrDrvUsr_mapMemoryRegion", status);

    /*! @retval PROCMGR_SUCCESS Operation successfully completed. */
    return status;
}


/*!
 *  @brief  Function to unmap the processor's memory regions to user space.
 *
 *  @param  procInfo    Processor information structure
 *
 *  @sa     _ProcMgrDrvUsr_mapMemoryRegion
 */
static
Int
_ProcMgrDrvUsr_unmapMemoryRegion (ProcMgr_ProcInfo * procInfo)
{
    Int                status = PROCMGR_SUCCESS;
    ProcMgr_AddrInfo * memEntry;
    Memory_UnmapInfo   unmapInfo;
    UInt16             i;

    GT_1trace (curTrace, GT_ENTER, "_ProcMgrDrvUsr_unmapMemoryRegion",
               procInfo);

    for (i = 0 ; i < procInfo->numMemEntries ; i++) {
        GT_assert (curTrace, (i < PROCMGR_MAX_MEMORY_REGIONS));
        /* Unmap all memory regions from user-space. */
        memEntry = &(procInfo->memEntries [i]);

        if (memEntry->isInit == TRUE) {
            /* Unmap memory region from user-space. */
            unmapInfo.addr = memEntry->addr [ProcMgr_AddrType_MasterUsrVirt];
            unmapInfo.size = memEntry->size;
            status = Memory_unmap (&unmapInfo);
            if (status < 0) {
                /*! @retval PROCMGR_E_OSFAILURE Memory unmap from user
                                                space failed */
                status = PROCMGR_E_OSFAILURE;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "ProcMgrDrvUsr_ioctl",
                                     status,
                                     "Memory unmap from user space failed!");
            }
            else {
                memEntry->isInit = FALSE;
            }
        }
    }

    GT_1trace (curTrace, GT_LEAVE, "_ProcMgrDrvUsr_unmapMemoryRegion", status);

    /*! @retval PROCMGR_SUCCESS Operation successfully completed. */
    return status;
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
