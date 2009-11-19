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
 *  @file   SharedRegion.c
 *
 *  @brief      Shared Region Manager
 *
 *              The SharedRegion module is designed to be used in a
 *              multi-processor environment where there are memory regions that
 *              are shared and accessed across different processors. This module
 *              creates a shared memory region lookup table. This lookup table
 *              contains the processor's view for every shared region in the
 *              system. Each processor must have its own lookup table. Each
 *              processor's view of a particular shared memory region can be
 *              determined by the same table index across all lookup tables.
 *              Each table entry is a base and length pair. During runtime,
 *              this table along with the shared region pointer is used to do a
 *              quick address translation.<br>
 *              The shared region pointer (#SharedRegion_SRPtr) is a 32-bit
 *              portable pointer composed of an index and offset. The most
 *              significant bits of a #SharedRegion_SRPtr are used for the index.
 *              The index corresponds to the index of the entry in the lookup
 *              table. The offset is the offset from the base of the shared
 *              memory region. The maximum number of table entries in the lookup
 *              table will determine the number of bits to be used for the index.
 *              An increase in the index means the range of the offset would
 *              decrease. Here is sample code for getting a #SharedRegion_SRPtr
 *              and then getting a the real address pointer back.
 *              @Example
 *              @code
 *                  SharedRegion_SRPtr srptr;
 *                  UInt  index;
 *
 *                  // to get the index of the address
 *                  index = SharedRegion_getIndex(addr);
 *
 *                  // to get the shared region pointer for the address
 *                  srptr = SharedRegion_getSRPtr(addr, index);
 *
 *                  // to get the address back from the shared region pointer
 *                  addr = SharedRegion_getPtr(srptr);
 *              @endcode
 *
 *
 *  ============================================================================
 */


/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <Memory.h>
#include <Trace.h>

/* Module level headers */
#include <MultiProc.h>
#include <ProcMgr.h>
#include <SharedRegion.h>
#include <SharedRegionDrv.h>
#include <SharedRegionDrvDefs.h>


#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 * Macros and types
 * =============================================================================
 */
/*!
 *  @def    IS_RANGE_VALID
 *  @brief  checks if addrress is within the range.
 */

#define IS_RANGE_VALID(x,min,max) (((x) < (max)) && ((x) >= (min)))

/* =============================================================================
 * Structure & Enums
 * =============================================================================
 */
/*!
 *  @brief  SharedRegion Module state object
 */
typedef struct SharedRegion_ModuleObject_tag {
    UInt32              setupRefCount;
    /*!< Reference count for number of times setup/destroy were called in this
         process. */
    Gate_Handle         gateHandle; /*!< Handle to a gate instance */
    SharedRegion_Info * table;      /*!< Pointer to the table */
    UInt32              bitOffset;  /*!< Index bit offset */
    UInt32              regionSize; /*!< Maximum size of each region */
    SharedRegion_Config cfg;        /*!< Current config values */
} SharedRegion_ModuleObject;


/* =============================================================================
 * Globals
 * =============================================================================
 */
/*!
 *  @var    SharedRegion_moduleState
 *
 *  @brief  ProcMgr state object variable
 */
#if !defined(SYSLINK_BUILD_DEBUG)
static
#endif /* if !defined(SYSLINK_BUILD_DEBUG) */
SharedRegion_ModuleObject SharedRegion_moduleState =
{
    .setupRefCount = 0,
    .cfg.heapHandle = NULL,
    .cfg.gateHandle = NULL,
    .cfg.maxRegions = 256u
};


/* =============================================================================
 * APIs
 * =============================================================================
 */
/*!
 *  @brief      Function to setup the SharedRegion module.
 *
 *  @param      cfgParams   Configuration values.
 */
Int32
SharedRegion_getConfig (SharedRegion_Config * config)
{
    Int                  status = SHAREDREGION_SUCCESS;
    SharedRegionDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "SharedRegion_getConfig", config);

    GT_assert (curTrace, (config != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (config == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "SharedRegion_getConfig",
                             SHAREDREGION_E_INVALIDARG,
                             "Argument of type (SharedRegion_Config *) passed "
                             "is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Temporarily open the handle to get the configuration. */
        status = SharedRegionDrv_open ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SharedRegion_getConfig",
                                 status,
                                 "Failed to open driver handle!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            cmdArgs.args.getConfig.config = config;
            status = SharedRegionDrv_ioctl (CMD_SHAREDREGION_GETCONFIG,
                                            &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                  GT_4CLASS,
                                  "SharedRegion_getConfig",
                                  status,
                                  "API (through IOCTL) failed on kernel-side!");

            }
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Close the driver handle. */
        SharedRegionDrv_close ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "SharedRegion_getConfig", status);

    /*! @retval SHAREDREGION_SUCCESS operation was successful */
    return status;
}


/*!
 *  @brief      Function to setup the SharedRegion module.
 *
 *              Here if user passes both maxRegions and indexBits, than
 *              maxRegions takes precedence over indexBits.
 *
 *  @param      cfg   Configuration values.
 *
 *  @sa         SharedRegion_destroy
 */
Int32
SharedRegion_setup (SharedRegion_Config * cfg)
{
    Int                     status  = SHAREDREGION_SUCCESS;
    SharedRegion_Config *   ptCfg   = NULL;
    SharedRegion_Config     tCfg;
    UInt32                  i;
    SharedRegionDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "SharedRegion_setup", cfg);

    /* TBD: Protect from multiple threads. */
    SharedRegion_moduleState.setupRefCount++;
    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */
    if (SharedRegion_moduleState.setupRefCount > 1) {
        /*! @retval SHAREDREGION_S_ALREADYSETUP Success: ProcMgr module has been
                                           already setup in this process */
        status = SHAREDREGION_S_ALREADYSETUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "SharedRegion module has been already setup in this process."
                   "\n  RefCount: [%d]\n",
                  SharedRegion_moduleState.setupRefCount);
    }
    else {
        /* Open the driver handle. */
        status = SharedRegionDrv_open ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SharedRegion_setup",
                                 status,
                                 "Failed to open driver handle!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            if (cfg == NULL) {
                SharedRegion_getConfig (&tCfg);
                ptCfg = &tCfg;
            }
            else {
                ptCfg = cfg;
            }

            SharedRegion_moduleState.table = Memory_calloc (
                                 NULL,
                                 (  (  sizeof (SharedRegion_Info)
                                     * ptCfg->maxRegions)
                                  * (MultiProc_getMaxProcessors() + 1u)),
                                 0);
            if (SharedRegion_moduleState.table == NULL) {
                status = SHAREDREGION_E_MEMORY;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "SharedRegion_setup",
                                     status,
                                     "Failed to allocate memory for table!");
            }
            else {
                cmdArgs.args.setup.table  = SharedRegion_moduleState.table;
                cmdArgs.args.setup.config = (SharedRegion_Config *) ptCfg;
                status = SharedRegionDrv_ioctl (CMD_SHAREDREGION_SETUP,
                                                &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
                if (status < 0) {
                    GT_setFailureReason (curTrace,
                                         GT_4CLASS,
                                         "SharedRegion_setup",
                                         status,
                                         "API (through IOCTL) failed on "
                                         "kernel-side!");
                }
            }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        }

        if ((ptCfg->maxRegions > 0u) && (status >= 0)) {
            /* copy the user provided values into the state object */
            Memory_copy ((Ptr) &SharedRegion_moduleState.cfg,
                         (Ptr) ptCfg,
                         sizeof (SharedRegion_Config));
        }
        else {
            /*! @retval SHAREDREGION_E_INVALIDARG cfg->maxRegions is 0 */
            status = SHAREDREGION_E_INVALIDARG;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SharedRegion_setup",
                                 status,
                                 "cfg->maxRegions is 0!");
        }

        if (status >= 0) {
            /* Calculate max entries */
            if (ptCfg->maxRegions > 0u) {
                for (i = ((sizeof (Ptr) * 8u) - 1u); i >= 0u; i--) {
                    if (ptCfg->maxRegions > (1u << i)) {
                        break;
                    }
                }
                SharedRegion_moduleState.bitOffset  =
                                               (((sizeof (Ptr) * 8u) - 1u) - i);
                SharedRegion_moduleState.regionSize =
                                     (1u << SharedRegion_moduleState.bitOffset);
            }
        }
    }

    GT_1trace (curTrace, GT_LEAVE, "SharedRegion_setup", status);

    /*! @retval SHAREDREGION_SUCCESS Operation successful */
    return (status);
}


