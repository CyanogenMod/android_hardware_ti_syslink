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
 *  @file   GatePeterson.c
 *
 *  @brief      Gate based on Peterson Algorithm.
 *
 *              This module implements the Peterson Algorithm for mutual
 *              exclusion of shared memory. This implementation works for only 2
 *              processors.<br>
 *              This module is instance based. Each instance requires a small
 *              piece of shared memory. This is specified via the sharedAddr
 *              parameter to the create. The proper sharedAddrSize parameter
 *              can be determined via the #GatePeterson_sharedMemReq call. Note:
 *              the parameters to this function must be the same that will used
 *              to create or open the instance.<br>
 *              The GatePeterson module uses a NameServer instance
 *              to store instance information when an instance is created and
 *              the name parameter is non-NULL. If a name is supplied, it must
 *              be unique for all GatePeterson instances.
 *              The #GatePeterson_create also initializes the shared memory as
 *              needed. The shared memory must be initialized to 0 or all ones
 *              (e.g. 0xFFFFFFFFF) before the GatePeterson instance is created.
 *              Once an instance is created, an open can be performed. The
 *              #GatePeterson_open is used to gain access to the same
 *              GatePeterson instance. Generally an instance is created on one
 *              processor and opened on the other processor.<br>
 *              The open returns a GatePeterson instance handle like the create,
 *              however the open does not modify the shared memory. Generally an
 *              instance is created on one processor and opened on the other
 *              processor.<br>
 *              There are two options when opening the instance:<br>
 *              - Supply the same name as specified in the create. The
 *              GatePeterson module queries the NameServer to get the needed
 *              information.<br>
 *              - Supply the same sharedAddr value as specified in the create.
 *              If the open is called before the instance is created, open
 *              returns NULL.<br>
 *              There is currently a list of restrictions for the module:<br>
 *              - Both processors must have the same endianness. Endianness
 *             conversion may supported in a future version of GatePeterson.<br>
 *              - The module will be made a gated module
 *
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
#include <GatePeterson.h>
#include <GatePetersonDrv.h>
#include <GatePetersonDrvDefs.h>
#include <SharedRegion.h>


#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 * Macros
 * =============================================================================
 */
/* Cache line size */
#define GATEPETERSON_CACHESIZE   128u


/* =============================================================================
 * Structures & Enums
 * =============================================================================
 */
/* Structure defining object for the Gate Peterson */
typedef struct GatePeterson_Obj_tag {
    Ptr         knlObject;
    /*!< Pointer to the kernel-side ProcMgr object. */
} GatePeterson_Obj;

/*!
 *  @brief  GatePeterson Module state object
 */
typedef struct GatePeterson_ModuleObject_tag {
    UInt32              setupRefCount;
    /*!< Reference count for number of times setup/destroy were called in this
         process. */
    GatePeterson_Config cfg;
    /* Current config */
} GatePeterson_ModuleObject;


/* =============================================================================
 *  Globals
 * =============================================================================
 */
/*!
 *  @var    GatePeterson_state
 *
 *  @brief  ProcMgr state object variable
 */
#if !defined(SYSLINK_BUILD_DEBUG)
static
#endif /* if !defined(SYSLINK_BUILD_DEBUG) */
GatePeterson_ModuleObject GatePeterson_state =
{
    .setupRefCount = 0
};


/* =============================================================================
 * APIS
 * =============================================================================
 */
/*!
 *  @brief      Get the default configuration for the GatePeterson module.
 *
 *              This function can be called by the application to get their
 *              configuration parameter to GatePeterson_setup filled in by
 *              the GatePeterson module with the default parameters. If the
 *              user does not wish to make any change in the default parameters,
 *              this API is not required to be called.
 *
 *  @param      cfgParams  Pointer to the GatePeterson module configuration
 *                         structure in which the default config is to be
 *                         returned.
 *
 *  @sa         GatePeterson_setup
 */
