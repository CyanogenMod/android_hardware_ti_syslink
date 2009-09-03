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
 *  @file   SysMemMgr.c
 *
 *  @brief      Manager for the system memory. System level memory are allocated
 *              through this module.
 *
 *  ============================================================================
 */


/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <GateMutex.h>
#include <Memory.h>
#include <Trace.h>
#include <Bitops.h>
#include <Atomic_Ops.h>

/* Module level headers */
#include <SysMemMgr.h>
#include <SysMemMgrDrv.h>
#include <SysMemMgrDrvDefs.h>


#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 * Macros
 * =============================================================================
 */
/*! @brief Event reserved for System memory manager */
#define SYSMEMMGR_EVENTNO       12u

/* Macro to make a correct module magic number with refCount */
#define SYSMEMMGR_MAKE_MAGICSTAMP(x) ((SYSMEMMGR_MODULEID << 12u) | (x))

/* Macro to maximum number of map information to be store */
#define SYSMEMMGR_MAX_MAPINFO   100u


/* =============================================================================
 * Structs & Enums
 * =============================================================================
 */
/*!
 *  @brief  Structure defining state object of system memory manager.
 */
typedef struct SysMemMgr_MapInfo {
    UInt32 size;
    /*!< Size of the chunk */
    UInt32 virtAddress;
    /* User virtual address */
    UInt32 kvirtAddress;
    /* User virtual address */
    UInt32 physAddress;
    /*!< Physical address */
} SysMemMgr_MapInfo;

/*!
 *  @brief  Structure defining state object of system memory manager.
 */
typedef struct SysMemMgr_ModuleObject {
    Atomic            refCount;
    /*!< Reference count */
    SysMemMgr_MapInfo mapTable[SYSMEMMGR_MAX_MAPINFO];
    /* Address of allocated chunks */
    Gate_Handle       gateHandle;
    /*!< Pointer to lock */
} SysMemMgr_ModuleObject;


/* =============================================================================
 * Globals
 * =============================================================================
 */
/*!
 *  @brief  Object containing state of the system memory manager.
 */
#if !defined(SYSLINK_BUILD_DEBUG)
static
#endif /* if !defined(SYSLINK_BUILD_DEBUG) */
SysMemMgr_ModuleObject SysMemMgr_state;


/* =============================================================================
 * APIS
 * =============================================================================
 */
/*!
 *  @brief      Function to setup the SysMemMgr module.
 *
 *  @param      config   Configuration values.
 */
Void
SysMemMgr_getConfig (SysMemMgr_Config * config)
{
    Int32                status = SYSMEMMGR_SUCCESS;
    SysMemMgrDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "SysMemMgr_getConfig", config);

    GT_assert (curTrace, (config != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (config == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "SysMemMgr_getConfig",
                             SYSMEMMGR_E_INVALIDARG,
                             "Argument of type (SysMemMgr_Config *) passed "
                             "is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Temporarily open the handle to get the configuration. */
        status = SysMemMgrDrv_open ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SysMemMgr_getConfig",
                                 status,
                                 "Failed to open driver handle!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            cmdArgs.args.getConfig.config = config;
            status = SysMemMgrDrv_ioctl (CMD_SYSMEMMGR_GETCONFIG,
                                            &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                  GT_4CLASS,
                                  "SysMemMgr_getConfig",
                                  status,
                                  "API (through IOCTL) failed on kernel-side!");

            }
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Close the driver handle. */
        SysMemMgrDrv_close ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_ENTER, "SysMemMgr_getConfig");
}


/*!
 *  @brief      Configures and initialize the system memory manager.
 *
 *  @param      params  configuration parameters.
 *
 *  @sa         SysMemMgr_destroy
 */
