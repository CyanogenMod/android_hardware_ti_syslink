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
 *  @file   ProcMgr.c
 *
 *  @brief      The Processor Manager on a master processor provides control
 *              functionality for a slave device.
 *              The ProcMgr module provides the following services for the
 *              slave processor:
 *              - Slave processor boot-loading
 *              - Read from or write to slave processor memory
 *              - Slave processor power management
 *              - Slave processor error handling
 *              - Dynamic Memory Mapping
 *              The Device Manager (Processor module) shall have interfaces for:
 *              - Loader: There may be multiple implementations of the Loader
 *                        interface within a single Processor instance.
 *                        For example, COFF, ELF, dynamic loader, custom types
 *                        of loaders may be written and plugged in.
 *              - Power Manager: The Power Manager implementation can be a
 *                        separate module that is plugged into the Processor
 *                        module. This allows the Processor code to remain
 *                        generic, and the Power Manager may be written and
 *                        maintained either by a separate team, or by customer.
 *              - Processor: The implementation of this interface provides all
 *                        other functionality for the slave processor, including
 *                        setup and initialization of the Processor module,
 *                        management of slave processor MMU (if available),
 *                        functions to write to and read from slave memory etc.
 *              All processors in the system shall be identified by unique
 *              processor ID. The management of this processor ID is done by the
 *              MultiProc module.
 *
 *              Processor state machine:
 *
 *                      attach
 *              Unknown -------> Powered
 *                 ^  ^           |
 *                 |   \          |
 *           slave |    \ detach  V
 *           error \     ------- Reset <-----
 *                 ^              |          \
 *                 |              | load      \
 *           slave |              V           |stop
 *           error \             Loaded       |
 *            or    \             |           |
 *            crash  \            | start     /
 *                    \           V          /
 *                     -------- Running ----
 *                               /   ^
 *                              /     \
 *                              |     |
 *                    Pwr state |     | Pwr state
 *                     change   |     |  change
 *                              \     /
 *                              V    /
 *                            Unavailable
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
#include <MultiProc.h>
#include <ProcMgr.h>
#include <ProcMgrDrvDefs.h>
#include <ProcMgrDrvUsr.h>
#include <_ProcMgrDefs.h>
#include <load.h>
#include <ArrayList.h>
#include <dload_api.h>


#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*!
 *  @brief  Checks if a value lies in given range.
 */
#define IS_RANGE_VALID(x,min,max) (((x) < (max)) && ((x) >= (min)))


/*!
 *  @brief  ProcMgr Module state object
 */
typedef struct ProcMgr_ModuleObject_tag {
    UInt32         setupRefCount;
    /*!< Reference count for number of times setup/destroy were called in this
         process. */
    ProcMgr_Handle procHandles [MULTIPROC_MAXPROCESSORS];
    /*!< Array of Handles of ProcMgr instances */
} ProcMgr_ModuleObject;

/*!
 *  @brief  ProcMgr instance object
 */
typedef struct ProcMgr_Object_tag {
    Ptr              knlObject;
    /*!< Pointer to the kernel-side ProcMgr object. */
    UInt32           openRefCount;
    /*!< Reference count for number of times open/close were called in this
         process. */
    Bool             created;
    /*!< Indicates whether the object was created in this process. */
    UInt16           procId;
    /*!< Processor ID */
    UInt16           numMemEntries;
    /*!< Number of valid memory entries */
    ProcMgr_AddrInfo memEntries [PROCMGR_MAX_MEMORY_REGIONS];
    /*!< Configuration of memory regions */
} ProcMgr_Object;


/* =============================================================================
 *  Globals
 * =============================================================================
 */
/*!
 *  @var    ProcMgr_state
 *
 *  @brief  ProcMgr state object variable
 */
#if !defined(SYSLINK_BUILD_DEBUG)
static
#endif /* if !defined(SYSLINK_BUILD_DEBUG) */
ProcMgr_ModuleObject ProcMgr_state =
{
    .setupRefCount = 0
};

extern UInt32 prog_handle;

/* =============================================================================
 *  APIs
 * =============================================================================
 */
/*!
 *  @brief      Function to get the default configuration for the ProcMgr
 *              module.
 *
 *              This function can be called by the application to get their
 *              configuration parameter to ProcMgr_setup filled in by the
 *              ProcMgr module with the default parameters. If the user does
 *              not wish to make any change in the default parameters, this API
 *              is not required to be called.
 *
 *  @param      cfg        Pointer to the ProcMgr module configuration structure
 *                         in which the default config is to be returned.
 *
 *  @sa         ProcMgr_setup, ProcMgrDrvUsr_open, ProcMgrDrvUsr_ioctl,
 *              ProcMgrDrvUsr_close
 */
Void
ProcMgr_getConfig (ProcMgr_Config * cfg)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                         status = PROCMGR_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    ProcMgr_CmdArgsGetConfig    cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "ProcMgr_getConfig", cfg);

    GT_assert (curTrace, (cfg != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (cfg == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_getConfig",
                             PROCMGR_E_INVALIDARG,
                             "Argument of type (ProcMgr_Config *) passed "
                             "is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Temporarily open the handle to get the configuration. */
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        ProcMgrDrvUsr_open ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMgr_getConfig",
                                 status,
                                 "Failed to open driver handle!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            cmdArgs.cfg = cfg;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            ProcMgrDrvUsr_ioctl (CMD_PROCMGR_GETCONFIG, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                  GT_4CLASS,
                                  "ProcMgr_getConfig",
                                  status,
                                  "API (through IOCTL) failed on kernel-side!");
            }
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Close the driver handle. */
        ProcMgrDrvUsr_close ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "ProcMgr_getConfig");
}


/*!
 *  @brief      Function to setup the ProcMgr module.
 *
 *              This function sets up the ProcMgr module. This function must
 *              be called before any other instance-level APIs can be invoked.
 *              Module-level configuration needs to be provided to this
 *              function. If the user wishes to change some specific config
 *              parameters, then ProcMgr_getConfig can be called to get the
 *              configuration filled with the default values. After this, only
 *              the required configuration values can be changed. If the user
 *              does not wish to make any change in the default parameters, the
 *              application can simply call ProcMgr_setup with NULL parameters.
 *              The default parameters would get automatically used.
 *
 *  @param      cfg   Optional ProcMgr module configuration. If provided as
 *                    NULL, default configuration is used.
 *
 *  @sa         ProcMgr_destroy, ProcMgrDrvUsr_open, ProcMgrDrvUsr_ioctl
 */
Int
ProcMgr_setup (ProcMgr_Config * cfg)
{
    Int                     status = PROCMGR_SUCCESS;
    ProcMgr_CmdArgsSetup    cmdArgs;
    Int                     i;

    GT_1trace (curTrace, GT_ENTER, "ProcMgr_setup", cfg);

    /* TBD: Protect from multiple threads. */
    ProcMgr_state.setupRefCount++;
    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */
    if (ProcMgr_state.setupRefCount > 1) {
        /*! @retval PROCMGR_S_ALREADYSETUP Success: ProcMgr module has been
                                           already setup in this process */
        status = PROCMGR_S_ALREADYSETUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "    ProcMgr_setup: ProcMgr module has been already setup "
                   "in this process.\n"
                   "        RefCount: [%d]\n",
                   (ProcMgr_state.setupRefCount - 1));
    }
    else {
        /* Set all handles to NULL -- in order for destroy() to work */
        for (i = 0 ; i < MULTIPROC_MAXPROCESSORS ; i++) {
            ProcMgr_state.procHandles [i] = NULL;
        }

        /* Open the driver handle. */
        status = ProcMgrDrvUsr_open ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMgr_setup",
                                 status,
                                 "Failed to open driver handle!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            cmdArgs.cfg = cfg;
            status = ProcMgrDrvUsr_ioctl (CMD_PROCMGR_SETUP, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "ProcMgr_setup",
                                     status,
                                     "API (through IOCTL) failed on kernel-side!");
            }
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

    GT_1trace (curTrace, GT_LEAVE, "ProcMgr_setup", status);

    /*! @retval PROCMGR_SUCCESS Operation successful */
    return (status);
}


/*!
 *  @brief      Function to destroy the ProcMgr module.
 *
 *              Once this function is called, other ProcMgr module APIs, except
 *              for the ProcMgr_getConfig API cannot be called anymore.
 *
 *  @sa         ProcMgr_setup, ProcMgrDrvUsr_ioctl, ProcMgrDrvUsr_close
 */