/*!
 *  @brief      Function to destroy the SharedRegion module.
 *
 *  @sa         SharedRegion_setup
 */
Int32 SharedRegion_destroy (Void)
{
    Int                     status = SHAREDREGION_SUCCESS;
    SharedRegionDrv_CmdArgs    cmdArgs;

    GT_0trace (curTrace, GT_ENTER, "SharedRegion_destroy");

    /* TBD: Protect from multiple threads. */
    SharedRegion_moduleState.setupRefCount--;
    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */
    if (SharedRegion_moduleState.setupRefCount >= 1) {
        /*! @retval SHAREDREGION_S_ALREADYSETUP Success: ProcMgr module has been
                                           already setup in this process */
        status = SHAREDREGION_S_ALREADYSETUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "ProcMgr module has been already setup in this process.\n"
                   "    RefCount: [%d]\n",
                   SharedRegion_moduleState.setupRefCount);
    }
    else {
        status = SharedRegionDrv_ioctl (CMD_SHAREDREGION_DESTROY, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SharedRegion_destroy",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

        /* Close the driver handle. */
        SharedRegionDrv_close ();
    }

    GT_1trace (curTrace, GT_LEAVE, "SharedRegion_destroy", status);

    return status;
}


/*!
 *  @brief      Add a memory segment to the lookup table during runtime by base
 *              and length.
 *
 *  @param      index  Shared region index.
 *  @param      base   Base address for the entry.
 *  @param      len    size for the entry.
 *
 *  @sa         SharedRegion_remove, SharedRegion_addUInt32
 */
