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
 *  @file   GateHWSpinLock.c
 *
 *  @brief      Gate based on Peterson Algorithm.
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

/* Module level headers */
#include <GateHWSpinLock.h>
#include <GateHWSpinLockDrv.h>
#include <GateHWSpinLockDrvDefs.h>


#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 * Structures & Enums
 * =============================================================================
 */
/* Structure defining object for the Gate Peterson */
typedef struct GateHWSpinLock_Obj_tag {
    Ptr         knlObject;
    /*!< Pointer to the kernel-side ProcMgr object. */
} GateHWSpinLock_Obj;

/*!
 *  @brief  GateHWSpinLock Module state object
 */
typedef struct GateHWSpinLock_ModuleObject_tag {
    UInt32              setupRefCount;
    /*!< Reference count for number of times setup/destroy were called in this
         process. */
    GateHWSpinLock_Config cfg;
    /* Current config */
} GateHWSpinLock_ModuleObject;


/* =============================================================================
 *  Globals
 * =============================================================================
 */
/*!
 *  @var    GateHWSpinLock_state
 *
 *  @brief  ProcMgr state object variable
 */
#if !defined(SYSLINK_BUILD_DEBUG)
static
#endif /* if !defined(SYSLINK_BUILD_DEBUG) */
GateHWSpinLock_ModuleObject GateHWSpinLock_state =
{
    .setupRefCount = 0
};


/* =============================================================================
 * APIS
 * =============================================================================
 */
/*!
 *  @brief      Get the default configuration for the GateHWSpinLock module.
 *
 *              This function can be called by the application to get their
 *              configuration parameter to GateHWSpinLock_setup filled in by
 *              the GateHWSpinLock module with the default parameters. If the
 *              user does not wish to make any change in the default parameters,
 *              this API is not required to be called.
 *
 *  @param      cfgParams  Pointer to the GateHWSpinLock module configuration
 *                         structure in which the default config is to be
 *                         returned.
 *
 *  @sa         GateHWSpinLock_setup
 */
Void
GateHWSpinLock_getConfig (GateHWSpinLock_Config * cfg)
{
    Int                  status = GATEHWSPINLOCK_SUCCESS;
    GateHWSpinLockDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "GateHWSpinLock_getConfig", cfg);

    GT_assert (curTrace, (cfg != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (cfg == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateHWSpinLock_getConfig",
                             GATEHWSPINLOCK_E_INVALIDARG,
                             "Argument of type (GateHWSpinLock_Config *) passed "
                             "is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Temporarily open the handle to get the configuration. */
        status = GateHWSpinLockDrv_open ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "GateHWSpinLock_getConfig",
                                 status,
                                 "Failed to open driver handle!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            cmdArgs.args.getConfig.config = cfg;
            status = GateHWSpinLockDrv_ioctl (CMD_GATEHWSPINLOCK_GETCONFIG,
                                                &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                  GT_4CLASS,
                                  "GateHWSpinLock_getConfig",
                                  status,
                                  "API (through IOCTL) failed on kernel-side!");

            }
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Close the driver handle. */
        GateHWSpinLockDrv_close ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_ENTER, "GateHWSpinLock_getConfig");


}


/*!
 *  @brief      Setup the GateHWSpinLock module.
 *
 *              This function sets up the GateHWSpinLock module. This function
 *              must be called before any other instance-level APIs can be
 *              invoked.
 *              Module-level configuration needs to be provided to this
 *              function. If the user wishes to change some specific config
 *              parameters, then GateHWSpinLock_getConfig can be called to get
 *              the configuration filled with the default values. After this,
 *              only the required configuration values can be changed. If the
 *              user does not wish to make any change in the default parameters,
 *              the application can simply call GateHWSpinLock_setup with NULL
 *              parameters. The default parameters would get automatically used.
 *
 *  @param      cfg   Optional GateHWSpinLock module configuration. If provided
 *                    as NULL, default configuration is used.
 *
 *  @sa         GateHWSpinLock_destroy, GateHWSpinLock_getConfig
 */
