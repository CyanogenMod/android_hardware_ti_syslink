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
 *  @file   List.c
 *
 *  @brief      Creates a doubly linked list.
 *
 *  ============================================================================
 */



/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <List.h>
#include <Memory.h>
#include <Trace.h>
#include <Gate.h>
#include <GateMutex.h>


/* Module level headers */

#if defined (__cplusplus)
extern "C" {
#endif


/*!
 *  @brief      Initialize this config-params structure with supplier-specified
 *              defaults before instance creation.
 *
 *  @param      params  Instance config-params structure.
 *
 *  @sa         List_create
 */
Void
List_Params_init (List_Params * params)
{
    GT_1trace (curTrace, GT_ENTER, "List_Params_init", params);

    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (params == NULL) {
        /* No retVal since this is a Void function. */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "List_Params_init",
                             LIST_E_INVALIDARG,
                             "Argument of type (List_Params *) is NULL!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        params->reserved = 0u;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "List_Params_init");
}


/*!
 *  @brief      Function to create a list object.
 *
 *  @param      params  Pointer to list creation parameters. If NULL is passed,
 *                      default parameters are used.
 *
 *  @sa         List_delete
 */
List_Handle
List_create (List_Params * params)
{
    List_Object * obj = NULL;

    GT_1trace (curTrace, GT_ENTER, "List_create", params);

    (void) params;

    /* heapHandle can be NULL if created from default OS memory. */
    obj = (List_Object *) Memory_alloc (NULL,
                                        sizeof (List_Object),
                                        0);


    GT_assert (curTrace, (obj != NULL));
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (obj == NULL) {
        /*! @retval NULL Allocate memory for handle failed */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "List_create",
                             LIST_E_MEMORY,
                             "Allocating memory for handle failed");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        obj->elem.next = obj->elem.prev = &(obj->elem);
        obj->listLock  = GateMutex_create();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (obj->listLock == NULL) {
            Memory_free (NULL, obj, sizeof (List_Object));
            obj = NULL;
            /*! @retval NULL Failed to create GateMutex */
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "List_create",
                                 LIST_E_FAIL,
                                 "Failed to create GateMutex");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "List_create", obj);

    /*! @retval valid-handle Operation successful */
    return (List_Handle) obj;
}


/*!
 *  @brief      Function to delete a list object.
 *
 *  @param      handle  Pointer to the list handle
 *
 *  @sa         List_delete
 */
Int
List_delete (List_Handle * handlePtr)
{
    Int           status = LIST_SUCCESS;
    List_Object * obj;

    GT_1trace (curTrace, GT_ENTER, "List_delete", handlePtr);

    GT_assert (curTrace, (handlePtr != NULL));
    GT_assert (curTrace, ((handlePtr != NULL) && (*handlePtr != NULL)));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handlePtr == NULL) {
        /*! @retval LIST_E_INVALIDARG handlePtr passed is invalid*/
        status = LIST_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "List_delete",
                             status,
                             "handlePtr passed is invalid!");
    }
    else if (*handlePtr == NULL) {
        /*! @retval LIST_E_INVALIDARG *handlePtr passed is invalid*/
        status = LIST_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "List_delete",
                             status,
                             "*handlePtr passed is invalid!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        obj = (List_Object *) (*handlePtr);
        status = GateMutex_delete (&(obj->listLock));
        GT_assert (curTrace, (status > 0));
        Memory_free (NULL, obj, sizeof (List_Object));
        *handlePtr = NULL;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "List_delete", status);

    /*! @retval LIST_SUCCESS Operation was successful */
    return status;
}


/*!
 *  @brief      Function to construct a list object.
 *
 *  @param      obj     Pointer to the list object to be constructed
 *  @param      params  Pointer to list construction parameters. If NULL is
 *                      passed, default parameters are used.
 *
 *  @sa         List_destruct
 */