Int
ProcMgr_destroy (Void)
{
    Int                     status = PROCMGR_SUCCESS;
    ProcMgr_CmdArgsDestroy  cmdArgs;
    UInt16                  i;

    GT_0trace (curTrace, GT_ENTER, "ProcMgr_destroy");

    /* TBD: Protect from multiple threads. */
    ProcMgr_state.setupRefCount--;
    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */
    if (ProcMgr_state.setupRefCount >= 1) {
        /*! @retval PROCMGR_S_SETUP Success: ProcMgr module has been setup
                                             by other clients in this process */
        status = PROCMGR_S_SETUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "ProcMgr module has been setup by other clients in this"
                   " process.\n"
                   "    RefCount: [%d]\n",
                   (ProcMgr_state.setupRefCount + 1));
    }
    else {
        status = ProcMgrDrvUsr_ioctl (CMD_PROCMGR_DESTROY, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMgr_destroy",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Check if any ProcMgr instances have not been deleted so far. If not,
         * delete them.
         */
        for (i = 0 ; i < MULTIPROC_MAXPROCESSORS ; i++) {
            GT_assert (curTrace, (ProcMgr_state.procHandles [i] == NULL));
            if (ProcMgr_state.procHandles [i] != NULL) {
                ProcMgr_delete (&(ProcMgr_state.procHandles [i]));
            }
        }

        /* Close the driver handle. */
        ProcMgrDrvUsr_close ();
    }

    GT_1trace (curTrace, GT_LEAVE, "ProcMgr_destroy", status);

    /*! @retval PROCMGR_SUCCESS Operation successful */
    return (status);
}


/*!
 *  @brief      Function to initialize the parameters for the ProcMgr instance.
 *
 *              This function can be called by the application to get their
 *              configuration parameter to ProcMgr_create filled in by the
 *              ProcMgr module with the default parameters.
 *
 *  @param      handle   Handle to the ProcMgr object. If specified as NULL,
 *                       the default global configuration values are returned.
 *  @param      params   Pointer to the ProcMgr instance params structure in
 *                       which the default params is to be returned.
 *
 *  @sa         ProcMgr_create, ProcMgrDrvUsr_ioctl
 */
Void
ProcMgr_Params_init (ProcMgr_Handle handle, ProcMgr_Params * params)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                         status          = PROCMGR_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    ProcMgr_Object *            procMgrHandle   = (ProcMgr_Object *) handle;
    ProcMgr_CmdArgsParamsInit   cmdArgs;

    GT_2trace (curTrace, GT_ENTER, "ProcMgr_Params_init", handle, params);

    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (params == NULL) {
        /* No retVal comment since this is a Void function. */
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_Params_init",
                             status,
                             "Argument of type (ProcMgr_Params *) passed "
                             "is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Check if the handle is NULL and pass it in directly to kernel-side in
         * that case. Otherwise send the kernel object pointer.
         */
        if (procMgrHandle == NULL) {
            cmdArgs.handle = handle;
        }
        else {
            cmdArgs.handle = procMgrHandle->knlObject;
        }
        cmdArgs.params = params;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        ProcMgrDrvUsr_ioctl (CMD_PROCMGR_PARAMS_INIT, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMgr_Params_init",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "ProcMgr_Params_init");
}


/*!
 *  @brief      Function to create a ProcMgr object for a specific slave
 *              processor.
 *
 *              This function creates an instance of the ProcMgr module and
 *              returns an instance handle, which is used to access the
 *              specified slave processor. The processor ID specified here is
 *              the ID of the slave processor as configured with the MultiProc
 *              module.
 *              Instance-level configuration needs to be provided to this
 *              function. If the user wishes to change some specific config
 *              parameters, then ProcMgr_Params_init can be called to get the
 *              configuration filled with the default values. After this, only
 *              the required configuration values can be changed. For this
 *              API, the params argument is not optional, since the user needs
 *              to provide some essential values such as loader, PwrMgr and
 *              Processor instances to be used with this ProcMgr instance.
 *
 *  @param      procId   Processor ID represented by this ProcMgr instance
 *  @param      params   ProcMgr instance configuration parameters.
 *
 *  @sa         ProcMgr_delete, Memory_calloc, ProcMgrDrvUsr_ioctl
 */
ProcMgr_Handle
ProcMgr_create (UInt16 procId, const ProcMgr_Params * params)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                     status = PROCMGR_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    ProcMgr_Object *        handle = NULL;
    /* TBD: UInt32                  key;*/
    ProcMgr_CmdArgsCreate   cmdArgs;

    GT_2trace (curTrace, GT_ENTER, "ProcMgr_create", procId, params);

    GT_assert (curTrace, IS_VALID_PROCID (procId));
    GT_assert (curTrace, (params != NULL));
    GT_assert (curTrace, ((params != NULL)) && (params->procHandle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (!IS_VALID_PROCID (procId)) {
        /*! @retval NULL Invalid procId specified */
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_create",
                             status,
                             "Invalid procId specified");
    }
    else if (params == NULL) {
        /*! @retval NULL Invalid NULL params pointer specified */
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_create",
                             status,
                             "Invalid NULL params pointer specified");
    }
    else if (params->procHandle == NULL) {
        /*! @retval NULL Invalid NULL procHandle specified in params */
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_create",
                             status,
                             "Invalid NULL procHandle specified in params");
    }
  
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* TBD: Enter critical section protection. */
        /* key = Gate_enter (ProcMgr_state.gateHandle); */
        if (ProcMgr_state.procHandles [procId] != NULL) {
            /* If the object is already created/opened in this process, return
             * handle in the local array.
             */
            handle = (ProcMgr_Object *) ProcMgr_state.procHandles [procId];
            handle->openRefCount++;
            GT_1trace (curTrace,
                       GT_2CLASS,
                       "    ProcMgr_create: Instance already exists in this"
                       " process space"
                       "        RefCount [%d]\n",
                       (handle->openRefCount - 1));
        }
        else {
            cmdArgs.procId = procId;
            /* Get the kernel objects of Processor, Loader and PwrMgr modules,
             * and pass them to the kernel-side.
             */
            cmdArgs.params.procHandle = ((ProcMgr_CommonObject *)
                                             (params->procHandle))->knlObject;

#if !defined(SYSLINK_BUILD_OPTIMIZE)
            status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            ProcMgrDrvUsr_ioctl (CMD_PROCMGR_CREATE, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                  GT_4CLASS,
                                  "ProcMgr_create",
                                  status,
                                  "API (through IOCTL) failed on kernel-side!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                /* Allocate memory for the handle */
                handle = (ProcMgr_Object *) Memory_calloc (NULL,
                                                        sizeof (ProcMgr_Object),
                                                        0);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
                if (handle == NULL) {
                    /*! @retval NULL Memory allocation failed for handle */
                    status = PROCMGR_E_MEMORY;
                    GT_setFailureReason (curTrace,
                                        GT_4CLASS,
                                        "ProcMgr_create",
                                        status,
                                        "Memory allocation failed for handle!");
                }
                else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                    /* Set pointer to kernel object into the user handle. */
                    handle->knlObject = cmdArgs.handle;
                    /* Indicate that the object was created in this process. */
                    handle->created = TRUE;
                    handle->procId = procId;
                    /* Store the ProcMgr handle in the local array. */
                    ProcMgr_state.procHandles [procId] = (ProcMgr_Handle)handle;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
                }
            }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        }
        /* TBD: Leave critical section protection. */
        /* Gate_leave (ProcMgr_state.gateHandle, key); */
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ProcMgr_create", handle);

    /*! @retval Valid Handle Operation successful */
    return (ProcMgr_Handle) handle;
}


/*!
 *  @brief      Function to delete a ProcMgr object for a specific slave
 *              processor.
 *
 *              Once this function is called, other ProcMgr instance level APIs
 *              that require the instance handle cannot be called.
 *
 *  @param      handlePtr   Pointer to Handle to the ProcMgr object
 *
 *  @sa         ProcMgr_create, Memory_free, ProcMgrDrvUsr_ioctl
 */
