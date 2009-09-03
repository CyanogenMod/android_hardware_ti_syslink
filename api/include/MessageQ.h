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
 *  @file   MessageQ.h
 *
 *  @brief      Defines MessageQ module.
 *
 *              The MessageQ module supports the structured sending and
 *              receiving of variable length messages. This module can be
 *              used for homogeneous or heterogeneous multi-processor messaging.
 *              <br><br>
 *              MessageQ provides more sophisticated messaging than other
 *              modules. It is typically used for complex situations such as
 *              multi-processor messaging.<br><br>
 *
 *              The following are key features of the MessageQ module:<br>
 *              -Writers and readers can be relocated to another processor with
 *               no runtime code changes.<br>
 *              -Timeouts are allowed when receiving messages.<br>
 *              -Readers can determine the writer and reply back.<br>
 *              -Receiving a message is deterministic when timeout is zero.<br>
 *              -Messages can reside on any message queue.<br>
 *              -Supports zero-copy transfers.<br>
 *              -Can send and receive from any type of thread.<br>
 *              -Notification mechanism is specified by application.<br>
 *              -Allows QoS (quality of service) on message buffer pools. For
 *               example, using specific buffer pools for specific message
 *               queues.<br><br>
 *              Messages are sent and received via a message queue. A reader is
 *              a thread that gets (reads) messages from a message queue.
 *              A writer is a thread that puts (writes) a message to a message
 *              queue. Each message queue has one reader and can have many
 *              writers. A thread may read from or write to multiple message
 *              queues.<br><br>
 *              Conceptually, the reader thread owns a message queue. The reader
 *              thread creates a message queue. Writer threads  a created
 *              message queues to get access to them.<br><br>
 *              Message queues are identified by a system-wide unique name.
 *              Internally, MessageQ uses the NameServer module for managing
 *              these names. The names are used for opening a message queue.
 *              Using names is not required.<br><br>
 *              Messages must be allocated from the MessageQ module. Once a
 *              message is allocated, it can be sent on any message queue. Once
 *              a message is sent, the writer loses ownership of the message and
 *              should not attempt to modify the message. Once the reader
 *              receives the message, it owns the message. It may either free
 *              the message or re-use the message.<br><br>
 *              Messages in a message queue can be of variable length. The only
 *              requirement is that the first field in the definition of a
 *              message must be a MsgHeader structure. For example:
 *              @code
 *              typedef struct MyMsg {
 *                  MessageQ_MsgHeader header;
 *                  ...
 *              } MyMsg;
 *              @endcode
 *              <br>
 *              The MessageQ API uses the #MessageQ_MsgHeader internally. Your
 *              application should not modify or directly access the fields in
 *              the #MessageQ_MsgHeader.<br><br>
 *              All messages sent via the MessageQ module must be allocated from
 *              a Heap implementation. The heap can be used for other memory
 *              allocation not related to MessageQ.<br><br>
 *              An application can use multiple heaps. The purpose of having
 *              multiple heaps is to allow an application to regulate its
 *              message usage. For example, an application can allocate critical
 *              messages from one heap of fast on-chip memory and non-critical
 *              messages from another heap of slower external memory.<br><br>
 *              MessageQ does support the usage of messages that are not
 *              allocated via the alloc function. Please refer to the
 *              #MessageQstaticMsgInit function description for more details.
 *              <br><br>
 *              In a multiple processor system, MessageQ communications to other
 *              processors via MessageQTransport instances. There must be one
 *              and only one MessageQ transport instance for each processor
 *              where communication is desired.
 *              So on a four processor system, each processor must have three
 *              MessageQ transport instances.
 *              <br><br>
 *              The user only needs to create the MessageQTransport instances.
 *              The instances are responsible for registering themselves with
 *              MessageQ. This is accomplished via the
 *              #MessageQ_registerTransport function.
 *
 *  ============================================================================
 */


#ifndef MESSAGEQ_H_0xded2
#define MESSAGEQ_H_0xded2