Int32
SharedRegion_add (UInt index, Ptr base, UInt32 len)
{
    Int32                   status = SHAREDREGION_SUCCESS;
    SharedRegion_Info *     entry  = NULL;
    SharedRegion_Info  *    table  = NULL;
    UInt32                  i;
    UInt16                  myProcId;
    UInt32                  physAddress;
    SharedRegionDrv_CmdArgs cmdArgs;

    GT_3trace (curTrace, GT_ENTER, "SharedRegion_add", index, base, len);


#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (status < 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "SharedRegion_add",
                             SHAREDREGION_E_INVALIDSTATE,
                             "address translation failed!");
    }
    else if (SharedRegion_moduleState.setupRefCount == 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "SharedRegion_create",
                             SHAREDREGION_E_INVALIDSTATE,
                             "Modules is invalidstate!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        physAddress = (UInt32) Memory_translate (base,
                                                 Memory_XltFlags_Virt2Phys);
        GT_assert (curTrace, (physAddress != (UInt32)NULL));
        cmdArgs.args.add.index = index;
        cmdArgs.args.add.base  = (Ptr) physAddress;
        cmdArgs.args.add.len = len;
        status = SharedRegionDrv_ioctl (CMD_SHAREDREGION_ADD, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SharedRegion_add",
                                 status,
                                 "API (through IOCTL) failed on "
                                 "kernel-side!");
        }
        else if (SharedRegion_moduleState.table == NULL) {
            /*! @retval  SHAREDREGION_E_INVALIDSTATE Module is not
             * initialized*/
            status = SHAREDREGION_E_INVALIDSTATE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SharedRegion_add",
                                 status,
                                 "Module is not initialized");
        }
        else if (index >= SharedRegion_moduleState.cfg.maxRegions) {
            /*! @retval  SHAREDREGION_E_INVALIDARG index is invalid */
            status = SHAREDREGION_E_INVALIDARG;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SharedRegion_add",
                                 status,
                                 "index is invalid");
        }
        else if (SharedRegion_moduleState.regionSize < len) {
            status = SHAREDREGION_E_INVALIDARG;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SharedRegion_add",
                                 status,
                                 "Entry's length is more than what is "
                                 "supported");

        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            GT_assert (curTrace, (   SharedRegion_moduleState.regionSize
                                  >= len));

            myProcId = MultiProc_getId (NULL);

            /* Enter the gate */
           /*TBD : key = Gate_enter (SharedRegion_moduleState.gateHandle);*/

            table = SharedRegion_moduleState.table;

            for (i = 0u; i < SharedRegion_moduleState.cfg.maxRegions; i++) {
                entry = (  table
                         + (  myProcId
                            * SharedRegion_moduleState.cfg.maxRegions)
                         + i);
                if (entry->isValid) {
                    if (base >= entry->base) {
                        if (   base
                            < (Ptr)((UInt32)entry->base + entry->len)) {
                            status = SHAREDREGION_E_OVERLAP;
                            GT_setFailureReason (curTrace,
                                                 GT_4CLASS,
                                                 "SharedRegion_add",
                                                 status,
                                                 "Base address overlap with"
                                                 "another entry");
                        }
                    }
                    else {
                        if ((Ptr)((UInt32)base + len) >= entry->base) {
                            /*! @retval  SHAREDREGION_E_OVERLAP Base address
                             * overlap with another entry*/
                            status = SHAREDREGION_E_OVERLAP;
                            GT_setFailureReason (curTrace,
                                                 GT_4CLASS,
                                                 "SharedRegion_add",
                                                 status,
                                                 "Base address overlap with"
                                                 "another entry");
                        }
                    }
                }
            }

            entry = (  table
                     + (myProcId * SharedRegion_moduleState.cfg.maxRegions)
                     + index);

        /* set the new entry or raise error if a valid one already exists */
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (entry->isValid == FALSE) {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                entry->base = (Ptr)base;
                entry->len = len;
                entry->isValid = TRUE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            }
            else {
                /*! @retval  SHAREDREGION_E_ALREADYEXIST indexed entry is
                 *  already used */
                status = SHAREDREGION_E_ALREADYEXIST;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "SharedRegion_add",
                                     status,
                                     "indexed entry is already used");
            }
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    /* Leave the gate */
   /* TBD : Gate_leave (SharedRegion_moduleState.gateHandle, key);*/
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */


    GT_1trace (curTrace, GT_LEAVE, "SharedRegion_add", status);

    /*! @retval  SHAREDREGION_SUCCESS Entry is added successfully */
    return status;
}