Int
SysMemMgr_setup (SysMemMgr_Config * config)
{
    Int32                status = SYSMEMMGR_SUCCESS;
    SysMemMgrDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "SysMemMgr_setup", config);

    GT_assert (curTrace, (NULL != config));

    /* This sets the refCount variable is not initialized, upper 16 bits is
     * written with module Id to ensure correctness of refCount variable.
     */
    Atomic_cmpmask_and_set (&SysMemMgr_state.refCount,
                            SYSMEMMGR_MAKE_MAGICSTAMP(0),
                            SYSMEMMGR_MAKE_MAGICSTAMP(0));

    if (   Atomic_inc_return (&SysMemMgr_state.refCount)
        != SYSMEMMGR_MAKE_MAGICSTAMP(1u)) {
        status = SYSMEMMGR_S_ALREADYSETUP;
        GT_0trace (curTrace,
                   GT_2CLASS,
                   "SysMemMgr Module already initialized!");
    }
    else {
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (NULL == config) {
            /*! @retval SYSMEMMGR_E_INVALIDARG Config parameters are not
             * provided */
            status = SYSMEMMGR_E_INVALIDARG;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SysMemMgr_setup",
                                 status,
                                 "Argument of type (SysMemMgr_Config *) passed"
                                 " is null!");
            Atomic_set (&SysMemMgr_state.refCount,
                        SYSMEMMGR_MAKE_MAGICSTAMP(0));
        }
        else if (   (config->staticVirtBaseAddr == (UInt32) NULL)
                 && (config->staticMemSize != 0)) {
            /*! @retval SYSMEMMGR_E_INVALIDARG Virtual Base address of static
             * memory region is NULL */
            status = SYSMEMMGR_E_INVALIDARG;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SysMemMgr_setup",
                                 status,
                                 "Virtual Base address of static memory region "
                                 "is NULL!");
            Atomic_set (&SysMemMgr_state.refCount,
                        SYSMEMMGR_MAKE_MAGICSTAMP(0));
        }
        else if (   (config->staticPhysBaseAddr == (UInt32) NULL)
                 && (config->staticMemSize != 0)) {
            /*! @retval SYSMEMMGR_E_INVALIDARG Physical Base address of static
             * memory region is NULL */
            status = SYSMEMMGR_E_INVALIDARG;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SysMemMgr_setup",
                                 status,
                                 "Physical Base address of static memory region"
                                 " is NULL!");
            Atomic_set (&SysMemMgr_state.refCount,
                        SYSMEMMGR_MAKE_MAGICSTAMP(0));
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            /* Open the driver handle. */
            status = SysMemMgrDrv_open ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "SysMemMgr_setup",
                                     status,
                                     "Failed to open driver handle!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                cmdArgs.args.setup.config = (SysMemMgr_Config *) config;
                status = SysMemMgrDrv_ioctl (CMD_SYSMEMMGR_SETUP, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
                if (status < 0) {
                    Atomic_set (&SysMemMgr_state.refCount,
                                SYSMEMMGR_MAKE_MAGICSTAMP(0));
                    GT_setFailureReason (curTrace,
                                         GT_4CLASS,
                                         "SysMemMgr_setup",
                                         status,
                                         "API (through IOCTL) failed on "
                                         "kernel-side!");
                }
                else {
#if !defined(SYSLINK_BUILD_OPTIMIZE)
                    Memory_set (&SysMemMgr_state.mapTable,
                                0,
                                ( sizeof (SysMemMgr_MapInfo)
                                 * SYSMEMMGR_MAX_MAPINFO));
                    SysMemMgr_state.gateHandle = GateMutex_create ();
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                    if (SysMemMgr_state.gateHandle == NULL) {
                        /*! @retval SYSMEMMGR_E_FAIL Gate creation failed */
                        status = SYSMEMMGR_E_FAIL;
                        GT_setFailureReason (curTrace,
                                             GT_4CLASS,
                                             "SysMemMgr_setup",
                                             status,
                                             "Gate creation failed!");
                        SysMemMgr_destroy ();
                        Atomic_set (&SysMemMgr_state.refCount,
                                    SYSMEMMGR_MAKE_MAGICSTAMP(0));
                    }
                }
            }
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

    GT_1trace (curTrace, GT_LEAVE, "SysMemMgr_setup", status);

    /*! @retval SYSMEMMGR_SUCCESS Operation successful */
    return status;
}


/*!
 *  @brief      Destroys the system memory manager.
 *
 *  @sa         SysMemMgr_setup
 */
Int
SysMemMgr_destroy (void)
{
    Int                  status = SYSMEMMGR_SUCCESS;
    SysMemMgrDrv_CmdArgs args;

    GT_0trace (curTrace, GT_ENTER, "SysMemMgr_destroy");

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (   Atomic_cmpmask_and_lt (&(SysMemMgr_state.refCount),
                                  SYSMEMMGR_MAKE_MAGICSTAMP(0),
                                  SYSMEMMGR_MAKE_MAGICSTAMP(1))
        == TRUE) {
        /*! @retval SYSMEMMGR_E_INVALIDSTATE Module was not initialized */
        status = SYSMEMMGR_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "SysMemMgr_destroy",
                             status,
                             "Module was not initialized!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        if (   Atomic_dec_return (&SysMemMgr_state.refCount)
            == SYSMEMMGR_MAKE_MAGICSTAMP(0)) {
            Memory_set (&SysMemMgr_state.mapTable,
                        0,
                        sizeof (SysMemMgr_MapInfo) * SYSMEMMGR_MAX_MAPINFO);
            status = SysMemMgrDrv_ioctl (CMD_SYSMEMMGR_DESTROY, &args);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "SysMemMgr_setup",
                                     status,
                                     "API (through IOCTL) failed on "
                                     "kernel-side!");
            }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            GateMutex_delete (&SysMemMgr_state.gateHandle);

            /* Close the driver handle. */
            SysMemMgrDrv_close ();
        }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "SysMemMgr_destroy", status);

    /*! @retval SYSMEMMGR_SUCCESS Operation successful */
    return status;
}