Int
ProcMgr_delete (ProcMgr_Handle * handlePtr)
{
    Int                     status    = PROCMGR_SUCCESS;
    Int                     tmpStatus = PROCMGR_SUCCESS;
    ProcMgr_Object *        handle;
    /* TBD: UInt32          key;*/
    ProcMgr_CmdArgsDelete   cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "ProcMgr_delete", handlePtr);

    GT_assert (curTrace, (handlePtr != NULL));
    GT_assert (curTrace, ((handlePtr != NULL) && (*handlePtr != NULL)));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handlePtr == NULL) {
        /*! @retval PROCMGR_E_INVALIDARG Invalid NULL handlePtr specified*/
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_delete",
                             status,
                             "Invalid NULL handlePtr specified");
    }
    else if (*handlePtr == NULL) {
        /*! @retval PROCMGR_E_HANDLE Invalid NULL *handlePtr specified */
        status = PROCMGR_E_HANDLE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_delete",
                             status,
                             "Invalid NULL *handlePtr specified");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        handle = (ProcMgr_Object *) (*handlePtr);
        /* TBD: Enter critical section protection. */
        /* key = Gate_enter (ProcMgr_state.gateHandle); */

        if (handle->openRefCount != 0) {
            /* There are still some open handles to this ProcMgr.
             * Give a warning, but still go ahead and delete the object.
             */
            status = PROCMGR_S_OPENHANDLE;
            GT_assert (curTrace, (handle->openRefCount != 0));
            GT_1trace (curTrace,
                       GT_1CLASS,
                       "    ProcMgr_delete: Warning, some handles are"
                       " still open!\n"
                       "        RefCount: [%d]\n",
                       handle->openRefCount);
        }

        if (handle->created == FALSE) {
            /*! @retval PROCMGR_E_ACCESSDENIED The ProcMgr object was not
                   created in this process and access is denied to delete it. */
            status = PROCMGR_E_ACCESSDENIED;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMgr_delete",
                                 status,
                                 "The ProcMgr object was not created in this"
                                 "process and access is denied to delete it.");
        }

        if (status >= 0) {
            /* Only delete the object if it was created in this process. */
            cmdArgs.handle = handle->knlObject;
            tmpStatus = ProcMgrDrvUsr_ioctl (CMD_PROCMGR_DELETE, &cmdArgs);
            if (tmpStatus < 0) {
                /* Only override the status if kernel call failed. Otherwise
                 * we want the status from above to carry forward.
                 */
                status = tmpStatus;
                GT_setFailureReason (curTrace,
                                  GT_4CLASS,
                                  "ProcMgr_delete",
                                  status,
                                  "API (through IOCTL) failed on kernel-side!");
            }
            else {
                /* Clear the ProcMgr handle in the local array. */
                GT_assert (curTrace,(handle->procId < MULTIPROC_MAXPROCESSORS));
                ProcMgr_state.procHandles [handle->procId] = NULL;
                Memory_free (NULL, handle, sizeof (ProcMgr_Object));
                *handlePtr = NULL;
            }
        }

        /* TBD: Leave critical section protection. */
        /* Gate_leave (ProcMgr_state.gateHandle, key); */
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ProcMgr_delete", status);

    /*! @retval PROCMGR_SUCCESS Operation successful */
    return (status);
}


/*!
 *  @brief      Function to open a handle to an existing ProcMgr object handling
 *              the procId.
 *
 *              This function returns a handle to an existing ProcMgr instance
 *              created for this procId. It enables other entities to access
 *              and use this ProcMgr instance.
 *
 *  @param      handlePtr   Return Parameter: Handle to the ProcMgr instance
 *  @param      procId      Processor ID represented by this ProcMgr instance
 *
 *  @sa         ProcMgr_close, Memory_calloc, ProcMgrDrvUsr_ioctl
 */
Int
ProcMgr_open (ProcMgr_Handle * handlePtr, UInt16 procId)
{
    Int                     status = PROCMGR_SUCCESS;
    ProcMgr_Object *        handle = NULL;
    /* UInt32           key; */
    ProcMgr_CmdArgsOpen     cmdArgs;

    GT_2trace (curTrace, GT_ENTER, "ProcMgr_open", handlePtr, procId);

    GT_assert (curTrace, (handlePtr != NULL));
    GT_assert (curTrace, IS_VALID_PROCID (procId));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handlePtr == NULL) {
        /*! @retval PROCMGR_E_INVALIDARG Invalid NULL handle pointer specified*/
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_open",
                             status,
                             "Invalid NULL handle pointer specified");
    }
    else if (!IS_VALID_PROCID (procId)) {
        *handlePtr = NULL;
        /*! @retval PROCMGR_E_INVALIDARG Invalid procId specified */
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_open",
                             status,
                             "Invalid procId specified");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* TBD: Enter critical section protection. */
        /* key = Gate_enter (ProcMgr_state.gateHandle); */
        if (ProcMgr_state.procHandles [procId] != NULL) {
            /* If the object is already created/opened in this process, return
             * handle in the local array.
             */
            handle = (ProcMgr_Object *) ProcMgr_state.procHandles [procId];
            handle->openRefCount++;
            status = PROCMGR_S_ALREADYEXISTS;
            GT_1trace (curTrace,
                       GT_1CLASS,
                       "    ProcMgr_open: Instance already exists in this"
                       " process space"
                       "        RefCount [%d]\n",
                       (handle->openRefCount - 1));
        }
        else {
            /* The object was not created/opened in this process. Need to drop
             * down to the kernel to get the object instance.
             */
            cmdArgs.procId = procId;
            cmdArgs.handle = NULL; /* Return parameter */
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            ProcMgrDrvUsr_ioctl (CMD_PROCMGR_OPEN, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                  GT_4CLASS,
                                  "ProcMgr_open",
                                  status,
                                  "API (through IOCTL) failed on kernel-side!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                /* Allocate memory for the handle */
                handle = (ProcMgr_Object *) Memory_calloc (NULL,
                                                        sizeof (ProcMgr_Object),
                                                        0);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
                if (handle == NULL) {
                    /*! @retval PROCMGR_E_MEMORY Memory allocation failed for
                                                 handle */
                    status = PROCMGR_E_MEMORY;
                    GT_setFailureReason (curTrace,
                                        GT_4CLASS,
                                        "ProcMgr_open",
                                        status,
                                        "Memory allocation failed for handle!");
                }
                else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                    /* Set pointer to kernel object into the user handle. */
                    handle->knlObject = cmdArgs.handle;
                    /* Store the ProcMgr handle in the local array. */
                    ProcMgr_state.procHandles [procId] = (ProcMgr_Handle)handle;
                    handle->openRefCount = 1;
                    handle->procId = procId;
                    handle->created = FALSE;

                    /* Store the memory information received, only if the Proc
                     * has been attached-to already, which will create the
                     * mappings on kernel-side.
                     */
                    if (cmdArgs.procInfo.numMemEntries != 0) {
                        handle->numMemEntries = cmdArgs.procInfo.numMemEntries;
                        Memory_copy (&(handle->memEntries),
                                     &(cmdArgs.procInfo.memEntries),
                                     sizeof (handle->memEntries));
                    }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
                 }
            }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        }

        *handlePtr = (ProcMgr_Handle) handle;
        /* TBD: Leave critical section protection. */
        /* Gate_leave (ProcMgr_state.gateHandle, key); */
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ProcMgr_open", status);

    /*! @retval PROCMGR_SUCCESS Operation successful */
    return (status);
}


/*!
 *  @brief      Function to close this handle to the ProcMgr instance.
 *
 *              This function closes the handle to the ProcMgr instance
 *              obtained through ProcMgr_open call made earlier.
 *
 *  @param      handle     Handle to the ProcMgr object
 *
 *  @sa         ProcMgr_open, Memory_free, ProcMgrDrvUsr_ioctl
 */
Int
ProcMgr_close (ProcMgr_Handle * handlePtr)
{
    Int                     status = PROCMGR_SUCCESS;
    ProcMgr_Object *        procMgrHandle;
    ProcMgr_CmdArgsClose    cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "ProcMgr_close", handlePtr);

    GT_assert (curTrace, (handlePtr != NULL));
    GT_assert (curTrace, ((handlePtr != NULL) && (*handlePtr != NULL)));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handlePtr == NULL) {
        /*! @retval PROCMGR_E_INVALIDARG Invalid NULL handlePtr specified */
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_delete",
                             status,
                             "Invalid NULL handlePtr specified");
    }
    else if (*handlePtr == NULL) {
        /*! @retval PROCMGR_E_HANDLE Invalid NULL *handlePtr specified */
        status = PROCMGR_E_HANDLE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_delete",
                             status,
                             "Invalid NULL *handlePtr specified");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        procMgrHandle = (ProcMgr_Object *) (*handlePtr);
        /* TBD: Enter critical section protection. */
        /* key = Gate_enter (ProcMgr_state.gateHandle); */
        if (procMgrHandle->openRefCount == 0) {
            /*! @retval PROCMGR_E_ACCESSDENIED All open handles to this ProcMgr
                                               object are already closed. */
            status = PROCMGR_E_ACCESSDENIED;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMgr_close",
                                 status,
                                 "All open handles to this ProcMgr object are"
                                 " already closed.");

        }
        else if (procMgrHandle->openRefCount > 1) {
            /* Simply reduce the reference count. There are other threads in
             * this process that have also opened handles to this ProcMgr
             * instance.
             */
            procMgrHandle->openRefCount--;
            status = PROCMGR_S_OPENHANDLE;
            GT_1trace (curTrace,
                       GT_1CLASS,
                       "    ProcMgr_close: Other handles to this instance"
                       " are still open\n"
                       "        RefCount: [%d]\n",
                       (procMgrHandle->openRefCount + 1));
        }
        else {
            /* The object can be closed now since all open handles are closed.*/
            cmdArgs.handle = procMgrHandle->knlObject;
            /* Copy memory information to command arguments. */
            cmdArgs.procInfo.numMemEntries = procMgrHandle->numMemEntries;
            Memory_copy (&(cmdArgs.procInfo.memEntries),
                         &(procMgrHandle->memEntries),
                         sizeof (procMgrHandle->memEntries));
            status = ProcMgrDrvUsr_ioctl (CMD_PROCMGR_CLOSE, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                  GT_4CLASS,
                                  "ProcMgr_close",
                                  status,
                                  "API (through IOCTL) failed on kernel-side!");
            }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

            if (procMgrHandle->created == FALSE) {
                /* Clear the ProcMgr handle in the local array. */
                GT_assert (curTrace,
                           (procMgrHandle->procId < MULTIPROC_MAXPROCESSORS));
                ProcMgr_state.procHandles [procMgrHandle->procId] = NULL;
                /* Free memory for the handle only if it was not created in
                 * this process.
                 */
                Memory_free (NULL, procMgrHandle, sizeof (ProcMgr_Object));
            }
            *handlePtr = NULL;
        }
        /* TBD: Leave critical section protection. */
        /* Gate_leave (ProcMgr_state.gateHandle, key); */
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ProcMgr_close", status);

    /*! @retval PROCMGR_SUCCESS Operation successful */
    return (status);
}


