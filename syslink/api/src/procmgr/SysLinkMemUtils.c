/*
 *  Syslink-IPC for TI OMAP Processors
 *
 *  Copyright (c) 2008-2010, Texas Instruments Incorporated
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  *  Neither the name of Texas Instruments Incorporated nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 *  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*============================================================================
 *  @file   SysLinkMemUtils.c
 *
 *  @brief     This modules provides syslink Mem utils functionality
 *  ============================================================================
 */


/* Linux specific header files */
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <Memory.h>
#include <Trace.h>

/* Module level headers */
#include <ti/ipc/MultiProc.h>
#include <ProcMgr.h>
#include <SysLinkMemUtils.h>


#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*! @brief Page size */
#define Page_SIZE_4K 4096
/*! @brief Page Mask */
#define Page_MASK(pg_size) (~((pg_size)-1))
/*! @brief Align to lower Page */
#define Page_ALIGN_LOW(addr, pg_size) ((addr) & Page_MASK(pg_size))
/*! @brief Start address of Tiler region */
#define TILER_ADDRESS_START         0x60000000
/*! @brief End address of Tiler region */
#define TILER_ADDRESS_END           0x80000000


/* =============================================================================
 *  APIs
 * =============================================================================
 */
/*!
 *  @brief      Function to Map Host processor to Remote processors
 *              module.
 *
 *              This function can be called by the application to map their
 *              address space to remote slave's address space.
 *
 *  @param      MpuAddr
 *
 *  @sa         SysLinkMemUtils_unmap
 */
Int
SysLinkMemUtils_map (SyslinkMemUtils_MpuAddrToMap   mpuAddrList[],
                     UInt32                         numOfBuffers,
                     UInt32 *                       mappedAddr,
                     ProcMgr_MapType                memType,
                     ProcMgr_ProcId                 procId)
{
    ProcMgr_Handle  procMgrHandle;
    UInt32          mappedSize;
    Int32           status = PROCMGR_SUCCESS;

    if (numOfBuffers > 1) {
        status = PROCMGR_E_INVALIDARG;
        Osal_printf ("SysLinkMemUtils_map numBufError [0x%x]\n", status);
        return status;
    }

    if (procId == PROC_APPM3) {
        procId = PROC_SYSM3;
    }

    /* Open a handle to the ProcMgr instance. */
    status = ProcMgr_open (&procMgrHandle, procId);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_open [0x%x]\n", status);
    }
    else {
        /* FIX ME: Add Proc reserve call */
        status = ProcMgr_map (procMgrHandle, (UInt32)mpuAddrList[0].mpuAddr,
                        (UInt32)mpuAddrList[0].size, mappedAddr, &mappedSize,
                        memType, procId);
         /* FIX ME: Add the table that keeps the C-D translations */
        if (status < 0) {
            Osal_printf ("Error in ProcMgr_map [0x%x]\n", status);
        }
        else {
            status = ProcMgr_close (&procMgrHandle);
        }
    }

    return status;
}


/*!
 *  @brief      Function to unmap
 *
 *
 *  @param      mappedAddr       The remote address to unmap
 *  @param      procId                 The remote Processor ID
 *
 *  @sa         SysLinkMemUtils_map
 */
Int
SysLinkMemUtils_unmap (UInt32 mappedAddr, ProcMgr_ProcId procId)
{
    ProcMgr_Handle procMgrHandle;
    Int32          status = PROCMGR_SUCCESS;

    /* Open a handle to the ProcMgr instance. */
    if (procId == PROC_APPM3) {
        procId = PROC_SYSM3;
    }

    /* Open a handle to the ProcMgr instance. */
    status = ProcMgr_open (&procMgrHandle, procId);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_open [0x%x]\n", status);
    }
    else {
        status = ProcMgr_unmap (procMgrHandle, mappedAddr, procId);
        /* FIX ME: Add Proc unreserve call */
        if (status < 0) {
            Osal_printf ("Error in ProcMgr_unmap [0x%x]\n", status);
        }
        else {
            status = ProcMgr_close (&procMgrHandle);
        }
    }

    return status;
}


