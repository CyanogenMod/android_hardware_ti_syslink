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
 *  @file   _ListMP.h
 *
 *  @brief      Internal definitions  for Internal Defines for shared memory
 *              doubly linked list.
 *
 *  
 *  ============================================================================
 */


#ifndef _LISTMP_H_0xfe5f
#define _LISTMP_H_0xfe5f

/* Standard headers */
#include <Std.h>


#if defined (__cplusplus)
extern "C" {
#endif


/*!
 *  @brief  Enum defining types of list for the ListMP module.
 */
typedef enum {
    ListMP_Type_SHARED = 0,
    /*!< List in shared memory */
    ListMP_Type_FAST   = 1
    /*!< Hardware Queue */
} ListMP_Type ;


/*!
 *  @brief  Structure defining list element for the ListMP.
 */
typedef struct ListMP_Elem_tag {
    volatile struct ListMP_Elem_tag * next;
    /*!< pointer to the next element inhe list */
    volatile struct ListMP_Elem_tag * prev;
    /*!< pointer to the previous element inhe list */
} ListMP_Elem;

/* =============================================================================
 *  Forward declarations
 * =============================================================================
 */
/*! @brief Forward declaration of structure defining object for the
 *         ListMP
 */
typedef struct ListMP_Object_tag ListMP_Object;

/*!
 *  @brief  Handle for the ListMP
 */
typedef struct ListMP_Object * ListMP_Handle;

/* =============================================================================
 *  Function pointer types for list operations
 * =============================================================================
 */
/* Function to check if list is empty */
typedef Bool (*ListMP_emptyFxn)  (ListMP_Handle listMPHandle);

/* Function to get head element from list */
typedef Ptr (*ListMP_getHeadFxn) (ListMP_Handle listMPHandle);

/* Function to get tail element from list */
typedef Ptr (*ListMP_getTailFxn) (ListMP_Handle listMPHandle);

/* Function to put head element into list */
typedef Int (*ListMP_putHeadFxn) (ListMP_Handle  listMPHandle,
                                  ListMP_Elem  * elem);

/* Function to put tail element into list */
typedef Int (*ListMP_putTailFxn) (ListMP_Handle  listMPHandle,
                                  ListMP_Elem  * elem);

/* Function to insert element into list */
typedef Int (*ListMP_insertFxn) (ListMP_Handle  listMPHandle,
                                 ListMP_Elem  * elem,
                                 ListMP_Elem  * curElem);

/* Function to remove element from list */
typedef Int (*ListMP_removeFxn) (ListMP_Handle  listMPHandle,
                                 ListMP_Elem  * elem);

/* Function to traverse to next element in list */
typedef Ptr (*ListMP_nextFxn) (ListMP_Handle   listMPHandle,
                               ListMP_Elem   * elem);

/* Function to traverse to prev element in list */
typedef Ptr (*ListMP_prevFxn) (ListMP_Handle  listMPHandle,
                               ListMP_Elem  * elem);

/*!
 *  @brief  Structure defining config parameters for the ListMPSharedMemory.
 */
struct ListMP_Object_tag {
    ListMP_emptyFxn               empty;
    /* Function to check if list is empty */
    ListMP_getHeadFxn             getHead;
    /* Function to get head element from list */
    ListMP_getTailFxn             getTail;
    /* Function to get tail element from list */
    ListMP_putHeadFxn             putHead;
    /* Function to put head element into list */
    ListMP_putTailFxn             putTail;
    /* Function to put tail element into list */
    ListMP_insertFxn              insert;
    /* Function to insert element into list */
    ListMP_removeFxn              remove;
    /* Function to remove element from list */
    ListMP_nextFxn                next;
    /* Function to traverse to next element in list */
    ListMP_prevFxn                prev;
    /* Function to traverse to prev element in list */
    Ptr                           obj;
    /* Handle to ListMP */
    ListMP_Type                   listType;
    /* Type of list */
};


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* _LISTMP_H_0xfe5f */