/*!
 *  @brief      Function to initialize the parameters for the ProcMgr attach
 *              function.
 *
 *              This function can be called by the application to get their
 *              configuration parameter to ProcMgr_attach filled in by the
 *              ProcMgr module with the default parameters. If the user does
 *              not wish to make any change in the default parameters, this API
 *              is not required to be called.
 *
 *  @param      handle   Handle to the ProcMgr object. If specified as NULL,
 *                       the default global configuration values are returned.
 *  @param      params   Pointer to the ProcMgr attach params structure in
 *                       which the default params is to be returned.
 *
 *  @sa         ProcMgr_attach, ProcMgrDrvUsr_ioctl
 */
Void
ProcMgr_getAttachParams (ProcMgr_Handle handle, ProcMgr_AttachParams * params)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                             status          = PROCMGR_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    ProcMgr_Object *                procMgrHandle   = (ProcMgr_Object *) handle;
    ProcMgr_CmdArgsGetAttachParams  cmdArgs;

    GT_2trace (curTrace, GT_ENTER, "ProcMgr_getAttachParams", handle, params);

    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (params == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_getAttachParams",
                             PROCMGR_E_INVALIDARG,
                             "Argument of type (ProcMgr_AttachParams *) passed "
                             "is NULL!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* If NULL, send the same to kernel-side. Otherwise translate and send
         * the kernel handle.
         */
        if (procMgrHandle == NULL) {
            cmdArgs.handle = handle;
        }
        else {
            cmdArgs.handle = procMgrHandle->knlObject;
        }
        cmdArgs.params = params;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        ProcMgrDrvUsr_ioctl (CMD_PROCMGR_GETATTACHPARAMS, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMgr_getAttachParams",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "ProcMgr_getAttachParams");
}


/*!
 *  @brief      Function to attach the client to the specified slave and also
 *              initialize the slave (if required).
 *
 *              This function attaches to an instance of the ProcMgr module and
 *              performs any hardware initialization required to power up the
 *              slave device. This function also performs the required state
 *              transitions for this ProcMgr instance to ensure that the local
 *              object representing the slave device correctly indicates the
 *              state of the slave device. Depending on the slave boot mode
 *              being used, the slave may be powered up, in reset, or even
 *              running state.
 *              Configuration parameters need to be provided to this
 *              function. If the user wishes to change some specific config
 *              parameters, then ProcMgr_getAttachParams can be called to get
 *              the configuration filled with the default values. After this,
 *              only the required configuration values can be changed. If the
 *              user does not wish to make any change in the default parameters,
 *              the application can simply call ProcMgr_attach with NULL
 *              parameters.
 *              The default parameters would get automatically used.
 *
 *  @param      handle   Handle to the ProcMgr object.
 *  @param      params   Optional ProcMgr attach parameters. If provided as
 *                       NULL, default configuration is used.
 *
 *  @sa         ProcMgr_detach, ProcMgrDrvUsr_ioctl
 */
Int
ProcMgr_attach (ProcMgr_Handle handle, ProcMgr_AttachParams * params)
{
    Int                    status           = PROCMGR_SUCCESS;
    ProcMgr_Object *       procMgrHandle    = (ProcMgr_Object *) handle;
    ProcMgr_CmdArgsAttach  cmdArgs;

    GT_2trace (curTrace, GT_ENTER, "ProcMgr_attach", handle, params);

    GT_assert (curTrace, (handle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        /*! @retval PROCMGR_E_HANDLE Invalid NULL handle specified */
        status = PROCMGR_E_HANDLE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_attach",
                             status,
                             "Invalid NULL handle specified");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.handle = procMgrHandle->knlObject;
        cmdArgs.params = params;
        status = ProcMgrDrvUsr_ioctl (CMD_PROCMGR_ATTACH, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMgr_attach",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            /* Store the memory information received. */
            procMgrHandle->numMemEntries = cmdArgs.procInfo.numMemEntries;
            Memory_copy (&(procMgrHandle->memEntries),
                         &(cmdArgs.procInfo.memEntries),
                         sizeof (procMgrHandle->memEntries));
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ProcMgr_attach", status);

    /*! @retval PROCMGR_SUCCESS Operation successful */
    return (status);
}


/*!
 *  @brief      Function to detach the client from the specified slave and also
 *              finalze the slave (if required).
 *
 *              This function detaches from an instance of the ProcMgr module
 *              and performs any hardware finalization required to power down
 *              the slave device. This function also performs the required state
 *              transitions for this ProcMgr instance to ensure that the local
 *              object representing the slave device correctly indicates the
 *              state of the slave device. Depending on the slave boot mode
 *              being used, the slave may be powered down, in reset, or left in
 *              its original state.
 *
 *  @param      handle     Handle to the ProcMgr object
 *
 *  @sa         ProcMgr_attach, ProcMgrDrvUsr_ioctl
 */
Int
ProcMgr_detach (ProcMgr_Handle handle)
{
    Int                    status           = PROCMGR_SUCCESS;
    ProcMgr_Object *       procMgrHandle    = (ProcMgr_Object *) handle;
    ProcMgr_CmdArgsDetach  cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "ProcMgr_detach", handle);

    GT_assert (curTrace, (handle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        /*! @retval PROCMGR_E_HANDLE Invalid NULL handle specified */
        status = PROCMGR_E_HANDLE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_detach",
                             status,
                             "Invalid NULL handle specified");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.handle = procMgrHandle->knlObject;
        /* Copy memory information. */
        cmdArgs.procInfo.numMemEntries = procMgrHandle->numMemEntries;
        Memory_copy (&(cmdArgs.procInfo.memEntries),
                     &(procMgrHandle->memEntries),
                     sizeof (procMgrHandle->memEntries));
        status = ProcMgrDrvUsr_ioctl (CMD_PROCMGR_DETACH, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMgr_detach",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            /* Update memory information back. */
            procMgrHandle->numMemEntries = cmdArgs.procInfo.numMemEntries;
            Memory_copy (&(procMgrHandle->memEntries),
                         &(cmdArgs.procInfo.memEntries),
                         sizeof (procMgrHandle->memEntries));
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ProcMgr_detach", status);

    /*! @retval PROCMGR_SUCCESS Operation successful */
    return (status);
}


/*!
 *  @brief      Function to load the specified slave executable on the slave
 *              Processor.
 *
 *              This function allows usage of different types of loaders. The
 *              loader specified when creating this instance of the ProcMgr
 *              is used for loading the slave executable. Depending on the type
 *              of loader, the imagePath parameter may point to the path of the
 *              file in the host file system, or it may be NULL. Some loaders
 *              may require specific parameters to be passed. This function
 *              returns a fileId, which can be used for further function calls
 *              that reference a specific file that has been loaded on the
 *              slave processor.
 *
 *  @param      handle     Handle to the ProcMgr object
 *  @param      imagePath  Full file path
 *  @param      argc       Number of arguments
 *  @param      argv       String array of arguments
 *  @param      params     Loader specific parameters
 *  @param      fileId     Return parameter: ID of the loaded file
 *
 *  @sa         ProcMgr_unload, ProcMgrDrvUsr_ioctl
 */
Int
ProcMgr_load (ProcMgr_Handle handle,
              String         imagePath,
              UInt32         argc,
              String *       argv,
              UInt32 *       entry_point,
              UInt32 *       fileId,
              ProcId         procID)
{
    Int                 status          = PROCMGR_SUCCESS;
    ProcMgr_CmdArgsLoad cmdArgs;
    Int                 prog_argc;
    Array_List          prog_argv;
    UInt32              proc_entry_point;

    GT_5trace (curTrace, GT_ENTER, "ProcMgr_load",
               handle, imagePath, argc, argv, procID);
    GT_assert (curTrace, (handle != NULL));
    GT_assert (curTrace, (fileId != NULL));
    /* imagePath may be NULL if a non-file based loader is used. In that case,
     * loader-specific params will contain the required information.
     */

    GT_assert (curTrace,
               (   ((argc == 0) && (argv == NULL))
                || ((argc != 0) && (argv != NULL))));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        /*! @retval  PROCMGR_E_HANDLE Invalid NULL handle specified */
        status = PROCMGR_E_HANDLE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_load",
                             status,
                             "Invalid NULL handle specified");
    }
    else if (   ((argc == 0) && (argv != NULL))
             || ((argc != 0) && (argv == NULL))) {
        /*! @retval  PROCMGR_E_INVALIDARG Invalid argument */
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_load",
                             status,
                             "Invalid argc/argv values specified");
    }
    else if (fileId == NULL) {
        /*! @retval  PROCMGR_E_INVALIDARG Invalid argument */
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_load",
                             status,
                             "Invalid fileId pointer specified");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        *fileId = 0; /* Initialize return parameter. */

        if(procID == PROC_TESLA || procID == PROC_MPU){
            fprintf(stdout, "Invalid Processor ID procID = %d\n", procID);
            fprintf(stdout, "Loading is only supported for processors "
                            "PROC_SYSM3 and PRC_APP3\n" );
            return PROCMGR_E_INVALIDARG;
        }

        status = ProcMgr_setState(ProcMgr_State_Loading);
        if(status != PROCMGR_SUCCESS) {
                fprintf(stdout, "Not Able to set the state 0x%x\n", status);
                return PROCMGR_E_INVALIDSTATE;
        }

        proc_entry_point = load_executable(imagePath, prog_argc,
                                (char**)(prog_argv.buf));

        /*---------------------------------------------------------------*/
        /* Did we get a valid program handle back from the loader?       */
        /*---------------------------------------------------------------*/
        if (!prog_handle)
        {
           fprintf(stderr, 
                   "<< D O L T >> FATAL: load_executable failed in "
                   "script. Terminating.\n");
           return PROCMGR_E_FAIL;
        }

        *entry_point = proc_entry_point;

#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMgr_load",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            *fileId = cmdArgs.fileId;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ProcMgr_load", status);

    /*! @retval PROCMGR_SUCCESS Operation successful */
    return status;
}


/*!
 *  @brief      Function to unload the previously loaded file on the slave
 *              processor.
 *
 *              This function unloads the file that was previously loaded on the
 *              slave processor through the ProcMgr_load API. It frees up any
 *              resources that were allocated during ProcMgr_load for this file.
 *              The fileId received from the load function must be passed to
 *              this function.
 *
 *  @param      handle     Handle to the ProcMgr object
 *  @param      fileId     ID of the loaded file to be unloaded
 *
 *  @sa         ProcMgr_load, ProcMgrDrvUsr_ioctl
 */
Int
ProcMgr_unload (ProcMgr_Handle handle, UInt32 fileId)
{
    Int                    status           = PROCMGR_SUCCESS;
    ProcMgr_Object *       procMgrHandle    = (ProcMgr_Object *) handle;
    ProcMgr_CmdArgsUnload  cmdArgs;

    GT_2trace (curTrace, GT_ENTER, "ProcMgr_unload", handle, fileId);

    GT_assert (curTrace, (handle != NULL));
    /* Cannot check for fileId because it is loader dependent. */

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        /*! @retval  PROCMGR_E_HANDLE Invalid NULL handle specified */
        status = PROCMGR_E_HANDLE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_unload",
                             status,
                             "Invalid NULL handle specified");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.handle = procMgrHandle->knlObject;
        cmdArgs.fileId = fileId;
        status = ProcMgrDrvUsr_ioctl (CMD_PROCMGR_UNLOAD, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMgr_unload",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ProcMgr_unload", status);

    /*! @retval PROCMGR_SUCCESS Operation successful */
    return status;
}


