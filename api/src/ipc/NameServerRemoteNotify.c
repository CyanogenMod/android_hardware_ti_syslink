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
 *  @file   NameServerRemoteNotify.c
 *
 *  @brief      NameServer Remote Transport
 *
 *  ============================================================================
 */


/* Standard headers */
#include <Std.h>

/* Utilities headers */
#include <String.h>
#include <MultiProc.h>
#include <Gate.h>
#include <Memory.h>
#include <Trace.h>
#include <String.h>

/* Module level headers */
#include <NameServerRemoteNotify.h>
#include <NameServerRemoteNotifyDrv.h>
#include <NameServerRemoteNotifyDrvDefs.h>
#include <SharedRegion.h>
#include <_NotifyDefs.h>


#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 * Macros and types
 * =============================================================================
 */
/*!
 *  @brief  NameServerRemoteNotify Module state object
 */
typedef struct NameServerRemoteNotify_ModuleObject_tag {
    UInt32      setupRefCount;
    /*!< Reference count for number of times setup/destroy were called in this
         process. */
} NameServerRemoteNotify_ModuleObject;

/* Structure defining object for the NameServerRemoteNotify */
typedef struct NameServerRemoteNotify_Object_tag {
    Ptr         knlObject;
    /*!< Pointer to the kernel-side NameServerRemoteNotify object. */
} NameServerRemoteNotify_Object;


/* =============================================================================
 *  Globals
 * =============================================================================
 */
/*!
 *  @var    NameServerRemoteNotify_state
 *
 *  @brief  NameServerRemoteNotify state object variable
 */
#if !defined(SYSLINK_BUILD_DEBUG)
static
#endif /* if !defined(SYSLINK_BUILD_DEBUG) */
NameServerRemoteNotify_ModuleObject NameServerRemoteNotify_state =
{
    .setupRefCount = 0
};



/* =============================================================================
 * APIS
 * =============================================================================
 */
/*!
 *  @brief      Get the default configuration for the NameServerRemoteNotify
 *              module.
 *
 *              This function can be called by the application to get their
 *              configuration parameter to NameServerRemoteNotify_setup filled
 *              in by the NameServerRemoteNotify module with the default
 *              parameters. If the user does not wish to make any change in the
 *              default parameters, this API is not required to be called.
 *
 *  @param      cfg        Pointer to the NameServerRemoteNotify module
 *                         configuration structure in which the default config
 *                         is to be returned.
 *
 *  @sa         NameServerRemoteNotify_setup, NameServerRemoteNotifyDrv_open,
 *              NameServerRemoteNotifyDrv_ioctl, NameServerRemoteNotifyDrv_close
 */
Void
NameServerRemoteNotify_getConfig (NameServerRemoteNotify_Config * cfgParams)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                        status = NAMESERVERREMOTENOTIFY_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    NameServerRemoteNotifyDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "NameServerRemoteNotify_getConfig",
               cfgParams);

    GT_assert (curTrace, (cfgParams != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (cfgParams == NULL) {
        /*! @retval NAMESERVERREMOTENOTIFY_E_INVALIDARG Argument of type
         *  (NameServerRemoteNotify_Config *) passed is null
         */
        status = NAMESERVERREMOTENOTIFY_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                      GT_4CLASS,
                      "NameServerRemoteNotify_getConfig",
                      status,
                      "Argument of type (NameServerRemoteNotify_Config *)"
                      " passed is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Temporarily open the handle to get the configuration. */
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        NameServerRemoteNotifyDrv_open ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NameServerRemoteNotify_getConfig",
                                 status,
                                 "Failed to open driver handle!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            cmdArgs.args.getConfig.config = cfgParams;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            NameServerRemoteNotifyDrv_ioctl (
                                        CMD_NAMESERVERREMOTENOTIFY_GETCONFIG,
                                        &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                  GT_4CLASS,
                                  "NameServerRemoteNotify_getConfig",
                                  status,
                                  "API (through IOCTL) failed on kernel-side!");

            }
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Close the driver handle. */
        NameServerRemoteNotifyDrv_close ();

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "NameServerRemoteNotify_getConfig");
}