Void
GatePeterson_getConfig (GatePeterson_Config * cfg)
{
    Int                  status = GATEPETERSON_SUCCESS;
    GatePetersonDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "GatePeterson_getConfig", cfg);

    GT_assert (curTrace, (cfg != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (cfg == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GatePeterson_getConfig",
                             GATEPETERSON_E_INVALIDARG,
                             "Argument of type (GatePeterson_Config *) passed "
                             "is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Temporarily open the handle to get the configuration. */
        status = GatePetersonDrv_open ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "GatePeterson_getConfig",
                                 status,
                                 "Failed to open driver handle!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            cmdArgs.args.getConfig.config = cfg;
            status = GatePetersonDrv_ioctl (CMD_GATEPETERSON_GETCONFIG,
                                            &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                  GT_4CLASS,
                                  "GatePeterson_getConfig",
                                  status,
                                  "API (through IOCTL) failed on kernel-side!");

            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                Memory_copy ((Ptr) &GatePeterson_state.cfg,
                              (Ptr) cfg,
                              sizeof (GatePeterson_Config));
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            }
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Close the driver handle. */
        GatePetersonDrv_close ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_ENTER, "GatePeterson_getConfig");


}


/*!
 *  @brief      Setup the GatePeterson module.
 *
 *              This function sets up the GatePeterson module. This function
 *              must be called before any other instance-level APIs can be
 *              invoked.
 *              Module-level configuration needs to be provided to this
 *              function. If the user wishes to change some specific config
 *              parameters, then GatePeterson_getConfig can be called to get
 *              the configuration filled with the default values. After this,
 *              only the required configuration values can be changed. If the
 *              user does not wish to make any change in the default parameters,
 *              the application can simply call GatePeterson_setup with NULL
 *              parameters. The default parameters would get automatically used.
 *
 *  @param      cfg   Optional GatePeterson module configuration. If provided
 *                    as NULL, default configuration is used.
 *
 *  @sa         GatePeterson_destroy, GatePeterson_getConfig
 */
Int32
GatePeterson_setup (const GatePeterson_Config * config)
{
    Int                  status = GATEPETERSON_SUCCESS;
    GatePetersonDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "GatePeterson_setup", config);

    /* TBD: Protect from multiple threads. */
    GatePeterson_state.setupRefCount++;
    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */
    if (GatePeterson_state.setupRefCount > 1) {
        /*! @retval GATEPETERSON_S_ALREADYSETUP Success: ProcMgr module has been
                                           already setup in this process */
        status = GATEPETERSON_S_ALREADYSETUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "ProcMgr module has been already setup in this process.\n"
                   "    RefCount: [%d]\n",
                   GatePeterson_state.setupRefCount);
    }
    else {
        /* Open the driver handle. */
        status = GatePetersonDrv_open ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "GatePeterson_setup",
                                 status,
                                 "Failed to open driver handle!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            cmdArgs.args.setup.config = (GatePeterson_Config *) config;
            status = GatePetersonDrv_ioctl (CMD_GATEPETERSON_SETUP, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "GatePeterson_setup",
                                     status,
                                     "API (through IOCTL) failed on kernel-side!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                if (config != NULL) {
                    Memory_copy ((Ptr) &GatePeterson_state.cfg,
                                  (Ptr) config,
                                  sizeof (GatePeterson_Config));
                }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            }
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

    GT_1trace (curTrace, GT_LEAVE, "GatePeterson_setup", status);

    /*! @retval GATEPETERSON_SUCCESS Operation successful */
    return (status);
}


/*!
 *  @brief      Function to destroy the GatePeterson module.
 *
 *  @sa         GatePeterson_setup
 */