Int32
GateHWSpinLock_setup (const GateHWSpinLock_Config * config)
{
    Int                  status = GATEHWSPINLOCK_SUCCESS;
    GateHWSpinLockDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "GateHWSpinLock_setup", config);

    /* TBD: Protect from multiple threads. */
    GateHWSpinLock_state.setupRefCount++;
    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */
    if (GateHWSpinLock_state.setupRefCount > 1) {
        /*! @retval GATEHWSPINLOCK_S_ALREADYSETUP Success: ProcMgr module has
                    been already setup in this process */
        status = GATEHWSPINLOCK_S_ALREADYSETUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "ProcMgr module has been already setup in this process.\n"
                   "    RefCount: [%d]\n",
                   GateHWSpinLock_state.setupRefCount);
    }
    else {
        /* Open the driver handle. */
        status = GateHWSpinLockDrv_open ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "GateHWSpinLock_setup",
                                 status,
                                 "Failed to open driver handle!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            cmdArgs.args.setup.config = (GateHWSpinLock_Config *) config;
            status = GateHWSpinLockDrv_ioctl (CMD_GATEHWSPINLOCK_SETUP,
                                                &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "GateHWSpinLock_setup",
                                     status,
                                     "API (through IOCTL) failed on kernel-side!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                Memory_copy ((Ptr) &GateHWSpinLock_state.cfg,
                              (Ptr) config,
                              sizeof (GateHWSpinLock_Config));
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            }
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

    GT_1trace (curTrace, GT_LEAVE, "GateHWSpinLock_setup", status);

    /*! @retval GATEHWSPINLOCK_SUCCESS Operation successful */
    return (status);
}


/*!
 *  @brief      Function to destroy the GateHWSpinLock module.
 *
 *  @sa         GateHWSpinLock_setup
 */
Int32
GateHWSpinLock_destroy (void)
{
    Int                     status = GATEHWSPINLOCK_SUCCESS;
    GateHWSpinLockDrv_CmdArgs    cmdArgs;

    GT_0trace (curTrace, GT_ENTER, "GateHWSpinLock_destroy");

    /* TBD: Protect from multiple threads. */
    GateHWSpinLock_state.setupRefCount--;
    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */
    if (GateHWSpinLock_state.setupRefCount > 1) {
        /*! @retval GATEHWSPINLOCK_S_ALREADYSETUP Success: ProcMgr module has
                    been already setup in this process */
        status = GATEHWSPINLOCK_S_ALREADYSETUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "ProcMgr module has been already setup in this process.\n"
                   "    RefCount: [%d]\n",
                   GateHWSpinLock_state.setupRefCount);
    }
    else {
        status = GateHWSpinLockDrv_ioctl (CMD_GATEHWSPINLOCK_DESTROY, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "GateHWSpinLock_destroy",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

    /* Close the driver handle. */
    GateHWSpinLockDrv_close ();

    GT_1trace (curTrace, GT_LEAVE, "GateHWSpinLock_destroy", status);

    return status;
}


/*!
 *  @brief      Initialize this config-params structure with supplier-specified
 *              defaults before instance creation.
 *
 *  @param      handle  If specified as NULL, default parameters are returned.
 *                      If not NULL, the parameters as configured for this
 *                      instance are returned.
 *  @param      params  Instance config-params structure.
 *
 *  @sa         GateHWSpinLock_create
 */
Void
GateHWSpinLock_Params_init (GateHWSpinLock_Handle   handle,
                          GateHWSpinLock_Params * params)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                     status = GATEHWSPINLOCK_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    GateHWSpinLockDrv_CmdArgs cmdArgs;

    GT_2trace (curTrace, GT_ENTER, "GateHWSpinLock_Params_init", handle, params);

    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (GateHWSpinLock_state.setupRefCount == 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateHWSpinLock_create",
                             GATEHWSPINLOCK_E_INVALIDSTATE,
                             "Modules is invalidstate!");
    }
    else if (params == NULL) {
        /* No retVal comment since this is a Void function. */
        status = GATEHWSPINLOCK_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateHWSpinLock_Params_init",
                             status,
                             "Argument of type (GateHWSpinLock_Params *) passed "
                             "is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        if (handle != NULL) {
            cmdArgs.args.ParamsInit.handle =
                                     ((GateHWSpinLock_Obj *) handle)->knlObject;
        }
        else {
            cmdArgs.args.ParamsInit.handle = handle;
        }
        cmdArgs.args.ParamsInit.params = params;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        GateHWSpinLockDrv_ioctl (CMD_GATEHWSPINLOCK_PARAMS_INIT, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "GateHWSpinLock_Params_init",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "GateHWSpinLock_Params_init");


}


