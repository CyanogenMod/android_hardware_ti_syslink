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
 *  @file   MessageQ.c
 *
 *  @brief      MessageQ module implementation
 *
 *  The MessageQ module supports the structured sending and receiving of
 *  variable length messages. This module can be used for homogeneous or
 *  heterogeneous multi-processor messaging.
 *
 *  MessageQ provides more sophisticated messaging than other modules. It is
 *  typically used for complex situations such as multi-processor messaging.
 *
 *  The following are key features of the MessageQ module:
 *  -Writers and readers can be relocated to another processor with no
 *   runtime code changes.
 *  -Timeouts are allowed when receiving messages.
 *  -Readers can determine the writer and reply back.
 *  -Receiving a message is deterministic when the timeout is zero.
 *  -Messages can reside on any message queue.
 *  -Supports zero-copy transfers.
 *  -Can send and receive from any type of thread.
 *  -Notification mechanism is specified by application.
 *  -Allows QoS (quality of service) on message buffer pools. For example,
 *   using specific buffer pools for specific message queues.
 *
 *  Messages are sent and received via a message queue. A reader is a thread
 *  that gets (reads) messages from a message queue. A writer is a thread that
 *  puts (writes) a message to a message queue. Each message queue has one
 *  reader and can have many writers. A thread may read from or write to multiple
 *  message queues.
 *
 *  Conceptually, the reader thread owns a message queue. The reader thread
 *  creates a message queue. Writer threads  a created message queues to
 *  get access to them.
 *
 *  Message queues are identified by a system-wide unique name. Internally,
 *  MessageQ uses the NameServermodule for managing
 *  these names. The names are used for opening a message queue. Using
 *  names is not required.
 *
 *  Messages must be allocated from the MessageQ module. Once a message is
 *  allocated, it can be sent on any message queue. Once a message is sent, the
 *  writer loses ownership of the message and should not attempt to modify the
 *  message. Once the reader receives the message, it owns the message. It
 *  may either free the message or re-use the message.
 *
 *  Messages in a message queue can be of variable length. The only
 *  requirement is that the first field in the definition of a message must be a
 *  MsgHeader structure. For example:
 *  typedef struct MyMsg {
 *      MessageQ_MsgHeader header;
 *      ...
 *  } MyMsg;
 *
 *  The MessageQ API uses the MessageQ_MsgHeader internally. Your application
 *  should not modify or directly access the fields in the MessageQ_MsgHeader.
 *
 *  All messages sent via the MessageQ module must be allocated from a
 *  Heap implementation. The heap can be used for
 *  other memory allocation not related to MessageQ.
 *
 *  An application can use multiple heaps. The purpose of having multiple
 *  heaps is to allow an application to regulate its message usage. For
 *  example, an application can allocate critical messages from one heap of fast
 *  on-chip memory and non-critical messages from another heap of slower
 *  external memory
 *
 *  MessageQ does support the usage of messages that allocated via the
 *  alloc function. Please refer to the staticMsgInit
 *  function description for more details.
 *
 *  In a multiple processor system, MessageQ communications to other
 *  processors via MessageQTransport instances. There must be one and
 *  only one MessageQTransport instance for each processor where communication
 *  is desired.
 *  So on a four processor system, each processor must have three
 *  MessageQTransport instance.
 *
 *  The user only needs to create the MessageQTransport instances. The instances
 *  are responsible for registering themselves with MessageQ.
 *  This is accomplished via the registerTransport function.
 *
 *  ============================================================================
 */



/* Standard headers */
#include <Std.h>

/* Osal And Utils  headers */
#include <String.h>
#include <List.h>
#include <Trace.h>
#include <Memory.h>

/* Module level headers */
#include <MultiProc.h>
#include <MessageQ.h>
#include <MessageQDrvDefs.h>
#include <MessageQDrv.h>
#include <SharedRegion.h>
#include <Heap.h>


#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 * Structures & Enums
 * =============================================================================
 */

/* Structure defining object for the Gate Peterson */
struct MessageQ_Object_tag {
    Ptr              knlObject;
    /*!< Pointer to the kernel-side MessageQ object. */
    MessageQ_QueueId queueId;
    /* Unique id */
};

/*!
 *  @brief  MessageQ Module state object
 */
typedef struct MessageQ_ModuleObject_tag {
    UInt32          setupRefCount;
    /*!< Reference count for number of times setup/destroy were called in this
         process. */
} MessageQ_ModuleObject;


/* =============================================================================
 *  Globals
 * =============================================================================
 */
/*!
 *  @var    MessageQ_state
 *
 *  @brief  MessageQ state object variable
 */
#if !defined(SYSLINK_BUILD_DEBUG)
static
#endif /* if !defined(SYSLINK_BUILD_DEBUG) */
MessageQ_ModuleObject MessageQ_state =
{
    .setupRefCount = 0
};


