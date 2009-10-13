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
/*
 * List.c
 *
 * IPCList module creates and manages doubly linked list
 */
#include <host_os.h>
#include <Std.h>
#include <List.h>
#include <Trace.h>

/*
* ======== List_create ========
* Purpose:
* This will create a list object
*
*  @param      heapHandle  Pointer to heapHandle (not used here,
*	       used for compatibility with syslink)
*/
Handle List_create(Handle heapHandle)
{
	List_Object * obj = NULL;

	GT_0trace(curTrace, GT_ENTER, "List_create \n");
	obj = (List_Object *)malloc(sizeof(List_Object));

	if (obj == NULL) {
		GT_setFailureReason(curTrace,
				GT_4CLASS,
				"List_create",
				LIST_E_MEMORY,
				"Allocating memory for handle failed");
	} else {
		obj->elem.next = &(obj->elem);
		obj->elem.prev = &(obj->elem);
	}
	GT_1trace(curTrace, GT_LEAVE, "List_create obj: %x \n", obj);
	/* @retval valid-handle Operation successful */
	return obj;
}

/*
* ======== List_delete ========
* Purpose:
* This will delete a list object
*
*  @param      obj	   Pointer to list object
*
*  @param      heapHandle  Pointer to heapHandle (not used here,
*	       used for compatibility with syslink)
*/
Int List_delete(Handle heapHandle, List_Object *obj)
{
	Int status = LIST_SUCCESS;

	GT_1trace(curTrace, GT_ENTER, "List_delete obj: %x \n", obj);
	if (obj == NULL) {
		status = LIST_E_INVALIDARG;
		GT_setFailureReason(curTrace,
				GT_4CLASS,
				"List_delete",
				status,
				"List object points to NULL!");
	} else {
		free(obj);
	}
	GT_0trace(curTrace, GT_LEAVE, "List_delete \n");
	/*! @retval LIST_SUCCESS Operation was successful */
	return status;
}

/*
* ======== List_elemClear ========
* Purpose:
* This will removes an element from the List
*
*  @param      elem Element to be removed
*
*/
Int List_elemClear(List_Elem *elem)
{
	Int status = LIST_SUCCESS;
	GT_1trace(curTrace, GT_ENTER, "List_elemClear elem: %x \n",
		elem);
	if (elem == NULL) {
		status = LIST_E_INVALIDARG;
		GT_setFailureReason(curTrace,
				GT_4CLASS,
				"List_elemClear",
				status,
				"Pointer to the element is null");
	} else {
    		elem->next = elem->prev = elem;
	}
	GT_1trace(curTrace, GT_LEAVE, "List_elemClear status: %x \n",
		status);
	/*! @retval LIST_SUCCESS Operation was successful */
	return status;
}

/*
* ======== List_empty ========
* Purpose:
* This will removes an element from the List
*
*  @param     obj Pointer to the list
*
*/
Bool List_empty(List_Object *obj)
{
	Bool isEmpty = FALSE;
	GT_1trace(curTrace, GT_ENTER, "List_empty obj: %x \n", obj);
	if (obj == NULL) {
		GT_setFailureReason(curTrace,
				GT_4CLASS,
				"List_empty",
				LIST_E_INVALIDARG,
				"List object points to null");
	} else {
		/*! @retval TRUE if list is empty */
		if (obj->elem.next == &(obj->elem))
			isEmpty = TRUE;
	}
	GT_1trace(curTrace, GT_LEAVE, "List_empty isEmpty: %x \n",
		isEmpty);
	/*! @retval FALSE if list is not empty */
	return isEmpty;
}


/*
* ======== List_get ========
* Purpose:
* This will get first element of List
*
*  @param     obj Pointer to the list
*
*/
Ptr List_get(List_Object *obj)
{
	List_Elem * elem = NULL;
	GT_1trace(curTrace, GT_ENTER, "List_get obj: %x \n",
		obj);
	if (obj == NULL) {
		GT_setFailureReason(curTrace,
			GT_4CLASS,
			"List_get",
			LIST_E_INVALIDARG,"");
	} else {
		elem = obj->elem.next;
		/* See if the List was empty */
		if (elem == (List_Elem *)obj) {
			elem = NULL;
		} else {
			obj->elem.next   = elem->next;
			elem->next->prev = &(obj->elem);
		}
	}
	GT_1trace(curTrace, GT_LEAVE, "List_get elem: %x \n", elem);
	/*! @retval NULL          if list is empty */
	/*! @retval valid-pointer Pointer to first element */
	return elem ;
}

/*
* ======== List_put ========
* Purpose:
* This will insert element at the end of List
*
*  @param     list	pointer to the list
*  @param     element	element to be inserted
*
*/
Int List_put(List_Object *obj, List_Elem *elem)
{
	Int status = LIST_SUCCESS;

	GT_2trace(curTrace, GT_ENTER, "List_put obj: %x, elem: %x",
		obj, elem);
	if (obj == NULL) {
		/*! @retval NULL List is not created */
		GT_setFailureReason(curTrace,
				GT_4CLASS,
				"List_put",
				LIST_E_INVALIDARG,
				"List object is null");
	} else if (elem == NULL) {
		/*! @retval NULL List is not created */
		GT_setFailureReason(curTrace,
				GT_4CLASS,
				"List_put",
				LIST_E_INVALIDARG,
				"Element is null");
	} else {
		elem->next           = &(obj->elem);
		elem->prev           = obj->elem.prev;
		obj->elem.prev->next = elem;
		obj->elem.prev       = elem;
	}
	GT_1trace(curTrace, GT_LEAVE, "List_PutTail status: %x \n",
		status);
	/*! @retval LIST_SUCCESS Operation was successful */
	return status;
}

