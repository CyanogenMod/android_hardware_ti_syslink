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
 *  @file   SysMgr.c
 *
 *  @brief      User side implementation of System manager.
 *
 *  ============================================================================
 */


/* Standard headers */
#include <Std.h>

/* Utilities & OSAL headers */
#include <Gate.h>
#include <Memory.h>
#include <Trace.h>
#include <String.h>

/* Module headers */
#include <SysMemMgr.h>
#include <SysMgr.h>
#include <SysMgrDrv.h>
#include <SysMgrDrvDefs.h>
#include <UsrUtilsDrv.h>
#include <ProcMgr.h>


#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 * Structures & Enums
 * =============================================================================
 */
/*!
 *  @brief  SysMgr Module state object
 */
typedef struct SysMgr_ModuleObject_tag {
    UInt32              setupRefCount;
    /*!< Reference count for number of times setup/destroy were called in this
         process. */
    /* Boot load page of the slaves */
    Bool          sysMemMgrInitFlag;
    /*!< MultiProc Initialize flag */
    Bool          multiProcInitFlag;
    /*!< MultiProc Initialize flag */
    Bool          gatePetersonInitFlag;
    /*!< Gatepeterson Initialize flag */
    Bool          sharedRegionInitFlag;
    /*!< SHAREDREGION Initialize flag */
    Bool          listMpInitFlag;
    /*!< ListMP Initialize flag */
    Bool          messageQInitFlag;
    /*!< MessageQ Initialize flag */
    Bool          notifyInitFlag;
    /*!< Notify Initialize flag */
    Bool          procMgrInitFlag;
    /*!< Processor manager Initialize flag */
    Bool          coffLoaderInitFlag;
    /*!< Coff loader Initialize flag */
    Bool          heapBufInitFlag;
    /*!< HeapBuf Initialize flag */
    Bool          nameServerInitFlag;
    /*!< nameServerRemoteNotify Initialize flag */
    Bool          listMPSharedMemoryInitFlag;
    /*!< LISTMPSHAREDMEMORY Initialize flag */
    Bool          messageQTransportShmInitFlag;
    /*!< messageQTransportShm Initialize flag */
    Bool          notifyDriverShmInitFlag;
    /*!< notifyDriverShm Initialize flag */
    Bool          nameServerRemoteNotifyInitFlag;
    /*!< nameServerRemoteNotify Initialize flag */
    Bool          clientNotifyMgrInitFlag;
    /*!< clientNotifierMgr Initialize flag */
    Bool          frameQBufMgrInitFlag;
    /*!< frameQBufMgr Initialize flag */
    Bool          frameQInitFlag;
    /*!< frameQ Initialize flag */
} SysMgr_ModuleObject;


/* =============================================================================
 *  Globals
 * =============================================================================
 */
/*!
 *  @var    SysMgr_state
 *
 *  @brief  SysMgr state object variable
 */
#if !defined(SYSLINK_BUILD_DEBUG)
static
#endif /* if !defined(SYSLINK_BUILD_DEBUG) */
SysMgr_ModuleObject SysMgr_state =
{
    .setupRefCount = 0
};

/* =============================================================================
 * APIS
 * =============================================================================
 */
/*!
 *  @brief      Function to get tyhe default values for confiurations.
 *
 *  @param      config   Configuration values.
 */