/*!
 *  @brief      Creates a new instance of GateHWSpinLock module.
 *
 *  @param      params  Instance config-params structure.
 *
 *  @sa         GateHWSpinLock_delete, GateHWSpinLock_open, GateHWSpinLock_close
 */
GateHWSpinLock_Handle
GateHWSpinLock_create (const GateHWSpinLock_Params * params)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                     status = 0;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    GateHWSpinLock_Handle     handle = NULL;
    GateHWSpinLockDrv_CmdArgs cmdArgs;
    GateHWSpinLock_Obj *      obj;

    GT_1trace (curTrace, GT_ENTER, "GateHWSpinLock_create", params);

    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (GateHWSpinLock_state.setupRefCount == 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateHWSpinLock_create",
                             GATEHWSPINLOCK_E_INVALIDSTATE,
                             "Modules is invalidstate!");
    }
    else if (params == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateHWSpinLock_create",
                             GATEHWSPINLOCK_E_INVALIDARG,
                             "params passed is NULL!");
    }
    else if (params->lockNum == -1u) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateHWSpinLock_create",
                             GATEHWSPINLOCK_E_INVALIDARG,
                             "Please provide a valid lock number!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.create.params = (GateHWSpinLock_Params *) params;
        if (params->name != NULL) {
            cmdArgs.args.create.nameLen = String_len (params->name) + 1;
        }
        else {
            cmdArgs.args.create.nameLen = 0;
        }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        GateHWSpinLockDrv_ioctl (CMD_GATEHWSPINLOCK_CREATE, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "GateHWSpinLock_create",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            /* Allocate memory for the generic handle */
            handle = (GateHWSpinLock_Handle) Memory_calloc (NULL,
                                               sizeof (GateHWSpinLock_Object),
                                               0);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (handle == NULL) {
                /*! @retval NULL Memory allocation failed for handle */
                status = GATEHWSPINLOCK_E_MEMORY;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "GateHWSpinLock_create",
                                     status,
                                     "Memory allocation failed for handle!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                /* Allocate memory for the specific object */
                obj = (GateHWSpinLock_Obj *) Memory_calloc (NULL,
                                                  sizeof (GateHWSpinLock_Obj),
                                                  0);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
                if (obj == NULL) {
                    /*! @retval NULL Memory allocation failed for handle */
                    status = GATEHWSPINLOCK_E_MEMORY;
                    GT_setFailureReason (curTrace,
                                         GT_4CLASS,
                                         "GateHWSpinLock_create",
                                         status,
                                         "Memory allocation failed for the"
                                         " internal object!");
                }
                else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                    /* Set pointer to kernel object into the user handle. */
                    obj->knlObject = cmdArgs.args.create.handle;
                    handle->obj   = obj;/* Assign GateHWSpinLock internal object */
                    handle->enter = (Lock_enter) &GateHWSpinLock_enter;
                    handle->leave = (Lock_leave) &GateHWSpinLock_leave;
                    handle->getKnlHandle = (Lock_getKnlHandle)
                                           &GateHWSpinLock_getKnlHandle;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
                }
             }
         }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "GateHWSpinLock_create", handle);

    /*! @retval valid-handle Operation successful*/
    /*! @retval NULL Operation failed */
    return handle;
}


/*!
 *  @brief      Deletes a instance of GateHWSpinLock module.
 *
 *  @param      handlePtr  Pointer to handle to previously created instance. The
 *                         pointer is reset upon successful completion.
 *
 *  @sa         GateHWSpinLock_create, GateHWSpinLock_open, GateHWSpinLock_close
 */
