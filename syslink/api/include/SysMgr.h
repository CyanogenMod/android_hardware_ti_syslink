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
/** ============================================================================
 *  @file   SysMgr.h
 *
 *  @brief      Defines for System manager.
 *
 *  ============================================================================
 */


#ifndef SYSMGR_H_0xF086
#define SYSMGR_H_0xF086

/* Utilities & Osal headers */
#include <Gate.h>

/* Module headers */
#include <MultiProc.h>
#include <GatePeterson.h>
#include <SharedRegion.h>
#include <ListMP.h>
#include <ListMPSharedMemory.h>
#include <MessageQ.h>
#include <MessageQTransportShm.h>
#include <Notify.h>
#include <NotifyDriverShm.h>
#include <NameServer.h>
#include <NameServerRemote.h>
#include <NameServerRemoteNotify.h>
#include <ProcMgr.h>
#if 0
#include <CoffLoader.h>
#endif
#include <Heap.h>
#include <HeapBuf.h>
#include <SysMemMgr.h>
#if 0
#include <ClientNotifyMgr.h>
#include <FrameQBufMgr.h>
#include <FrameQ.h>
#endif

#if defined (__cplusplus)
extern "C" {
#endif

/*!
 *  @def    SYSMGR_MODULEID
 *  @brief  Unique module ID.
 */
#define SYSMGR_MODULEID       (0xF086)


/* =============================================================================
 * Module Success and Failure codes
 * =============================================================================
 */
/*!
 *  @def    SYSMGR_STATUSCODEBASE
 *  @brief  Error code base for System manager.
 */
#define SYSMGR_STATUSCODEBASE  (SYSMGR_MODULEID << 12u)

/*!
 *  @def    SYSMGR_MAKE_FAILURE
 *  @brief  Macro to make error code.
 */
#define SYSMGR_MAKE_FAILURE(x)          ((Int)  (  0x80000000                  \
                                                     + (SYSMGR_STATUSCODEBASE  \
                                                     + (x))))

/*!
 *  @def    SYSMGR_MAKE_SUCCESS
 *  @brief  Macro to make success code.
 */
#define SYSMGR_MAKE_SUCCESS(x)         (SYSMGR_STATUSCODEBASE + (x))

/*!
 *  @def    SYSMGR_E_INVALIDARG
 *  @brief  Argument passed to a function is invalid.
 */
#define SYSMGR_E_INVALIDARG            SYSMGR_MAKE_FAILURE(1)

/*!
 *  @def    SYSMGR_E_MEMORY
 *  @brief  Memory allocation failed.
 */
#define SYSMGR_E_MEMORY                SYSMGR_MAKE_FAILURE(2)

/*!
 *  @def    SYSMGR_E_FAIL
 *  @brief  General failure.
 */
#define SYSMGR_E_FAIL                  SYSMGR_MAKE_FAILURE(3)

/*!
 *  @def    SYSMGR_E_INVALIDSTATE
 *  @brief  Module is in invalid state.
 */
#define SYSMGR_E_INVALIDSTATE          SYSMGR_MAKE_FAILURE(4)

/*!
 *  @def    SYSMGR_E_OSFAILURE
 *  @brief  Failure in OS call.
 */
#define SYSMGR_E_OSFAILURE             SYSMGR_MAKE_FAILURE(5)

/*!
 *  @def    SYSMGR_SUCCESS
 *  @brief  Operation successful.
 */
#define SYSMGR_SUCCESS                 SYSMGR_MAKE_SUCCESS(0)

/*!
 *  @def    SYSMGR_S_ALREADYSETUP
 *  @brief  Module is already initialized.
 */
#define SYSMGR_S_ALREADYSETUP          SYSMGR_MAKE_SUCCESS(1)

/*!
 *  @def    SYSMGR_CMD_SCALABILITY
 *  @brief  Command ID for scalability info.
 */
#define SYSMGR_CMD_SCALABILITY                (0x00000000)

/*!
 *  @def    SYSMGR_CMD_SHAREDREGION_ENTRY_BASE
 *  @brief  Base of command IDs for entries used by Shared region.
 */
#define SYSMGR_CMD_SHAREDREGION_ENTRY_START (0x00000001)
#define SYSMGR_CMD_SHAREDREGION_ENTRY_END   (0x00001000)


/* =============================================================================
 * Structures & Enums
 * =============================================================================
 */
/*!
 *  @brief  Structure defining config parameters for overall System.
 */
typedef struct SysMgr_Config {
    SysMemMgr_Config                sysMemMgrConfig;
    /*!< System memory manager config parameter */

    MultiProc_Config                multiProcConfig;
    /*!< Multiproc config parameter */

    GatePeterson_Config             gatePetersonConfig;
    /*!< Gatepeterson config parameter */

    SharedRegion_Config             sharedRegionConfig;
    /*!< SharedRegion config parameter */

    MessageQ_Config                 messageQConfig;
    /*!< MessageQ config parameter */

    Notify_Config                   notifyConfig;
    /*!< Notify config parameter */

    ProcMgr_Config                  procMgrConfig;
    /*!< Processor manager config parameter */

#if 0
    CoffLoader_Config               coffLoaderConfig;
    /*!< Coff loader config parameter */
#endif

    HeapBuf_Config                  heapBufConfig;
    /*!< Heap Buf config parameter */

    ListMPSharedMemory_Config       listMPSharedMemoryConfig;
    /*!< ListMPSharedMemory config parameter */

    MessageQTransportShm_Config     messageQTransportShmConfig;
    /*!< MessageQTransportShm config parameter */

    NotifyDriverShm_Config          notifyDriverShmConfig;
    /*!< NotifyDriverShm config parameter */

    NameServerRemoteNotify_Config   nameServerRemoteNotifyConfig;
    /*!< NameServerRemoteNotify config parameter */

#if 0
    ClientNotifyMgr_Config          clientNotifyMgrCfgParams;
    /*!< ClientNotifyMgr config parameter */

    FrameQBufMgr_Config             frameQBufMgrCfgParams;
    /*!< FrameQBufMgr config parameter */

    FrameQ_Config                   frameQCfgParams;
    /*!< FrameQ config parameter */
#endif
} SysMgr_Config;


/* =============================================================================
 * APIs
 * =============================================================================
 */
/* Function to initialize the parameter structure */
Void
SysMgr_getConfig (SysMgr_Config * config);

/* Function to initialize SysMgr module */
Int32
SysMgr_setup (const SysMgr_Config * config);

/* Function to Finalize SysMgr module */
Int32
SysMgr_destroy (void);

/* Function to SysMgr load callback */
Int32
SysMgr_loadCallback (ProcMgr_ProcId procId);

/* Function to SysMgr start callback */
Int32
SysMgr_startCallback (ProcMgr_ProcId procId);

/* Function to SysMgr stop callback */
Int32
SysMgr_stopCallback (ProcMgr_ProcId procId);

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* ifndef SYSMGR_H_0xF086 */