/* Standard headers */
#include <List.h>

/* Utilities headers */
#include <ListMP.h>
#include <MessageQTransportShm.h>


#if defined (__cplusplus)
extern "C" {
#endif


/*!
 *  @def    MESSAGEQ_MODULEID
 *  @brief  Unique module ID.
 */
#define MESSAGEQ_MODULEID               (0xded2)

/* =============================================================================
 *  All success and failure codes for the module
 * =============================================================================
 */

/*!
 *  @def    MESSAGEQ_STATUSCODEBASE
 *  @brief  Error code base for MessageQ.
 */
#define MESSAGEQ_STATUSCODEBASE  (MESSAGEQ_MODULEID << 12u)

/*!
 *  @def    MESSAGEQ_MAKE_FAILURE
 *  @brief  Macro to make error code.
 */
#define MESSAGEQ_MAKE_FAILURE(x)    ((Int)  (  0x80000000                  \
                                         + (MESSAGEQ_STATUSCODEBASE  \
                                         + (x))))

/*!
 *  @def    MESSAGEQ_MAKE_SUCCESS
 *  @brief  Macro to make success code.
 */
#define MESSAGEQ_MAKE_SUCCESS(x)    (MESSAGEQ_STATUSCODEBASE + (x))

/*!
 *  @def    MESSAGEQ_E_INVALIDARG
 *  @brief  Argument passed to a function is invalid.
 */
#define MESSAGEQ_E_INVALIDARG       MESSAGEQ_MAKE_FAILURE(1)

/*!
 *  @def    MESSAGEQ_E_MEMORY
 *  @brief  Memory allocation failed.
 */
#define MESSAGEQ_E_MEMORY           MESSAGEQ_MAKE_FAILURE(2)

/*!
 *  @def    MESSAGEQ_E_BUSY
 *  @brief  the name is already registered or not.
 */
#define MESSAGEQ_E_BUSY             MESSAGEQ_MAKE_FAILURE(3)

/*!
 *  @def    MESSAGEQ_E_FAIL
 *  @brief  Generic failure.
 */
#define MESSAGEQ_E_FAIL             MESSAGEQ_MAKE_FAILURE(4)

/*!
 *  @def    MESSAGEQ_E_NOTFOUND
 *  @brief  name not found in the nameserver.
 */
#define MESSAGEQ_E_NOTFOUND         MESSAGEQ_MAKE_FAILURE(5)

/*!
 *  @def    MESSAGEQ_E_INVALIDSTATE
 *  @brief  Module is not initialized.
 */
#define MESSAGEQ_E_INVALIDSTATE     MESSAGEQ_MAKE_FAILURE(6)

/*!
 *  @def    MESSAGEQ_E_NOTONWER
 *  @brief  Instance is not created on this processor.
 */
#define MESSAGEQ_E_NOTONWER         MESSAGEQ_MAKE_FAILURE(7)

/*!
 *  @def    MESSAGEQ_E_REMOTEACTIVE
 *  @brief  Remote opener of the instance has not closed the instance.
 */
#define MESSAGEQ_E_REMOTEACTIVE     MESSAGEQ_MAKE_FAILURE(8)

/*!
 *  @def    MESSAGEQ_E_INUSE
 *  @brief  Indicates that the instance is in use..
 */
#define MESSAGEQ_E_INUSE            MESSAGEQ_MAKE_FAILURE(9)

/*!
 *  @def    MESSAGEQ_E_INVALIDCONTEXT
 *  @brief  Indicates that the api is called with wrong handle
 */
#define MESSAGEQ_E_INVALIDCONTEXT   MESSAGEQ_MAKE_FAILURE(10)

/*!
 *  @def    MESSAGEQ_E_INVALIDMSG
 *  @brief  Indicates that an invalid msg has been specified
 *
 */
#define MESSAGEQ_E_INVALIDMSG       MESSAGEQ_MAKE_FAILURE(11)