Int32
GatePeterson_destroy (void)
{
    Int                     status = GATEPETERSON_SUCCESS;
    GatePetersonDrv_CmdArgs    cmdArgs;

    GT_0trace (curTrace, GT_ENTER, "GatePeterson_destroy");

    /* TBD: Protect from multiple threads. */
    GatePeterson_state.setupRefCount--;
    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */
    if (GatePeterson_state.setupRefCount > 1) {
        /*! @retval GATEPETERSON_S_ALREADYSETUP Success: ProcMgr module has been
                                           already setup in this process */
        status = GATEPETERSON_S_ALREADYSETUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "ProcMgr module has been already setup in this process.\n"
                   "    RefCount: [%d]\n",
                   GatePeterson_state.setupRefCount);
    }
    else {
        status = GatePetersonDrv_ioctl (CMD_GATEPETERSON_DESTROY, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "GatePeterson_destroy",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

    /* Close the driver handle. */
    GatePetersonDrv_close ();

    GT_1trace (curTrace, GT_LEAVE, "GatePeterson_destroy", status);

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
 *  @sa         GatePeterson_create
 */
Void
GatePeterson_Params_init (GatePeterson_Handle   handle,
                          GatePeterson_Params * params)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                     status = GATEPETERSON_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    GatePetersonDrv_CmdArgs cmdArgs;

    GT_2trace (curTrace, GT_ENTER, "GatePeterson_Params_init", handle, params);

    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (GatePeterson_state.setupRefCount == 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GatePeterson_create",
                             GATEPETERSON_E_INVALIDSTATE,
                             "Modules is invalidstate!");
    }
    else if (params == NULL) {
        /* No retVal comment since this is a Void function. */
        status = GATEPETERSON_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GatePeterson_Params_init",
                             status,
                             "Argument of type (GatePeterson_Params *) passed "
                             "is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        if (handle != NULL) {
            cmdArgs.args.ParamsInit.handle =
                                     ((GatePeterson_Obj *) handle)->knlObject;
        }
        else {
            cmdArgs.args.ParamsInit.handle = handle;
        }
        cmdArgs.args.ParamsInit.params = params;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        GatePetersonDrv_ioctl (CMD_GATEPETERSON_PARAMS_INIT, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "GatePeterson_Params_init",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "GatePeterson_Params_init");


}


/*!
 *  @brief      Creates a new instance of GatePeterson module.
 *
 *  @param      params  Instance config-params structure.
 *
 *  @sa         GatePeterson_delete, GatePeterson_open, GatePeterson_close
 */
GatePeterson_Handle
GatePeterson_create (const GatePeterson_Params * params)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                     status = 0;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    GatePeterson_Handle     handle = NULL;
    GatePetersonDrv_CmdArgs cmdArgs;
    GatePeterson_Obj *      obj;
    Int32                   index;

    GT_1trace (curTrace, GT_ENTER, "GatePeterson_create", params);

    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (GatePeterson_state.setupRefCount == 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GatePeterson_create",
                             GATEPETERSON_E_INVALIDSTATE,
                             "Modules is invalidstate!");
    }
    else if (params == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GatePeterson_create",
                             GATEPETERSON_E_INVALIDARG,
                             "params passed is NULL!");
    }
    else if (params->sharedAddr == (UInt32) NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GatePeterson_create",
                             GATEPETERSON_E_INVALIDARG,
                             "Please provide a valid shared region "
                             "address!");
    }
    else if (  params->sharedAddrSize
             < GatePeterson_sharedMemReq (params)) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GatePeterson_create",
                             GATEPETERSON_E_INVALIDARG,
                             "Shared memory size is less than required!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.create.params = (GatePeterson_Params *) params;

        /* Translate sharedAddr to SrPtr. */
        index = SharedRegion_getIndex (params->sharedAddr);
        cmdArgs.args.create.sharedAddrSrPtr = SharedRegion_getSRPtr (
                                                            params->sharedAddr,
                                                            index);
        if (params->name != NULL) {
            cmdArgs.args.create.nameLen = String_len (params->name) + 1;
        }
        else {
            cmdArgs.args.create.nameLen = 0;
        }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        GatePetersonDrv_ioctl (CMD_GATEPETERSON_CREATE, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "GatePeterson_create",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            /* Allocate memory for the generic handle */
            handle = (GatePeterson_Handle) Memory_calloc (NULL,
                                                   sizeof (GatePeterson_Object),
                                                   0);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (handle == NULL) {
                /*! @retval NULL Memory allocation failed for handle */
                status = GATEPETERSON_E_MEMORY;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "GatePeterson_create",
                                     status,
                                     "Memory allocation failed for handle!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                /* Allocate memory for the specific object */
                obj = (GatePeterson_Obj *) Memory_calloc (NULL,
                                                      sizeof (GatePeterson_Obj),
                                                      0);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
                if (obj == NULL) {
                    /*! @retval NULL Memory allocation failed for handle */
                    status = GATEPETERSON_E_MEMORY;
                    GT_setFailureReason (curTrace,
                                         GT_4CLASS,
                                         "GatePeterson_create",
                                         status,
                                         "Memory allocation failed for the"
                                         " internal object!");
                }
                else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                    /* Set pointer to kernel object into the user handle. */
                    obj->knlObject = cmdArgs.args.create.handle;
                    handle->obj   = obj;/* Assign GatePeterson internal object*/
                    handle->enter = (Lock_enter) &GatePeterson_enter;
                    handle->leave = (Lock_leave) &GatePeterson_leave;
                    handle->getKnlHandle = (Lock_getKnlHandle)
                                           &GatePeterson_getKnlHandle;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
                }
             }
         }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "GatePeterson_create", handle);

    /*! @retval valid-handle Operation successful*/
    /*! @retval NULL Operation failed */
    return handle;
}


