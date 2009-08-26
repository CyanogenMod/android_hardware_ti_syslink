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
 *  @file   ListMP.c
 *
 *  @brief      Defines for shared memory doubly linked list on HLOS side.
 *
 *              This module implements the ListMP on hlos side.
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
 *              The ListMP module uses a #NameServer instance
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
 *              create. If the open is called before the instance is created,
 *              open returns NULL.
 *              There is currently a list of restrictions for the module:
 *              @li Both processors must have the same endianness. Endianness
 *              conversion may supported in a future version of ListMP.
 *
 *  ============================================================================
 */



/* Standard headers */
#include <Std.h>

/* Osal And Utils  headers */
#include <String.h>
#include <List.h>
#include <Trace.h>

/* Module level headers */
#include <NameServer.h>
#include <_ListMP.h>
#include <ListMP.h>
#include <ListMPSharedMemory.h>


#if defined (__cplusplus)
extern "C" {
#endif

/*!
 *  @brief      Initializes ListMP parameters.
 *
 *  @param      handle  ListMP handle.
 *  @param      params  Instance config-params structure.
 *
 *  @sa         ListMP_Params_init
 */
Void ListMP_Params_init (ListMP_Handle   handle,
                         ListMP_Params * params)
{
    GT_2trace (curTrace, GT_ENTER, "ListMP_Params_init", handle, params);

    GT_assert (curTrace, (params != NULL));
    /* handle may be NULL and hence is not checked. */

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (params == NULL) {
        /*! @retval NULL Invalid NULL params pointer
         *          specified
         */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMP_Params_init",
                             LISTMP_E_INVALIDARG,
                             "Invalid NULL params pointer specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        ListMPSharedMemory_Params_init (handle, params);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    GT_0trace (curTrace, GT_LEAVE, "ListMP_Params_init");
}


/*!
 *  @brief      Creates and initializes ListMP module.
 *
 *  @param      params  Instance config-params structure.
 *
 *  @sa         ListMP_delete
 *              ListMPSharedMemory_create
 */
ListMP_Handle ListMP_create (ListMP_Params * params)
{
    ListMP_Object * handle  = NULL;

    GT_1trace (curTrace, GT_ENTER, "LISTMP_create", params);

    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (params == NULL) {
        /*! @retval NULL Invalid NULL params pointer
         *          specified
         */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMP_create",
                             LISTMP_E_INVALIDARG,
                             "Invalid NULL params pointer specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        if (params->listType == ListMP_Type_SHARED) {
            handle = (ListMP_Object *)ListMPSharedMemory_create (params);
        }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "LISTMP_create", handle);

    /*! @retval Valid-handle Operation successful */
    /*! @retval NULL         Operation not successful */
    return ((ListMP_Handle) handle);
}


/*!
 *  @brief      Deletes an instance of ListMPSharedMemory module.
 *
 *  @param      listMPHandlePtr  Pointer to Instance handle
 *
 *  @sa         ListMP_create
 */
Int ListMP_delete (ListMP_Handle * listMPHandlePtr)
{
    Int  status = LISTMP_SUCCESS;

    GT_1trace (curTrace, GT_ENTER, "ListMP_delete", listMPHandlePtr);

    GT_assert (curTrace, (listMPHandlePtr != NULL));
    GT_assert (curTrace, (   (listMPHandlePtr != NULL)
                          && (*listMPHandlePtr != NULL)));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (listMPHandlePtr == NULL) {
        /*! @retval LISTMP_E_INVALIDARG listMPHandlePtr passed is NULL */
        status = LISTMP_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMP_delete",
                             status,
                             "The parameter listMPHandlePtr i.e. pointer to "
                             "handle passed is NULL!");
    }
    else if (*listMPHandlePtr == NULL) {
        /*! @retval LISTMP_E_INVALIDARG *listMPHandlePtr passed is NULL */
        status = LISTMP_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMP_delete",
                             status,
                             "Invalid NULL passed for *listMPHandlePtr!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        if (((ListMP_Object *)(*listMPHandlePtr))->listType ==
                                                           ListMP_Type_SHARED) {
            status = ListMPSharedMemory_delete (listMPHandlePtr);
        }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ListMP_delete", status);

    /*! @retval LISTMP_SUCCESS Operation successful */
    return (status);
}