/*!
 *  @brief      Function to retrieve physical entries given a remote
 *              Processor's virtual address.
 *
 *              This function returns success state of this function
 *
 *  @param      remoteAddr  The slave's address
 *  @param      size        size of buffer
 *  @param      physEntries Translated physical addresses of each Page.
 *  @param      procId      Remote Processor Id.
 *  @param      flags       Used to pass any custom information for optimization.
 *
 *  @sa         SysLinkMemUtils_virtToPhys
 */
Int
SysLinkMemUtils_virtToPhysPages (UInt32         remoteAddr,
                                 UInt32         numOfPages,
                                 UInt32         physEntries[],
                                 ProcMgr_ProcId procId)
{
    Int             i;
    Int32           status = PROCMGR_SUCCESS;
    ProcMgr_Handle  procMgrHandle;

    GT_1trace (curTrace, GT_ENTER, "SysLinkMemUtils_virtToPhys: remote Addr",
                    remoteAddr);

    if (physEntries == NULL || (numOfPages == 0)) {
        Osal_printf("SysLinkMemUtils_virtToPhys: ERROR:Input arguments invalid:"
                    "physEntries = 0x%x, numOfPages = %d\n",
                    (UInt32)physEntries, numOfPages);
        return PROCMGR_E_FAIL;
    }

    if (procId == PROC_APPM3) {
        procId = PROC_SYSM3;
    }

    Osal_printf ("testing with ProcMgr_virtToPhysPages\n");

    /* Open a handle to the ProcMgr instance. */
    status = ProcMgr_open (&procMgrHandle, procId);
    if (status < 0) {
        Osal_printf ("Error in ProcMgr_open [0x%x]\n", status);
        return PROCMGR_E_FAIL;
    }
    /* TODO: Hack for tiler */
    if(remoteAddr >= TILER_ADDRESS_START && remoteAddr < TILER_ADDRESS_END) {
        for(i = 0; i < numOfPages; i++) {
            physEntries[i] = Page_ALIGN_LOW(remoteAddr, Page_SIZE_4K) + (4096 * i);
        }
    }
    else {
        status = ProcMgr_virtToPhysPages (procMgrHandle, remoteAddr,
                    numOfPages, physEntries, procId);
    }

    if (status < 0) {
        Osal_printf ("Error in ProcMgr_virtToPhysPages [0x%x]\n", status);
    }
    else {
        for (i = 0; i < numOfPages; i++) {
            Osal_printf("physEntries[%d] = 0x%x\n", i, physEntries[i]);
        }
    }

    status = ProcMgr_close (&procMgrHandle);

    return status;
}


/*!
 *  @brief      Function to retrieve physical address of given a remote
 *              Processor's virtual address.
 *
 *              Return value of less than or equal to zero
 *              indicates the translation failure
 *
 *  @param      remoteAddr  The slave's address
 *  @param      physAddr    Translated physical address.
 *  @param      procId      Remote Processor Id.
  *
 *  @sa         SysLinkMemUtils_virtToPhysPages
 */
Int
SysLinkMemUtils_virtToPhys (UInt32          remoteAddr,
                            UInt32 *        physAddr,
                            ProcMgr_ProcId  procId)
{
    /* FIX ME: Hack for Tiler */
    if (remoteAddr >= TILER_ADDRESS_START && remoteAddr <= TILER_ADDRESS_END) {
        *physAddr = remoteAddr;
        printf("Translated Address = 0x%x\n", remoteAddr);
        return PROCMGR_SUCCESS;
    }
    else {
        Osal_printf("NON-TILER ADDRESS TRANSLATIONS NOT SUPPORTED"
                    "remoteAddr = 0x%x\n",remoteAddr );
        return PROCMGR_E_FAIL;
    }
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