/*!
 *  @brief      Deletes a instance of GatePeterson module.
 *
 *  @param      handlePtr  Pointer to handle to previously created instance. The
 *                         pointer is reset upon successful completion.
 *
 *  @sa         GatePeterson_create, GatePeterson_open, GatePeterson_close
 */
Int32
GatePeterson_delete (GatePeterson_Handle * handlePtr)
{
    Int32                   status = GATEPETERSON_SUCCESS;
    GatePetersonDrv_CmdArgs cmdArgs;
    GatePeterson_Obj *      obj;

    GT_1trace (curTrace, GT_ENTER, "GatePeterson_delete", handlePtr);

    GT_assert (curTrace, (handlePtr != NULL));
    GT_assert (curTrace, ((handlePtr != NULL) && (*handlePtr != NULL)));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (GatePeterson_state.setupRefCount == 0) {
        /*! @retval GATEPETERSON_E_INVALIDSTATE Modules is invalidstate*/
        status = GATEPETERSON_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GatePeterson_delete",
                             status,
                             "Modules is invalidstate!");
    }
    else if (handlePtr == NULL) {
        /*! @retval GATEPETERSON_E_INVALIDARG handlePtr passed is NULL*/
        status = GATEPETERSON_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GatePeterson_delete",
                             status,
                             "handlePtr passed is NULL!");
    }
    else if (*handlePtr == NULL) {
        /*! @retval GATEPETERSON_E_INVALIDARG *handlePtr passed is NULL*/
        status = GATEPETERSON_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GatePeterson_delete",
                             status,
                             "*handlePtr passed is NULL!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        obj = (GatePeterson_Obj *) ((*handlePtr)->obj);
        cmdArgs.args.deleteInstance.handle = obj->knlObject;
        status = GatePetersonDrv_ioctl (CMD_GATEPETERSON_DELETE, &cmdArgs);
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
            Memory_free (NULL, obj, sizeof (GatePeterson_Obj));
            Memory_free (NULL, *handlePtr, sizeof (GatePeterson_Object));
            *handlePtr = NULL;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "GatePeterson_delete", status);

    /*! @retval GATEPETERSON_SUCCESS Operation successful */
    return status;
}