Int32
GateHWSpinLock_delete (GateHWSpinLock_Handle * handlePtr)
{
    Int32                   status = GATEHWSPINLOCK_SUCCESS;
    GateHWSpinLockDrv_CmdArgs cmdArgs;
    GateHWSpinLock_Obj *      obj;

    GT_1trace (curTrace, GT_ENTER, "GateHWSpinLock_delete", handlePtr);

    GT_assert (curTrace, (handlePtr != NULL));
    GT_assert (curTrace, ((handlePtr != NULL) && (*handlePtr != NULL)));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (GateHWSpinLock_state.setupRefCount == 0) {
        /*! @retval GATEHWSPINLOCK_E_INVALIDSTATE Modules is invalidstate*/
        status = GATEHWSPINLOCK_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateHWSpinLock_delete",
                             status,
                             "Modules is invalidstate!");
    }
    else if (handlePtr == NULL) {
        /*! @retval GATEHWSPINLOCK_E_INVALIDARG handlePtr passed is NULL*/
        status = GATEHWSPINLOCK_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateHWSpinLock_delete",
                             status,
                             "handlePtr passed is NULL!");
    }
    else if (*handlePtr == NULL) {
        /*! @retval GATEHWSPINLOCK_E_INVALIDARG *handlePtr passed is NULL*/
        status = GATEHWSPINLOCK_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateHWSpinLock_delete",
                             status,
                             "*handlePtr passed is NULL!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        obj = (GateHWSpinLock_Obj *) ((*handlePtr)->obj);
        cmdArgs.args.deleteInstance.handle = obj->knlObject;
        status = GateHWSpinLockDrv_ioctl (CMD_GATEHWSPINLOCK_DELETE, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "HeapBuf_delete",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            Memory_free (NULL, obj, sizeof (GateHWSpinLock_Obj));
            Memory_free (NULL, *handlePtr, sizeof (GateHWSpinLock_Object));
            *handlePtr = NULL;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "GateHWSpinLock_delete", status);

    /*! @retval GATEHWSPINLOCK_SUCCESS Operation successful */
    return status;
}


/*!
 *  @brief      Opens a created instance of GateHWSpinLock module. Once an
 *              instance is created, an open can be performed. The open is used
 *              to gain access to the same GateHWSpinLock instance. Generally an
 *              instance is created on one processor and opened on the other
 *              processor.<br>
 *              The open accepts a pointer to a GateHWSpinLock handle whose value
 *              is set if the open is successful. A status code is returned
 *              indicating whether the open was successful. If the open was not
 *              successful, an error code indicates that the gate was has not
 *              been created yet.<br>
 *              There are two ways to open a GateHWSpinLock instance that has been
 *              created either locally or remotely:<br>
 *              (1) Supply the same name as specified in the create. The
 *                  GateHWSpinLock module queries the NameServer to get the needed
 *                  information.<br>
 *              (2) Supply the same sharedAddr value as specified in the create.
 *                  Call GateHWSpinLock_close when the opened instance is no
 *                  longer needed.
 *
 *  @param      handlePtr   Pointer to GateHWSpinLock handle to be opened
 *  @param      params      Parameters for creating the instance.
 *
 *  @sa         GateHWSpinLock_create, GateHWSpinLock_delete, GateHWSpinLock_close
 */