/*!
 *  @brief      Allocates a memory block.
 *
 *  @param      size    request block size.
 *  @param      flag
 *
 *  @sa         SysMemMgr_free
 */
Ptr
SysMemMgr_alloc (UInt32 size, SysMemMgr_AllocFlag flag)
{
    Int                  status = SYSMEMMGR_SUCCESS;
    Ptr                  retPtr = NULL;
    SysMemMgrDrv_CmdArgs cmdArgs;
    UInt32               key;
    UInt32               i;

    GT_2trace (curTrace, GT_ENTER, "SysMemMgr_alloc", size, flag);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (   Atomic_cmpmask_and_lt (&(SysMemMgr_state.refCount),
                                  SYSMEMMGR_MAKE_MAGICSTAMP(0),
                                  SYSMEMMGR_MAKE_MAGICSTAMP(1))
        == TRUE) {
        /*! @retval NULL Module was not initialized */
        status = SYSMEMMGR_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "SysMemMgr_alloc",
                             status,
                             "Module was not initialized!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.alloc.size  = size;
        cmdArgs.args.alloc.flags = flag;
        status = SysMemMgrDrv_ioctl (CMD_SYSMEMMGR_ALLOC, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SysMemMgr_alloc",
                                 status,
                                 "API (through IOCTL) failed on "
                                 "kernel-side!");
        }
        else {
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            key = Gate_enter (SysMemMgr_state.gateHandle);
            for (i = 0; i < SYSMEMMGR_MAX_MAPINFO; i++) {
                if (SysMemMgr_state.mapTable[i].size == 0) {
                    break;
                }
            }

#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (i != SYSMEMMGR_MAX_MAPINFO) {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                SysMemMgr_state.mapTable[i].size = size;
                SysMemMgr_state.mapTable[i].virtAddress =
                                                (UInt32) cmdArgs.args.alloc.buf;
                SysMemMgr_state.mapTable[i].kvirtAddress =
                                               (UInt32) cmdArgs.args.alloc.kbuf;
                SysMemMgr_state.mapTable[i].physAddress =
                                               (UInt32) cmdArgs.args.alloc.phys;
                retPtr = cmdArgs.args.alloc.buf;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            }
            else {
                /*! @retval NULL Unable to get a free mapInfo slot */
                status = SYSMEMMGR_E_FAIL;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "SysMemMgr_alloc",
                                     status,
                                     "Unable to get a free mapInfo slot!");
                SysMemMgr_free (cmdArgs.args.alloc.buf, size, flag);
            }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            Gate_leave (SysMemMgr_state.gateHandle, key);
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_assert (curTrace, (retPtr != NULL));

    GT_1trace (curTrace, GT_LEAVE, "SysMemMgr_alloc", retPtr);

    /*! @retval Non-null Operation successful */
    return retPtr;
}


/*!
 *  @brief      De-allocates a previous allocated memory block.
 *
 *  @param      blk     Pointer to the block.
 *  @param      size    allocated block size.
 *  @param      flag    Flag saying type of memory block.
 *
 *  @sa         SysMemMgr_alloc
 */