/*!
 *  @brief      Opens an instance of previously created ListMPSharedMemory
 *              module.
 *
 *  @param      listMPHandlePtr  Pointer to Instance handle
 *  @param      params           Instance config-params structure.
 *
 *  @sa         ListMP_close
 */
Int ListMP_open (ListMP_Handle * listMPHandlePtr, ListMP_Params * params)
{
    Int status = LISTMP_SUCCESS;

    GT_2trace (curTrace, GT_ENTER, "ListMP_open", listMPHandlePtr, params);

    GT_assert (curTrace, (listMPHandlePtr != NULL));
    GT_assert (curTrace, (params != NULL));


#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (listMPHandlePtr == NULL) {
        /*! @retval NULL Invalid NULL listMPHandlePtr pointer
         *          specified
         */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMP_open",
                             LISTMP_E_INVALIDARG,
                             "Invalid NULL listMPHandlePtr pointer specified!");
    }
    else if (params == NULL) {
        /*! @retval NULL Invalid NULL params pointer
         *          specified
         */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMP_open",
                             LISTMP_E_INVALIDARG,
                             "Invalid NULL params pointer specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        if (params->listType == ListMP_Type_SHARED) {
            status = ListMPSharedMemory_open (listMPHandlePtr, params);
        }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ListMP_open", status);

    /*! @retval LISTMP_SUCCESS Operation successful */
    return (status);
}



/*!
 *  @brief      Closes an instance of a previously opened ListMPSharedMemory
 *              module.
 *
 *  @param      listMPHandle  Instance handle
 *
 *  @sa         ListMP_open
 */
Int ListMP_close (ListMP_Handle * listMPHandle)
{
    Int  status = LISTMP_SUCCESS;

    GT_1trace (curTrace, GT_ENTER, "ListMP_close", listMPHandle);

    GT_assert (curTrace, (listMPHandle != NULL));
    GT_assert (curTrace, ((listMPHandle != NULL) && (*listMPHandle != NULL)));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (listMPHandle == NULL) {
        /*! @retval LISTMP_E_INVALIDARG Invalid NULL listMPHandle pointer
         *          specified
         */
        status = LISTMP_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMP_close",
                             status,
                             "Invalid NULL listMPHandle pointer specified!");
    }
    else if (*listMPHandle == NULL) {
        /*! @retval LISTMP_E_INVALIDARG Invalid NULL listMPHandle pointer
         *          specified
         */
        status = LISTMP_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMP_close",
                             status,
                             "Invalid NULL *listMPHandle specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        if (((ListMP_Object *)(*listMPHandle))->listType == ListMP_Type_SHARED) {
            status = ListMPSharedMemory_close (listMPHandle);
        }

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ListMP_close", status);

    /*! @retval LISTMP_SUCCESS Operation successful */
    return (status);
}


/*!
 *  @brief      Function to check if list is empty
 *
 *  @param      listMPHandle Handle to listMP instance
 *
 *  @sa         none
 */