/* =============================================================================
 * Constants
 * =============================================================================
 */
/*
 *  Used to denote a message that was initialized
 *  with the MessageQ_staticMsgInit function.
 */
#define MESSAGEQ_STATICMSG              0xFFFF

/* =============================================================================
 * APIS
 * =============================================================================
 */
/*!
 *  @brief      Function to get the default configuration for the MessageQ
 *              module.
 *
 *              This function can be called by the application to get their
 *              configuration parameter to MessageQ_setup filled in by the
 *              MessageQ module with the default parameters. If the user does
 *              not wish to make any change in the default parameters, this API
 *              is not required to be called.
 *
 *  @param      cfg     Pointer to the MessageQ module configuration structure
 *                      in which the default config is to be returned.
 *
 *
 *  @sa         MessageQ_setup, MessageQDrv_open, MessageQDrv_ioctl,
 *              MessageQDrv_close
 */
Void
MessageQ_getConfig (MessageQ_Config * cfg)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int status = MESSAGEQ_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    MessageQDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "MessageQ_getConfig", cfg);

    GT_assert (curTrace, (cfg != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (cfg == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_getConfig",
                             MESSAGEQ_E_INVALIDARG,
                             "Argument of type (MessageQ_Config *) passed "
                             "is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Temporarily open the handle to get the configuration. */
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        MessageQDrv_open ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MessageQ_getConfig",
                                 status,
                                 "Failed to open driver handle!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            cmdArgs.args.getConfig.config = cfg;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            MessageQDrv_ioctl (CMD_MESSAGEQ_GETCONFIG, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                  GT_4CLASS,
                                  "MessageQ_getConfig",
                                  status,
                                  "API (through IOCTL) failed on kernel-side!");

            }
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Close the driver handle. */
        MessageQDrv_close ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_ENTER, "MessageQ_getConfig");
}


/*!
 *  @brief      Function to setup the MessageQ module.
 *
 *              This function sets up the MessageQ module. This function must
 *              be called before any other instance-level APIs can be invoked.
 *              Module-level configuration needs to be provided to this
 *              function. If the user wishes to change some specific config
 *              parameters, then MessageQ_getConfig can be called to get the
 *              configuration filled with the default values. After this, only
 *              the required configuration values can be changed. If the user
 *              does not wish to make any change in the default parameters, the
 *              application can simply call MessageQ with NULL parameters.
 *              The default parameters would get automatically used.
 *
 *  @param      cfg   Optional MessageQ module configuration. If provided as
 *                    NULL, default configuration is used.
 *
 *  @sa         MessageQ_destroy, MessageQDrvUsr_open, MessageQDrvUsr_ioctl
 */
Int
MessageQ_setup (const MessageQ_Config * config)
{
    Int                 status = MESSAGEQ_SUCCESS;
    MessageQDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "MessageQ_setup", config);

    /* TBD: Protect from multiple threads. */
    MessageQ_state.setupRefCount++;
    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */
    if (MessageQ_state.setupRefCount > 1) {
        /*! @retval MESSAGEQ_S_ALREADYSETUP Success:
         *          MessageQ module has been
         *          already setup in this process
         */
        status = MESSAGEQ_S_ALREADYSETUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "MessageQ module has been already setup in this process.\n"
                   "    RefCount: [%d]\n",
                   MessageQ_state.setupRefCount);
    }
    else {
        /* Open the driver handle. */
        status = MessageQDrv_open ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MessageQ_setup",
                                 status,
                                 "Failed to open driver handle!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            cmdArgs.args.setup.config = (MessageQ_Config *) config;
            status = MessageQDrv_ioctl (CMD_MESSAGEQ_SETUP, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "MessageQ_setup",
                                     status,
                                     "API (through IOCTL) failed on kernel-side!");
            }
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

    GT_1trace (curTrace, GT_LEAVE, "MessageQ_setup", status);

    /*! @retval MESSAGEQ_SUCCESS Operation Successsful */
    return status;
}


/*!
 *  @brief      Function to destroy the MessageQ module.
 *
 *              Once this function is called, other MessageQ module APIs, except
 *              for the MessageQ_getConfig API cannot be called anymore.
 *
 *  @sa         MessageQ_setup, MessageQDrvUsr_ioctl, MessageQDrvUsr_close
 */
