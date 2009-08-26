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
 *  @file   MessageQTransportShm.c
 *
 *  @brief      MessageQ transport
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

/* Module level headers */
#include <MessageQ.h>
#include <MessageQTransportShm.h>
#include <MessageQTransportShmDrv.h>
#include <MessageQTransportShmDrvDefs.h>
#include <_NotifyDefs.h>


#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 * Macros and types
 * =============================================================================
 */
/*!
 *  @var    MESSAGEQTRANSPORTSHM_CACHESIZE
 *
 *  @brief  Cache size
 */
#define     MESSAGEQTRANSPORTSHM_CACHESIZE               128u


/*!
 *  @brief  MessageQTransportShm Module state object
 */
typedef struct MessageQTransportShm_ModuleObject_tag {
    UInt32                      setupRefCount;
    /*!< Reference count for number of times setup/destroy were called in this
         process. */
} MessageQTransportShm_ModuleObject;

/* Structure defining object for the Gate Peterson */
struct MessageQTransportShm_Object_tag {
    Ptr         knlObject;
    /*!< Pointer to the kernel-side MessageQTransportShm object. */
};


/* =============================================================================
 *  Globals
 * =============================================================================
 */
/*!
 *  @var    MessageQTransportShm_state
 *
 *  @brief  MessageQTransportShm state object variable
 */
#if !defined(SYSLINK_BUILD_DEBUG)
static
#endif /* if !defined(SYSLINK_BUILD_DEBUG) */
MessageQTransportShm_ModuleObject MessageQTransportShm_state =
{
    .setupRefCount = 0
};



/* =============================================================================
 * APIS
 * =============================================================================
 */
/*!
 *  @brief      Get the default configuration for the MessageQTransportShm
 *              module.
 *
 *              This function can be called by the application to get their
 *              configuration parameter to MessageQTransportShm_setup filled in
 *              by the MessageQTransportShm module with the default parameters.
 *              If the user does not wish to make any change in the default
 *              parameters, this API is not required to be called.
 *
 *  @param      cfg        Pointer to the MessageQTransportShm module
 *                         configuration structure in which the default config
 *                         is to be returned.
 *
 *  @sa         MessageQTransportShm_setup, MessageQTransportShmDrv_open,
 *              MessageQTransportShmDrv_ioctl, MessageQTransportShmDrv_close
 */
Void
MessageQTransportShm_getConfig (MessageQTransportShm_Config * cfgParams)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                        status = MESSAGEQTRANSPORTSHM_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    MessageQTransportShmDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "MessageQTransportShm_getConfig", cfgParams);

    GT_assert (curTrace, (cfgParams != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (cfgParams == NULL) {
        /*! @retval MESSAGEQTRANSPORTSHM_E_INVALIDARG Argument of type
         *  (MessageQTransportShm_Config *) passed is null
         */
        status = MESSAGEQTRANSPORTSHM_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                      GT_4CLASS,
                      "MessageQTransportShm_getConfig",
                      status,
                      "Argument of type (MessageQTransportShm_Config *) passed "
                      "is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Temporarily open the handle to get the configuration. */
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        MessageQTransportShmDrv_open ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MessageQTransportShm_getConfig",
                                 status,
                                 "Failed to open driver handle!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            cmdArgs.args.getConfig.config = cfgParams;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            MessageQTransportShmDrv_ioctl (CMD_MESSAGEQTRANSPORTSHM_GETCONFIG,
                                           &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                  GT_4CLASS,
                                  "MessageQTransportShm_getConfig",
                                  status,
                                  "API (through IOCTL) failed on kernel-side!");

            }
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Close the driver handle. */
        MessageQTransportShmDrv_close ();

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "MessageQTransportShm_getConfig");
}