Bool ListMP_empty (ListMP_Handle listMPHandle)
{
    Bool            isEmpty   = FALSE;
    ListMP_Object * handle    = NULL;

    GT_1trace (curTrace, GT_ENTER, "ListMP_empty", listMPHandle);

    GT_assert (curTrace, (listMPHandle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (listMPHandle == NULL) {
        /*! @retval LISTMP_E_INVALIDARG Invalid NULL listMPHandle pointer
         *          specified
         */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMP_empty",
                             LISTMP_E_INVALIDARG,
                             "Invalid NULL listMPHandle pointer specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

        handle = (ListMP_Object *)listMPHandle;

        GT_assert (curTrace, (handle->empty != NULL));
        isEmpty = handle->empty (listMPHandle);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ListMP_empty", isEmpty);

    /*! @retval TRUE list is empty */
    /*! @retval FALSE list is not empty */
    return (isEmpty);
}


/*!
 *  @brief      Function to get head element from list
 *
 *  @param      listMPHandle Handle to listMP instance
 *
 *  @sa         ListMP_getTail
 */
Ptr ListMP_getHead (ListMP_Handle listMPHandle)
{
    ListMP_Object * handle       = NULL;
    ListMP_Elem   * elem         = NULL;

    GT_1trace (curTrace, GT_ENTER, "ListMP_getHead", listMPHandle);

    GT_assert (curTrace, (listMPHandle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (listMPHandle == NULL) {

        /*! @retval NULL Invalid NULL listMPHandle pointer specified*/
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMP_getHead",
                             LISTMP_E_INVALIDARG,
                             "Invalid NULL listMPHandle pointer specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        handle = (ListMP_Object *)listMPHandle;

        GT_assert (curTrace, (handle->getHead != NULL));
        elem = handle->getHead (listMPHandle);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ListMP_getHead", elem);

    /*! @retval Valid-head-element Operation Successful */
    return elem;
}


/*!
 *  @brief      Function to get tail element from list
 *
 *  @param      listMPHandle Handle to listMP instance
 *
 *  @sa         ListMP_getHead
 */
Ptr ListMP_getTail (ListMP_Handle listMPHandle)
{
    ListMP_Object * handle = NULL;
    ListMP_Elem   * elem   = NULL;

    GT_1trace (curTrace, GT_ENTER, "ListMP_getTail", listMPHandle);

    GT_assert (curTrace, (listMPHandle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (listMPHandle == NULL) {
        /*! @retval NULL Invalid NULL listMPHandle pointer specified
         */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMP_getTail",
                             LISTMP_E_INVALIDARG,
                             "Invalid NULL listMPHandle pointer specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        handle = (ListMP_Object *)listMPHandle;

        GT_assert (curTrace, (handle->getTail != NULL));
        elem   = handle->getTail (listMPHandle);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ListMP_getTail", elem);

    /*! @retval Valid-tail-element Operation Successful */
    return elem;
}


/*!
 *  @brief      Function to put head element into list
 *
 *  @param      listMPHandle Handle to listMP instance
 *  @param      elem         ListMP_Elem element to be be added at head
 *
 *  @sa         ListMP_putTail
 */
Int ListMP_putHead(ListMP_Handle     listMPHandle,
                   ListMP_Elem     * elem)
{
    Int             status = LISTMP_SUCCESS;
    ListMP_Object * handle = NULL;

    GT_2trace (curTrace, GT_ENTER, "ListMP_putHead",
                                    listMPHandle,
                                    elem);

    GT_assert (curTrace, (listMPHandle != NULL));
    GT_assert (curTrace, (elem         != NULL));


#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (listMPHandle == NULL) {
        /*! @retval LISTMPSHAREDMEMORY_E_INVALIDARG
         *          Invalid NULL listMPHandle pointer
         *          specified
         */
        status = LISTMP_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMP_putHead",
                             status,
                             "Invalid NULL listMPHandle pointer specified!");
    }
    else if (elem == NULL) {
        /*! @retval LISTMPSHAREDMEMORY_E_INVALIDARG
         *          Invalid NULL elem pointer specified
         */
        status = LISTMP_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMP_putHead",
                             status,
                             "Invalid NULL elem pointer specified");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        handle = (ListMP_Object *) listMPHandle;

        GT_assert (curTrace, (handle->putHead != NULL));
        status = handle->putHead (listMPHandle, elem);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ListMP_putHead", status);

    /*! @retval LISTMP_SUCCESS Operation Successful*/
    return status;
}


/*!
 *  @brief      Function to put tail element from list
 *
 *  @param      listMPHandle Handle to listMP instance
 *  @param      elem         ListMP_Elem element to be added at tail
 *
 *  @sa         ListMP_putHead
 */
Int ListMP_putTail (ListMP_Handle    listMPHandle,
                    ListMP_Elem    * elem)
{
    Int             status       = LISTMP_SUCCESS;
    ListMP_Object * handle       = NULL;

    GT_2trace (curTrace, GT_ENTER, "ListMP_putTail", listMPHandle,
                                                     elem);

    GT_assert (curTrace, (listMPHandle != NULL));
    GT_assert (curTrace, (elem         != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (listMPHandle == NULL) {
        /*! @retval LISTMPSHAREDMEMORY_E_INVALIDARG
         *          Invalid NULL listMPHandle pointer
         *          specified
         */
        status = LISTMP_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMP_putTail",
                             status,
                             "Invalid NULL listMPHandle pointer specified!");
    }
    else if (elem == NULL) {
        /*! @retval LISTMPSHAREDMEMORY_E_INVALIDARG
         *          Invalid NULL elem pointer specified
         */
        status = LISTMP_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMP_putTail",
                             status,
                             "Invalid NULL elem pointer specified");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        handle = (ListMP_Object *)listMPHandle;

        GT_assert (curTrace, (handle->putTail != NULL));
        status = handle->putTail (listMPHandle,
                                  elem);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ListMP_putTail", status);

    /*! @retval LISTMP_SUCCESS Operation Successful*/
    return status;
}


/*!
 *  @brief      Function to insert element into list
 *
 *  @param      listMPHandle Handle to listMP instance
 *  @param      elem         ListMP_Elem element to be inserted
 *  @param      curElem      Current element before which element
 *                           to be inserted. If NULL, element
 *                           is inserted before head.
 *
 *  @sa         ListMP_insert
 */
Int ListMP_insert (ListMP_Handle    listMPHandle,
                   ListMP_Elem    * elem,
                   ListMP_Elem    * curElem)
{
    Int                   status   = LISTMP_SUCCESS;
    ListMP_Object         * handle = NULL;

    GT_3trace (curTrace, GT_ENTER, "ListMP_insert",
                                   listMPHandle,
                                   elem,
                                   curElem);

    GT_assert (curTrace, (listMPHandle != NULL));
    GT_assert (curTrace, (elem         != NULL));
    GT_assert (curTrace, (curElem      != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (listMPHandle == NULL) {
        /*! @retval LISTMPSHAREDMEMORY_E_INVALIDARG
         *          Invalid NULL listMPHandle pointer
         *          specified
         */
        status = LISTMP_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMP_insert",
                             status,
                             "Invalid NULL listMPHandle pointer specified!");
    }
    else if (elem == NULL) {
        /*! @retval LISTMPSHAREDMEMORY_E_INVALIDARG
         *          Invalid NULL elem pointer specified
         */
        status = LISTMP_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMP_insert",
                             status,
                             "Invalid NULL elem pointer specified");
    }
    else if (curElem == NULL) {
        /*! @retval LISTMPSHAREDMEMORY_E_INVALIDARG
         *          Invalid NULL curElem pointer specified
         */
        status = LISTMP_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMP_insert",
                             status,
                             "Invalid NULL curElem pointer specified");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        handle = (ListMP_Object *)listMPHandle;


        GT_assert (curTrace, (handle->insert != NULL));
        status = handle->insert (listMPHandle,
                                 elem,
                                 curElem);


#if !defined(SYSLINK_BUILD_OPTIMIZE)
            }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ListMP_insert", status);

    /*! @retval SUCCESS Operation Successful*/
    return status;
}


/*!
 *  @brief      Function to remove element from list
 *
 *  @param      listMPHandle Handle to listMP instance
 *  @param      elem         ListMP_Elem element to be removed
 *
 *  @sa         ListMP_insert
 */
Int ListMP_remove (ListMP_Handle   listMPHandle,
                   ListMP_Elem   * elem)
{
    Int             status   = LISTMP_SUCCESS;
    ListMP_Object * handle   = NULL;

    GT_2trace (curTrace,
               GT_ENTER,
               "ListMP_remove",
               listMPHandle,
               elem);

    GT_assert (curTrace, (listMPHandle != NULL));
    GT_assert (curTrace, (elem         != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (listMPHandle == NULL) {
        /*! @retval LISTMPSHAREDMEMORY_E_INVALIDARG Invalid NULL listMPHandle
         *          pointer specified
         */
        status = LISTMP_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMP_remove",
                             status,
                             "Invalid NULL listMPHandle pointer specified!");
    }
    else if (elem == NULL) {
        /*! @retval LISTMPSHAREDMEMORY_E_INVALIDARG
         *          Invalid NULL elem pointer specified
         */
        status = LISTMP_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMP_remove",
                             status,
                             "Invalid NULL elem pointer specified");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        handle = (ListMP_Object *)listMPHandle;

        GT_assert (curTrace, (handle->remove != NULL));
        status = handle->remove (listMPHandle, elem);


#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ListMP_remove", status);

    /*! @retval LISTMP_SUCCESS Operation Successful*/
    return status ;
}


/*!
 *  @brief      Function to put next element into list
 *
 *  @param      listMPHandle Handle to listMP instance
 *  @param      elem         ListMP_Elem element whose next
 *                           is to be traversed. If NULL
 *                           traversal starts after head
 *
 *
 *  @sa         ListMP_next
 */
Ptr ListMP_next (ListMP_Handle   listMPHandle,
                 ListMP_Elem   * elem)
{
    ListMP_Object * handle       = NULL;
    ListMP_Elem   * nextElem     = NULL;

    GT_1trace (curTrace, GT_ENTER, "ListMP_next", listMPHandle);

    GT_assert (curTrace, (listMPHandle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (listMPHandle == NULL) {
        /*! @retval LISTMPSHAREDMEMORY_E_INVALIDARG
         *          Invalid NULL listMPHandle pointer
         *          specified
         */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMP_next",
                             LISTMP_E_INVALIDARG,
                             "Invalid NULL listMPHandle pointer specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        handle = (ListMP_Object *)listMPHandle;

        GT_assert (curTrace, (handle->next != NULL));
        nextElem = handle->next (listMPHandle, elem);


#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ListMP_next", nextElem);

    /*! @retval Next-element Operation Successful */
    return nextElem;
}


/*!
 *  @brief      Function to get prev element from list
 *
 *  @param      listMPHandle Handle to listMP instance
 *  @param      elem         ListMP_Elem element whose prev
 *                           is to be traversed. If NULL
 *                           traversal starts before head
 *
 *  @sa         ListMP_next
 */
Ptr ListMP_prev (ListMP_Handle  listMPHandle,
                 ListMP_Elem  * elem)
{
    ListMP_Object * handle       = NULL;
    ListMP_Elem   * prevElem     = NULL;

    GT_1trace (curTrace, GT_ENTER, "ListMP_prev", listMPHandle);

    GT_assert (curTrace, (listMPHandle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (listMPHandle == NULL) {
        /*! @retval LISTMPSHAREDMEMORY_E_INVALIDARG
         *          Invalid NULL listMPHandle pointer
         *          specified
         */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMP_prev",
                             LISTMP_E_INVALIDARG,
                             "Invalid NULL listMPHandle pointer specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        handle = (ListMP_Object *)listMPHandle;

        GT_assert (curTrace, (handle->prev != NULL));
        prevElem = handle->prev (listMPHandle, elem);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ListMP_prev", prevElem);

    /*! @retval Previous-element Operation Successful */
    return prevElem;
}


/*!
 *  @brief      Gets shared memory rewuirement for the module.
 *
 *  @param      params  Instance config-params structure.
 *
 *  @sa         ListMP_delete
 */
Int ListMP_sharedMemReq (ListMP_Params * params)
{
    Int sharedMemReq = 0;

    GT_1trace (curTrace, GT_ENTER, "ListMP_sharedMemReq",params);

    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (params == NULL) {
        /*! @retval 0 Invalid NULL params pointer specified */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMP_sharedMemReq",
                             LISTMP_E_INVALIDARG,
                             "Invalid NULL params pointer specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        sharedMemReq = ListMPSharedMemory_sharedMemReq (params);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    GT_0trace (curTrace, GT_LEAVE, "ListMP_sharedMemReq");

    /*! @retval Shared-memory-requirements Operation Successful */
    return (sharedMemReq);
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