/*!
 *  @def    MESSAGEQ_E_INVALIDHEAPID
 *  @brief  Indicates that an invalid heap has been specified
 */
#define MESSAGEQ_E_INVALIDHEAPID    MESSAGEQ_MAKE_FAILURE(12)

/*!
 *  @def    MESSAGEQ_E_INVALIDPROCID
 *  @brief  Indicates that an invalid proc id has been specified
 */
#define MESSAGEQ_E_INVALIDPROCID    MESSAGEQ_MAKE_FAILURE(13)

/*!
 *  @def    MESSAGEQ_E_MAXREACHED
 *  @brief  Indicates that all message queues are taken
 */
#define MESSAGEQ_E_MAXREACHED       MESSAGEQ_MAKE_FAILURE(14)

/*!
 *  @def    MESSAGEQ_E_UNREGISTERHEAPID
 *  @brief  Indicates that heap id has not been registered
 */
#define MESSAGEQ_E_UNREGISTERHEAPID MESSAGEQ_MAKE_FAILURE(15)

/*!
 *  @def    MESSAGEQ_E_CANNOTFREESTATICMSG
 *  @brief  Indicates that static msg cannot be freed
 */
#define MESSAGEQ_E_CANNOTFREESTATICMSG MESSAGEQ_MAKE_FAILURE(16)

/*!
 *  @def    MESSAGEQ_E_HEAPIDINVALID
 *  @brief  Indicates that the heap id is invalid
 */
#define MESSAGEQ_E_HEAPIDINVALID    MESSAGEQ_MAKE_FAILURE(17)

/*!
 *  @def    MESSAGEQ_E_PROCIDINVALID
 *  @brief  Indicates that the proc id is invalid
 */
#define MESSAGEQ_E_PROCIDINVALID    MESSAGEQ_MAKE_FAILURE(18)

/*!
 *  @def    MESSAGEQ_E_OSFAILURE
 *  @brief  Failure in OS call.
 */
#define MESSAGEQ_E_OSFAILURE        MESSAGEQ_MAKE_FAILURE(19)

/*!
 *  @def    MESSAGEQ_E_ALREADYEXISTS
 *  @brief  Specified entity already exists
 */
#define MESSAGEQ_E_ALREADYEXISTS    MESSAGEQ_MAKE_FAILURE(20)

/*!
 *  @def    MESSAGEQ_E_TIMEOUT
 *  @brief  Timeout while attempting to get a message
 */
#define MESSAGEQ_E_TIMEOUT          MESSAGEQ_MAKE_FAILURE(21)

/*!
 *  @def    MESSAGEQ_SUCCESS
 *  @brief  Operation successful.
 */
#define MESSAGEQ_SUCCESS            MESSAGEQ_MAKE_SUCCESS(0)

/*!
 *  @def    MESSAGEQ_S_ALREADYSETUP
 *  @brief  The MESSAGEQ module has already been setup in this process.
 */
#define MESSAGEQ_S_ALREADYSETUP     MESSAGEQ_MAKE_SUCCESS(1)


/* =============================================================================
 * Macros and types
 * =============================================================================
 */
/*!
 *  @brief  Mask to extract version setting
 */
#define MESSAGEQ_HEADERVERSION         0x2000u
/*!
 *  @brief  Mask to extract priority setting
 */
#define MESSAGEQ_PRIORITYMASK          0x3u

/*!
 *  @brief  Mask to extract priority setting
 */
#define MESSAGEQ_TRANSPORTPRIORITYMASK 0x01u

/*!
 *  @brief  Mask to extract version setting
 */
#define MESSAGEQ_VERSIONMASK           0xE000u;

/*!
 *  @brief  Used as the timeout value to specify wait forever
 */
#define MESSAGEQ_FOREVER              (~((UInt32) 0u))

/*!
 *  @brief  Invalid message id
 */
#define MESSAGEQ_INVALIDMSGID         0xFFFFu