/*!
 *  @brief      Setup the MessageQTransportShm module.
 *
 *              This function sets up the MessageQTransportShm module. This
 *              function must be called before any other instance-level APIs can
 *              be invoked.
 *              Module-level configuration needs to be provided to this
 *              function. If the user wishes to change some specific config
 *              parameters, then MessageQTransportShm_getConfig can be called to
 *              get the configuration filled with the default values. After
 *              this, only the required configuration values can be changed. If
 *              the user does not wish to make any change in the default
 *              parameters, the application can simply call
 *              MessageQTransportShm_setup with NULL parameters. The default
 *              parameters would get automatically used.
 *
 *  @param      cfg   Optional MessageQTransportShm module configuration.
 *                    If provided as NULL, default configuration is used.
 *
 *  @sa         MessageQTransportShm_destroy, MessageQTransportShmDrv_open,
 *              MessageQTransportShmDrv_ioctl
 */
Int
MessageQTransportShm_setup (const MessageQTransportShm_Config * config)
{
    Int                        status = MESSAGEQTRANSPORTSHM_SUCCESS;
    MessageQTransportShmDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "MessageQTransportShm_setup", config);

    /* TBD: Protect from multiple threads. */
    MessageQTransportShm_state.setupRefCount++;
    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */
    if (MessageQTransportShm_state.setupRefCount > 1) {
        /*! @retval MESSAGEQTRANSPORTSHM_S_ALREADYSETUP Success:
         *           MessageQTransportShm module has been already setup
         *           in this process
         */
        status = MESSAGEQTRANSPORTSHM_S_ALREADYSETUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "MessageQTransportShm module has been already setup in this"
                   " process.\n RefCount: [%d]\n",
                   MessageQTransportShm_state.setupRefCount);
    }
    else {
        /* Open the driver handle. */
        status = MessageQTransportShmDrv_open ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MessageQTransportShm_setup",
                                 status,
                                 "Failed to open driver handle!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            cmdArgs.args.setup.config = (MessageQTransportShm_Config *) config;
            status = MessageQTransportShmDrv_ioctl (
                                                CMD_MESSAGEQTRANSPORTSHM_SETUP,
                                                &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "MessageQTransportShm_setup",
                                     status,
                                     "API (through IOCTL) failed"
                                     " on kernel-side!");
            }
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

    GT_1trace (curTrace, GT_LEAVE, "MessageQTransportShm_setup", status);

    /*! @retval MESSAGEQTRANSPORTSHM_SUCCESS Operation Successful*/
    return status;
}

/*!
 *  @brief      Destroy the MessageQTransportShm module.
 *
 *              Once this function is called, other MessageQTransportShm module
 *              APIs, except for the MessageQTransportShm_getConfig API cannot
 *              be called anymore.
 *
 *  @sa         MessageQTransportShm_setup, MessageQTransportShmDrv_ioctl,
 *              MessageQTransportShmDrv_close
 */