/*!
 *  @brief      Function to initialize the parameters for the ProcMgr start
 *              function.
 *
 *              This function can be called by the application to get their
 *              configuration parameter to ProcMgr_start filled in by the
 *              ProcMgr module with the default parameters. If the user does
 *              not wish to make any change in the default parameters, this API
 *              is not required to be called.
 *
 *  @param      handle   Handle to the ProcMgr object. If specified as NULL,
 *                       the default global configuration values are returned.
 *  @param      params   Pointer to the ProcMgr start params structure in
 *                       which the default params is to be returned.
 *
 *  @sa         ProcMgr_start, ProcMgrDrvUsr_ioctl
 */
Void
ProcMgr_getStartParams (ProcMgr_Handle handle, ProcMgr_StartParams * params)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                             status          = PROCMGR_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    ProcMgr_Object *                procMgrHandle   = (ProcMgr_Object *) handle;
    ProcMgr_CmdArgsGetStartParams   cmdArgs;

    GT_2trace (curTrace, GT_ENTER, "ProcMgr_getStartParams", handle, params);

    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (params == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_getStartParams",
                             PROCMGR_E_INVALIDARG,
                             "Argument of type (ProcMgr_StartParams *) passed "
                             "is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.handle = procMgrHandle->knlObject;
        cmdArgs.params = params;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        ProcMgrDrvUsr_ioctl (CMD_PROCMGR_GETSTARTPARAMS, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMgr_getStartParams",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "ProcMgr_getStartParams");
}


/*!
 *  @brief      Function to start the slave processor running.
 *
 *              Function to start execution of the loaded code on the slave
 *              from the entry point specified in the slave executable loaded
 *              earlier by call to ProcMgr_load ().
 *              After successful completion of this function, the ProcMgr
 *              instance is expected to be in the ProcMgr_State_Running state.
 *
 *  @param      handle   Handle to the ProcMgr object
 *  @param      params   Optional ProcMgr start parameters. If provided as NULL,
 *                       default parameters are used.
 *
 *  @sa         ProcMgr_stop, ProcMgrDrvUsr_ioctl
 */
Int
ProcMgr_start (ProcMgr_Handle        handle,
               UInt32                entry_point,
               ProcMgr_StartParams * params)
{
    Int                     status          = PROCMGR_SUCCESS;
    ProcMgr_Object *        procMgrHandle   = (ProcMgr_Object *) handle;
    ProcMgr_CmdArgsStart    cmdArgs;

    GT_2trace (curTrace, GT_ENTER, "ProcMgr_start", handle, params);

    GT_assert (curTrace, (handle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        /*! @retval  PROCMGR_E_HANDLE Invalid NULL handle specified */
        status = PROCMGR_E_HANDLE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_start",
                             status,
                             "Invalid NULL handle specified");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.handle = procMgrHandle->knlObject;
        cmdArgs.params = params;
        cmdArgs.entry_point = entry_point;
        status = ProcMgrDrvUsr_ioctl (CMD_PROCMGR_START, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMgr_start",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ProcMgr_start", status);

    /*! @retval PROCMGR_SUCCESS Operation successful */
    return status;
}


/*!
 *  @brief      Function to stop the slave processor.
 *
 *              Function to stop execution of the slave processor.
 *              Depending on the boot mode, after successful completion of this
 *              function, the ProcMgr instance may be in the ProcMgr_State_Reset
 *              state.
 *
 *  @param      handle   Handle to the ProcMgr object
 *
 *  @sa         ProcMgr_start, ProcMgrDrvUsr_ioctl
 */
Int
ProcMgr_stop (ProcMgr_Handle handle, ProcMgr_StopParams * params)
{
    Int                 status          = PROCMGR_SUCCESS;
    ProcMgr_Object *    procMgrHandle   = (ProcMgr_Object *) handle;
    ProcMgr_CmdArgsStop cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "ProcMgr_stop", handle);

    GT_assert (curTrace, (handle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        /*! @retval  PROCMGR_E_HANDLE Invalid NULL handle specified */
        status = PROCMGR_E_HANDLE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_stop",
                             status,
                             "Invalid NULL handle specified");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.handle = procMgrHandle->knlObject;
        cmdArgs.params = params;
        status = ProcMgrDrvUsr_ioctl (CMD_PROCMGR_STOP, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMgr_stop",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ProcMgr_stop", status);

    /*! @retval PROCMGR_SUCCESS Operation successful */
    return status;
}


/*!
 *  @brief      Function to get the current state of the slave Processor.
 *
 *              This function gets the state of the slave processor as
 *              maintained on the master Processor state machine. It does not
 *              go to the slave processor to get its actual state at the time
 *              when this API is called.
 *
 *  @param      handle   Handle to the ProcMgr object
 *
 *  @sa         Processor_getState, ProcMgrDrvUsr_ioctl
 */
ProcMgr_State
ProcMgr_getState (ProcMgr_Handle handle)
{
    Int                     status          = PROCMGR_SUCCESS;
    ProcMgr_State           state           = ProcMgr_State_Unknown;
    ProcMgr_Object *        procMgrHandle   = (ProcMgr_Object *) handle;
    ProcMgr_CmdArgsGetState cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "ProcMgr_getState", handle);

    GT_assert (curTrace, (handle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        /* No status set here since this function does not return status. */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_getState",
                             PROCMGR_E_HANDLE,
                             "Invalid NULL handle specified");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.handle = procMgrHandle->knlObject;
        status = ProcMgrDrvUsr_ioctl (CMD_PROCMGR_GETSTATE, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMgr_getState",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            state = cmdArgs.procMgrState;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ProcMgr_getState", state);

    /*! @retval Processor state */
    return state;
}

Int ProcMgr_setState (ProcMgr_State  state) 
{
    return PROCMGR_SUCCESS;
}


/*!
 *  @brief      Function to read from the slave processor's memory.
 *
 *              This function reads from the specified address in the
 *              processor's address space and copies the required number of
 *              bytes into the specified buffer.
 *              It returns the number of bytes actually read in the numBytes
 *              parameter.
 *
 *  @param      handle     Handle to the ProcMgr object
 *  @param      procAddr   Address in space processor's address space of the
 *                         memory region to read from.
 *  @param      numBytes   IN/OUT parameter. As an IN-parameter, it takes in the
 *                         number of bytes to be read. When the function
 *                         returns, this parameter contains the number of bytes
 *                         actually read.
 *  @param      buffer     User-provided buffer in which the slave processor's
 *                         memory contents are to be copied.
 *
 *  @sa         ProcMgr_write, ProcMgrDrvUsr_ioctl
 */
Int
ProcMgr_read (ProcMgr_Handle handle,
              UInt32         procAddr,
              UInt32 *       numBytes,
              Ptr            buffer)
{
    Int                  status         = PROCMGR_SUCCESS;
    ProcMgr_Object *     procMgrHandle  = (ProcMgr_Object *) handle;
    ProcMgr_CmdArgsRead  cmdArgs;

    GT_4trace (curTrace, GT_ENTER, "ProcMgr_read",
               handle, procAddr, numBytes, buffer);

    GT_assert (curTrace, (handle   != NULL));
    GT_assert (curTrace, (procAddr != 0));
    GT_assert (curTrace, (numBytes != NULL));
    GT_assert (curTrace, (buffer   != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        /*! @retval  PROCMGR_E_HANDLE Invalid NULL handle specified */
        status = PROCMGR_E_HANDLE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_read",
                             status,
                             "Invalid NULL handle specified");
    }
    else if (procAddr == 0u) {
        /*! @retval  PROCMGR_E_INVALIDARG Invalid value 0 provided for
                     argument procAddr */
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_read",
                             status,
                             "Invalid value 0 provided for argument procAddr");
    }
    else if (numBytes == NULL) {
        /*! @retval  PROCMGR_E_INVALIDARG Invalid value NULL provided for
                     argument numBytes */
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                           GT_4CLASS,
                           "ProcMgr_read",
                           status,
                           "Invalid value NULL provided for argument numBytes");
    }
    else if (buffer == NULL) {
        /*! @retval  PROCMGR_E_INVALIDARG Invalid value NULL provided for
                     argument buffer */
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_read",
                             status,
                             "Invalid value NULL provided for argument buffer");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.handle = procMgrHandle->knlObject;
        cmdArgs.procAddr = procAddr;
        cmdArgs.numBytes = *numBytes;
        cmdArgs.buffer = buffer;
        status = ProcMgrDrvUsr_ioctl (CMD_PROCMGR_READ, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMgr_read",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            /* Return number of bytes actually read. */
            *numBytes = cmdArgs.numBytes;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ProcMgr_read", status);

    /*! @retval PROCMGR_SUCCESS Operation successful */
    return status;
}


/*!
 *  @brief      Function to write into the slave processor's memory.
 *
 *              This function writes into the specified address in the
 *              processor's address space and copies the required number of
 *              bytes from the specified buffer.
 *              It returns the number of bytes actually written in the numBytes
 *              parameter.
 *
 *  @param      handle     Handle to the ProcMgr object
 *  @param      procAddr   Address in space processor's address space of the
 *                         memory region to write into.
 *  @param      numBytes   IN/OUT parameter. As an IN-parameter, it takes in the
 *                         number of bytes to be written. When the function
 *                         returns, this parameter contains the number of bytes
 *                         actually written.
 *  @param      buffer     User-provided buffer from which the data is to be
 *                         written into the slave processor's memory.
 *
 *  @sa         ProcMgr_read, ProcMgrDrvUsr_ioctl
 */
Int
ProcMgr_write (ProcMgr_Handle handle,
               UInt32         procAddr,
               UInt32 *       numBytes,
               Ptr            buffer)
{
    Int                     status          = PROCMGR_SUCCESS;
    ProcMgr_Object *        procMgrHandle   = (ProcMgr_Object *) handle;
    ProcMgr_CmdArgsWrite    cmdArgs;

    GT_4trace (curTrace, GT_ENTER, "ProcMgr_write",
               handle, procAddr, numBytes, buffer);

    GT_assert (curTrace, (handle   != NULL));
    GT_assert (curTrace, (procAddr != 0));
    GT_assert (curTrace, (numBytes != NULL));
    GT_assert (curTrace, (buffer   != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        /*! @retval  PROCMGR_E_HANDLE Invalid NULL handle specified */
        status = PROCMGR_E_HANDLE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_write",
                             status,
                             "Invalid NULL handle specified");
    }
    else if (procAddr == 0u) {
        /*! @retval  PROCMGR_E_INVALIDARG Invalid value 0 provided for
                     argument procAddr */
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_write",
                             status,
                             "Invalid value 0 provided for argument procAddr");
    }
    else if (numBytes == NULL) {
        /*! @retval  PROCMGR_E_INVALIDARG Invalid value NULL provided for
                     argument numBytes */
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                           GT_4CLASS,
                           "ProcMgr_write",
                           status,
                           "Invalid value NULL provided for argument numBytes");
    }
    else if (buffer == NULL) {
        /*! @retval  PROCMGR_E_INVALIDARG Invalid value NULL provided for
                     argument buffer */
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_write",
                             status,
                             "Invalid value NULL provided for argument buffer");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.handle = procMgrHandle->knlObject;
        cmdArgs.procAddr = procAddr;
        cmdArgs.numBytes = *numBytes;
        cmdArgs.buffer = buffer;
        status = ProcMgrDrvUsr_ioctl (CMD_PROCMGR_WRITE, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMgr_write",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            /* Return number of bytes actually written. */
            *numBytes = cmdArgs.numBytes;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ProcMgr_write", status);

    /*! @retval PROCMGR_SUCCESS Operation successful */
    return status;
}