/*!
 *  @brief      Setup the NameServerRemoteNotify module.
 *
 *              This function sets up the NameServerRemoteNotify module. This
 *              function must be called before any other instance-level APIs can
 *              be invoked.
 *              Module-level configuration needs to be provided to this
 *              function. If the user wishes to change some specific config
 *              parameters, then NameServerRemoteNotify_getConfig can be called
 *              to get the configuration filled with the default values. After
 *              this, only the required configuration values can be changed. If
 *              the user does not wish to make any change in the default
 *              parameters, the application can simply call
 *              NameServerRemoteNotify_setup with NULL parameters. The default
 *              parameters would get automatically used.
 *
 *  @param      cfg   Optional NameServerRemoteNotify module configuration.
 *                    If provided as NULL, default configuration is used.
 *
 *  @sa         NameServerRemoteNotify_destroy, NameServerRemoteNotifyDrv_open,
 *              NameServerRemoteNotifyDrv_ioctl
 */
Int
NameServerRemoteNotify_setup (NameServerRemoteNotify_Config * config)
{
    Int                        status = NAMESERVERREMOTENOTIFY_SUCCESS;
    NameServerRemoteNotifyDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "NameServerRemoteNotify_setup", config);

    /* TBD: Protect from multiple threads. */
    NameServerRemoteNotify_state.setupRefCount++;
    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */
    if (NameServerRemoteNotify_state.setupRefCount > 1) {
        /*! @retval NAMESERVERREMOTENOTIFY_S_ALREADYSETUP Success:
         *           NameServerRemoteNotify module has been already setup
         *           in this process
         */
        status = NAMESERVERREMOTENOTIFY_S_ALREADYSETUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "NameServerRemoteNotify module has been already setup in"
                   " this process.\n RefCount: [%d]\n",
                   NameServerRemoteNotify_state.setupRefCount);
    }
    else {
        /* Open the driver handle. */
        status = NameServerRemoteNotifyDrv_open ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NameServerRemoteNotify_setup",
                                 status,
                                 "Failed to open driver handle!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            cmdArgs.args.setup.config = (NameServerRemoteNotify_Config *)config;
            status = NameServerRemoteNotifyDrv_ioctl (
                                               CMD_NAMESERVERREMOTENOTIFY_SETUP,
                                               &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "NameServerRemoteNotify_setup",
                                     status,
                                     "API (through IOCTL) failed"
                                     " on kernel-side!");
            }
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

    GT_1trace (curTrace, GT_LEAVE, "NameServerRemoteNotify_setup", status);

    /*! @retval NAMESERVERREMOTENOTIFY_SUCCESS Operation Successful*/
    return status;
}

/*!
 *  @brief      Destroy the NameServerRemoteNotify module.
 *
 *              Once this function is called, other NameServerRemoteNotify module
 *              APIs, except for the NameServerRemoteNotify_getConfig API cannot
 *              be called anymore.
 *
 *  @sa         NameServerRemoteNotify_setup, NameServerRemoteNotifyDrv_ioctl,
 *              NameServerRemoteNotifyDrv_close
 */