/*!
 *  @brief      Opens a created instance of GatePeterson module. Once an
 *              instance is created, an open can be performed. The open is used
 *              to gain access to the same GatePeterson instance. Generally an
 *              instance is created on one processor and opened on the other
 *              processor.<br>
 *              The open accepts a pointer to a GatePeterson handle whose value
 *              is set if the open is successful. A status code is returned
 *              indicating whether the open was successful. If the open was not
 *              successful, an error code indicates that the gate was has not
 *              been created yet.<br>
 *              There are two ways to open a GatePeterson instance that has been
 *              created either locally or remotely:<br>
 *              (1) Supply the same name as specified in the create. The
 *                  GatePeterson module queries the NameServer to get the needed
 *                  information.<br>
 *              (2) Supply the same sharedAddr value as specified in the create.
 *                  Call GatePeterson_close when the opened instance is no
 *                  longer needed.
 *
 *  @param      handlePtr   Pointer to GatePeterson handle to be opened
 *  @param      params      Parameters for creating the instance.
 *
 *  @sa         GatePeterson_create, GatePeterson_delete, GatePeterson_close
 */
Int32
GatePeterson_open (GatePeterson_Handle * handlePtr,
                   GatePeterson_Params * params)
{
    Int32                   status = GATEPETERSON_SUCCESS;
    GatePetersonDrv_CmdArgs cmdArgs;
    GatePeterson_Obj *      obj;
    Int32                   index;
    Ptr                     sharedAddr;

    GT_2trace (curTrace, GT_ENTER, "GatePeterson_open", handlePtr, params);

    GT_assert (curTrace, (params != NULL));
    GT_assert (curTrace, (handlePtr != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (GatePeterson_state.setupRefCount == 0) {
        /*! @retval GATEPETERSON_E_INVALIDSTATE Modules is invalidstate*/
        status = GATEPETERSON_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GatePeterson_open",
                             status,
                             "Modules is invalidstate!");
    }
    else if (params == NULL) {
        /*! @retval GATEPETERSON_E_INVALIDARG params passed is NULL */
        status = GATEPETERSON_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GatePeterson_open",
                             status,
                             "params passed is NULL!");
    }
    else if (handlePtr == NULL) {
        /*! @retval GATEPETERSON_E_INVALIDARG handle passed is NULL */
        status = GATEPETERSON_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GatePeterson_open",
                             status,
                             "handlePtr passed is NULL!");
    }
    else if (   (GatePeterson_state.cfg.useNameServer == FALSE)
             && (params->sharedAddr == (UInt32)NULL)) {
        /*! @retval GATEPETERSON_E_INVALIDARG Please provide a valid shared
         * memory address */
        status = GATEPETERSON_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GatePeterson_open",
                             status,
                             "Please provide a valid shared memory "
                             "address!");
    }
    else if (   (GatePeterson_state.cfg.useNameServer == TRUE)
             && (params->sharedAddr == (UInt32)NULL)
             && (params->name == NULL)) {
        /*! @retval GATEPETERSON_E_INVALIDARG Please provide name when shared
         * memory is NULL */
        status = GATEPETERSON_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GatePeterson_open",
                             status,
                             "Please provide name when shared memory is NULL!");
    }
    else if (  (params->name == NULL)
             && (  params->sharedAddrSize
                 < GatePeterson_sharedMemReq (params))) {
        /*! @retval GATEPETERSON_E_INVALIDARG Shared memory size is less than
         * required */
        status = GATEPETERSON_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GatePeterson_open",
                             status,
                             "Shared memory size is less than required!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.open.params = (GatePeterson_Params *) params;
        if (params->name != NULL) {
            cmdArgs.args.open.nameLen = String_len (params->name) + 1;
        }
        else {
            cmdArgs.args.open.nameLen = 0;
        }

        /* Translate sharedAddr to SrPtr. */
        sharedAddr = params->sharedAddr;
        index = SharedRegion_getIndex (params->sharedAddr);
        cmdArgs.args.open.sharedAddrSrPtr =
                                     SharedRegion_getSRPtr (sharedAddr, index);
        status = GatePetersonDrv_ioctl (CMD_GATEPETERSON_OPEN, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "GatePeterson_open",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            /* Allocate memory for the handle */
            *handlePtr = (GatePeterson_Handle) Memory_calloc (NULL,
                                                   sizeof (GatePeterson_Object),
                                                   0);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (*handlePtr == NULL) {
    /*! @retval GATEPETERSON_E_MEMORY Memory allocation failed for handle */
                status = GATEPETERSON_E_MEMORY;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "GatePeterson_open",
                                     status,
                                     "Memory allocation failed for handle!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                obj = (GatePeterson_Obj *) Memory_calloc (NULL,
                                                     sizeof (GatePeterson_Obj),
                                                     0);
                if (obj == NULL) {
                    /*! @retval NULL Memory allocation failed for handle */
                    status = GATEPETERSON_E_MEMORY;
                    GT_setFailureReason (curTrace,
                                         GT_4CLASS,
                                         "GatePeterson_open",
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
                    (*handlePtr)->enter = (Lock_enter) &GatePeterson_enter;
                    (*handlePtr)->leave = (Lock_leave) &GatePeterson_leave;
                    (*handlePtr)->getKnlHandle = (Lock_getKnlHandle)
                                           &GatePeterson_getKnlHandle;
                }

#if !defined(SYSLINK_BUILD_OPTIMIZE)
            }
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "GatePeterson_open", status);

    /*! @retval GATEPETERSON_SUCCESS Operation successful */
    return status;
}