/*!
 *  @brief      Function to perform device-dependent operations.
 *
 *              This function performs control operations supported by the
 *              as exposed directly by the specific implementation of the
 *              Processor interface. These commands and their specific argument
 *              types are used with this function.
 *
 *  @param      handle     Handle to the ProcMgr object
 *  @param      cmd        Device specific processor command
 *  @param      arg        Arguments specific to the type of command.
 *
 *  @sa         ProcMgrDrvUsr_ioctl
 */
Int
ProcMgr_control (ProcMgr_Handle handle, Int32 cmd, Ptr arg)
{
    Int                     status          = PROCMGR_SUCCESS;
    ProcMgr_Object *        procMgrHandle   = (ProcMgr_Object *) handle;
    ProcMgr_CmdArgsControl  cmdArgs;

    GT_3trace (curTrace, GT_ENTER, "ProcMgr_control", handle, cmd, arg);

    GT_assert (curTrace, (handle != NULL));
    /* cmd and arg can be 0/NULL, so cannot check for validity. */

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        /*! @retval  PROCMGR_E_HANDLE Invalid NULL handle specified */
        status = PROCMGR_E_HANDLE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_control",
                             status,
                             "Invalid NULL handle specified");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.handle = procMgrHandle->knlObject;
        cmdArgs.cmd = cmd;
        cmdArgs.arg = arg;
        status = ProcMgrDrvUsr_ioctl (CMD_PROCMGR_CONTROL, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMgr_control",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ProcMgr_control", status);

    /*! @retval PROCMGR_SUCCESS Operation successful */
    return status;
}