/*!
 *  @brief  Invalid message queue
 */
#define MESSAGEQ_INVALIDMESSAGEQ      0xFFFFu

/*!
 *  @brief  Indicates that if maximum number of message queues are already
 *          created, should allow growth to create additional Message Queue.
 */
#define MESSAGEQ_ALLOWGROWTH         (~((UInt32) 0u))


/*!
 *  @brief  Type used with open/setReplyQueue/getReplyQueue/putQueue
 */
typedef UInt32 MessageQ_QueueId;

/*!
 *  @brief  Type defining Index into MesssageQ queue array
 */
typedef UInt16 MessageQ_QueueIndex;


/*!
 *  @brief  Enumerates the different types of message priority levels
 */
typedef enum MessageQ_Priority_tag {
    MESSAGEQ_NORMALPRI   =  0u,
    /*!< Normal priority message */
    MESSAGEQ_HIGHPRI     =  1u,
    /*!< High priority message */
    MESSAGEQ_RESERVEDPRI =  2u,
    /*!< Urgent priority message */
    MESSAGEQ_URGENTPRI   =  3u
    /*!< Reserved value for message priority */
} MessageQ_Priority;

/*!
 *  @brief  Structure which defines the required first field in every message
 */
typedef struct MessageQ_MsgHeader_tag {
    UInt32       reserved0;
    /*!< Reserved field */
    UInt32       reserved1;
    /*!< Reserved field */
    UInt32       msgSize;
    /*!< Size of the message (including header) */
    UInt16       flags;
    /*!< Flags */
    UInt16       msgId;
    /*!< Message ID for application usage */
    UInt16       dstId;
    /*!< Destination Message Queue ID */
    UInt16       dstProc;
    /*!< Destination Proc ID */
    UInt16       replyId;
    /*!< Reply Message Queue ID for sending reply message */
    UInt16       replyProc;
    /*!< Reply Processor ID for sending reply message */
    UInt16       srcProc;
    /*!< Source Processor ID */
    UInt16       heapId;
    /*!< Heap ID of the heap from which message has been allocated*/
    UInt32       reserved;
    /*!< Reserved field */
} MessageQ_MsgHeader;

/*!
 *  @brief  Type defining a Message pointer.
 */
typedef MessageQ_MsgHeader * MessageQ_Msg;


/* =============================================================================
 *  Forward declarations
 * =============================================================================
 */
/*! @brief Forward declaration of structure defining object for the MessageQ. */
typedef struct MessageQ_Object_tag MessageQ_Object;

/*!
 *  @brief  Handle for the Message Queue object.
 */
typedef struct MessageQ_Object * MessageQ_Handle;

/*!
 *  @brief  Structure defining config parameters for the MessageQ Buf module.
 */
typedef struct MessageQ_Config_tag {
    UInt16 numHeaps;
    /*!< Number of heapIds in the system
     * This allows MessageQ to pre-allocate the heaps table.
     * The heaps table is used when registering heaps.
     * The default is 1 since generally all systems need at least one heap.
     *  There is no default heap, so unless the system is only using
     *  staticMsgInit, the application must register a heap.
     */
    UInt maxRuntimeEntries;
    /*!< Maximum number of MessageQs that can be dynamically created */
    Void *placeHolder;
    UInt maxNameLen;
    /*!< Maximum length for Message queue names */
} MessageQ_Config;

/*!
 *  @brief  Parameters for creation of a MessageQ instance.
 */
typedef struct MessageQ_Params_tag {
    UInt32 reserved;
    /*!< No parameters required currently. Reserved field. */
} MessageQ_Params;


/* =============================================================================
 *  APIs
 * =============================================================================
 */
/* Function to get default configuration for the MessageQ module. */
Void MessageQ_getConfig (MessageQ_Config * cfg);

/* Function to setup the MessageQ module. */
Int MessageQ_setup (const MessageQ_Config * cfg);