Int
NameServerRemoteNotify_destroy (Void)
{
    Int                               status = NAMESERVERREMOTENOTIFY_SUCCESS;
    NameServerRemoteNotifyDrv_CmdArgs cmdArgs;

    GT_0trace (curTrace, GT_ENTER, "NameServerRemoteNotify_destroy");

    /* TBD: Protect from multiple threads. */
    NameServerRemoteNotify_state.setupRefCount--;
    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */
    if (NameServerRemoteNotify_state.setupRefCount > 1) {
        /*! @retval NAMESERVERREMOTENOTIFY_S_ALREADYSETUP Success:
         *           NameServerRemoteNotify module has been already setup
         *           in this process
         */
        status = NAMESERVERREMOTENOTIFY_S_ALREADYSETUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "NameServerRemoteNotify module has been already setup in"
                   " this process.\n  RefCount: [%d]\n",
                   NameServerRemoteNotify_state.setupRefCount);
    }
    else {
        status = NameServerRemoteNotifyDrv_ioctl(
                                            CMD_NAMESERVERREMOTENOTIFY_DESTROY,
                                            &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NameServerRemoteNotify_destroy",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

    /* Close the driver handle. */
    NameServerRemoteNotifyDrv_close ();

    GT_1trace (curTrace, GT_LEAVE, "NameServerRemoteNotify_destroy", status);

    /*! @retval NAMESERVERREMOTENOTIFY_SUCCESS Operation Successful*/
    return status;
}


/*!
 *  @brief      Function to initialize the parameters for the
 *              NameServerRemoteNotify instance.
 *
 *              This function can be called by the application to get their
 *              configuration parameter to NameServerRemoteNotify_create filled
 *              in by NameServerRemoteNotify module with the default parameters.
 *
 *  @param      handle   Handle to the NameServerRemoteNotify object. If
 *                       specified as NULL, the default global configuration
 *                       values are returned.
 *  @param      params   Pointer to the NameServerRemoteNotify instance params
 *                       structure in which the default params is to be returned
 *
 *  @sa         NameServerRemoteNotify_create, NameServerRemoteNotifyDrv_ioctl
 */
Void
NameServerRemoteNotify_Params_init (NameServerRemoteNotify_Handle   handle,
                                    NameServerRemoteNotify_Params * params)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                                status = NAMESERVERREMOTENOTIFY_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    NameServerRemoteNotifyDrv_CmdArgs  cmdArgs;

    GT_2trace (curTrace,
               GT_ENTER,
               "NameServerRemoteNotify_Params_init",
               handle,
               params);

    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (params == NULL) {
        /*! @retval None */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServerRemoteNotify_Params_init",
                             NAMESERVERREMOTENOTIFY_E_INVALIDARG,
                             "Argument of type "
                             "(NameServerRemoteNotify_Params *) is "
                             "NULL!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        if (handle != NULL) {
            cmdArgs.args.ParamsInit.handle =
                          ((NameServerRemoteNotify_Object *) handle)->knlObject;
        }
        else {
            cmdArgs.args.ParamsInit.handle = handle;
        }
        cmdArgs.args.ParamsInit.params = params;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            NameServerRemoteNotifyDrv_ioctl
                         (CMD_NAMESERVERREMOTENOTIFY_PARAMS_INIT,
                          &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NameServerRemoteNotify_Params_init",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "NameServerRemoteNotify_Params_init");
}


/*
 *  @brief      Function to create a NameServerRemoteNotify object for a
 *              specific slave processor.
 *
 *              This function creates an instance of the NameServerRemoteNotify
 *              module and returns an instance handle. The processor ID
 *              specified here is the ID of the slave processor as configured
 *              with the MultiProc module.
 *              Instance-level configuration needs to be provided to this
 *              function. If the user wishes to change some specific config
 *              parameters, then NameServerRemoteNotify_Params_init can be
 *              called to get the configuration filled with the default values.
 *              After this, only the required configuration values can be
 *              API, the params argument is not optional, since the user needs
 *              to provide some essential values such as sharedAddr etc. to be
 *              used with this NameServerRemoteNotify instance.
 *
 *  @param    procId     Remote processor ID with which this instance
 *                       communicates
 *  @param    params     Instance creation parameters
 *
 *  @sa       NameServerRemoteNotify_delete, NameServerRemoteNotifyDrv_ioctl
 */
NameServerRemoteNotify_Handle
NameServerRemoteNotify_create
                           (      UInt16                              procId,
                            const NameServerRemoteNotify_Params     * params)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                           status = NAMESERVERREMOTENOTIFY_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    NameServerRemoteNotify_Object * handle = NULL;
    NameServerRemoteNotifyDrv_CmdArgs  cmdArgs;
    Int32                              index;
    Ptr                                sharedAddr;

    GT_2trace (curTrace, GT_ENTER, "NameServerRemoteNotify_create", procId,
                                                                  params);

    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (params == NULL) {
        /*! @retval NULL params passed is null */
        status = NAMESERVERREMOTENOTIFY_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServerRemoteNotify_create",
                             NAMESERVERREMOTENOTIFY_E_INVALIDARG,
                             "params passed is null!");
    }
    else if (params->sharedAddrSize <
             NameServerRemoteNotify_sharedMemReq (params)) {
        /*! @retval NULL Less shared region size specified */
        status = NAMESERVERREMOTENOTIFY_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServerRemoteNotify_create",
                             NAMESERVERREMOTENOTIFY_E_INVALIDARG,
                             "Less shared region size specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.create.procId = procId;
        cmdArgs.args.create.params = (NameServerRemoteNotify_Params *) params;

        /* Translate sharedAddr to SrPtr. */
        sharedAddr = params->sharedAddr;
        GT_1trace (curTrace,
                   GT_2CLASS,
                   "    NameServerRemoteNotify_create: Shared addr [0x%x]\n",
                   sharedAddr);
        index = SharedRegion_getIndex (params->sharedAddr);
        cmdArgs.args.create.params->sharedAddr =
                                (Ptr) SharedRegion_getSRPtr (sharedAddr, index);
        GT_1trace (curTrace,
                   GT_2CLASS,
                   "    NameServerRemoteNotify_create: Shared addr"
                   " SrPtr [0x%x]\n",
                   cmdArgs.args.create.params->sharedAddr);

        /* Translate Gate handle to kernel-side gate handle. */
        cmdArgs.args.create.params->gate = Gate_getKnlHandle (params->gate);

        /* Get the kernel object for Notify driver. */
        cmdArgs.args.create.params->notifyDriver = ((Notify_CommonObject *)
                                            (params->notifyDriver))->knlObject;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        NameServerRemoteNotifyDrv_ioctl (CMD_NAMESERVERREMOTENOTIFY_CREATE,
                                       &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NameServerRemoteNotify_create",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            /* Allocate memory for the handle */
            handle = (NameServerRemoteNotify_Object *) Memory_calloc
                                        (NULL,
                                         sizeof (NameServerRemoteNotify_Object),
                                         0);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (handle == NULL) {
                /*! @retval NULL Memory allocation failed for handle */
                status = NAMESERVERREMOTENOTIFY_E_MEMORY;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "NameServerRemoteNotify_create",
                                     status,
                                     "Memory allocation failed for handle!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                /* Set pointer to kernel object into the user handle. */
                handle->knlObject = cmdArgs.args.create.handle;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
             }
         }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "NameServerRemoteNotify_create", handle);

    /*! @retval Valid-handle  Operation successful
     */
    return (NameServerRemoteNotify_Handle) handle;
}

