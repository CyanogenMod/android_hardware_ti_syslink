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
 *  @file   ListMP.h
 *
 *  @brief      Defines for shared memory doubly linked list.
 *
 *              This module implements the ListMP.
 *              ListMP is a linked-list based module designed to be used in a
 *              multi-processor environment.  It is designed to provide a means
 *              of communication between different processors.
 *              processors.
 *              This module is instance based. Each instance requires a small
 *              piece of shared memory. This is specified via the #sharedAddr
 *              parameter to the create. The proper #sharedAddrSize parameter
 *              can be determined via the #sharedMemReq call. Note: the
 *              parameters to this function must be the same that will used to
 *              create or open the instance.
 *              The ListMP module uses a #ti.sdo.utils.NameServer instance
 *              to store instance information when an instance is created and
 *              the name parameter is non-NULL. If a name is supplied, it must
 *              be unique for all ListMP instances.
 *              The #create also initializes the shared memory as needed. The
 *              shared memory must be initialized to 0 or all ones
 *              (e.g. 0xFFFFFFFFF) before the ListMP instance is created.
 *              Once an instance is created, an open can be performed. The #open
 *              is used to gain access to the same ListMP instance.
 *              Generally an instance is created on one processor and opened
 *              on the other processor.
 *              The open returns a ListMP instance handle like the create,
 *              however the open does not modify the shared memory. Generally an
 *              instance is created on one processor and opened on the other
 *              processor.
 *              There are two options when opening the instance:
 *              @li Supply the same name as specified in the create. The
 *              ListMP module queries the NameServer to get the needed
 *              information.
 *              @li Supply the same #sharedAddr value as specified in the
 *              create.
 *              If the open is called before the instance is created, open
 *              returns NULL.
 *              There is currently a list of restrictions for the module:
 *              @li Both processors must have the same endianness. Endianness
 *              conversion may supported in a future version of ListMP.
 *              @li The module will be made a gated module
 *
 *  ============================================================================
 */


#ifndef LISTMP_H_0xA413
#define LISTMP_H_0xA413

/* Utilities headers */
#include <List.h>
#include <Gate.h>
#include <Heap.h>
#include <_ListMP.h>

#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 *  All success and failure codes for the module
 * =============================================================================
 */
/*!
 *  @def    LISTMP_MODULEID
 *  @brief  Unique module ID.
 */
#define LISTMP_MODULEID            (0xa413)

/*!
 *  @def    LISTMP_STATUSCODEBASE
 *  @brief  Error code base for ListMP.
 */
#define LISTMP_STATUSCODEBASE       (LISTMP_MODULEID << 12u)

/*!
 *  @def    LISTMP_MAKE_FAILURE
 *  @brief  Macro to make error code.
 */
#define LISTMP_MAKE_FAILURE(x)    ((Int)  (  0x80000000              \
                                           + (LISTMP_STATUSCODEBASE   \
                                           + (x))))

/*!
 *  @def    LISTMP_MAKE_SUCCESS
 *  @brief  Macro to make success code.
 */
#define LISTMP_MAKE_SUCCESS(x)    (LISTMP_STATUSCODEBASE + (x))

/*!
 *  @def    LISTMP_E_INVALIDARG
 *  @brief  Argument passed to a function is invalid.
 */
#define LISTMP_E_INVALIDARG       LISTMP_MAKE_FAILURE(1)

/*!
 *  @def    LISTMP_E_FAIL
 *  @brief  Generic failure
 */
#define LISTMP_E_FAIL             LISTMP_MAKE_FAILURE(2)

/*!
 *  @def    LISTMP_SUCCESS
 *  @brief  Operation successful.
 */
#define LISTMP_SUCCESS            LISTMP_MAKE_SUCCESS(0)

/*!
 *  @brief  Structure defining config parameters for the ListMP instances.
 */
typedef struct ListMP_Params_tag {

    Bool cacheFlag;
    /*!< Set to 1 by the open() call. No one else should touch this!   */
    Gate_Handle gate;
    /*!< Lock used for critical region management of the list */
    Ptr    sharedAddr;
    /*!< shared memory address */
    UInt32 sharedAddrSize;
    /*!< shared memory size */
    String name;
    /*!< Name of the object    */
    Int resourceId;
    /*!<
     *  resourceId  Specifies the resource id number.
     *  Parameter is used only when type is set to Fast List
     */
    ListMP_Type listType ;
    /*!< Type of list */
} ListMP_Params;

/* =============================================================================
 *  Functions to create instance of a list
 * =============================================================================
 */
/* Function to create an instance of ListMP */
ListMP_Handle ListMP_create (ListMP_Params * params);

/* Function to delete an instance of ListMP */
Int ListMP_delete (ListMP_Handle * listMPHandlePtr);
/* Function to initialize params of ListMP */
Void ListMP_Params_init(ListMP_Handle   handle,
                        ListMP_Params * params);
/* Function to get shared memory requirements */
Int ListMP_sharedMemReq (ListMP_Params * params);


/* =============================================================================
 *  Functions to open/close handle to list instance
 * =============================================================================
 */

/* Function to open a previously created instance */
Int ListMP_open  (ListMP_Handle * listMpHandlePtr,
                  ListMP_Params * params);

/* Function to close a previously opened instance */
Int ListMP_close  (ListMP_Handle * listMPHandle);

/* =============================================================================
 *  Function pointer types for list operations
 * =============================================================================
 */
/* Function to check if list is empty */
Bool   ListMP_empty   (ListMP_Handle listMPHandle);

/* Function to get head element from list */
Ptr    ListMP_getHead (ListMP_Handle listMPHandle);

/* Function to get tail element from list */
Ptr    ListMP_getTail (ListMP_Handle listMPHandle);

/* Function to put head element into list */
Int    ListMP_putHead (ListMP_Handle  listMPHandle,
                       ListMP_Elem  * elem);

/* Function to put tail element into list */
Int    ListMP_putTail (ListMP_Handle  listMPHandle,
                       ListMP_Elem  * elem);

/* Function to insert element into list */
Int    ListMP_insert (ListMP_Handle  listMPHandle,
                      ListMP_Elem  * elem,
                      ListMP_Elem  * curElem);

/* Function to remove element from list */
Int    ListMP_remove (ListMP_Handle  listMPHandle,
                      ListMP_Elem  * elem);

/* Function to traverse to next element in list */
Ptr    ListMP_next (ListMP_Handle   listMPHandle,
                    ListMP_Elem   * elem);

/* Function to traverse to prev element in list */
Ptr    ListMP_prev (ListMP_Handle   listMPHandle,
                    ListMP_Elem   * elem);


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* LISTMP_H_0xF414 */