Int
List_construct (List_Object * obj, List_Params * params)
{
    Int status = LIST_SUCCESS;

    GT_1trace (curTrace, GT_ENTER, "List_construct", obj);

    GT_assert (curTrace, (obj != NULL));
    /* params may be provided as NULL. */
    (void) params;

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (obj == NULL) {
        /*! @retval LIST_E_INVALIDARG Invalid NULL passed for obj parameter */
        status = LIST_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "List_construct",
                             status,
                             "Invalid NULL passed for obj parameter");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        obj->elem.next = obj->elem.prev = &(obj->elem);
        obj->listLock  = GateMutex_create();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (obj->listLock == NULL) {
            /*! @retval LIST_E_FAIL Failed to create GateMutex */
            status = LIST_E_FAIL;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "List_construct",
                                 status,
                                 "Failed to create GateMutex");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "List_construct", status);

    /*! @retval LIST_SUCCESS Operation was successful */
    return status;
}


/*!
 *  @brief      Function to destruct a list object.
 *
 *  @param      obj  Pointer to the list object to be destructed
 */
Int
List_destruct (List_Object * obj)
{
    Int status = LIST_SUCCESS;

    GT_1trace (curTrace, GT_ENTER, "List_destruct", obj);

    GT_assert (curTrace, (obj != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (obj == NULL) {
        /*! @retval LIST_E_INVALIDARG Invalid NULL passed for obj parameter */
        status = LIST_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "List_destruct",
                             status,
                             "Invalid NULL passed for obj parameter");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        status = GateMutex_delete (&(obj->listLock));
        GT_assert (curTrace, (status > 0));
        obj->elem.next = NULL;
        obj->elem.prev = NULL;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "List_destruct", status);

    /*! @retval LIST_SUCCESS Operation was successful */
    return status;
}


/*!
 *  @brief     Function to clear element contents.
 *
 *  @param     elem Element to be cleared
 */
Int
List_elemClear (List_Elem * elem)
{
    Int status = LIST_SUCCESS;

    GT_1trace (curTrace, GT_ENTER, "List_elemClear", elem);

    GT_assert (curTrace, (elem != NULL));
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (elem == NULL) {
        /*! @retval LIST_E_INVALIDARG Invalid NULL passed for elem parameter */
        status = LIST_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "List_elemClear",
                             status,
                             "Invalid NULL passed for elem parameter");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
         elem->next = elem->prev = elem;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "List_elemClear", status);

    /*! @retval LIST_SUCCESS Operation was successful */
    return status;
}


/*!
 *  @brief     Function to check if list is empty.
 *
 *  @param     handle  Pointer to the list
 */
Bool
List_empty (List_Handle handle)
{
    Bool          isEmpty = FALSE;
    List_Object * obj     = (List_Object *) handle;

    GT_1trace (curTrace, GT_ENTER, "List_empty", handle);

    GT_assert (curTrace, (handle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "List_empty",
                             LIST_E_INVALIDARG,
                             "Invalid NULL passed for handle parameter");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        if (obj->elem.next == &(obj->elem)) {
            /*! @retval TRUE List is empty */
            isEmpty = TRUE;
        }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "List_empty", isEmpty);

    /*! @retval FALSE List is not empty */
    return isEmpty;
}

/*!
 *  @brief     Function to get first element of List.
 *
 *  @param     handle  Pointer to the list
 */
Ptr
List_get (List_Handle handle)
{
    List_Elem *   elem = NULL;
    List_Object * obj  = (List_Object *) handle;
    UInt32        key;

    GT_1trace (curTrace, GT_ENTER, "List_get", handle);

    GT_assert (curTrace, (handle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        /*! @retval NULL Invalid NULL passed for handle parameter */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "List_get",
                             LIST_E_INVALIDARG,
                             "Invalid NULL passed for handle parameter");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        GT_assert (curTrace, (obj->listLock != NULL));
        key = Gate_enter (obj->listLock);
        elem = obj->elem.next;
        /* See if the List was empty */
        if (elem == (List_Elem *)obj) {
            /*! @retval NULL List is empty */
            elem = NULL;
        }
        else {
            obj->elem.next   = elem->next;
            elem->next->prev = &(obj->elem);
        }
        Gate_leave(obj->listLock, key);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "List_get", elem);

    /*! @retval Valid-pointer Pointer to first element */
    return elem ;
}