/*!
 *  @brief      Returns the index for the specified address pointer.
 *
 *  @param      addr   address to be checked.
 *
 *  @sa         SharedRegion_add, SharedRegion_remove, SharedRegion_addUInt32
 */
Int32
SharedRegion_getIndex (Ptr addr)
{
    SharedRegion_Info *  entry  = NULL;
    SharedRegion_Info  * table  = NULL;
    Int                  id = -1u;
    UInt32               i;
    UInt16               myProcId;
    /* TBD : UInt32      key; */

    GT_1trace (curTrace, GT_ENTER, "SharedRegion_getIndex", addr);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (SharedRegion_moduleState.table == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "SharedRegion_getIndex",
                             SHAREDREGION_E_INVALIDSTATE,
                             "Module is not initialized");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        myProcId = MultiProc_getId (NULL);

        /* Enter the gate */
        /* TBD: key = Gate_enter (SharedRegion_moduleState.gateHandle);*/

        table = SharedRegion_moduleState.table;

        for (i = 0; i < SharedRegion_moduleState.cfg.maxRegions; i++) {
            entry = (  table
                     + (myProcId * SharedRegion_moduleState.cfg.maxRegions)
                     + i);
            if ((addr >= entry->base) &&
                (addr < (Ptr)((UInt32)entry->base + (entry->len)))) {
                /*! @retval >=0 Operation is successful */
                id = i;
                break;
            }
        }

        /* Leave the gate */
        /* TBD : Gate_leave (SharedRegion_moduleState.gateHandle, key);*/
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    GT_1trace (curTrace, GT_LEAVE, "SharedRegion_getIndex", id);

    /*! @retval -1  Address doesnot match any entry */
    return id;

}


/*!
 *  @brief      Returns the address pointer associated with the shared region
 *              pointer.
 *
 *  @param      srPtr  Shared region pointer.
 *
 *  @sa         SharedRegion_add, SharedRegion_getIndex
 */
