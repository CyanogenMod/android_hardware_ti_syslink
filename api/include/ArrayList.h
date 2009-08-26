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
/**********************************************************************/
/* ArrayList.h                                                        */
/*                                                                    */
/* This implementation of ArrayList is a replacement for the C++      */
/* vector class in C.                                                 */
/*                                                                    */
/* This class tries to emulate a resizable array along the lines of   */
/* a C++ vector or Java ArrayList class in C, and uses the convention */
/* of passing a pointer to the current "object" as the first          */
/* argument.                                                          */
/*                                                                    */
/* Usage is defined as follows:                                       */
/*                                                                    */
/* Array_List obj;                                                    */
/* AL_initialize(&obj, sizeof(type_name));                            */
/*                                                                    */
/* ...                                                                */
/*                                                                    */
/* type_name* ptr = (type_name*)(obj.buf);                            */
/* for(i=0; i<AL_size(&obj); i++)                                     */
/*     do_something_to(ptr[i]);                                       */
/* type_name to_append = ...;                                         */
/* AL_append(&obj, &to_append);                                       */
/* AL_destroy(&obj);                                                  */
/**********************************************************************/
#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#include <inttypes.h>

/**********************************************************************/
/* Array_List - structure type specification.                         */
/**********************************************************************/
typedef struct 
{
   void   *buf;
   int32_t type_size;
   int32_t size;
   int32_t buffer_size;
} Array_List;

/*--------------------------------------------------------------------*/
/* Array_List Member Functions:                                       */
/*                                                                    */
/* AL_initialize() - Initialize a newly created Array_List object.    */
/* AL_append() - Append an element to the end of an Array_List.       */
/* AL_size() - Get number of elements in an Array_List.               */
/* AL_destroy() - Free memory associated with an Array_List that is   */
/*                no longer in use.                                   */
/*--------------------------------------------------------------------*/
void     AL_initialize(Array_List* obj, int32_t type_size, int32_t num_elem);
void     AL_append(Array_List* obj, void* to_append);
int32_t  AL_size(Array_List* obj);
void     AL_destroy(Array_List* obj);

#endif
