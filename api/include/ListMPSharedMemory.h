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
 *  @file   ListMPSharedMemory.h
 *
 *  @brief      Defines for shared memory doubly linked list.
 *
 *              This module implements the ListMPSharedMemory.
 *              ListMPSharedMemory is a linked-list based module designed to be
 *              used in a multi-processor environment.  It is designed to
 *              provide a means of communication between different processors.
 *              processors.
 *              This module is instance based. Each instance requires a small
 *              piece of shared memory. This is specified via the #sharedAddr
 *              parameter to the create. The proper #sharedAddrSize parameter
 *              can be determined via the #sharedMemReq call. Note: the
 *              parameters to this function must be the same that will used to
 *              create or open the instance.
 *              The ListMPSharedMemory module uses a #ti.sdo.utils.NameServer
 *              instance to store instance information when an instance is
 *              created and the name parameter is non-NULL. If a name is
 *              supplied, it must be unique for all ListMPSharedMemory
 *              instances.
 *              The #create also initializes the shared memory as needed. The
 *              shared memory must be initialized to 0 or all ones
 *              (e.g. 0xFFFFFFFFF) before the ListMPSharedMemory instance is
 *              created.  Once an instance is created, an open can be performed.
 *              The #open is used to gain access to the same ListMPSharedMemory
 *              instance.
 *              Generally an instance is created on one processor and opened
 *              on the other processor.
 *              The open returns a ListMPSharedMemory instance handle like the
 *              create, however the open does not modify the shared memory.
 *              Generally an instance is created on one processor and opened on
 *              the other processor.
 *              There are two options when opening the instance:
 *              @li Supply the same name as specified in the create. The
 *              ListMPSharedMemory module queries the NameServer to get the
 *              needed information.
 *              @li Supply the same #sharedAddr value as specified in the
 *              create.
 *              If the open is called before the instance is created, open
 *              returns NULL.
 *              There is currently a list of restrictions for the module:
 *              @li Both processors must have the same endianness. Endianness
 *              conversion may supported in a future version of
 *              ListMPSharedMemory.
 *              @li The module will be made a gated module
 *
 *  ============================================================================
 */


#ifndef LISTMPSHAREDMEMORY_H_0xDD3C
#define LISTMPSHAREDMEMORY_H_0xDD3C

/* Utilities headers */

/* Other headers */
#include <ListMP.h>