/*!
 *  @brief      Function to translate between two types of address spaces.
 *
 *              This function translates addresses between two types of address
 *              spaces. The destination and source address types are indicated
 *              through parameters specified in this function.
 *
 *  @param      handle      Handle to the ProcMgr object
 *  @param      dstAddr     Return parameter: Pointer to receive the translated
 *                          address.
 *  @param      dstAddrType Destination address type requested
 *  @param      srcAddr     Source address in the source address space
 *  @param      srcAddrType Source address type
 *
 *  @sa         ProcMgrDrvUsr_ioctl
 */
Int
ProcMgr_translateAddr (ProcMgr_Handle   handle,
                       Ptr *            dstAddr,
                       ProcMgr_AddrType dstAddrType,
                       Ptr              srcAddr,
                       ProcMgr_AddrType srcAddrType)
{
    Int                             status          = PROCMGR_SUCCESS;
    ProcMgr_Object *                procMgrHandle   = (ProcMgr_Object *) handle;
    UInt32                          fmAddrBase      = (UInt32) NULL;
    UInt32                          toAddrBase      = (UInt32) NULL;
    Bool                            found           = FALSE;
    ProcMgr_AddrInfo *              memEntry;
    UInt16                          i;

    GT_5trace (curTrace, GT_ENTER, "ProcMgr_translateAddr",
               handle, dstAddr, dstAddrType, srcAddr, srcAddrType);

    GT_assert (curTrace, (handle        != NULL));
    GT_assert (curTrace, (dstAddr       != NULL));
    GT_assert (curTrace, (dstAddrType   < ProcMgr_AddrType_EndValue));
    GT_assert (curTrace, (srcAddr       != NULL));
    GT_assert (curTrace, (srcAddrType   < ProcMgr_AddrType_EndValue));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        /*! @retval  PROCMGR_E_HANDLE Invalid NULL handle specified */
        status = PROCMGR_E_HANDLE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_translateAddr",
                             status,
                             "Invalid NULL handle specified");
    }
    else if (dstAddr == NULL) {
        /*! @retval  PROCMGR_E_INVALIDARG Invalid value NULL provided for
                     argument dstAddr */
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                            GT_4CLASS,
                            "ProcMgr_translateAddr",
                            status,
                            "Invalid value NULL provided for argument dstAddr");
    }
    else if (dstAddrType >= ProcMgr_AddrType_EndValue) {
        /*! @retval  PROCMGR_E_INVALIDARG Invalid value provided for
                     argument dstAddrType */
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_translateAddr",
                             status,
                             "Invalid value provided for argument dstAddrType");
    }
    else if (srcAddr == NULL) {
        /*! @retval  PROCMGR_E_INVALIDARG Invalid value NULL provided for
                     argument srcAddr */
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                            GT_4CLASS,
                            "ProcMgr_translateAddr",
                            status,
                            "Invalid value NULL provided for argument srcAddr");
    }
    else if (srcAddrType >= ProcMgr_AddrType_EndValue) {
        /*! @retval  PROCMGR_E_INVALIDARG Invalid value provided for
                     argument srcAddrType */
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_translateAddr",
                             status,
                             "Invalid value provided for argument srcAddrType");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        *dstAddr = NULL;
        for (i = 0 ; i < procMgrHandle->numMemEntries ; i++) {
            GT_assert (curTrace, (i < PROCMGR_MAX_MEMORY_REGIONS));
            memEntry = &(procMgrHandle->memEntries [i]);
            /* Determine which way to convert */
            fmAddrBase = memEntry->addr [srcAddrType];
            toAddrBase = memEntry->addr [dstAddrType];
            GT_3trace (curTrace,
                       GT_3CLASS,
                       "    ProcMgr_translateAddr: Entry %d\n"
                       "        Source address base [0x%x]\n"
                       "        Dest   address base [0x%x]\n",
                       i,
                       fmAddrBase,
                       toAddrBase);
            if (IS_RANGE_VALID ((UInt32) srcAddr,
                                fmAddrBase,
                                (fmAddrBase + memEntry->size))) {
                GT_2trace (curTrace,
                           GT_2CLASS,
                           "    ProcMgr_translateAddr: Found entry!\n"
                           "        Region address base [0x%x]\n"
                           "        Region size         [0x%x]\n",
                           fmAddrBase,
                           memEntry->size);
                found = TRUE;
                *dstAddr = (Ptr) (((UInt32) srcAddr - fmAddrBase) + toAddrBase);
                break;
            }
        }

        /* This check must not be removed even with build optimize. */
        if (found == FALSE) {
            /*! @retval PROCMGR_E_TRANSLATE Failed to translate address. */
            status = PROCMGR_E_TRANSLATE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMgr_translateAddr",
                                 status,
                                 "Failed to translate address");
        }
        else {
            GT_2trace (curTrace,
                       GT_1CLASS,
                       "    ProcMgr_translateAddr: srcAddr [0x%x] "
                       "dstAddr [0x%x]\n",
                       srcAddr,
                      *dstAddr);
        }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ProcMgr_translateAddr", status);

    /*! @retval PROCMGR_SUCCESS Operation successful */
    return status;
}


/*!
 *  @brief      Function to retrieve the target address of a symbol from the
 *              specified file.
 *
 *  @param      handle   Handle to the ProcMgr object
 *  @param      fileId   ID of the file received from the load function
 *  @param      symName  Name of the symbol
 *  @param      symValue Return parameter: Symbol address
 *
 *  @sa         ProcMgrDrvUsr_ioctl
 */
Int
ProcMgr_getSymbolAddress (ProcMgr_Handle handle,
                          UInt32         fileId,
                          String         symbolName,
                          UInt32 *       symValue)
{
    Int                             status          = PROCMGR_SUCCESS;
    ProcMgr_Object *                procMgrHandle   = (ProcMgr_Object *) handle;
    ProcMgr_CmdArgsGetSymbolAddress cmdArgs;

    GT_4trace (curTrace, GT_ENTER, "ProcMgr_getSymbolAddress",
               handle, fileId, symbolName, symValue);

    GT_assert (curTrace, (handle      != NULL));
    /* fileId may be 0, so no check for fileId. */
    GT_assert (curTrace, (symbolName  != NULL));
    GT_assert (curTrace, (symValue    != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        /*! @retval  PROCMGR_E_HANDLE Invalid NULL handle specified */
        status = PROCMGR_E_HANDLE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_getSymbolAddress",
                             status,
                             "Invalid NULL handle specified");
    }
    else if (symbolName == NULL) {
        /*! @retval  PROCMGR_E_INVALIDARG Invalid value NULL provided for
                     argument symbolName */
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                         GT_4CLASS,
                         "ProcMgr_getSymbolAddress",
                         status,
                         "Invalid value NULL provided for argument symbolName");
    }
    else if (symValue == NULL) {
        /*! @retval  PROCMGR_E_INVALIDARG Invalid value NULL provided for
                     argument symValue */
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                         GT_4CLASS,
                         "ProcMgr_getSymbolAddress",
                         status,
                         "Invalid value NULL provided for argument symValue");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.handle = procMgrHandle->knlObject;
        cmdArgs.fileId = fileId;
        cmdArgs.symbolName = symbolName;
        cmdArgs.symValue = 0u; /* Return parameter. */
        status = ProcMgrDrvUsr_ioctl (CMD_PROCMGR_GETSYMBOLADDRESS, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMgr_getSymbolAddress",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            /* Return symbol address. */
            *symValue = cmdArgs.symValue;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ProcMgr_getSymbolAddress", status);

    /*! @retval PROCMGR_SUCCESS Operation successful */
    return status;
}


/*!
 *  @brief      Function to map address to slave address space.
 *
 *              This function maps the provided slave address to a host address
 *              and returns the mapped address and size.
 *
 *  @param      handle      Handle to the ProcMgr object
 *  @param      procAddr    Slave address to be mapped
 *  @param      size        Size (in bytes) of region to be mapped
 *  @param      mappedAddr  Return parameter: Mapped address in host address
 *                          space
 *  @param      mappedSize  Return parameter: Mapped size
 *  @param      type        Type of mapping.
 *
 *  @sa         ProcMgrDrvUsr_ioctl
 */