Void
SysMgr_getConfig (SysMgr_Config * config)
{
    Int32  status = SYSMGR_SUCCESS;

    GT_1trace (curTrace, GT_ENTER, "SysMgr_getConfig", config);

    GT_assert (curTrace, (config != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (config == NULL) {
        status = SYSMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "SysMgr_getConfig",
                             status,
                             "Argument of type (SysMgr_getConfig *) passed "
                             "is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

        /* Get the SysMemMgr default config */
        SysMemMgr_getConfig (&config->sysMemMgrConfig);

        /* Get the MultiProc default config */
        MultiProc_getConfig (&config->multiProcConfig);

        /* Get the GatePeterson default config */
        GatePeterson_getConfig (&config->gatePetersonConfig);

        /* Get the SharedRegion default config */
        SharedRegion_getConfig (&config->sharedRegionConfig);

        /* Get the MESSAGEQ default config */
        MessageQ_getConfig (&config->messageQConfig);

        /* Get the NOTIFY default config */
        Notify_getConfig (&config->notifyConfig);

        /* Get the PROCMGR default config */
        ProcMgr_getConfig (&config->procMgrConfig);

#if 0
        /* Get the CoffLoader default config */
        CoffLoader_getConfig (&config->coffLoaderConfig);
#endif

        /* Get the HeapBuf default config */
        HeapBuf_getConfig (&config->heapBufConfig);

        /* Get the LISTMPSHAREDMEMORY default config */
        ListMPSharedMemory_getConfig (&config->listMPSharedMemoryConfig);

        /* Get the MESSAGEQTRANSPORTSHM default config */
        MessageQTransportShm_getConfig (&config->messageQTransportShmConfig);

        /* Get the NOTIFYSHMDRIVER default config */
        NotifyDriverShm_getConfig (&config->notifyDriverShmConfig);

        /* Get the NAMESERVERREMOTENOTIFY default config */
        NameServerRemoteNotify_getConfig(&config->nameServerRemoteNotifyConfig);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "SysMgr_getConfig");
}


/*!
 *  @brief      Function to setup the System.
 *
 *  @param      cfg  Configuration values
 *
 *  @sa         SysMgr_destroy
 */
Int32
SysMgr_setup (const SysMgr_Config * cfg)
{
    Int32             status = SYSMGR_SUCCESS;
    SysMgr_Config *   config = NULL;
    SysMgr_Config     tConfig;
    SysMgrDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "SysMgr_setup", cfg);

    if (cfg == NULL) {
        SysMgr_getConfig (&tConfig);
        config = &tConfig;
    }
    else {
        config = (SysMgr_Config *) cfg;
    }

    /* TBD: Protect from multiple threads. */
    SysMgr_state.setupRefCount++;
    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */
    if (SysMgr_state.setupRefCount > 1) {
        /*! @retval SYSMGR_S_ALREADYSETUP Success: SysMgr module has been
                                           already setup in this process */
        status = SYSMGR_S_ALREADYSETUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "SysMgr module has been already setup in this process.\n"
                   "    RefCount: [%d]\n",
                   SysMgr_state.setupRefCount);
    }
    else {
        /* Open the driver handle. */
        status = SysMgrDrv_open ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SysMgr_setup",
                                 status,
                                 "Failed to open driver handle!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            cmdArgs.args.setup.config = (SysMgr_Config *) config;
            status = SysMgrDrv_ioctl (CMD_SYSMGR_SETUP, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "SysMgr_setup",
                                     status,
                                     "API (through IOCTL) failed on kernel-side!");
            }
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

    UsrUtilsDrv_setup ();

    if (status >= 0) {
        status = SysMemMgr_setup (&(config->sysMemMgrConfig));
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SysMgr_setup",
                                 status,
                                 "SysMemMgr_setup failed!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            SysMgr_state.sysMemMgrInitFlag = TRUE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

/* Initialize NAMESERVER */
    if (status >= 0) {
        status = NameServer_setup ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SysMgr_setup",
                                 status,
                                 "NameServer_setup failed!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            SysMgr_state.nameServerInitFlag = TRUE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

    if (status >= 0) {
        status = MultiProc_setup (&(config->multiProcConfig));
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SysMgr_setup",
                                 status,
                                 "MultiProc_setup failed!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            SysMgr_state.multiProcInitFlag = TRUE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

/* Initialize PROCMGR */
    if (status >= 0) {
        status = ProcMgr_setup (&(config->procMgrConfig));
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SysMgr_setup",
                                 status,
                                 "ProcMgr_setup failed!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            SysMgr_state.procMgrInitFlag = TRUE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

/* Initialize SharedRegion */
    if (status >= 0) {
        status = SharedRegion_setup (&config->sharedRegionConfig);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SysMgr_setup",
                                 status,
                                 "SharedRegion_setup failed!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            SysMgr_state.sharedRegionInitFlag = TRUE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

/* Initialize NOTIFY */
    if (status >= 0) {
        status = Notify_setup (&config->notifyConfig);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SysMgr_setup",
                                 status,
                                 "Notify_setup failed!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            SysMgr_state.notifyInitFlag = TRUE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

/* Initialize GATEPETERSON */
    if (status >= 0) {
        status = GatePeterson_setup (&config->gatePetersonConfig);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SysMgr_setup",
                                 status,
                                 "GatePeterson_setup failed!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            SysMgr_state.gatePetersonInitFlag = TRUE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

/* Intialize MESSAGEQ */
    if (status >= 0) {
        status = MessageQ_setup (&config->messageQConfig);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SysMgr_setup",
                                 status,
                                 "MessageQ_setup failed!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            SysMgr_state.messageQInitFlag = TRUE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

#if 0
/* Intialize coff loader */
    if (status >= 0) {
        status = CoffLoader_setup (&config->coffLoaderConfig);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SysMgr_setup",
                                 status,
                                 "CoffLoader_setup failed!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            SysMgr_state.coffLoaderInitFlag = TRUE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }
#endif

/* Intialize heap buf */
    if (status >= 0) {
        status = HeapBuf_setup (&config->heapBufConfig);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SysMgr_setup",
                                 status,
                                 "HeapBuf_setup failed!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            SysMgr_state.heapBufInitFlag = TRUE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

/* Get the LISTMPSHAREDMEMORY default config */
    if (status >= 0) {
        status = ListMPSharedMemory_setup (&config->listMPSharedMemoryConfig);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SysMgr_setup",
                                 status,
                                 "ListMpSharedMemory_setup failed!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            SysMgr_state.listMPSharedMemoryInitFlag = TRUE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }


/* Get the MESSAGEQTRANSPORTSHM default config */
    if (status >= 0) {
        status = MessageQTransportShm_setup (
                                           &config->messageQTransportShmConfig);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SysMgr_setup",
                                 status,
                                 "MessageQTransportShm_setup failed!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            SysMgr_state.messageQTransportShmInitFlag = TRUE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

/* Get the NOTIFYSHMDRIVER default config */
    if (status >= 0) {
        status = NotifyDriverShm_setup (&config->notifyDriverShmConfig);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SysMgr_setup",
                                 status,
                                 "NotifyDriverShm_setup failed!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            SysMgr_state.notifyDriverShmInitFlag = TRUE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

/* Get the NAMESERVERREMOTENOTIFY default config */
    if (status >= 0) {
        status = NameServerRemoteNotify_setup (
                                         &config->nameServerRemoteNotifyConfig);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SysMgr_setup",
                                 status,
                                 "NameServerRemoteNotify_setup failed!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            SysMgr_state.nameServerRemoteNotifyInitFlag = TRUE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

    GT_1trace (curTrace, GT_LEAVE, "SysMgr_setup", status);

    /*! @retval SYSMGR_SUCCESS Operation successful */
    return status;
}


/*!
 *  @brief      Function to destroy the System.
 *
 *  @sa         SysMgr_setup
 */
Int32
SysMgr_destroy (void)
{
    Int32             status = SYSMGR_SUCCESS;
    SysMgrDrv_CmdArgs cmdArgs;

    GT_0trace (curTrace, GT_ENTER, "SysMgr_destroy");

    /* TBD: Protect from multiple threads. */
    SysMgr_state.setupRefCount--;
    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */
    if (SysMgr_state.setupRefCount > 1) {
        /*! @retval SYSMGR_S_ALREADYSETUP Success: ProcMgr module has been
                                           already setup in this process */
        status = SYSMGR_S_ALREADYSETUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "SysMgr module has been already setup in this process.\n"
                   "    RefCount: [%d]\n",
                   SysMgr_state.setupRefCount);
    }
    else {

        /* Finalize NAMESERVERREMOTENOTIFY */
        if (SysMgr_state.nameServerRemoteNotifyInitFlag == TRUE) {
            status = NameServerRemoteNotify_destroy ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "SysMgr_destroy",
                                     status,
                                     "NameServerRemoteNotify_destroy failed!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                SysMgr_state.nameServerRemoteNotifyInitFlag = FALSE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        }

        /* Finalize MESSAGEQTRANSPORTSHM */
        if (SysMgr_state.messageQTransportShmInitFlag == TRUE) {
            status = MessageQTransportShm_destroy ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "SysMgr_setup",
                                     status,
                                     "MessageQTransportShm_destroy failed!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                SysMgr_state.messageQTransportShmInitFlag = FALSE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        }

        /* Finalize LISTMPSHAREDMEMORY */
        if (SysMgr_state.listMPSharedMemoryInitFlag == TRUE) {
            status = ListMPSharedMemory_destroy ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "SysMgr_setup",
                                     status,
                                     "ListMpSharedMemory_destroy failed!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                SysMgr_state.listMPSharedMemoryInitFlag = FALSE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        }

        /* Finalize heap buf */
        if (SysMgr_state.heapBufInitFlag == TRUE) {
            status = HeapBuf_destroy ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "SysMgr_destroy",
                                     status,
                                     "HeapBuf_destroy failed!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                SysMgr_state.heapBufInitFlag = FALSE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        }

#if 0
        /* Finalize coff loader */
        if (SysMgr_state.coffLoaderInitFlag == TRUE) {
            status = CoffLoader_destroy ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "SysMgr_destroy",
                                     status,
                                     "CoffLoader_destroy failed!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                SysMgr_state.coffLoaderInitFlag = FALSE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        }
#endif

        /* Finalize MESSAGEQ */
        if (SysMgr_state.messageQInitFlag == TRUE) {
            status = MessageQ_destroy ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "SysMgr_destroy",
                                     status,
                                     "MessageQ_destroy failed!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                SysMgr_state.messageQInitFlag = FALSE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        }

        /* Finalize NOTIFYSHMDRIVER */
        if (SysMgr_state.notifyDriverShmInitFlag == TRUE) {
            status = NotifyDriverShm_destroy ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "SysMgr_setup",
                                     status,
                                     "NotifyDriverShm_destroy failed!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                SysMgr_state.notifyDriverShmInitFlag = FALSE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        }

        /* Finalize SYSMGR */
        if (SysMgr_state.gatePetersonInitFlag == TRUE) {
            status = GatePeterson_destroy ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "SysMgr_destroy",
                                     status,
                                     "GatePeterson_destroy failed!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                SysMgr_state.gatePetersonInitFlag = FALSE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        }

        /* Finalize NOTIFY */
        if (SysMgr_state.notifyInitFlag == TRUE) {
            status = Notify_destroy ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "SysMgr_destroy",
                                     status,
                                     "Notify_destroy failed!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                SysMgr_state.notifyInitFlag = FALSE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        }

        /* Finalize SharedRegion */
        if (SysMgr_state.sharedRegionInitFlag == TRUE) {
            status = SharedRegion_destroy ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "SysMgr_destroy",
                                     status,
                                     "SharedRegion_destroy failed!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                SysMgr_state.sharedRegionInitFlag = FALSE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        }

        /* Finalize PROCMGR */
        if (SysMgr_state.procMgrInitFlag == TRUE) {
            status = ProcMgr_destroy ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "SysMgr_destroy",
                                     status,
                                     "ProcMgr_destroy failed!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                SysMgr_state.procMgrInitFlag = FALSE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        }

        /* Finalize NAMESERVER */
        if (SysMgr_state.nameServerInitFlag == TRUE) {
            status = NameServer_destroy ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "SysMgr_destroy",
                                     status,
                                     "NameServer_destroy failed!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                SysMgr_state.nameServerInitFlag = FALSE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        }

        /* Finalize PROCMGR */
        if (SysMgr_state.sysMemMgrInitFlag == TRUE) {
            status = SysMemMgr_destroy ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "SysMgr_destroy",
                                     status,
                                     "SysMemMgr_destroy failed!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                SysMgr_state.sysMemMgrInitFlag = FALSE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        }

        /* Destroy the multiProc */
        MultiProc_destroy ();

        status = SysMgrDrv_ioctl (CMD_SYSMGR_DESTROY, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SysMgr_destroy",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

        UsrUtilsDrv_destroy ();

        /* Close the driver handle. */
        SysMgrDrv_close ();


    } /* Reference count check */

    GT_1trace (curTrace, GT_LEAVE, "SysMgr_destroy", status);

    /*! @retval SYSMGR_SUCCESS Operation successful */
    return status;
}


/*!
 *  @brief      Function to invoke load callback.
 *
 *  @param      procId  Processor Id
 *
 *  @sa         SysMgr_startCallback, SysMgr_stopCallback
 */
Int32
SysMgr_loadCallback (ProcMgr_ProcId procId)
{
    Int32             status = SYSMGR_SUCCESS;
    SysMgrDrv_CmdArgs cmdArgs;

    GT_0trace (curTrace, GT_ENTER, "SysMgr_loadCallback");

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (procId >= PROC_END) {
        GT_setFailureReason (curTrace,
                                 GT_7CLASS,
                                 "SysMgr_loadCallback",
                                 SYSMGR_E_INVALIDARG,
                                 "SysMgr_loadCallback INVALID PROC_ID");
        return SYSMGR_E_INVALIDARG;
    }
#endif

    cmdArgs.args.procId = procId;
    status = SysMgrDrv_ioctl (CMD_SYSMGR_LOADCALLBACK, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (status < 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "SysMgr_loadCallback",
                             status,
                             "API (through IOCTL) failed on kernel-side!");
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "SysMgr_loadCallback", status);

    return status;
}


/*!
 *  @brief      Function to start callback.
 *
 *  @param      procId  Processor Id
 *
 *  @sa         SysMgr_loadCallback, SysMgr_stopCallback
 */
Int32
SysMgr_startCallback (ProcMgr_ProcId procId)
{
    Int32             status = SYSMGR_SUCCESS;
    SysMgrDrv_CmdArgs cmdArgs;

    GT_0trace (curTrace, GT_ENTER, "SysMgr_startCallback");

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (procId >= PROC_END) {
        GT_setFailureReason (curTrace,
                             GT_7CLASS,
                             "SysMgr_startCallback",
                             SYSMGR_E_INVALIDARG,
                             "SysMgr_startCallback INVALID PROC_ID");
        return SYSMGR_E_INVALIDARG;
    }
#endif

    cmdArgs.args.procId = procId;
    status = SysMgrDrv_ioctl (CMD_SYSMGR_STARTCALLBACK, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (status < 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "SysMgr_startCallback",
                             status,
                             "API (through IOCTL) failed on kernel-side!");
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "SysMgr_startCallback", status);

    return status;
}

/*!
 *  @brief      Function to stop callback.
 *
 *  @param      procId  Processor Id
 *
 *  @sa         SysMgr_loadCallback, SysMgr_startCallback
 */
Int32
SysMgr_stopCallback (ProcMgr_ProcId procId)
{
    Int32             status = SYSMGR_SUCCESS;
    SysMgrDrv_CmdArgs cmdArgs;

    GT_0trace (curTrace, GT_ENTER, "SysMgr_stopCallback");

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (procId >= PROC_END) {
        GT_setFailureReason (curTrace,
                                 GT_7CLASS,
                                 "SysMgr_stopCallback",
                                 SYSMGR_E_INVALIDARG,
                                 "SysMgr_stopCallback INVALID PROC_ID");
        return SYSMGR_E_INVALIDARG;
    }
#endif

    cmdArgs.args.procId = procId;
    status = SysMgrDrv_ioctl (CMD_SYSMGR_STOPCALLBACK, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (status < 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "SysMgr_stopCallback",
                             status,
                             "API (through IOCTL) failed on kernel-side!");
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "SysMgr_stopCallback", status);

    return status;
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
