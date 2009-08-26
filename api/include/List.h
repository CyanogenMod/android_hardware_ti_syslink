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
 *  @file   List.h
 *
 *  @brief      Creates a doubly linked list. It works as utils for other
 *              modules
 *
 *  ============================================================================
 */


#ifndef LIST_H_0XB734
#define LIST_H_0XB734


/* Standard headers */
#include <Gate.h>


#if defined (__cplusplus)
extern "C" {
#endif


/*!
 *  @def    LIST_MODULEID
 *  @brief  Unique module ID.
 */
#define LIST_MODULEID      (UInt16) 0xb734

/* =============================================================================
 *  All success and failure codes for the module
 * =============================================================================
 */
/*!
 *  @def    LIST_STATUSCODEBASE
 *  @brief  Error code base for List module.
 */
#define LIST_STATUSCODEBASE    (LIST_MODULEID << 12u)

/*!
 *  @def    LIST_MAKE_FAILURE
 *  @brief  Macro to make failure code.
 */
#define LIST_MAKE_FAILURE(x)   (0x80000000 | (LIST_STATUSCODEBASE + (x)))

/*!
 *  @def    LIST_MAKE_SUCCESS
 *  @brief  Macro to make success code.
 */
#define LIST_MAKE_SUCCESS(x)   (LIST_STATUSCODEBASE + (x))

/*!
 *  @def    LIST_E_INVALIDARG
 *  @brief  Argument passed to a function is invalid.
 */
#define LIST_E_INVALIDARG      LIST_MAKE_FAILURE(1)

/*!
 *  @def    LIST_E_FAIL
 *  @brief  Generic failure.
 */
#define LIST_E_FAIL            LIST_MAKE_FAILURE(2)

/*!
 *  @def    LIST_E_MEMORY
 *  @brief  Memory allocation failure.
 */
#define LIST_E_MEMORY          LIST_MAKE_FAILURE(3)

/*!
 *  @def    LIST_E_NOTFOUND
 *  @brief  Element not found.
 */
#define LIST_E_NOTFOUND        LIST_MAKE_FAILURE(4)

/*!
 *  @def    LIST_SUCCESS
 *  @brief  Operation is successful.
 */
#define LIST_SUCCESS           LIST_MAKE_SUCCESS(0)


/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*!
 *  @def    LIST_E_MEMORY
 *  @brief  Memory allocation failure.
 */
#define List_traverse(x,y) for(x = (List_Elem *)((List_Object *)y)->elem.next; \
                               (UInt32) x != (UInt32)&((List_Object *)y)->elem;\
                                x = x->next)

/*!
 *  @brief  Structure defining object for the list element.
 */
typedef struct List_Elem_tag {
    struct List_Elem_tag *  next; /*!< Pointer to the next element */
    struct List_Elem_tag *  prev; /*!< Pointer to the previous element */
} List_Elem;

/*!
 *  @brief  Structure defining object for the list.
 */
typedef struct List_Object_tag {
    List_Elem    elem;
    /*!< Head pointing to next element */
    Gate_Handle  listLock;
    /*!< Handle to lock for protecting objList */
} List_Object;

/*! @brief Defines List handle type */
typedef List_Object * List_Handle;

/*!
 *  @brief  Structure defining params for the list.
 */
typedef struct List_Params_tag {
    UInt32 reserved; /*!< Reserved value (not currently used) */
} List_Params;


/* =============================================================================
 *  APIs
 * =============================================================================
 */
/* Function to initialize the parameters structure */
Void List_Params_init (List_Params * params);

/* Function to create a list. */
List_Handle List_create (List_Params * params);

/* Function to delete the list */
Int List_delete (List_Handle * handlePtr);

/* Function to initialize the List head */
Int List_construct (List_Object * obj, List_Params * params);

/* Function to terminate List */
Int List_destruct (List_Object * obj);

/* Function to check if list is empty */
Bool List_empty (List_Handle handle);

/* Function to get front element */
Ptr List_get (List_Handle handle);

/* Function to put elem at the end */
Int List_put (List_Handle handle, List_Elem *elem);

/* Function to get next elem of current one */
Ptr List_next (List_Handle handle, List_Elem * elem);

/* Function to get previous elem of current one */
Ptr List_prev (List_Handle handle, List_Elem * elem);

/* Function to insert elem before existing elem */
Int List_insert (List_Handle handle, List_Elem * newElem, List_Elem * curElem);

/* Function to remove specific elem from list */
Int List_remove (List_Handle handle, List_Elem * elem);

/* Function to remove specific elem from list */
Int List_elemClear (List_Elem * elem);

/* Function to put element before head */
Int List_putHead (List_Handle handle, List_Elem * elem);


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* LIST_H_0XB734 */