Int
MessageQTransportShm_destroy (void)
{
    Int                           status = MESSAGEQTRANSPORTSHM_SUCCESS;
    MessageQTransportShmDrv_CmdArgs    cmdArgs;

    GT_0trace (curTrace, GT_ENTER, "MessageQTransportShm_destroy");

    /* TBD: Protect from multiple threads. */
    MessageQTransportShm_state.setupRefCount--;
    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */
    if (MessageQTransportShm_state.setupRefCount > 1) {
        /*! @retval MESSAGEQTRANSPORTSHM_S_ALREADYSETUP Success:
         *           MessageQTransportShm module has been already setup
         *           in this process
         */
        status = MESSAGEQTRANSPORTSHM_S_ALREADYSETUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "MessageQTransportShm module has been already setup in this"
                   " process.\n  RefCount: [%d]\n",
                   MessageQTransportShm_state.setupRefCount);
    }
    else {
        status = MessageQTransportShmDrv_ioctl(CMD_MESSAGEQTRANSPORTSHM_DESTROY,
                                              &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MessageQTransportShm_destroy",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

    /* Close the driver handle. */
    MessageQTransportShmDrv_close ();

    GT_1trace (curTrace, GT_LEAVE, "MessageQTransportShm_destroy", status);

    /*! @retval MESSAGEQTRANSPORTSHM_SUCCESS Operation Successful*/
    return status;
}


/*!
 *  @brief      Get Instance parameters
 *
 *  @param      handle          Handle to MessageQTransportShm instance
 *  @param      params          Instance creation parameters
 *
 *  @sa         MessageQ_create
 */
Void
MessageQTransportShm_Params_init (MessageQTransportShm_Handle          handle,
                                  MessageQTransportShm_Params        * params)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                           status = MESSAGEQTRANSPORTSHM_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    MessageQTransportShmDrv_CmdArgs  cmdArgs;

    GT_2trace (curTrace,
               GT_ENTER,
               "MessageQTransportShm_Params_init",
               handle,
               params);

    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (params == NULL) {
        /*! @retval None */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQTransportShm_Params_init",
                             MESSAGEQTRANSPORTSHM_E_INVALIDARG,
                             "Argument of type "
                             "(MessageQTransportShm_Params *) is "
                             "NULL!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        if (handle != NULL) {
            cmdArgs.args.ParamsInit.handle =
                            ((MessageQTransportShm_Object *) handle)->knlObject;
        }
        else {
            cmdArgs.args.ParamsInit.handle = handle;
        }
        cmdArgs.args.ParamsInit.params = params;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            MessageQTransportShmDrv_ioctl
                         (CMD_MESSAGEQTRANSPORTSHM_PARAMS_INIT,
                          &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MessageQTransportShm_Params_init",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "MessageQTransportShm_Params_init");
}


/*
 *  @brief    Create a transport instance
 *
 *  @param    procId     Destination processor id for notification
 *  @param    params     Instance creation parameters
 *
 *  @sa       MessageQTransportShm_delete
 *            Notify_registerEvent
 */
MessageQTransportShm_Handle
MessageQTransportShm_create
                           (      UInt16                              procId,
                            const MessageQTransportShm_Params       * params)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                              status = MESSAGEQTRANSPORTSHM_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    MessageQTransportShm_Object *    handle = NULL;
    MessageQTransportShmDrv_CmdArgs  cmdArgs;
    Int32                            index;

    GT_2trace (curTrace, GT_ENTER, "MessageQTransportShm_create", procId,
                                                                  params);

    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (params == NULL) {
        /*! @retval NULL params passed is null */
        status = MESSAGEQTRANSPORTSHM_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQTransportShm_create",
                             MESSAGEQTRANSPORTSHM_E_INVALIDARG,
                             "params passed is null!");
    }
    else if (params->sharedAddrSize <
             MessageQTransportShm_sharedMemReq (params)) {
        /*! @retval NULL Less shared region size specified */
        status = MESSAGEQTRANSPORTSHM_E_INVALIDSIZE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQTransportShm_create",
                             MESSAGEQTRANSPORTSHM_E_INVALIDSIZE,
                             "Less shared region size specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.create.procId = procId;
        cmdArgs.args.create.params = (MessageQTransportShm_Params *) params;

        /* Translate sharedAddr to SrPtr. */
        index = SharedRegion_getIndex (params->sharedAddr);
        cmdArgs.args.create.sharedAddrSrPtr = SharedRegion_getSRPtr (
                                                            params->sharedAddr,
                                                            index);

        /* Translate Gate handle to kernel-side gate handle. */
        cmdArgs.args.create.knlLockHandle = Gate_getKnlHandle (params->gate);

        /* Get the kernel object for Notify driver. */
        cmdArgs.args.create.knlNotifyDriver = ((Notify_CommonObject *)
                                            (params->notifyDriver))->knlObject;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        MessageQTransportShmDrv_ioctl (CMD_MESSAGEQTRANSPORTSHM_CREATE,
                                       &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MessageQTransportShm_create",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            /* Allocate memory for the handle */
            handle = (MessageQTransportShm_Object *) Memory_calloc
                                          (NULL,
                                           sizeof (MessageQTransportShm_Object),
                                           0);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (handle == NULL) {
                /*! @retval NULL Memory allocation failed for handle */
                status = MESSAGEQTRANSPORTSHM_E_MEMORY;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "MessageQTransportShm_create",
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

    GT_1trace (curTrace, GT_LEAVE, "MessageQTransportShm_create", handle);

    /*! @retval Valid-handle  Operation successful
     */
    return (MessageQTransportShm_Handle) handle;
}

/*
 *  @brief    Delete instance
 *
 *  @param    obj        Handle to MessageQTransportShm instance
 *
 *  @sa       MessageQTransportShm_create
 *            MessageQTransportShm_delete
 *            MessageQTransportShm_close
 *            Notify_unregisterEvent
 */
Int
MessageQTransportShm_delete (MessageQTransportShm_Handle * handlePtr)
{
    Int                              status = MESSAGEQTRANSPORTSHM_SUCCESS;
    MessageQTransportShmDrv_CmdArgs  cmdArgs;
    MessageQTransportShm_Object *    obj;

    GT_1trace (curTrace, GT_ENTER, "MessageQTransportShm_delete", handlePtr);

    GT_assert (curTrace, (handlePtr != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handlePtr == NULL) {
        /*! @retval MESSAGEQTRANSPORTSHM_E_INVALIDARG Invalid NULL handlePtr
                                                      pointer specified*/
        status = MESSAGEQTRANSPORTSHM_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQTransportShm_delete",
                             status,
                             "Invalid NULL handlePtr pointer specified");
    }
    else if (*handlePtr == NULL) {
        /*! @retval MESSAGEQTRANSPORTSHM_E_HANDLE Invalid NULL handle
                                                  specified */
        status = MESSAGEQTRANSPORTSHM_E_HANDLE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQTransportShm_delete",
                             status,
                             "Invalid NULL handle specified");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        obj = (MessageQTransportShm_Object *) *handlePtr;
        cmdArgs.args.deleteTransport.handle = obj->knlObject;
        status = MessageQTransportShmDrv_ioctl (CMD_MESSAGEQTRANSPORTSHM_DELETE,
                                                &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MessageQTransportShm_delete",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        Memory_free (NULL, obj, sizeof (MessageQTransportShm_Object));
        *handlePtr = NULL;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "MessageQTransportShm_delete");

    /*! @retval MESSAGEQTRANSPORTSHM_SUCCESS Operation successful */
    return (status);
}

/*
 * @brief     Put msg to remote list
 *
 * @param     obj        Handle to MessageQTransportShm instance
 * @param     msg        Message to be delivered to remote list
 *
 * @sa        Notify_sendEvent
 *            MessageQTransportShm_putTail
 */
Int
MessageQTransportShm_put (MessageQTransportShm_Handle  handle,
                          Ptr                          msg)
{
    Int                           status = MESSAGEQTRANSPORTSHM_SUCCESS;
    MessageQTransportShm_Object * obj = (MessageQTransportShm_Object *) handle;
    MessageQTransportShmDrv_CmdArgs  cmdArgs;
    Int32               index;

    GT_2trace (curTrace, GT_ENTER, "MessageQTransportShm_put", handle, msg);

    GT_assert (curTrace, (msg != NULL));
    GT_assert (curTrace, (obj != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (msg == NULL) {
        status = MESSAGEQTRANSPORTSHM_E_INVALIDARG;
        /*! @retval MESSAGEQTRANSPORTSHM_E_INVALIDARG
         *          Invalid NULL msg pointer specified
         */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQTransportShm_put",
                             status,
                             "Invalid NULL msg pointer specified");
    }
    else if (obj == NULL) {
        /*! @retval MESSAGEQTRANSPORTSHM_E_INVALIDARG
         *          Invalid NULL obj pointer specified
         */
        status = MESSAGEQTRANSPORTSHM_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQTransportShm_put",
                             status,
                             "Invalid NULL obj pointer specified");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.put.handle = obj->knlObject;
        index = SharedRegion_getIndex (msg);
        cmdArgs.args.put.msgSrPtr = SharedRegion_getSRPtr (msg, index);
        status = MessageQTransportShmDrv_ioctl (CMD_MESSAGEQTRANSPORTSHM_PUT,
                                                &cmdArgs);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MessageQTransportShm_put",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "MessageQTransportShm_put", status);

    /*! @retval MESSAGEQTRANSPORTSHM_SUCCESS Operation successful */
    return (status);
}

/*
 * @brief     Control Function
 *
 * @param     obj        Handle to MessageQTransportShm instance
 * @param     cmd        Command ID
 * @param     cmdArg     Command specific arguments
 *
 * @sa        None
 */
Int
MessageQTransportShm_control (MessageQTransportShm_Handle   obj,
                              UInt                          cmd,
                              UArg                          cmdArg)
{
    GT_3trace (curTrace,
               GT_ENTER,
               "MessageQTransportShm_control",
               obj,
               cmd,
               cmdArg);

    GT_assert (curTrace, (obj != NULL));

    GT_1trace (curTrace,
               GT_LEAVE,
               "MessageQTransportShm_control",
               MESSAGEQTRANSPORTSHM_E_NOTSUPPORTED);

    /*! @retval MESSAGEQTRANSPORTSHM_E_NOTSUPPORTED Specified operation is not
                                                    supported. */
    return (MESSAGEQTRANSPORTSHM_E_NOTSUPPORTED);
}

/*
 * @brief     Get status
 *
 * @param     obj        Handle to MessageQTransportShm instance
 *
 * @sa        None
 */
MessageQTransportShm_Status
MessageQTransportShm_getStatus (MessageQTransportShm_Handle handle)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                           status = MESSAGEQTRANSPORTSHM_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    MessageQTransportShm_Status   msgStatus = MessageQTransportShm_Status_INIT;
    MessageQTransportShm_Object * obj = (MessageQTransportShm_Object *) handle;

    MessageQTransportShmDrv_CmdArgs  cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "MessageQTransportShm_getStatus", handle);

    GT_assert (curTrace, (handle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        /*! @retval MESSAGEQTRANSPORTSHM_E_INVALIDARG
         *          Invalid NULL obj pointer specified
         */
        status = MESSAGEQTRANSPORTSHM_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQTransportShm_getStatus",
                             status,
                             "Invalid NULL obj pointer specified");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.getStatus.handle = obj->knlObject;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            MessageQTransportShmDrv_ioctl (CMD_MESSAGEQTRANSPORTSHM_GETSTATUS,
                                           &cmdArgs);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                    GT_4CLASS,
                    "MessageQTransportShm_getStatus",
                    status,
                    "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            msgStatus = cmdArgs.args.getStatus.status;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    /*! @retval Status-of-MessageQ-Transport Operation successful. */
    return (msgStatus);
}


/*
 * @brief     Get shared memory requirements
 *
 * @param     params     Instance creation parameters
 *
 * @sa        None
 */
UInt32
MessageQTransportShm_sharedMemReq (const MessageQTransportShm_Params * params)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                              status    = MESSAGEQTRANSPORTSHM_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    UInt32                           totalSize = 0;
    MessageQTransportShmDrv_CmdArgs  cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "MessageQTransportShm_sharedMemReq", params);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (params == NULL) {
        /*! @retval MESSAGEQTRANSPORTSHM_E_INVALIDARG
         *          Invalid NULL params specified
         */
        status = MESSAGEQTRANSPORTSHM_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQTransportShm_sharedMemReq",
                             status,
                             "Invalid NULL params specified");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.sharedMemReq.params = (MessageQTransportShm_Params *)
                                                                        params;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        MessageQTransportShmDrv_ioctl (CMD_MESSAGEQTRANSPORTSHM_SHAREDMEMREQ,
                                       &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                    GT_4CLASS,
                    "MessageQTransportShm_sharedMemReq",
                    status,
                    "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            totalSize = cmdArgs.args.sharedMemReq.bytes;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace,
               GT_LEAVE,
               "MessageQTransportShm_sharedMemReq",
               totalSize);

    /*! @retval Shared-memory-requirements Operation successfully completed. */
    return (totalSize);
}


/*
 * @brief     Set the asynchronous error function for the transport module
 *
 * @param     errFxn     Error function to be set
 *
 * @sa        None
 */
Void
MessageQTransportShm_setErrFxn (MessageQTransportShm_ErrFxn errFxn)
{
    /* TBD: To be implemented. Should use local Notify to callback user fxn. */
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