Int
MessageQ_destroy (void)
{
    Int                 status = MESSAGEQ_SUCCESS;
    MessageQDrv_CmdArgs    cmdArgs;

    GT_0trace (curTrace, GT_ENTER, "MessageQ_destroy");

    /* TBD: Protect from multiple threads. */
    MessageQ_state.setupRefCount--;
    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */
    if (MessageQ_state.setupRefCount >= 1) {
        /*! @retval MESSAGEQ_S_ALREADYSETUP Success:
         *          MessageQ module has been already setup in this
         *          process
         */
        status = MESSAGEQ_S_ALREADYSETUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "MessageQ module has been already setup in this process.\n"
                   "    RefCount: [%d]\n",
                   MessageQ_state.setupRefCount);
    }
    else {
        status = MessageQDrv_ioctl (CMD_MESSAGEQ_DESTROY, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MessageQ_destroy",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

        /* Close the driver handle. */
        MessageQDrv_close ();
    }

    GT_1trace (curTrace, GT_LEAVE, "MessageQ_destroy", status);

    /*! @retval MESSAGEQ_SUCCESS Operation Successsful */
    return status;
}


/*!
 *  @brief      Function to initialize the parameters for the MessageQ instance.
 *
 *              This function can be called by the application to get their
 *              configuration parameter to #MessageQ_create filled in by the
 *              MessageQ module with the default parameters.
 *
 *  @param      handle   Handle to the MessageQ object. If specified as NULL,
 *                       the default global configuration values are returned.
 *  @param      params   Pointer to the MessageQ instance params structure in
 *                       which the default params is to be returned.
 *
 *  @sa         MessageQ_create, MessageQDrvUsr_ioctl
 */
Void
MessageQ_Params_init (MessageQ_Handle         handle,
                      MessageQ_Params       * params)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int               status = MESSAGEQ_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    MessageQDrv_CmdArgs   cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "MessageQ_Params_init", params);

    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (MessageQ_state.setupRefCount == 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_Params_init",
                             MESSAGEQ_E_INVALIDSTATE,
                             "Modules is not initialized!");
    }
    else if (params == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_Params_init",
                             MESSAGEQ_E_INVALIDARG,
                             "Argument of type (MessageQ_Params *) passed "
                             "is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        if (handle != NULL) {
            cmdArgs.args.ParamsInit.handle =
                                    ((MessageQ_Object *) handle)->knlObject;
        }
        else {
            cmdArgs.args.ParamsInit.handle = handle;
        }
        cmdArgs.args.ParamsInit.params = params;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        MessageQDrv_ioctl (CMD_MESSAGEQ_PARAMS_INIT, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MessageQ_Params_init",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "MessageQ_Params_init");

    /* @retval  None */
    return;
}


/*!
 *  @brief      Function to create a MessageQ object.
 *
 *              This function creates an instance of the MessageQ module and
 *              returns an instance handle, which is used to access the
 *              specified MessageQ.
 *              Instance-level configuration needs to be provided to this
 *              function. If the user wishes to change some specific config
 *              parameters, then #MessageQ_Params_init can be called to get the
 *              configuration filled with the default values. After this, only
 *              the required configuration values can be changed.
 *
 *  @param      name    Name of the Message Queue to be created.
 *  @param      params  Instance config-params structure.
 *
 *  @sa         MessageQ_delete, Memory_calloc, MessageQDrvUsr_ioctl
 */
MessageQ_Handle
MessageQ_create (      String            name,
                 const MessageQ_Params * params)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                   status = 0;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    MessageQ_Object *     handle = NULL;
    MessageQDrv_CmdArgs   cmdArgs;

    GT_2trace (curTrace, GT_ENTER, "MessageQ_create", name, params);

    /* NULL name is allowed for unnamed (anonymous queues) */
    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (MessageQ_state.setupRefCount == 0) {
        /* @retval  NULL Module is not initialized */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_create",
                             MESSAGEQ_E_INVALIDSTATE,
                             "Modules is not initialized!");
    }
    else if (params == NULL) {
        /* @retval  NULL Invalid NULL params pointer specified */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_create",
                             MESSAGEQ_E_INVALIDARG,
                             "Invalid NULL params pointer specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.create.params = (MessageQ_Params *) params;
        cmdArgs.args.create.name = name;
        if (name != NULL) {
            cmdArgs.args.create.nameLen = (String_len (name) + 1);
        }
        else {
            cmdArgs.args.create.nameLen = 0;
        }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        MessageQDrv_ioctl (CMD_MESSAGEQ_CREATE, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            /* @retval  NULL API (through IOCTL) failed on kernel-side */
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MessageQ_create",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            /* Allocate memory for the handle */
            handle = (MessageQ_Object *) Memory_calloc (NULL,
                                                       sizeof (MessageQ_Object),
                                                       0);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (handle == NULL) {
                /*! @retval NULL Memory allocation failed for handle */
                status = MESSAGEQ_E_MEMORY;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "MessageQ_create",
                                     status,
                                     "Memory allocation failed for handle!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                /* Set pointer to kernel object into the user handle. */
                handle->knlObject = cmdArgs.args.create.handle;
                handle->queueId = cmdArgs.args.create.queueId;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
             }
         }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "MessageQ_create", handle);

    /*! @retval valid-handle Operation Successful*/
    return (MessageQ_Handle) handle;
}