/* Function to destroy the MessageQ module. */
Int MessageQ_destroy (void);

/* Initialize this config-params structure with supplier-specified
 * defaults before instance creation.
 */
Void MessageQ_Params_init (MessageQ_Handle         handle,
                           MessageQ_Params       * params);

/* Create a message queue */
MessageQ_Handle MessageQ_create (      String            name,
                                 const MessageQ_Params * params);

/* Deletes a instance of MessageQ module. */
Int MessageQ_delete (MessageQ_Handle * msgHandle);

/* Open a message queue */
Int MessageQ_open (String             name,
                   MessageQ_QueueId * queueId);

/* Close an opened message queue handle */
Void MessageQ_close (MessageQ_QueueId * queueId);

/* Allocates a message from the heap */
MessageQ_Msg MessageQ_alloc (UInt16 heapId, UInt32 size);

/* Frees a message back to the heap */
Int MessageQ_free (MessageQ_Msg msg);

/* Initializes a message not obtained from MessageQ_alloc */
Void MessageQ_staticMsgInit (MessageQ_Msg msg, UInt32 size);

/* Place a message onto a message queue */
Int MessageQ_put (MessageQ_QueueId queueId,
                  MessageQ_Msg     msg);

/* Gets a message for a message queue and blocks if the queue is empty */
Int MessageQ_get (MessageQ_Handle handle,
                  MessageQ_Msg *  msg,
                  UInt            timeout);

/* Register a heap with MessageQ */
Int  MessageQ_registerHeap (Heap_Handle heap, UInt16 heapId);

/* Unregister a heap with MessageQ */
Int  MessageQ_unregisterHeap (UInt16 heapId);

/* Returns the number of messages in a message queue */
Int MessageQ_count (MessageQ_Handle handle);

/* Get the queue Id of the message. */
MessageQ_QueueId MessageQ_getQueueId (MessageQ_Handle handle);

/* Get the proc Id of the message. */
UInt16 MessageQ_getProcId (MessageQ_Handle handle);

/* Embeds a source message queue into a message. */
Void MessageQ_setReplyQueue (MessageQ_Handle handle, MessageQ_Msg msg);


/* =============================================================================
 *  Macros
 * =============================================================================
 */
/*!
 *  @brief   Extract the destination queue from a message.
 *           <br>
 *           Can only be used on the same processor where the destination queue
 *           resides. This function should only be used by Message Queue
 *           Transport writers.
 *
 *  @param   msg    Message
 *  @retval  Destination-Queue-ID    Destination message queue.
 */
#define MessageQ_getDstQueue(msg)                                             \
             (msg->dstId == (MessageQ_QueueIndex) MessageQ_INVALIDMESSAGEQ) ? \
                          MessageQ_INVALIDMESSAGEQ :                          \
                          (  ((UInt32)   MultiProc_getId (NULL)<< 16u)        \
                           | ((MessageQ_Msg) (msg))->dstId)


/*!
 *  @brief   Retrieves the message ID of a message.<br>
 *           This function retrieves the message ID from the message. The
 *           #MessageQ_setMsgId function is used to insert the message ID.
 *           <br>
 *           The message id is part of the #MessageQ_MsgHeader header and is in
 *           every MessageQ message. All message ids are initialized to
 *           #MESSAGEQ_INVALIDMSGID in the #MessageQ_alloc and
 *           #MessageQ_staticMsgInit calls.
 *
 *  @param   msg     Message
 *  @retval  size    Message ID from the message
 */
#define MessageQ_getMsgId(msg) (((MessageQ_Msg) (msg))->msgId)

/*!
 *  @brief   Returns the size of the specified message. This function is helpful
 *           when re-using a message.
 *
 *  @param   msg     Message
 *  @retval  size    Size of the message
 */
#define MessageQ_getMsgSize(msg) (((MessageQ_Msg) (msg))->msgSize)

/*!
 *  @brief   Gets the message priority of a message
 *
 *  @retval  priority Priority of the message
 */