/*
* ======== List_next ========
* Purpose:
* This will traverse to the next element in the list
*
*  @param      list  pointer to the list
*  @param      elem  pointer to the current element
*
*/
Ptr List_next (List_Object *obj, List_Elem *elem)
{
	List_Elem * retElem = NULL;

	GT_2trace(curTrace, GT_ENTER, "List_Next obj: %x, elem: %x \n",
		obj, elem);
	if (obj == NULL) {
		GT_setFailureReason(curTrace,
				GT_4CLASS,
				"List_next",
				LIST_E_INVALIDARG,
				"List object is null");
	} else {
		/* elem == NULL -> start at the head */
		if (elem== NULL)
			retElem = obj->elem.next;
		else
			retElem = elem->next;

		if (retElem == (List_Elem *)obj)
			retElem = NULL;
	}	
	GT_1trace(curTrace, GT_LEAVE, "List_Next retElem: %x \n",
		retElem);
	/*! @retval NULL is list reaches end or list is empty */
	/*! @retval valid-pointer Pointer to the next element */
	return retElem;
}

/*
* ======== List_prev ========
* Purpose:
* This will  traverse to the previous element in the list
*
*  @param      obj   pointer to the list
*  @param      elem  pointer to the current element
*
*/
Ptr List_prev(List_Object *obj, List_Elem *elem)
{
	List_Elem * retElem = NULL;

	GT_2trace(curTrace, GT_ENTER, "List_prev obj: %x, elem = %x \n",
		obj, elem);
	if (obj == NULL) {
		GT_setFailureReason(curTrace,
				GT_4CLASS,
				"List_prev",
				LIST_E_INVALIDARG,
				"List object points to null");
	} else if (elem == NULL) {
		GT_setFailureReason(curTrace,
				GT_4CLASS,
				"List_prev",
				LIST_E_INVALIDARG,
				"Element points to null");
	} else {
		/* elem == NULL -> start at the head */
		if (elem == NULL)
			retElem = obj->elem.prev;
		else 
			retElem = elem->prev;

		if (retElem == (List_Elem *)obj)
			retElem = NULL;
	}	
	GT_1trace(curTrace, GT_LEAVE, "List_prev retElem: %x \n",
		retElem);
	/*! @retval NULL is list reaches end or list is empty */
	/*! @retval valid-pointer Pointer to the prev element */
	return retElem;
}

/*
* ======== List_insert ========
* Purpose:
* This will  insert element before the existing element
*  in the list.
*
*  @param     obj     pointer to the list
*  @param     newElem element to be inserted
*  @param     curElem existing element before which new one is to be inserted
*
*/
Int List_insert(List_Object *obj, List_Elem *newElem, List_Elem *curElem)
{
	Int status = LIST_SUCCESS;
	GT_3trace (curTrace, GT_ENTER, "List_insert obj: %x, newElem: %x,"
		"curElem: %x \n", obj, newElem, curElem);
	if (obj == NULL) {
		/*! @retval LIST_E_INVALIDARG List object points to null */
		status = LIST_E_INVALIDARG;
		GT_setFailureReason(curTrace,
				GT_4CLASS,
				"List_insert",
				status,
				"List object points to null");
	} else if (newElem == NULL) {
	/*! @retval LIST_E_INVALIDARG New element points to null */
		status = LIST_E_INVALIDARG;
		GT_setFailureReason(curTrace,
				GT_4CLASS,
				"List_insert",
				status,
				"New element points to null");
	} else {
		/* If NULL change curElem to the obj */
		if (curElem == NULL)
			curElem = obj->elem.next;

		newElem->next       = curElem;
		newElem->prev       = curElem->prev;
		newElem->prev->next = newElem;
		curElem->prev       = newElem;
	}
	GT_1trace(curTrace, GT_LEAVE, "List_insert status: %x \n", status);
	/*! @retval LIST_SUCCESS Operation was successful */
	return status;
}


/*
* ======== List_insert ========
* Purpose:
* This will  removes element from the List
*
*  @param     obj	pointer to the list
*  @param     element	element to be removed
*
*/
Int List_remove(List_Object *obj, List_Elem *elem)
{
	Int status = LIST_SUCCESS;

	GT_2trace(curTrace, GT_ENTER, "List_remove obj: %x, elem: %x \n",
		obj, elem);
	if (obj == NULL) {
		/*! @retval LIST_E_INVALIDARG List object is null */
		status = LIST_E_INVALIDARG;
		GT_setFailureReason(curTrace,
				GT_4CLASS,
				"List_remove",
				status,
				"List object is null");
	} else if (elem == NULL) {
		/*! @retval LIST_E_INVALIDARG Element is null */
		status = LIST_E_INVALIDARG;
		GT_setFailureReason (curTrace,
			GT_4CLASS,
			"List_remove",
			status,
			"Element is null");
	} else {
		elem->prev->next = elem->next;
		elem->next->prev = elem->prev;
	}
	GT_1trace (curTrace, GT_LEAVE, "List_remove status: %x \n", status);
	/*! @retval LIST_SUCCESS Operation was successful */
	return status;
}