Int
SysMemMgr_free (Ptr blk, UInt32 size, SysMemMgr_AllocFlag flag)
{
    Int                  status = SYSMEMMGR_SUCCESS;
    SysMemMgrDrv_CmdArgs cmdArgs;
    UInt32               key;
    UInt32               i;

    GT_3trace (curTrace, GT_ENTER, "SysMemMgr_free", blk, size, flag);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (   Atomic_cmpmask_and_lt (&(SysMemMgr_state.refCount),
                                  SYSMEMMGR_MAKE_MAGICSTAMP(0),
                                  SYSMEMMGR_MAKE_MAGICSTAMP(1))
        == TRUE) {
        /*! @retval SYSMEMMGR_E_INVALIDSTATE Module was not initialized */
        status = SYSMEMMGR_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "SysMemMgr_free",
                             status,
                             "Module was not initialized!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.free.buf   = blk;
        cmdArgs.args.free.size  = size;
        cmdArgs.args.free.flags = flag;
        status = SysMemMgrDrv_ioctl (CMD_SYSMEMMGR_FREE, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SysMemMgr_free",
                                 status,
                                 "API (through IOCTL) failed on "
                                 "kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            key = Gate_enter (SysMemMgr_state.gateHandle);
            for (i = 0; i < SYSMEMMGR_MAX_MAPINFO; i++) {
                if (SysMemMgr_state.mapTable[i].virtAddress == (UInt32) blk) {
                    SysMemMgr_state.mapTable[i].virtAddress = 0;
                    SysMemMgr_state.mapTable[i].size = 0;
                    break;
                }
            }
            Gate_leave (SysMemMgr_state.gateHandle, key);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "SysMemMgr_free", status);

    /*! @retval SYSMEMMGR_SUCCESS Operation successful */
    return status;
}


/*!
 *  @brief      Translates an address among different address space.
 *
 *  @param      srcAddr Given source address.
 *  @param      flag    From address space to which address space flag.
 *
 *  @sa         none.
 */
Ptr
SysMemMgr_translate (Ptr srcAddr, SysMemMgr_XltFlag flags)
{
    Int                     status = SYSMEMMGR_SUCCESS;
    Ptr                     retPtr = NULL;
    UInt32                  frmAddr = 0;
    UInt32                  toAddr  = 0;
    SysMemMgrDrv_CmdArgs    cmdArgs;
    UInt32                  key;
    UInt32                  i;

    GT_2trace (curTrace, GT_ENTER, "SysMemMgr_translate", srcAddr, flags);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (   Atomic_cmpmask_and_lt (&(SysMemMgr_state.refCount),
                                  SYSMEMMGR_MAKE_MAGICSTAMP(0),
                                  SYSMEMMGR_MAKE_MAGICSTAMP(1))
        == TRUE) {
        /*! @retval NULL Module was not initialized */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "SysMemMgr_translate",
                             SYSMEMMGR_E_INVALIDSTATE,
                             "Module was not initialized!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        if (   (flags == SysMemMgr_XltFlag_Uvirt2Phys)
            || (flags == SysMemMgr_XltFlag_Phys2Uvirt)) {
            key = Gate_enter (SysMemMgr_state.gateHandle);
            for (i = 0; i < SYSMEMMGR_MAX_MAPINFO; i++) {
                if (SysMemMgr_state.mapTable[i].size != 0) {
                    if (flags == SysMemMgr_XltFlag_Uvirt2Phys) {
                        frmAddr = SysMemMgr_state.mapTable[i].virtAddress;
                        toAddr  = SysMemMgr_state.mapTable[i].physAddress;
                    }
                    else if (flags == SysMemMgr_XltFlag_Phys2Uvirt) {
                        toAddr  = SysMemMgr_state.mapTable[i].virtAddress;
                        frmAddr = SysMemMgr_state.mapTable[i].physAddress;
                    }
                    else if (flags == SysMemMgr_XltFlag_Phys2Kvirt) {
                        toAddr  = SysMemMgr_state.mapTable[i].kvirtAddress;
                        frmAddr = SysMemMgr_state.mapTable[i].physAddress;
                    }
                    else if (flags == SysMemMgr_XltFlag_Kvirt2Phys) {
                        frmAddr = SysMemMgr_state.mapTable[i].kvirtAddress;
                        toAddr  = SysMemMgr_state.mapTable[i].physAddress;
                    }
                    else if (flags == SysMemMgr_XltFlag_Kvirt2Uvirt) {
                        toAddr  = SysMemMgr_state.mapTable[i].virtAddress;
                        frmAddr = SysMemMgr_state.mapTable[i].kvirtAddress;
                    }
                    else if (flags == SysMemMgr_XltFlag_Uvirt2Kvirt) {
                        toAddr  = SysMemMgr_state.mapTable[i].kvirtAddress;
                        frmAddr = SysMemMgr_state.mapTable[i].virtAddress;
                    }
                    if (   (((UInt32) srcAddr) >= frmAddr)
                        && (  ((UInt32) srcAddr)
                            < (frmAddr + SysMemMgr_state.mapTable[i].size))) {
                        retPtr = (Ptr) (toAddr + ((UInt32)srcAddr - frmAddr));
                        break;
                    }
                }
            }
            Gate_leave (SysMemMgr_state.gateHandle, key);
        }
        else {
            cmdArgs.args.translate.buf   = srcAddr;
            cmdArgs.args.translate.flags = flags;
            status = SysMemMgrDrv_ioctl (CMD_SYSMEMMGR_TRANSLATE, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "SysMemMgr_free",
                                     status,
                                     "API (through IOCTL) failed on "
                                     "kernel-side!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                retPtr = cmdArgs.args.translate.retPtr;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "SysMemMgr_translate", retPtr);

    /*! @retval Non-null Operation successful */
    return retPtr;
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