/*!
 *  @brief      Function to delete a MessageQ object for a specific slave
 *              processor.
 *
 *              Once this function is called, other MessageQ instance level APIs
 *              that require the instance handle cannot be called.
 *
 *  @param      handlePtr  Pointer to Handle to the MessageQ object
 *                         Reset to NULL when the function successfully
 *                         completes.
 *
 *  @sa         MessageQ_create
 */
Int
MessageQ_delete (MessageQ_Handle * handlePtr)
{
    Int                 status = MESSAGEQ_SUCCESS;
    MessageQDrv_CmdArgs    cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "MessageQ_delete", handlePtr);

    GT_assert (curTrace, (handlePtr != NULL));
    GT_assert (curTrace, ((handlePtr != NULL) && (*handlePtr != NULL)));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (MessageQ_state.setupRefCount == 0) {
        /*! @retval MESSAGEQ_E_INVALIDSTATE Modules is in an invalid state*/
        status = MESSAGEQ_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_delete",
                             status,
                             "Modules is in an invalid state!");
    }
    else if (handlePtr == NULL) {
        /*! @retval MESSAGEQ_E_INVALIDARG handlePtr pointer passed is NULL*/
        status = MESSAGEQ_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_delete",
                             status,
                             "handlePtr pointer passed is NULL!");
    }
    else if (*handlePtr == NULL) {
        /*! @retval MESSAGEQ_E_INVALIDARG *handlePtr passed is NULL*/
        status = MESSAGEQ_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_delete",
                             status,
                             "*handlePtr passed is NULL!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.deleteMessageQ.handle =
                            ((MessageQ_Object *)(*handlePtr))->knlObject;
        status = MessageQDrv_ioctl (CMD_MESSAGEQ_DELETE, &cmdArgs);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MessageQ_delete",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            Memory_free (NULL, *handlePtr, sizeof (MessageQ_Object));
            *handlePtr = NULL;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "MessageQ_delete", status);

    /*! @retval MESSAGEQ_SUCCESS Operation Successful */
    return status;
}


/*!
 *  @brief      Opens a created instance of MessageQ module.
 *
 *  @param      name        Name of Message Queue to be opened
 *  @param      queueId     Return parameter: Opened Message Queue ID that can
 *                          be used for #MessageQ_put
 *
 *  @sa         MessageQ_create, MessageQ_delete, MessageQ_close
 *              NameServer_get
 */