#define MessageQ_getMsgPri(msg)        \
                 ((((MessageQ_Msg) (msg))->flags & MESSAGEQ_PRIORITYMASK))

/*!
 *  @brief   Retrieves the embedded source message queue from a message.
 *           <br>
 *           This function along with the #MessageQ_setReplyQueue} function can
 *           be used instead of the open function. The sender of a message can
 *           embed a messageQ into the message with the #MessageQ_setReplyQueue
 *           function. The receiver of the message can extract the message queue
 *           ID with this function.
 *           <br>
 *           This method is particularing useful in a client/server relationship
 *           where the server does not want to know who the clients are. The
 *           clients can embed their message queue into the message to the
 *           server and the server extracts it and uses it to reply.
 *
 *  @param   msg         Message
 *  @retval  Reply-Queue Reply Message Queue ID
 */
#define MessageQ_getReplyQueue(msg)           \
                        (  (UInt32) ((MessageQ_Msg) (msg))->replyProc << 16u) \
                      | ((MessageQ_Msg) (msg))->replyId

/*!
 *  @brief   Sets the message id in a message.<br>
 *           This function sets the message ID in the message. The
 *           #MessageQ_getMsgId function is used to retrieve the message ID.<br>
 *           The message id is part of the #MessageQ_MsgHeader header and is in
 *           every MessageQ message. All message ids are initialized to
 *           #MESSAGEQ_INVALIDMSGID in the #MessageQ_alloc and
 *           #MessageQ_staticMsgInit calls.
 *
 *  @param   msg     Message
 */
#define MessageQ_setMsgId(msg, id) ((MessageQ_Msg) (msg))->msgId = id

/*!
 *  @brief   Sets the message priority of a message
 *
 *  @param   msg      Message
 *  @param   priority Priority of message to be set.
 */
#define MessageQ_setMsgPri(msg, priority) \
        (((MessageQ_Msg) (msg))->flags = (priority & MESSAGEQ_PRIORITYMASK))


/* =============================================================================
 *  APIs called internally by MessageQ transports
 * =============================================================================
 */
/* Register a transport with MessageQ */
Int  MessageQ_registerTransport (MessageQTransportShm_Handle transport,
                                 UInt16                      procId,
                                 UInt                        priority);

/* Unregister a transport with MessageQ */
Int  MessageQ_unregisterTransport (UInt16 procId, UInt priority);


/* =============================================================================
 *  Compatibility layer for SYSBIOS
 * =============================================================================
 */
#define MessageQ_MODULEID           MESSAGEQ_MODULEID
#define MessageQ_STATUSCODEBASE     MESSAGEQ_STATUSCODEBASE
#define MessageQ_MAKE_FAILURE       MESSAGEQ_MAKE_FAILURE
#define MessageQ_MAKE_SUCCESS       MESSAGEQ_MAKE_SUCCESS
#define MessageQ_SUCCESS            MESSAGEQ_SUCCESS
#define MessageQ_PRIORITYMASK       MESSAGEQ_PRIORITYMASK
#define MessageQ_VERSIONMASK        MESSAGEQ_VERSIONMASK
#define MessageQ_FOREVER            MESSAGEQ_FOREVER
#define MessageQ_INVALIDMSGID       MESSAGEQ_INVALIDMSGID
#define MessageQ_INVALIDMESSAGEQ    MESSAGEQ_INVALIDMESSAGEQ
#define MessageQ_ALLOWGROWTH        MESSAGEQ_INVALIDMESSAGEQ
#define MessageQ_NORMALPRI          MESSAGEQ_NORMALPRI
#define MessageQ_HIGHPRI            MESSAGEQ_HIGHPRI
#define MessageQ_URGENTPRI          MESSAGEQ_URGENTPRI
#define MessageQ_RESERVEDPRI        MESSAGEQ_RESERVEDPRI


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* MESSAGEQ_H_0xded2 */