Ptr
SharedRegion_getPtr (SharedRegion_SRPtr srptr)
{

    SharedRegion_Info *  entry  = NULL;
    Ptr                  returnPtr = NULL;
    UInt16               myProcId;
    /* TBD : UInt32      key; */

    GT_1trace (curTrace, GT_ENTER, "SharedRegion_getPtr", srptr);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (SharedRegion_moduleState.table == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "SharedRegion_getPtr",
                             SHAREDREGION_E_INVALIDSTATE,
                             "Module is not initialized");
    }
    else if (SHAREDREGION_INVALIDSRPTR == srptr) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "SharedRegion_getPtr",
                             SHAREDREGION_E_INVALIDARG,
                             "SharedRegion_SRPtr passed is invalid");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        myProcId = MultiProc_getId (NULL);

        /* Enter the gate */
        /* TBD: key = Gate_enter (SharedRegion_moduleState.gateHandle);*/

        entry = (  SharedRegion_moduleState.table
                 + (myProcId * SharedRegion_moduleState.cfg.maxRegions)
                 + ((UInt32)srptr >> SharedRegion_moduleState.bitOffset));

        returnPtr = ((Ptr)( (   (UInt32) srptr
                             &  ((1 << SharedRegion_moduleState.bitOffset) - 1))
                           +    (UInt32)entry->base));

        /* Leave the gate */
        /* TBD: Gate_leave (SharedRegion_moduleState.gateHandle, key); */
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    GT_1trace (curTrace, GT_LEAVE, "SharedRegion_getPtr", returnPtr);

    /*! @retval NULL           No shared region exists for the given address */
    /*! @retval Valid-Pointer  Operation successful */
    return returnPtr;

}


/*!
 *  @brief      Returns the address pointer associated with the shared region
 *              pointer.
 *
 *  @param      addr   address for the entry.
 *  @param      index  index for the entry.
 *
 *  @sa         SharedRegion_add, SharedRegion_getIndex
 */
SharedRegion_SRPtr
SharedRegion_getSRPtr (Ptr addr, Int index)
{

    SharedRegion_Info * entry  = NULL;
    SharedRegion_SRPtr  retPtr = SHAREDREGION_INVALIDSRPTR ;
    UInt32              myProcId;
    /* TBD : UInt32     key; */

    GT_2trace (curTrace, GT_ENTER, "SharedRegion_getSRPtr", addr, index);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (SharedRegion_moduleState.table == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "SharedRegion_getSRPtr",
                             SHAREDREGION_E_INVALIDSTATE,
                             "Module is not initialized");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Enter the gate */
        if (index < SharedRegion_moduleState.cfg.maxRegions) {
            /* TBD: key = Gate_enter (SharedRegion_moduleState.gateHandle);*/

            myProcId = MultiProc_getId (NULL);
            entry = (  SharedRegion_moduleState.table
                     + (myProcId * SharedRegion_moduleState.cfg.maxRegions)
                     + index);


            if (IS_RANGE_VALID ((UInt32) addr,
                                (UInt32) entry->base,
                                (UInt32) entry->base + entry->len)) {
                retPtr = (SharedRegion_SRPtr)
                            (   (index << SharedRegion_moduleState.bitOffset)
                             |  ((UInt32)addr - (UInt32)entry->base));
            }
            /* Leave the gate */
           /* TBD: Gate_leave (SharedRegion_moduleState.gateHandle, key);*/
        }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "SharedRegion_getSRPtr", retPtr);

    /*! @retval NULL           No shared region exists for the given address */
    /*! @retval Valid-Pointer  Operation successful */
    return retPtr;
}


/*!
 *  @brief      Gets the table entry information for the specified index and id.
 *
 *  @param      index  Shared region index.
 *  @param      procId Processor identifier.
 *  @param      info   Information pointer.
 *
 *  @sa         SharedRegion_getTableInfo
 */
Void
SharedRegion_getTableInfo (UInt                index,
                           UInt16              procId,
                           SharedRegion_Info * info)/*TBD*/
{

    SharedRegion_Info * entry = NULL;
    SharedRegion_Info * table = NULL;
    /* TBD : UInt32     key; */


    GT_3trace (curTrace, GT_ENTER, "SharedRegion_getTableInfo",
               index, procId, info);
    GT_assert (curTrace, (info != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (SharedRegion_moduleState.table == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "SharedRegion_getTableInfo",
                             SHAREDREGION_E_INVALIDSTATE,
                             "Module is not initialized");
    }
    else if (info == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "SharedRegion_getTableInfo",
                             SHAREDREGION_E_INVALIDARG,
                             "Info container passed is null");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Enter the gate */
        /* TBD: key = Gate_enter (SharedRegion_moduleState.gateHandle);*/

        if (procId <= MultiProc_getMaxProcessors()) {
            table = SharedRegion_moduleState.table;
            entry = (  table
                     + (procId * SharedRegion_moduleState.cfg.maxRegions)
                     + index);
            Memory_copy ((Ptr) info, (Ptr) entry, sizeof (SharedRegion_Info));
        }
        else {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SharedRegion_getTableInfo",
                                 SHAREDREGION_E_INVALIDARG,
                                 "ProcId passed is more than max Id");
        }
        /* Leave the gate */
        /* TBD:Gate_leave (SharedRegion_moduleState.gateHandle, key);*/
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    GT_0trace (curTrace, GT_LEAVE, "SharedRegion_getTableInfo");
}