Int32
GateHWSpinLock_open (GateHWSpinLock_Handle * handlePtr,
                   GateHWSpinLock_Params * params)
{
    Int32                   status = GATEHWSPINLOCK_SUCCESS;
    GateHWSpinLockDrv_CmdArgs cmdArgs;
    GateHWSpinLock_Obj *      obj;

    GT_2trace (curTrace, GT_ENTER, "GateHWSpinLock_open", handlePtr, params);

    GT_assert (curTrace, (params != NULL));
    GT_assert (curTrace, (handlePtr != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (GateHWSpinLock_state.setupRefCount == 0) {
        /*! @retval GATEHWSPINLOCK_E_INVALIDSTATE Modules is invalidstate*/
        status = GATEHWSPINLOCK_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateHWSpinLock_open",
                             status,
                             "Modules is invalidstate!");
    }
    else if (params == NULL) {
        /*! @retval GATEHWSPINLOCK_E_INVALIDARG params passed is NULL */
        status = GATEHWSPINLOCK_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateHWSpinLock_open",
                             status,
                             "params passed is NULL!");
    }
    else if (handlePtr == NULL) {
        /*! @retval GATEHWSPINLOCK_E_INVALIDARG handle passed is NULL */
        status = GATEHWSPINLOCK_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateHWSpinLock_open",
                             status,
                             "handlePtr passed is NULL!");
    }
    else if (   (GateHWSpinLock_state.cfg.useNameServer == FALSE)
             && (params->lockNum == -1u)) {
        /*! @retval GATEHWSPINLOCK_E_INVALIDARG Please provide a valid lock
         * number */
        status = GATEHWSPINLOCK_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateHWSpinLock_open",
                             status,
                             "Please provide a valid lock number!");
    }
    else if (   (GateHWSpinLock_state.cfg.useNameServer == TRUE)
             && (params->lockNum == -1u)
             && (params->name == NULL)) {
        /*! @retval GATEHWSPINLOCK_E_INVALIDARG Please provide name when lock
         * number is -1u*/
        status = GATEHWSPINLOCK_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateHWSpinLock_open",
                             status,
                             "Please provide name when lock number is -1u!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.open.params = (GateHWSpinLock_Params *) params;
        if (params->name != NULL) {
            cmdArgs.args.open.nameLen = String_len (params->name) + 1;
        }
        else {
            cmdArgs.args.open.nameLen = 0;
        }

        status = GateHWSpinLockDrv_ioctl (CMD_GATEHWSPINLOCK_OPEN, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "GateHWSpinLock_open",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            /* Allocate memory for the handle */
            *handlePtr = (GateHWSpinLock_Handle) Memory_calloc (NULL,
                                                   sizeof (GateHWSpinLock_Object),
                                                   0);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (*handlePtr == NULL) {
    /*! @retval GATEHWSPINLOCK_E_MEMORY Memory allocation failed for handle */
                status = GATEHWSPINLOCK_E_MEMORY;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "GateHWSpinLock_open",
                                     status,
                                     "Memory allocation failed for handle!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                obj = (GateHWSpinLock_Obj *) Memory_calloc (NULL,
                                                     sizeof (GateHWSpinLock_Obj),
                                                     0);
                if (obj == NULL) {
                    /*! @retval NULL Memory allocation failed for handle */
                    status = GATEHWSPINLOCK_E_MEMORY;
                    GT_setFailureReason (curTrace,
                                         GT_4CLASS,
                                         "GateHWSpinLock_open",
                                         status,
                                         "Memory allocation failed for the "
                                         "internal object!");
                }
                else {
                    obj->knlObject = cmdArgs.args.open.handle;

                    /* Set pointer to gatepeterson internal object
                     * into the user handle.
                     */
                    (*handlePtr)->obj = obj;
                    (*handlePtr)->enter = (Lock_enter) &GateHWSpinLock_enter;
                    (*handlePtr)->leave = (Lock_leave) &GateHWSpinLock_leave;
                    (*handlePtr)->getKnlHandle = (Lock_getKnlHandle)
                                           &GateHWSpinLock_getKnlHandle;
                }

#if !defined(SYSLINK_BUILD_OPTIMIZE)
            }
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "GateHWSpinLock_open", status);

    /*! @retval GATEHWSPINLOCK_SUCCESS Operation successful */
    return status;
}


/*!
 *  @brief      Closes previously opened instance of GateHWSpinLock module.
 *              Close may not be used to finalize a gate whose handle has been
 *              acquired using create. In this case, delete should be used
 *              instead.
 *
 *  @param      handlePtr  Pointer to GateHWSpinLock handle whose instance has
 *                         been opened
 *
 *  @sa         GateHWSpinLock_create, GateHWSpinLock_delete, GateHWSpinLock_open
 */