Int
ProcMgr_map (ProcMgr_Handle     handle,
             UInt32             procAddr,
             UInt32             size,
             UInt32 *           mappedAddr,
             UInt32 *           mappedSize,
             ProcMgr_MapType    type)
{
    Int                 status          = PROCMGR_SUCCESS;
    ProcMgr_Object *    procMgrHandle   = (ProcMgr_Object *) handle;
    ProcMgr_CmdArgsMap  cmdArgs;

    GT_5trace (curTrace, GT_ENTER, "ProcMgr_map",
               handle, procAddr, size, mappedAddr, mappedSize);

    GT_assert (curTrace, (handle        != NULL));
    GT_assert (curTrace, (procAddr      != 0));
    GT_assert (curTrace, (size          != 0));
    GT_assert (curTrace, (mappedAddr    != NULL));
    GT_assert (curTrace, (mappedSize    != NULL));
    GT_assert (curTrace, (type < ProcMgr_MapType_EndValue));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        /*! @retval  PROCMGR_E_HANDLE Invalid NULL handle specified */
        status = PROCMGR_E_HANDLE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_map",
                             status,
                             "Invalid NULL handle specified");
    }
    else if (procAddr == 0u) {
        /*! @retval  PROCMGR_E_INVALIDARG Invalid value 0 provided for
                     argument procAddr */
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_map",
                             status,
                             "Invalid value 0 provided for argument procAddr");
    }
    else if (size == 0u) {
        /*! @retval  PROCMGR_E_INVALIDARG Invalid value 0 provided for
                     argument size */
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_map",
                             status,
                             "Invalid value 0 provided for argument size");
    }
    else if (mappedAddr == NULL) {
        /*! @retval  PROCMGR_E_INVALIDARG Invalid value NULL provided for
                     argument mappedAddr */
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                         GT_4CLASS,
                         "ProcMgr_map",
                         status,
                         "Invalid value NULL provided for argument mappedAddr");
    }
    else if (mappedSize == NULL) {
        /*! @retval  PROCMGR_E_INVALIDARG Invalid value NULL provided for
                     argument mappedSize */
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                         GT_4CLASS,
                         "ProcMgr_map",
                         status,
                         "Invalid value NULL provided for argument mappedSize");
    }
    else if (type >= ProcMgr_MapType_EndValue) {
        /*! @retval  PROCMGR_E_INVALIDARG Invalid value provided for
                     argument type */
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_map",
                             status,
                             "Invalid value provided for argument type");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.handle = procMgrHandle->knlObject;
        cmdArgs.procAddr = procAddr;
        cmdArgs.size = size;
        cmdArgs.type = type;
        cmdArgs.mappedAddr = 0u; /* Return parameter. */
        cmdArgs.mappedSize = 0u; /* Return parameter. */
        status = ProcMgrDrvUsr_ioctl (CMD_PROCMGR_MAP, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMgr_map",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            /* Return mapped address and size. */
            *mappedAddr = cmdArgs.mappedAddr;
            *mappedSize = cmdArgs.mappedSize;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ProcMgr_map", status);

    /*! @retval PROCMGR_SUCCESS Operation successful */
    return status;
}


/*!
 *  @brief      Function to unmap address from the slave address space.
 *
 *              This function unmaps the already mapped slave address.
 *
 *  @param      handle      Handle to the ProcMgr object
 *  @param      mappedAddr  Mapped address in host address space
 *
 *  @sa         ProcMgrDrvUsr_ioctl
 */
Int
ProcMgr_unmap (ProcMgr_Handle   handle,
               UInt32           mappedAddr)
{
    Int                   status          = PROCMGR_SUCCESS;
    ProcMgr_Object *      procMgrHandle   = (ProcMgr_Object *) handle;
    ProcMgr_CmdArgsUnMap  cmdArgs;

    Osal_printf ("Enter ProcMgr_unmap\n");
#if defined(SYSLINK_BUILD_DEBUG)
    GT_assert (curTrace, (handle        != NULL));
    GT_assert (curTrace, (mappedAddr    != NULL));
#endif
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        /*! @retval  PROCMGR_E_HANDLE Invalid NULL handle specified */
        status = PROCMGR_E_HANDLE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_map",
                             status,
                             "Invalid NULL handle specified");
    }
    else if ((Void *)mappedAddr == NULL) {
        /*! @retval  PROCMGR_E_INVALIDARG Invalid value NULL provided for
                     argument mappedAddr */
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                         GT_4CLASS,
                         "ProcMgr_map",
                         status,
                         "Invalid value NULL provided for argument mappedAddr");
    }
    else {
        cmdArgs.handle = procMgrHandle->knlObject;
        cmdArgs.mappedAddr = mappedAddr; /* Return parameter. */
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        status = ProcMgrDrvUsr_ioctl (CMD_PROCMGR_UNMAP, &cmdArgs);
    }

    Osal_printf ("Exit ProcMgr_unmap \n" );
    return status;
}


/*!
 *  @brief      Function that registers for notification when the slave
 *              processor transitions to any of the states specified.
 *
 *              This function allows the user application to register for
 *              changes in processor state and take actions accordingly.
 *
 *  @param      handle      Handle to the ProcMgr object
 *  @param      fxn         Handling function to be registered.
 *  @param      args        Optional arguments associated with the handler fxn.
 *  @param      state       Array of target states for which registration is
 *                          required.
 *
 *  @sa         ProcMgrDrvUsr_ioctl
 */
Int
ProcMgr_registerNotify (ProcMgr_Handle      handle,
                        ProcMgr_CallbackFxn fxn,
                        Ptr                 args,
                        ProcMgr_State       state [])
{
    Int                             status          = PROCMGR_SUCCESS;
    ProcMgr_Object *                procMgrHandle   = (ProcMgr_Object *) handle;
    ProcMgr_CmdArgsRegisterNotify   cmdArgs;

    GT_4trace (curTrace, GT_ENTER, "ProcMgr_registerNotify",
               handle, fxn, args, state);

    GT_assert (curTrace, (handle        != NULL));
    GT_assert (curTrace, (fxn           != 0));
    /* args is optional and may be NULL. */

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        /*! @retval  PROCMGR_E_HANDLE Invalid NULL handle specified */
        status = PROCMGR_E_HANDLE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_registerNotify",
                             status,
                             "Invalid NULL handle specified");
    }
    else if (fxn == NULL) {
        /*! @retval  PROCMGR_E_INVALIDARG Invalid value NULL provided for
                     argument fxn */
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_registerNotify",
                             status,
                             "Invalid value NULL provided for argument fxn");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.handle = procMgrHandle->knlObject;
        cmdArgs.fxn = fxn;
        cmdArgs.args = args;
        /* State TBD. */
        status = ProcMgrDrvUsr_ioctl (CMD_PROCMGR_REGISTERNOTIFY, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMgr_registerNotify",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ProcMgr_registerNotify", status);

    /*! @retval PROCMGR_SUCCESS Operation successful */
    return status;
}


/*!
 *  @brief      Function that returns information about the characteristics of
 *              the slave processor.
 *
 *  @param      handle      Handle to the ProcMgr object
 *  @param      procInfo    Pointer to the ProcInfo object to be populated.
 *
 *  @sa         ProcMgrDrvUsr_ioctl
 */
Int
ProcMgr_getProcInfo (ProcMgr_Handle     handle,
                     ProcMgr_ProcInfo * procInfo)
{
    Int                         status          = PROCMGR_SUCCESS;
    ProcMgr_Object *            procMgrHandle   = (ProcMgr_Object *) handle;
    ProcMgr_CmdArgsGetProcInfo  cmdArgs;

    GT_2trace (curTrace, GT_ENTER, "ProcMgr_getProcInfo", handle, procInfo);

    GT_assert (curTrace, (handle    != NULL));
    GT_assert (curTrace, (procInfo  != 0));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        /*! @retval  PROCMGR_E_HANDLE Invalid NULL handle specified */
        status = PROCMGR_E_HANDLE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMgr_getProcInfo",
                             status,
                             "Invalid NULL handle specified");
    }
    else if (procInfo == NULL) {
        /*! @retval  PROCMGR_E_INVALIDARG Invalid value NULL provided for
                     argument procInfo */
        status = PROCMGR_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                           GT_4CLASS,
                           "ProcMgr_getProcInfo",
                           status,
                           "Invalid value NULL provided for argument procInfo");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.handle = procMgrHandle->knlObject;
        cmdArgs.procInfo = procInfo;
        status = ProcMgrDrvUsr_ioctl (CMD_PROCMGR_GETPROCINFO, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMgr_getProcInfo",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            /* Update user virtual address in memory configuration. */
            procInfo->numMemEntries = procMgrHandle->numMemEntries;
            Memory_copy (&(procInfo->memEntries),
                         &(procMgrHandle->memEntries),
                         sizeof (procMgrHandle->memEntries));
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ProcMgr_getProcInfo", status);

    /*! @retval PROCMGR_SUCCESS Operation successful */
    return status;
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