Int
MessageQ_open (String name, MessageQ_QueueId * queueId)
{
    Int32            status = MESSAGEQ_SUCCESS;
    MessageQDrv_CmdArgs cmdArgs;

    GT_2trace (curTrace, GT_ENTER, "MessageQ_open", name, queueId);

    GT_assert (curTrace, (queueId != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (MessageQ_state.setupRefCount == 0) {
        /*! @retval MESSAGEQ_E_INVALIDSTATE Modules is in an invalid state */
        status = MESSAGEQ_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_open",
                             status,
                             "Modules is in an invalid state!");
    }
    else if (name == NULL) {
        /*! @retval MESSAGEQ_E_INVALIDARG name passed is NULL */
        status = MESSAGEQ_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_open",
                             status,
                             "name passed is NULL!");
    }
    else if (queueId == NULL) {
        /*! @retval MESSAGEQ_E_INVALIDARG name passed is NULL */
        status = MESSAGEQ_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_open",
                             status,
                             "name passed is NULL!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.open.name = name;
        if (name != NULL) {
            cmdArgs.args.open.nameLen = (String_len (name) + 1);
        }
        else {
            cmdArgs.args.open.nameLen = 0;
        }

        /* Initialize return queue ID to invalid. */
        *queueId = MESSAGEQ_INVALIDMESSAGEQ;

        status = MessageQDrv_ioctl (CMD_MESSAGEQ_OPEN, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MessageQ_open",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            *queueId = cmdArgs.args.open.queueId;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "MessageQ_open", status);

    /*! @retval MESSAGEQ_SUCCESS Operation Successful */
    return status;
}


/*!
 *  @brief      Closes previously opened/created instance of MessageQ module.
 *
 *  @param      queueId    Pointer to ID for the opened queue. This is the
 *                         ID that was returned from #MessageQ_open.
 *                         Reset to invalid when the function successfully
 *                         completes.
 *
 *  @sa         MessageQ_create, MessageQ_delete, MessageQ_open
 */
Void
MessageQ_close (MessageQ_QueueId * queueId)
{
    Int32               status = MESSAGEQ_SUCCESS;
    MessageQDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "MessageQ_close", queueId);

    GT_assert (curTrace, (queueId != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (MessageQ_state.setupRefCount == 0) {
        /* @retval  NULL Module is not initialized */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_close",
                             MESSAGEQ_E_INVALIDSTATE,
                             "Modules is not initialized!");
    }
    else if (queueId == NULL) {
        /*! @retval MESSAGEQ_E_INVALIDARG queueId passed is null */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_close",
                             MESSAGEQ_E_INVALIDARG,
                             "queueId passed is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.close.queueId = *queueId;
        status = MessageQDrv_ioctl (CMD_MESSAGEQ_CLOSE, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MessageQ_close",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            *queueId = MESSAGEQ_INVALIDMESSAGEQ;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "MessageQ_close");

    /*! @retval MESSAGEQ_SUCCESS Operation Successsful */
    return;
}


/*
 *  @brief   Place a message onto a message queue.
 *           <br>
 *           This call places the message onto the specified message queue.
 *           The message queue could be local or remote. The MessageQ module
 *           manages the delivery.
 *           <br>
 *           In the case where the queue is remote, MessageQ does not guarantee
 *           that the message is actually delivered before the MessageQ_put call
 *           returns.
 *           <br>
 *           The queue must have been returned from one of the following
 *           functions:<br>
 *           - #MessageQ_open<br>
 *           - #MessageQ_getReplyQueue<br>
 *           - #MessageQ_getDstQueue<br>
 *           <br>
 *           After the message is placed onto the final destination, the queue's
 *           synchronizer signal function is called.<br>
 *           The application loses ownership of the message once put is called.
 *
 *  @param   queueId    ID of the destination message queue
 *  @param   msg        Message to be sent
 *
 *  @sa      MessageQ_get
 */
Int
MessageQ_put (MessageQ_QueueId queueId,
              MessageQ_Msg     msg)
{
    Int                 status = MESSAGEQ_SUCCESS;
    MessageQDrv_CmdArgs cmdArgs;
    Int32               index;

    GT_2trace (curTrace, GT_ENTER, "MessageQ_put", queueId, msg);

    GT_assert (curTrace, (msg != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (MessageQ_state.setupRefCount == 0) {
        /* @retval  MESSAGEQ_E_INVALIDSTATE Module is not initialized */
        status = MESSAGEQ_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_put",
                             status,
                             "Modules is not initialized!");
    }
    else if (msg  == NULL) {
        /*! @retval MESSAGEQ_E_INVALIDARG Argument of type
         *                             (MessageQ_Config *) passed is null
         */
        status = MESSAGEQ_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_put",
                             status,
                             "Argument of type (MessageQ_Config *) "
                             "passed is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.put.queueId  = queueId;
        index = SharedRegion_getIndex (msg);
        cmdArgs.args.put.msgSrPtr = SharedRegion_getSRPtr (msg, index);

        status = MessageQDrv_ioctl (CMD_MESSAGEQ_PUT, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MessageQ_put",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "MessageQ_put", status);

    /*! @retval Operation Successsful */
    return (status);
}


/*
 *  @brief   Gets a message for a message queue and blocks if the queue is
 *           empty.
 *           <br>
 *           If a message is present, it returns it.  Otherwise it blocks
 *           waiting for a message to arrive.
 *           <br>
 *           When a message is returned, it is owned by the caller.
 *
 *  @param   handle     Handle to the Message Queue
 *  @param   msg        Location to receive the message pointer
 *  @param   timeout    Timeout to wait for
 *
 *  @sa      MessageQ_put
 */
Int
MessageQ_get (MessageQ_Handle handle, MessageQ_Msg * msg ,UInt timeout)
{
    Int                 status   = MESSAGEQ_SUCCESS;
    SharedRegion_SRPtr  msgSrPtr = SHAREDREGION_INVALIDSRPTR;
    MessageQDrv_CmdArgs cmdArgs;

    GT_2trace (curTrace, GT_ENTER, "MessageQ_get", handle, timeout);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        /*! @retval NULL Invalid NULL obj pointer specified */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_count",
                             MESSAGEQ_E_INVALIDMSG,
                             "obj pointer passed is null!");
    }
    else if (msg == NULL) {
        /*! @retval MESSAGEQ_E_INVALIDARG Invalid NULL msg pointer specified */
        status = MESSAGEQ_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_get",
                             status,
                             "msg pointer passed is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.get.handle = ((MessageQ_Object *)(handle))->knlObject;
        cmdArgs.args.get.timeout = timeout;

        /* Initialize return message pointer to NULL. */
        *msg = NULL;
        status = MessageQDrv_ioctl (CMD_MESSAGEQ_GET, &cmdArgs);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MessageQ_get",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            msgSrPtr = cmdArgs.args.get.msgSrPtr;
            if (msgSrPtr != SHAREDREGION_INVALIDSRPTR) {
                *msg = (MessageQ_Msg) SharedRegion_getPtr (msgSrPtr);
            }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "MessageQ_get", status);

    /*! @retval MESSAGEQ_SUCCESS Operation successfully completed. */
    return (status);
}


/*
 *  @brief   Return a count of the number of messages in the queue
 *
 *  @param   handle     Handle to the Message Queue
 *
 *  @sa      None
 */
Int
MessageQ_count (MessageQ_Handle handle)
{
    Int32               status = MESSAGEQ_SUCCESS;
    Int                 count  = 0;
    MessageQDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "MessageQ_count", handle);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (MessageQ_state.setupRefCount == 0) {
        /* @retval 0 Module is not initialized */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_count",
                             MESSAGEQ_E_INVALIDSTATE,
                             "Modules is not initialized!");
    }
    else if (handle == NULL) {
        /*! @retval 0 Invalid NULL obj pointer specified */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_count",
                             MESSAGEQ_E_INVALIDMSG,
                             "Invalid NULL obj pointer specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.count.handle = ((MessageQ_Object *)(handle))->knlObject;
        status = MessageQDrv_ioctl (CMD_MESSAGEQ_COUNT, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MessageQ_get",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            count = cmdArgs.args.count.count;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "MessageQ_count", count);

    /*! @retval Number-of-messages-in-MessageQ Operation Successsful */
    return (count);
}


/*
 *  @brief  Initializes a message not obtained from #MessageQ_alloc.
 *          <br>
 *          There are several fields in the #MessageQ_MsgHeader that are
 *          initialized by the #MessageQ_alloc function. The
 *          #MessageQ_staticMsgInit can be used to initial these fields for
 *          messages that are not allocated from MessageQ.
 *          <br>
 *          There is one strict constraint with using messages not allocated
 *          from MessageQ. The message cannot be free via #MessageQ_free
 *          function. This includes:<br>
 *          - The application calling #MessageQ_free on the same processor
 *          - The application calling #MessageQ_free on a different processor
 *          - The application cannot send the message to another processor
 *            where the transport might call #MessageQ_free on the message. For
 *            example, any copy based transport calls free after sending the
 *            message.
 *          <br>
 *          If a staticMsgInit'd msg is passed to #MessageQ_free, an error is
 *          returned.
 *
 *  @param  msg     Message to be initialized
 *  @param  size    Size of the message
 *
 *
 *  @sa     None
 */
Void
MessageQ_staticMsgInit (MessageQ_Msg msg, UInt32 size)
{
    GT_2trace (curTrace, GT_ENTER, "MessageQ_staticMsgInit", msg, size);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (MessageQ_state.setupRefCount == 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_staticMsgInit",
                             MESSAGEQ_E_INVALIDSTATE,
                             "Modules is not initialized!");
    }
    else if (msg == NULL) {
        /*! @retval None */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_staticMsgInit",
                             MESSAGEQ_E_INVALIDMSG,
                             "Msg is invalid!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Fill in the fields of the message */
        msg->heapId  = MESSAGEQ_STATICMSG;
        msg->msgSize = size;
        msg->replyId = (UInt16) MESSAGEQ_INVALIDMESSAGEQ;
        msg->msgId   = MESSAGEQ_INVALIDMSGID;
        msg->dstId   = (UInt16) MESSAGEQ_INVALIDMESSAGEQ;
        msg->flags   = 0x0;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "MessageQ_staticMsgInit");
}


/*
 *  @brief  Allocate a message and initialize the needed fields (note some
 *          of the fields in the header are set via other APIs or in the
 *          #MessageQ_put function.)
 *
 *  @param  heapId     Heap ID from which to allocate the message
 *  @param  size       Size of the message to be allocated
 *
 *  @sa     MessageQ_free
 */
MessageQ_Msg
MessageQ_alloc (UInt16 heapId, UInt32 size)
{
    Int32               status   = MESSAGEQ_SUCCESS;
    SharedRegion_SRPtr  msgSrPtr = SHAREDREGION_INVALIDSRPTR;
    MessageQ_Msg        msg      = NULL;
    MessageQDrv_CmdArgs cmdArgs;

    GT_2trace (curTrace, GT_ENTER, "MessageQ_alloc", heapId, size);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (MessageQ_state.setupRefCount == 0) {
        /* @retval  NULL Module is not initialized */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_alloc",
                             MESSAGEQ_E_INVALIDSTATE,
                             "Modules is not initialized!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.alloc.heapId = heapId;
        cmdArgs.args.alloc.size   = size;
        status = MessageQDrv_ioctl (CMD_MESSAGEQ_ALLOC, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            /*! @retval NULL API (through IOCTL) failed on kernel-side! */
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MessageQ_alloc",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            msgSrPtr = cmdArgs.args.alloc.msgSrPtr;
            if (msgSrPtr != SHAREDREGION_INVALIDSRPTR) {
                msg = SharedRegion_getPtr (msgSrPtr);
            }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    GT_1trace (curTrace, GT_LEAVE, "MessageQ_alloc", msg);

    /*! @retval Valid-message Operation Successsful */
    return msg;
}


/*
 *  @brief   Frees the message back to the heap that was used to allocate it.
 *
 *  @param   msg     Pointer to message to be freed
 *
 *  @sa      MessageQ_alloc
 */
Int
MessageQ_free (MessageQ_Msg msg)
{
    UInt32              status = MESSAGEQ_SUCCESS;
    MessageQDrv_CmdArgs cmdArgs;
    Int32               index;

    GT_1trace (curTrace, GT_ENTER, "MessageQ_free", msg);

    GT_assert (curTrace, (msg != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (MessageQ_state.setupRefCount == 0) {
        /* @retval  MESSAGEQ_E_INVALIDSTATE Module is not initialized */
        status = MESSAGEQ_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_free",
                             status,
                             "Modules is not initialized!");
    }
    else if (msg == NULL) {
        /*! @retval MESSAGEQ_E_INVALIDARG msg passed is null */
        status = MESSAGEQ_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_free",
                             status,
                             "msg passed is null!");
    }
    else if (msg->heapId ==  MESSAGEQ_STATICMSG) {
        /*! @retval MESSAGEQ_E_CANNOTFREESTATICMSG Static message has been
        *                     passed and cannot be freed.
         */
        status = MESSAGEQ_E_CANNOTFREESTATICMSG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_free",
                             MESSAGEQ_E_CANNOTFREESTATICMSG,
                             "Static message has been passed!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        index = SharedRegion_getIndex (msg);
        cmdArgs.args.free.msgSrPtr = SharedRegion_getSRPtr (msg, index);
        status = MessageQDrv_ioctl (CMD_MESSAGEQ_FREE, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MessageQ_free",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "MessageQ_free", status);

    /*! @retval MESSAGEQ_SUCCESS Operation Successsful */
    return status;
}


/*
 *  @brief   Register a heap with MessageQ.
 *           <br>
 *           This function registers a heap with MessageQ. The user selects the
 *           heapId associated with this heap. When a message is allocated via
 *           the #MessageQ_alloc function, the heapId is specified. Internally,
 *           MessageQ uses the heapId to access the heap.
 *           <br>
 *           Care must be taken when assigning heapIds. Internally MessageQ
 *           stores the heapId into the message. When the message is freed
 *           (via #MessageQ_free), the heapId is used to determine which heap to
 *           use. On systems with shared memory the heapIds must match on
 *           corresponding processors. For example, assume there is a heap
 *           called myHeap which acts on shared memory and processors 0 and 1
 *           both use this heap. When you register the heap with MessageQ, the
 *           same heapId must be used on both processor 0 and 1.
 *           <br>
 *           If a heap is already registered for the specified heapId, no action
 *           is taken.
 *
 *  @param   handle     Handle of the heap to be registered
 *  @param   heapId     Statically defined ID of the heap
 *
 *
 *  @sa      MessageQ_unregisterHeap
 */
Int
MessageQ_registerHeap (Heap_Handle handle, UInt16 heapId)
{
    Int                 status = MESSAGEQ_SUCCESS;
    MessageQDrv_CmdArgs cmdArgs;

    GT_2trace (curTrace, GT_ENTER, "MessageQ_registerHeap", handle, heapId);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (MessageQ_state.setupRefCount == 0) {
        /* @retval  MESSAGEQ_E_INVALIDSTATE Module is not initialized */
        status = MESSAGEQ_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_registerHeap",
                             status,
                             "Modules is not initialized!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Translate Gate handle to kernel-side gate handle. */
        cmdArgs.args.create.handle = Heap_getKnlHandle (handle);
        cmdArgs.args.registerHeap.heapId = heapId;
        status = MessageQDrv_ioctl (CMD_MESSAGEQ_REGISTERHEAP, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                    GT_4CLASS,
                    "MessageQ_registerHeap",
                    status,
                    "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "MessageQ_registerHeap", status);

    /*! @retval MESSAGEQ_SUCCESS Operation Successful */
    return status;
}


/*
 *  @brief   Unregister a heap with MessageQ.
 *           <br>
 *           This function unregisters the heap associated with the heapId.
 *           Care must be taken to ensure that there are no outstanding messages
 *           allocated from this heap. If there are outstanding messages, an
 *           attempt to free the message will result in non-deterministic
 *           results.
 *
 *  @param   heapId     Statically defined ID of the heap to be unregistered.
 *
 *  @sa      MessageQ_registerHeap
 */
Int
MessageQ_unregisterHeap (UInt16 heapId)
{
    Int                 status = MESSAGEQ_SUCCESS;
    MessageQDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "MessageQ_unregisterHeap", heapId);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (MessageQ_state.setupRefCount == 0) {
        /* @retval  MESSAGEQ_E_INVALIDSTATE Module is not initialized */
        status = MESSAGEQ_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_unregisterHeap",
                             status,
                             "Modules is not initialized!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.unregisterHeap.heapId = heapId;
        status = MessageQDrv_ioctl (CMD_MESSAGEQ_UNREGISTERHEAP, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                    GT_4CLASS,
                    "MessageQ_unregisterHeap",
                    status,
                    "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "MessageQ_unregisterHeap", status);

    /*! @retval MESSAGEQ_SUCCESS Operation Successsful */
    return status;
}


/*!
 *  @brief   Embeds a source message queue into a message.
 *           <br>
 *           This function along with the #MessageQ_getReplyQueue function can
 *           be used instead of the locates functions. The sender of a message
 *           can embed a messageQ into the message with this function. The
 *           receiver of the message can extract the message queue id with the
 *           #MessageQ_getReplyQueue function.
 *           <br>
 *           This method is particularly useful in a client/server relationship
 *           where the server does not want to know who the clients are. The
 *           clients can embed their message queue into the message to the
 *           server and the server extracts it and uses it to reply.
 *
 *  @param   handle     Handle to the MessageQ to be used as a reply queue.
 *  @param   msg        Message for which the reply queue is to be set.
 *
 *  @sa      MessageQ_getReplyQueue
 */
Void
MessageQ_setReplyQueue (MessageQ_Handle   handle,
                        MessageQ_Msg      msg)
{
    MessageQ_Object * obj = (MessageQ_Object *) handle;

    GT_2trace (curTrace, GT_ENTER, "MessageQ_setReplyQueue", handle, msg);

    GT_assert (curTrace, (handle != NULL));
    GT_assert (curTrace, (msg != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (MessageQ_state.setupRefCount == 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_setReplyQueue",
                             MESSAGEQ_E_INVALIDSTATE,
                             "Modules is not initialized!");
    }
    else if (handle == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_setReplyQueue",
                             MESSAGEQ_E_INVALIDARG,
                             "handle passed is null!");
    }
    else if (msg == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_setReplyQueue",
                             MESSAGEQ_E_INVALIDARG,
                             "msg passed is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        msg->replyId   = (UInt16)(obj->queueId);
        msg->replyProc = (UInt16)(obj->queueId >> 16);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "MessageQ_setReplyQueue");
}


/*!
 *  @brief   Returns the QueueId associated with the handle.
 *           <br>
 *           Since the #MessageQ_put function takes a QueueId, the creator of a
 *           message queue cannot send a message to themself. This function
 *           extracts the QueueId from the object.
 *
 *  @param   handle     Handle to the MessageQ
 *
 *  @sa      MessageQ_put
 */
MessageQ_QueueId
MessageQ_getQueueId (MessageQ_Handle handle)
{
    MessageQ_Object * obj     = (MessageQ_Object *) handle;
    UInt32            queueId = MESSAGEQ_INVALIDMESSAGEQ;

    GT_1trace (curTrace, GT_ENTER, "MessageQ_getQueueId", obj);

    GT_assert (curTrace, (handle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (MessageQ_state.setupRefCount == 0) {
        /*! @retval MESSAGEQ_INVALIDMESSAGEQ Modules is not initialized */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_unregisterTransport",
                             MESSAGEQ_E_INVALIDSTATE,
                             "Modules is not initialized!");
    }
    else if (handle == NULL) {
        /*! @retval MESSAGEQ_INVALIDMESSAGEQ handle passed is null */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_getQueueId",
                             MESSAGEQ_E_INVALIDARG,
                             "handle passed is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        queueId = (obj->queueId);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "MessageQ_getQueueId", queueId);

    /*! @retval Queue-Id Operation successful */
    return queueId;
}


/*!
 *  @brief   Returns the MultiProc processor id on which the queue resides.
 *           <br>
 *           Message queues reside on the processor that created them. This
 *           function allows the caller to determined on which processor the
 *           queue resides.
 *
 *  @param   handle     Handle to the MessageQ
 *
 *  @sa      None
 */
UInt16
MessageQ_getProcId (MessageQ_Handle handle)
{
    MessageQ_Object * obj    = (MessageQ_Object *) handle;
    UInt16            procId = MULTIPROC_INVALIDID;

    GT_1trace (curTrace, GT_ENTER, "MessageQ_getProcId", handle);

    GT_assert (curTrace, (handle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (MessageQ_state.setupRefCount == 0) {
        /* @retval  MULTIPROC_INVALIDID Module is not initialized */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_unregisterTransport",
                             MESSAGEQ_E_INVALIDSTATE,
                             "Modules is not initialized!");
    }
    else if (handle == NULL) {
        /*! @retval MULTIPROC_INVALIDID handle passed is null */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MessageQ_getProcId",
                             MESSAGEQ_E_INVALIDARG,
                             "handle passed is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        procId = (UInt16)(obj->queueId >> 16);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "MessageQ_getProcId", procId);

    /*! @retval Proc-ID Operation successful */
    return procId;
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