/*!
 *  @brief     Function to insert element at the end of List.
 *
 *  @param     handle  Pointer to the list
 *  @param     element Element to be inserted
 */
Int
List_put (List_Handle handle, List_Elem * elem)
{
    Int           status = LIST_SUCCESS;
    List_Object * obj    = (List_Object *) handle;
    UInt32        key;

    GT_2trace (curTrace, GT_ENTER, "List_put", handle, elem);

    GT_assert (curTrace, (handle != NULL));
    GT_assert (curTrace, (elem != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        /*! @retval LIST_E_INVALIDARG Invalid NULL passed for handle parameter*/
        status = LIST_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "List_put",
                             LIST_E_INVALIDARG,
                             "Invalid NULL passed for handle parameter");
    }
    else if (elem == NULL) {
        /*! @retval LIST_E_INVALIDARG Invalid NULL passed for elem parameter */
        status = LIST_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "List_put",
                             LIST_E_INVALIDARG,
                             "Invalid NULL passed for elem parameter");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        GT_assert (curTrace, (obj->listLock != NULL));
        key = Gate_enter (obj->listLock);
        elem->next           = &(obj->elem);
        elem->prev           = obj->elem.prev;
        obj->elem.prev->next = elem;
        obj->elem.prev       = elem;
        Gate_leave (obj->listLock, key);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "List_put", status);

    /*! @retval LIST_SUCCESS Operation was successful */
    return status;
}


/*!
 *  @brief      Function to traverse to the next element in the list
 *
 *  @param     handle  Pointer to the list
 *  @param     elem    Pointer to the current element
 */
Ptr
List_next (List_Handle handle, List_Elem * elem)
{
    List_Elem *   retElem = NULL;
    List_Object * obj     = (List_Object *) handle;

    GT_2trace (curTrace, GT_ENTER, "List_Next", handle, elem);

    GT_assert (curTrace, (handle != NULL)) ;

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        /*! @retval NULL Invalid NULL passed for handle parameter */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "List_next",
                             LIST_E_INVALIDARG,
                             "Invalid NULL passed for handle parameter");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* elem == NULL -> start at the head */
        if (elem == NULL) {
            retElem = obj->elem.next;
        }
        else {
            retElem = elem->next;
        }

        if (retElem == (List_Elem *) obj) {
            /*! @retval NULL List reached end or list is empty */
            retElem = NULL;
        }

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "List_Next", retElem);

    /*! @retval Valid-pointer Pointer to the next element */
    return retElem;
}


/*!
 *  @brief     Function to traverse to the previous element in the list
 *
 *  @param     handle  Pointer to the list
 *  @param     elem    Pointer to the current element
 *
 *  @sa
 */
Ptr
List_prev (List_Handle handle, List_Elem * elem)
{
    List_Elem *   retElem = NULL;
    List_Object * obj     = (List_Object *) handle;

    GT_2trace (curTrace, GT_ENTER, "List_prev", handle, elem);

    GT_assert (curTrace, (handle != NULL)) ;
    GT_assert (curTrace, (elem != NULL)) ;

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        /*! @retval NULL Invalid NULL passed for handle parameter */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "List_prev",
                             LIST_E_INVALIDARG,
                             "Invalid NULL passed for handle parameter");
    }
    else if (elem == NULL) {
        /*! @retval NULL Invalid NULL passed for elem parameter */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "List_prev",
                             LIST_E_INVALIDARG,
                             "Invalid NULL passed for elem parameter");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
      /* elem == NULL -> start at the head */
        if (elem == NULL) {
            retElem = obj->elem.prev;
        }
        else {
            retElem = elem->prev;
        }

        if (retElem == (List_Elem *)obj) {
            /*! @retval NULL List reached end or list is empty */
            retElem = NULL;
        }

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "List_prev", retElem);

    /*! @retval Valid-pointer Pointer to the prev element */
    return retElem;
}