/*!
 *  @brief      Closes previously opened instance of GatePeterson module.
 *              Close may not be used to finalize a gate whose handle has been
 *              acquired using create. In this case, delete should be used
 *              instead.
 *
 *  @param      handlePtr  Pointer to GatePeterson handle whose instance has
 *                         been opened
 *
 *  @sa         GatePeterson_create, GatePeterson_delete, GatePeterson_open
 */
Int32
GatePeterson_close (GatePeterson_Handle * handlePtr)
{
    Int32                   status = GATEPETERSON_SUCCESS;
    GatePetersonDrv_CmdArgs cmdArgs;
    GatePeterson_Obj *      obj;

    GT_1trace (curTrace, GT_ENTER, "GatePeterson_close", handlePtr);

    GT_assert (curTrace, (handlePtr != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (GatePeterson_state.setupRefCount == 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GatePeterson_create",
                             GATEPETERSON_E_INVALIDSTATE,
                             "Modules is invalidstate!");
    }
    else if (handlePtr == NULL) {
        /*! @retval GATEPETERSON_E_INVALIDARG handlePtr passed is null */
        status = GATEPETERSON_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GatePeterson_close",
                             status,
                             "handlePtr passed is null!");
    }
    else if (*handlePtr == NULL) {
        /*! @retval GATEPETERSON_E_INVALIDARG Module *handlePtr passed is null*/
        status = GATEPETERSON_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GatePeterson_close",
                             status,
                             "*handlePtr passed is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        obj = (GatePeterson_Obj *) ((*handlePtr)->obj);
        cmdArgs.args.close.handle = obj->knlObject;
        status = GatePetersonDrv_ioctl (CMD_GATEPETERSON_CLOSE, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "GatePeterson_close",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            Memory_free (NULL, obj, sizeof (GatePeterson_Obj));
            Memory_free (NULL, *handlePtr, sizeof (GatePeterson_Object));
            *handlePtr = NULL;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "GatePeterson_close", status);

    /*! @retval GATEPETERSON_SUCCESS Operation successful */
    return status;
}


/*!
 *  @brief      Enters the GatePeterson instance.
 *
 *  @param      handle  Handle to previously created/opened instance.
 *
 *  @sa         GatePeterson_leave
 */
UInt32
GatePeterson_enter (GatePeterson_Handle handle)
{
    Int32                    status = GATEPETERSON_SUCCESS;
    UInt32                   key    = 0;
    GatePetersonDrv_CmdArgs  cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "GatePeterson_enter", handle);

    GT_assert (curTrace, (handle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (GatePeterson_state.setupRefCount == 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GatePeterson_create",
                             GATEPETERSON_E_INVALIDSTATE,
                             "Modules is invalidstate!");
    }
    else if (handle == NULL) {
        /*! @retval GATEPETERSON_E_INVALIDARG handle passed is NULL */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GatePeterson_enter",
                             GATEPETERSON_E_INVALIDARG,
                             "handle passed is NULL!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.enter.handle =
                            ((GatePeterson_Obj *) (handle->obj))->knlObject;
        status = GatePetersonDrv_ioctl (CMD_GATEPETERSON_ENTER, &cmdArgs);
        key = cmdArgs.args.enter.flags;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "GatePeterson_enter",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "GatePeterson_enter", key);

    /*! @retval Key Key to be provided to GatePeterson_leave. */
    return (key);
}


/*!
 *  @brief      Leaves the GatePeterson instance.
 *
 *  @param      handle  Handle to previously created/opened instance.
 *  @param      key     Key received from GatePeterson_enter call.
 *
 *  @sa         GatePeterson_enter
 */
Void
GatePeterson_leave (GatePeterson_Handle handle, UInt32 key)
{
    Int32                   status = GATEPETERSON_SUCCESS;
    GatePetersonDrv_CmdArgs cmdArgs;

    GT_2trace (curTrace, GT_ENTER, "GatePeterson_leave", handle, key);

    GT_assert (curTrace, (handle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (GatePeterson_state.setupRefCount == 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GatePeterson_create",
                             GATEPETERSON_E_INVALIDSTATE,
                             "Modules is invalidstate!");
    }
    else if (handle == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GatePeterson_leave",
                             GATEPETERSON_E_INVALIDARG,
                             "handle passed is NULL!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.leave.handle =
                               ((GatePeterson_Obj *) (handle->obj))->knlObject;
        cmdArgs.args.leave.flags = key;
        status = GatePetersonDrv_ioctl (CMD_GATEPETERSON_LEAVE, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "GatePeterson_leave",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "GatePeterson_leave");
}


/*!
 *  @brief      Returns the shared memory size requirement for a single
 *              instance.
 *
 *  @param      params  GatePeterson create parameters
 *
 *  @sa         GatePeterson_create
 */
UInt32
GatePeterson_sharedMemReq (const GatePeterson_Params * params)
{
    Int retVal = 0u;

    GT_1trace (curTrace, GT_ENTER, "GatePeterson_sharedMemReq", params);

    retVal = (GATEPETERSON_CACHESIZE * 4u);

    GT_1trace (curTrace, GT_LEAVE, "GatePeterson_sharedMemReq", retVal);

    /*! @retval Amount-of-shared-memory-required On success */
    return retVal;
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
 *  @sa         GatePeterson_create
 */
Void *
GatePeterson_getKnlHandle (GatePeterson_Handle handle)
{
    Ptr objPtr = NULL;

    GT_1trace (curTrace, GT_ENTER, "GatePeterson_getKnlHandle", handle);

    GT_assert (curTrace, (handle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (GatePeterson_state.setupRefCount == 0) {
        /*! @retval NULL Modules is in invalid state */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GatePeterson_getKnlHandle",
                             GATEPETERSON_E_INVALIDSTATE,
                             "Modules is in invalid state!");
    }
    else if (handle == NULL) {
        /*! @retval NULL handle passed is NULL */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "GatePeterson_getKnlHandle",
                             GATEPETERSON_E_INVALIDARG,
                             "handle passed is NULL!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        objPtr = ((GatePeterson_Obj *) (handle->obj))->knlObject;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (objPtr == NULL) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "GatePeterson_getKnlHandle",
                                 GATEPETERSON_E_INVALIDSTATE,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "GatePeterson_getKnlHandle", objPtr);

    /*! @retval Kernel-Object-handle Operation successfully completed. */
    return (objPtr);
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