#if defined (__cplusplus)
extern "C" {
#endif


/*!
 *  @def    LISTMPSHAREDMEMORY_MODULEID
 *  @brief  Unique module ID.
 */
#define LISTMPSHAREDMEMORY_MODULEID             (0xDD3C)

/* =============================================================================
 *  All success and failure codes for the module
 * =============================================================================
 */
/*!
 *  @def    LISTMPSHAREDMEMORY_STATUSCODEBASE
 *  @brief  Error code base for ListMPSharedMemory.
 */
#define LISTMPSHAREDMEMORY_STATUSCODEBASE      \
    (LISTMPSHAREDMEMORY_MODULEID << 12u)

/*!
 *  @def    LISTMPSHAREDMEMORY_MAKE_FAILURE
 *  @brief  Macro to make error code.
 */
#define LISTMPSHAREDMEMORY_MAKE_FAILURE(x)    \
    ((Int)  (  0x80000000                     \
     + (LISTMPSHAREDMEMORY_STATUSCODEBASE      \
     + (x))))

/*!
 *  @def    LISTMPSHAREDMEMORY_MAKE_SUCCESS
 *  @brief  Macro to make success code.
 */
#define LISTMPSHAREDMEMORY_MAKE_SUCCESS(x)    \
    (LISTMPSHAREDMEMORY_STATUSCODEBASE + (x))

/*!
 *  @def    LISTMPSHAREDMEMORY_E_INVALIDARG
 *  @brief  Argument passed to a function is invalid.
 */
#define LISTMPSHAREDMEMORY_E_INVALIDARG       LISTMPSHAREDMEMORY_MAKE_FAILURE(1)

/*!
 *  @def    LISTMPSHAREDMEMORY_E_MEMORY
 *  @brief  Memory allocation failed.
 */
#define LISTMPSHAREDMEMORY_E_MEMORY           LISTMPSHAREDMEMORY_MAKE_FAILURE(2)

/*!
 *  @def    LISTMPSHAREDMEMORY_E_FAIL
 *  @brief  Generic failure.
 */
#define LISTMPSHAREDMEMORY_E_FAIL             LISTMPSHAREDMEMORY_MAKE_FAILURE(3)

/*!
 *  @def    LISTMPSHAREDMEMORY_E_INVALIDSTATE
 *  @brief  Module is not initialized.
 */
#define LISTMPSHAREDMEMORY_E_INVALIDSTATE     LISTMPSHAREDMEMORY_MAKE_FAILURE(4)
/*!
 *  @def    LISTMPSHAREDMEMORY_E_INVALIDSTATE
 *  @brief  Module is not initialized.
 */
#define LISTMPSHAREDMEMORY_E_OSFAILURE        LISTMPSHAREDMEMORY_MAKE_FAILURE(5)
/*!
 *  @def    LISTMPSHAREDMEMORY_E_NOTONWER
 *  @brief  Instance is not created on this processor.
 */
#define LISTMPSHAREDMEMORY_E_NOTONWER         LISTMPSHAREDMEMORY_MAKE_FAILURE(6)

/*!
 *  @def    LISTMPSHAREDMEMORY_E_REMOTEACTIVE
 *  @brief  Remote opener of the instance has not closed the instance.
 */
#define LISTMPSHAREDMEMORY_E_REMOTEACTIVE     LISTMPSHAREDMEMORY_MAKE_FAILURE(7)

/*!
 *  @def    LISTMPSHAREDMEMORY_E_INUSE
 *  @brief  Indicates that the instance is in use..
 */
#define LISTMPSHAREDMEMORY_E_INUSE            LISTMPSHAREDMEMORY_MAKE_FAILURE(8)

/*!
 *  @def    LISTMPSHAREDMEMORY_E_NOTFOUND
 *  @brief  Name not found in the nameserver.
 */
#define LISTMPSHAREDMEMORY_E_NOTFOUND         LISTMPSHAREDMEMORY_MAKE_FAILURE(9)

/*!
 *  @def    LISTMPSHAREDMEMORY_E_VERSION
 *  @brief  Version mismatch error.
 */
#define LISTMPSHAREDMEMORY_E_VERSION        LISTMPSHAREDMEMORY_MAKE_FAILURE(0xA)

/*!
 *  @def    LISTMPSHAREDMEMORY_SUCCESS
 *  @brief  Operation successful.
 */
#define LISTMPSHAREDMEMORY_SUCCESS            LISTMPSHAREDMEMORY_MAKE_SUCCESS(0)

/*!
 *  @def    LISTMPSHAREDMEMORY_S_ALREADYSETUP
 *  @brief  The LISTMPSHAREDMEMORY module has already been setup in this
 *          process.
 */
#define LISTMPSHAREDMEMORY_S_ALREADYSETUP     LISTMPSHAREDMEMORY_MAKE_SUCCESS(1)


/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*!
 *  @brief  Structure defining config parameters for the ListMP module.
 */
typedef struct ListMPSharedMemory_Config_tag {
    UInt32 maxNameLen;
    /*!< Maximum length of name */
    Bool   useNameServer;
    /*!< Whether to have this module use the NameServer or not. If the
     *   NameServer is not needed, set this configuration parameter to false.
     *   This informs ListMPSharedMemory not to pull in the NameServer module.
     *   In this case, all names passed into create and open are ignored.
     */
} ListMPSharedMemory_Config;

/*!
 *  @brief  Defines shared memory params for ListMPSharedMemory type.
 */
#define ListMPSharedMemory_Params ListMP_Params

/*! @brief Forward declaration of structure defining object for the
 *         ListMPSharedMemory.
 */
/*!
 *  @brief  Object for the ListMPSharedMemory Handle
 */
#define     ListMPSharedMemory_Object         ListMP_Object

/*!
 *  @brief  Handle for the ListMPSharedMemory
 */
#define     ListMPSharedMemory_Handle         ListMP_Handle


/* =============================================================================
 *  Functions to create the module
 * =============================================================================
 */
/*  Function to get configuration parameters to setup
 *  the ListMPSharedMemory module.
 */
Void ListMPSharedMemory_getConfig (ListMPSharedMemory_Config * cfgParams);

/* Function to setup the ListMPSharedMemory module.  */
Int ListMPSharedMemory_setup (ListMPSharedMemory_Config * config) ;

/* Function to destroy the ListMPSharedMemory module. */
Int ListMPSharedMemory_destroy (void);

/* =============================================================================
 *  Functions to create instance of a list
 * =============================================================================
 */
/*
 *  Initialize this config-params structure with supplier-specified
 *  defaults before instance creation.
 */
Void
ListMPSharedMemory_Params_init (ListMPSharedMemory_Handle   handle,
                                ListMPSharedMemory_Params * params);
/* Function to create an instance of ListMP */
ListMPSharedMemory_Handle ListMPSharedMemory_create
                         (ListMPSharedMemory_Params * params);

/* Function to delete an instance of ListMP */
Int ListMPSharedMemory_delete (ListMPSharedMemory_Handle * listMPHandlePtr);

/* =============================================================================
 *  Functions to open/close handle to list instance
 * =============================================================================
 */

/* Function to open a previously created instance */
Int ListMPSharedMemory_open  (ListMPSharedMemory_Handle * listMpHandlePtr,
                              ListMPSharedMemory_Params * params);

/* Function to close a previously opened instance */
Int ListMPSharedMemory_close  (ListMPSharedMemory_Handle * listMPHandle);

/* =============================================================================
 *  Functions for list operations
 * =============================================================================
 */
/* Function to check if list is empty */
Bool ListMPSharedMemory_empty  (ListMPSharedMemory_Handle listMPHandle);

/* Function to get head element from list */
Ptr ListMPSharedMemory_getHead (ListMPSharedMemory_Handle listMPHandle);

/* Function to get tail element from list */
Ptr ListMPSharedMemory_getTail (ListMPSharedMemory_Handle listMPHandle);

/* Function to put head element into list */
Int ListMPSharedMemory_putHead (ListMPSharedMemory_Handle  listMPHandle,
                                ListMP_Elem              * elem);

/* Function to put tail element into list */
Int ListMPSharedMemory_putTail (ListMPSharedMemory_Handle  listMPHandle,
                                ListMP_Elem              * elem);

/* Function to insert element into list */
Int ListMPSharedMemory_insert (ListMPSharedMemory_Handle   listMPHandle,
                               ListMP_Elem               * elem,
                               ListMP_Elem               * curElem);

/* Function to traverse to remove element from list */
Int ListMPSharedMemory_remove (ListMPSharedMemory_Handle  listMPHandle,
                               ListMP_Elem              * elem);

/* Function to traverse to next element in list */
Ptr ListMPSharedMemory_next (ListMPSharedMemory_Handle    listMPHandle,
                             ListMP_Elem                * elem);

/* Function to traverse to prev element in list */
Ptr ListMPSharedMemory_prev (ListMPSharedMemory_Handle    listMPHandle,
                             ListMP_Elem                * elem);

/* =============================================================================
 *  Functions for shared memory requirements
 * =============================================================================
 */
/* Amount of shared memory required for creation of each instance. */
Int
ListMPSharedMemory_sharedMemReq (ListMPSharedMemory_Params * params);


/* =============================================================================
 *  Internal macros and types
 * =============================================================================
 */
/*!
 *  @def    ListMPSharedMemory_CREATED
 *  @brief  Creation of list succesful.
*/
#define LISTMPSHAREDMEMORY_CREATED            (0x12181964u)

/*!
 *  @def    ListMPSharedMemory_VERSION
 *  @brief  Version.
 */
#define LISTMPSHAREDMEMORY_VERSION            (1u)

/*!
 *  @brief  Structure defining attribute parameters for the
 *          ListMP module.
 */
typedef struct ListMPSharedMemory_Attrs_tag {
    volatile UInt32 version;
    /*!< Version of module */
    volatile UInt32 status;
    /*!< Status of module */
    volatile UInt32 sharedAddrSize;
    /*!< Shared address size of module */
} ListMPSharedMemory_Attrs;

/*!
 *  @brief  Structure defining processor related information for the
 *          ListMP module.
 */
typedef struct ListMPSharedMemory_ProcAttrs_tag {
    Bool   creator;   /*!< Creator or opener */
    UInt16 procId;    /*!< Processor Identifier */
    UInt32 openCount; /*!< How many times it is opened on a processor */
} ListMPSharedMemory_ProcAttrs;


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* LISTMPSHAREDMEMORY_H_0xDD3C */