/*!
 *  @brief      Function to insert element before the existing element
 *              in the list.
 *
 *  @param     handle  Pointer to the list
 *  @param     newElem Element to be inserted
 *  @param     curElem Existing element before which new one is to be inserted
 */
Int
List_insert  (List_Handle handle, List_Elem * newElem, List_Elem * curElem)
{
    Int           status = LIST_SUCCESS;
    List_Object * obj    = (List_Object *) handle;

    GT_3trace (curTrace, GT_ENTER, "List_insert", handle, newElem, curElem);

    GT_assert (curTrace, (handle     != NULL));
    GT_assert (curTrace, (newElem != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (obj == NULL) {
        /*! @retval LIST_E_INVALIDARG Invalid NULL passed for handle parameter*/
        status = LIST_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "List_insert",
                             status,
                             "Invalid NULL passed for handle parameter");
    }
    else if (newElem == NULL) {
        /*! @retval LIST_E_INVALIDARG Invalid NULL passed for newElem
                                      parameter */
        status = LIST_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "List_insert",
                             status,
                             "Invalid NULL passed for newElem parameter");
    }
    else if (curElem == NULL) {
        /*! @retval LIST_E_INVALIDARG NULL passed for curElem parameter */
        status = LIST_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "List_insert",
                             status,
                  "Invalid NULL passed for curElem parameter use List_putHead");
    }else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            newElem->next       = curElem;
            newElem->prev       = curElem->prev;
            newElem->prev->next = newElem;
            curElem->prev       = newElem;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "List_insert", status);

    /*! @retval LIST_SUCCESS Operation was successful */
    return status;
}


/*!
 *  @brief      Function to removes element from the List.
 *
 *  @param     handle    Pointer to the list
 *  @param     element   Element to be removed
 */
Int
List_remove (List_Handle handle, List_Elem * elem)
{
    Int status = LIST_SUCCESS;

    GT_2trace (curTrace, GT_ENTER, "List_remove", handle, elem);

    GT_assert (curTrace, (handle != NULL));
    GT_assert (curTrace, (elem != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        /*! @retval LIST_E_INVALIDARG Invalid NULL passed for handle parameter*/
        status = LIST_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "List_remove",
                             status,
                             "Invalid NULL passed for handle parameter");
    }
    else if (elem == NULL) {
        /*! @retval LIST_E_INVALIDARG Invalid NULL passed for elem parameter */
        status = LIST_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "List_remove",
                             status,
                             "Invalid NULL passed for elem parameter");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        elem->prev->next = elem->next;
        elem->next->prev = elem->prev;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "List_remove", status);

    /*! @retval LIST_SUCCESS Operation was successful */
    return status;
}


/*!
 *  @brief      Function to put the element before head.
 *
 *  @param     handle    Pointer to the list
 *  @param     element   Element to be removed
 */
Int
List_putHead (List_Handle handle, List_Elem *elem)
{
    Int           status = LIST_SUCCESS;
    List_Object * obj    = (List_Object *) handle;
    UInt32        key;

    GT_2trace (curTrace, GT_ENTER, "List_putHead", handle, elem);

    GT_assert (curTrace, (handle != NULL));
    GT_assert (curTrace, (elem != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        /*! @retval LIST_E_INVALIDARG Invalid NULL passed for handle parameter*/
        status = LIST_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "List_putHead",
                             LIST_E_INVALIDARG,
                             "Invalid NULL passed for handle parameter");
    }
    else if (elem == NULL) {
        /*! @retval LIST_E_INVALIDARG Invalid NULL passed for elem parameter */
        status = LIST_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "List_putHead",
                             LIST_E_INVALIDARG,
                             "Invalid NULL passed for elem parameter");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        GT_assert (curTrace, (obj->listLock != NULL));
        key = Gate_enter (obj->listLock);

        elem->next           = obj->elem.next;
        elem->prev           = &(obj->elem);
        obj->elem.next->prev = elem;
        obj->elem.next       = elem;

        Gate_leave (obj->listLock, key);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "List_putHead",status);

    return(status);

}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