/*
 *  @brief      Function to delete a NameServerRemoteNotify object for a
 *              specific slave processor.
 *
 *              Once this function is called, other NameServerRemoteNotify
 *              instance level APIs that require the instance handle cannot be
 *              called.
 *
 *  @param      handlePtr   Pointer to Handle to the NameServerRemoteNotify
 *                          object
 *
 *  @sa         NameServerRemoteNotify_create, NameServerRemoteNotifyDrv_ioctl
 */
Int
NameServerRemoteNotify_delete (NameServerRemoteNotify_Handle * handlePtr)
{
    Int                              status = NAMESERVERREMOTENOTIFY_SUCCESS;
    NameServerRemoteNotifyDrv_CmdArgs  cmdArgs;
    NameServerRemoteNotify_Object *    obj;

    GT_1trace (curTrace, GT_ENTER, "NameServerRemoteNotify_delete", handlePtr);

    GT_assert (curTrace, (handlePtr != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handlePtr == NULL) {
        /*! @retval NAMESERVERREMOTENOTIFY_E_INVALIDARG Invalid NULL handlePtr
                                                      pointer specified*/
        status = NAMESERVERREMOTENOTIFY_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServerRemoteNotify_delete",
                             status,
                             "Invalid NULL handlePtr pointer specified");
    }
    else if (*handlePtr == NULL) {
        /*! @retval NAMESERVERREMOTENOTIFY_E_HANDLE Invalid NULL handle
                                                  specified */
        status = NAMESERVERREMOTENOTIFY_E_HANDLE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServerRemoteNotify_delete",
                             status,
                             "Invalid NULL handle specified");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        obj = (NameServerRemoteNotify_Object *) *handlePtr;
        cmdArgs.args.deleteInstance.handle = obj->knlObject;
        status = NameServerRemoteNotifyDrv_ioctl (
                                        CMD_NAMESERVERREMOTENOTIFY_DELETE,
                                        &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NameServerRemoteNotify_delete",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "NameServerRemoteNotify_delete");

    /*! @retval NAMESERVERREMOTENOTIFY_SUCCESS Operation successful */
    return (status);
}

/*!
 *  @brief      Function to get remote name value pair.
 *
 *  @params     handle          Handle to the NameServerRemotetransport instance
 *  @param      instanceName    Name of the NameServer instance.
 *  @param      name            Name of the value.
 *  @param      value           Return parameter: Value
 *  @param      valueLen        Size of the value buffer in bytes
 *  @param      reserved        Reserved parameter, can be passed as NULL
 *
 *  @sa         NameServerRemoteNotifyDrv_ioctl
 */
Int
NameServerRemoteNotify_get (NameServerRemoteNotify_Handle   handle,
                            String                          instanceName,
                            String                          name,
                            Ptr                             value,
                            Int                             valueLen,
                            Ptr                             reserved)
{
    Int                               status = NAMESERVERREMOTENOTIFY_SUCCESS;
    NameServerRemoteNotify_Object *   obj = (NameServerRemoteNotify_Object *)
                                                                handle;
    NameServerRemoteNotifyDrv_CmdArgs cmdArgs;
    Int                               len = 0;

    GT_5trace (curTrace,
               GT_ENTER,
               "NameServerRemoteNotify_get",
               handle,
               instanceName,
               name,
               value,
               valueLen);

    GT_assert (curTrace, (handle != NULL));
    GT_assert (curTrace, (instanceName != NULL));
    GT_assert (curTrace, (name != NULL));
    GT_assert (curTrace, (value > 0u));
    GT_assert (curTrace, (valueLen > 0u));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServerRemoteNotify_get",
                             NAMESERVERREMOTENOTIFY_E_INVALIDARG,
                             "Handle passed is NULL!");
    }
    else if (instanceName == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServerRemoteNotify_get",
                             NAMESERVERREMOTENOTIFY_E_INVALIDARG,
                             "instanceName passed is NULL!");
    }
    else if (name == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServerRemoteNotify_get",
                             NAMESERVERREMOTENOTIFY_E_INVALIDARG,
                             "name passed is NULL!");
    }
    else if (value == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServerRemoteNotify_get",
                             NAMESERVERREMOTENOTIFY_E_INVALIDARG,
                             "value passed is NULL!");
    }
    else if (valueLen == 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServerRemoteNotify_get",
                             NAMESERVERREMOTENOTIFY_E_INVALIDARG,
                             "valueLen passed is 0!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.get.handle = obj->knlObject;
        cmdArgs.args.get.instanceName = instanceName;
        cmdArgs.args.get.instanceNameLen = String_len (instanceName) + 1;
        cmdArgs.args.get.name = name;
        cmdArgs.args.get.nameLen = String_len (name) + 1;
        cmdArgs.args.get.value = value;
        cmdArgs.args.get.valueLen = valueLen;
        cmdArgs.args.get.reserved = reserved;
        status = NameServerRemoteNotifyDrv_ioctl(CMD_NAMESERVERREMOTENOTIFY_GET,
                                                 &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NameServerRemoteNotify_get",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        len = cmdArgs.args.get.len;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "NameServerRemoteNotify_get", len);

    /*! @retval 0 Operation was not successful */
    /*! @retval +ve Operation was successful */
    return (len);
}


/*
 * @brief     Get shared memory requirements
 *
 * @param     params     Instance creation parameters
 *
 * @sa        None
 */
UInt32
NameServerRemoteNotify_sharedMemReq (
                                const NameServerRemoteNotify_Params * params)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                                 status = NAMESERVERREMOTENOTIFY_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    UInt32                              sharedMemSize = 0;
    /* TBD: UInt32                      key;*/
    NameServerRemoteNotifyDrv_CmdArgs   cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "NameServerRemoteNotify_sharedMemReq",
               params);

    /* NULL parameters are permitted for this module. */

    /* TBD: Enter critical section protection. */
    /* key = Gate_enter (NameServerRemoteNotify_state.gateHandle); */
    cmdArgs.args.sharedMemReq.params = (NameServerRemoteNotify_Params *) params;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    NameServerRemoteNotifyDrv_ioctl (CMD_NAMESERVERREMOTENOTIFY_SHAREDMEMREQ,
                                     &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (status < 0) {
        GT_setFailureReason (curTrace,
                          GT_4CLASS,
                          "NameServerRemoteNotify_sharedMemReq",
                          status,
                          "API (through IOCTL) failed on kernel-side!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        sharedMemSize = cmdArgs.args.sharedMemReq.sharedMemSize;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    /* TBD: Leave critical section protection. */
    /* Gate_leave (NameServerRemoteNotify_state.gateHandle, key); */

    GT_1trace (curTrace, GT_LEAVE, "NameServerRemoteNotify_sharedMemReq",
               sharedMemSize);

    /*! @retval Shared-memory-size Operation successful */
    return (sharedMemSize);
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