Int32
GateHWSpinLock_close (GateHWSpinLock_Handle * handlePtr)
{
    Int32                   status = GATEHWSPINLOCK_SUCCESS;
    GateHWSpinLockDrv_CmdArgs cmdArgs;
    GateHWSpinLock_Obj *      obj;

    GT_1trace (curTrace, GT_ENTER, "GateHWSpinLock_close", handlePtr);

    GT_assert (curTrace, (handlePtr != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (GateHWSpinLock_state.setupRefCount == 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateHWSpinLock_create",
                             GATEHWSPINLOCK_E_INVALIDSTATE,
                             "Modules is invalidstate!");
    }
    else if (handlePtr == NULL) {
        /*! @retval GATEHWSPINLOCK_E_INVALIDARG handlePtr passed is null */
        status = GATEHWSPINLOCK_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateHWSpinLock_close",
                             status,
                             "handlePtr passed is null!");
    }
    else if (*handlePtr == NULL) {
        /*! @retval GATEHWSPINLOCK_E_INVALIDARG Module *handlePtr passed is null*/
        status = GATEHWSPINLOCK_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateHWSpinLock_close",
                             status,
                             "*handlePtr passed is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        obj = (GateHWSpinLock_Obj *) ((*handlePtr)->obj);
        cmdArgs.args.close.handle = obj->knlObject;
        status = GateHWSpinLockDrv_ioctl (CMD_GATEHWSPINLOCK_CLOSE, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "GateHWSpinLock_close",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            Memory_free (NULL, obj, sizeof (GateHWSpinLock_Obj));
            Memory_free (NULL, *handlePtr, sizeof (GateHWSpinLock_Object));
            *handlePtr = NULL;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "GateHWSpinLock_close", status);

    /*! @retval GATEHWSPINLOCK_SUCCESS Operation successful */
    return status;
}


/*!
 *  @brief      Enters the GateHWSpinLock instance.
 *
 *  @param      handle  Handle to previously created/opened instance.
 *
 *  @sa         GateHWSpinLock_leave
 */
UInt32
GateHWSpinLock_enter (GateHWSpinLock_Handle handle)
{
    Int32                    status = GATEHWSPINLOCK_SUCCESS;
    UInt32                   key    = 0;
    GateHWSpinLockDrv_CmdArgs  cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "GateHWSpinLock_enter", handle);

    GT_assert (curTrace, (handle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (GateHWSpinLock_state.setupRefCount == 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateHWSpinLock_create",
                             GATEHWSPINLOCK_E_INVALIDSTATE,
                             "Modules is invalidstate!");
    }
    else if (handle == NULL) {
        /*! @retval GATEHWSPINLOCK_E_INVALIDARG handle passed is NULL */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateHWSpinLock_enter",
                             GATEHWSPINLOCK_E_INVALIDARG,
                             "handle passed is NULL!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.enter.handle =
                            ((GateHWSpinLock_Obj *) (handle->obj))->knlObject;
        status = GateHWSpinLockDrv_ioctl (CMD_GATEHWSPINLOCK_ENTER, &cmdArgs);
        key = cmdArgs.args.enter.flags;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "GateHWSpinLock_enter",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "GateHWSpinLock_enter", key);

    /*! @retval Key Key to be provided to GateHWSpinLock_leave. */
    return (key);
}


/*!
 *  @brief      Leaves the GateHWSpinLock instance.
 *
 *  @param      handle  Handle to previously created/opened instance.
 *  @param      key     Key received from GateHWSpinLock_enter call.
 *
 *  @sa         GateHWSpinLock_enter
 */
Void
GateHWSpinLock_leave (GateHWSpinLock_Handle handle, UInt32 key)
{
    Int32                   status = GATEHWSPINLOCK_SUCCESS;
    GateHWSpinLockDrv_CmdArgs cmdArgs;

    GT_2trace (curTrace, GT_ENTER, "GateHWSpinLock_leave", handle, key);

    GT_assert (curTrace, (handle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (GateHWSpinLock_state.setupRefCount == 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateHWSpinLock_create",
                             GATEHWSPINLOCK_E_INVALIDSTATE,
                             "Modules is invalidstate!");
    }
    else if (handle == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateHWSpinLock_leave",
                             GATEHWSPINLOCK_E_INVALIDARG,
                             "handle passed is NULL!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.leave.handle =
                               ((GateHWSpinLock_Obj *) (handle->obj))->knlObject;
        cmdArgs.args.leave.flags = key;
        status = GateHWSpinLockDrv_ioctl (CMD_GATEHWSPINLOCK_LEAVE, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "GateHWSpinLock_leave",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "GateHWSpinLock_leave");
}


/* =============================================================================
 * Internal functions
 * =============================================================================
 */
/*!
 *  @brief      Returns the gatepeterson kernel object pointer.
 *
 *  @param      handle  Handle to previously created/opened instance.
 *
 *  @sa         GateHWSpinLock_create
 */
Void *
GateHWSpinLock_getKnlHandle (GateHWSpinLock_Handle handle)
{
    Ptr objPtr = NULL;

    GT_1trace (curTrace, GT_ENTER, "GateHWSpinLock_getKnlHandle", handle);

    GT_assert (curTrace, (handle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (GateHWSpinLock_state.setupRefCount == 0) {
        /*! @retval NULL Modules is in invalid state */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateHWSpinLock_getKnlHandle",
                             GATEHWSPINLOCK_E_INVALIDSTATE,
                             "Modules is in invalid state!");
    }
    else if (handle == NULL) {
        /*! @retval NULL handle passed is NULL */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GateHWSpinLock_getKnlHandle",
                             GATEHWSPINLOCK_E_INVALIDARG,
                             "handle passed is NULL!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        objPtr = ((GateHWSpinLock_Obj *) (handle->obj))->knlObject;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (objPtr == NULL) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "GateHWSpinLock_getKnlHandle",
                                 GATEHWSPINLOCK_E_INVALIDSTATE,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "GateHWSpinLock_getKnlHandle", objPtr);

    /*! @retval Kernel-Object-handle Operation successfully completed. */
    return (objPtr);
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