/*!
 *  @brief      Sets the table entry information for the specified index and id.
 *
 *  @param      index  Shared region index.
 *  @param      procId Processor identifier.
 *  @param      info   Information pointer.
 *
 *  @sa         SharedRegion_getTableInfo
 */
Void
SharedRegion_setTableInfo (UInt                index,
                           UInt16              procId,
                           SharedRegion_Info * info) /*TBD*/
{

    SharedRegion_Info * entry = NULL;
    SharedRegion_Info * table = NULL;
    /* TBD : UInt32     key; */

    GT_3trace (curTrace, GT_ENTER, "SharedRegion_setTableInfo",
               index, procId, info);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (SharedRegion_moduleState.table == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "SharedRegion_setTableInfo",
                             SHAREDREGION_E_INVALIDSTATE,
                             "Module is not initialized");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Enter the gate */
        /*key = Gate_enter (SharedRegion_moduleState.gateHandle);*/

        if (procId <= MultiProc_getMaxProcessors()) {
            table = SharedRegion_moduleState.table;
            entry = (  table
                     + (procId * SharedRegion_moduleState.cfg.maxRegions)
                     + index);
            Memory_copy ((Ptr) entry, (Ptr) info, sizeof (SharedRegion_Info));
        }
        else {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SharedRegion_setTableInfo",
                                 SHAREDREGION_E_INVALIDARG,
                                 "ProcId passed is more than max Id");
        }
        /* Leave the gate */
        /*Gate_leave (SharedRegion_moduleState.gateHandle, key);*/
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    GT_0trace (curTrace, GT_LEAVE, "SharedRegion_setTableInfo");
}


/*!
 *  @brief      Removes the memory segment at the specified index from the
 *              lookup table at runtime.
 *
 *  @param      index  Shared region index.
 *
 *  @sa         SharedRegion_add
 */
Int32
SharedRegion_remove (UInt index)
{
    Int32                   status = SHAREDREGION_SUCCESS;
    SharedRegionDrv_CmdArgs cmdArgs;
    SharedRegion_Info *     entry = NULL;
    SharedRegion_Info *     table = NULL;
    /* TBD : UInt32         key; */
    UInt16                  myProcId;

    GT_1trace (curTrace, GT_ENTER, "SharedRegion_remove", index);


#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (SharedRegion_moduleState.setupRefCount == 0) {
        /*! @retval  SHAREDREGION_E_INVALIDSTATE Module is in invalid state! */
        status = SHAREDREGION_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "SharedRegion_remove",
                             status,
                             "Module is in invalid state!");
    }
    else if (index >= SharedRegion_moduleState.cfg.maxRegions) {
        /*! @retval  SHAREDREGION_E_INVALIDARG index is outside range of
                                                configured maxRegions. */
        status = SHAREDREGION_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "SharedRegion_remove",
                             status,
                             "index is outside range of configured maxRegions");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.remove.index  = index;
        status = SharedRegionDrv_ioctl (CMD_SHAREDREGION_REMOVE, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SharedRegion_remove",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else if (SharedRegion_moduleState.table == NULL) {
            /*! @retval  SHAREDREGION_E_INVALIDSTATE Module is not
                                                     initialized! */
            status = SHAREDREGION_E_INVALIDSTATE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "SharedRegion_remove",
                                 status,
                                 "Module is not initialized");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            /* Enter the gate */
            /* TBD: key = Gate_enter (SharedRegion_moduleState.gateHandle);*/

            /* mark entry invalid */
            table = SharedRegion_moduleState.table;
            myProcId = MultiProc_getId (NULL);

            entry = (  table
                     + (myProcId * SharedRegion_moduleState.cfg.maxRegions)
                     + index);
            entry->base    = NULL;
            entry->len     = 0u;
            entry->isValid = FALSE;

            /* Leave the gate */
            /* TBD: Gate_leave (SharedRegion_moduleState.gateHandle, key);*/
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "SharedRegion_remove", status);

    /*! @retval  SHAREDREGION_SUCCESS Entry is added successfully */
    return status;
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
