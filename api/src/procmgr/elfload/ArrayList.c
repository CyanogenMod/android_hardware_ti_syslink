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
/*****************************************************************************/
/* ArrayList.c                                                               */
/*                                                                           */
/* Array_List is a C implementation of a C++ vector class.                   */
/*                                                                           */
/* This class tries to emulate a resizable array along the lines of          */
/* a C++ vector or Java ArrayList class in C, and uses the convention        */
/* of passing a pointer to the current "object" as the first                 */
/* argument.                                                                 */
/*                                                                           */
/* Usage is defined as follows:                                              */
/*                                                                           */
/* Array_List obj;                                                           */
/* AL_initialize(&obj, sizeof(type_name));                                   */
/*                                                                           */
/* ...                                                                       */
/*                                                                           */
/* type_name* ptr = (type_name*)(obj.buf);                                   */
/* for(i=0; i<AL_size(&obj); i++)                                            */
/*     do_something_to(ptr[i]);                                              */
/* type_name to_append = ...;                                                */
/* AL_append(&obj, &to_append);                                              */
/* AL_destroy(&obj);                                                         */
/*****************************************************************************/
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "ArrayList.h"
#include "dload_api.h"

/*****************************************************************************/
/* AL_INITIALIZE() - Initialize a newly created Array_List object.           */
/*****************************************************************************/
void AL_initialize(Array_List* obj, int32_t type_size, int32_t num_elem)
{
   if (num_elem == 0) num_elem = 1;
   obj->buf = malloc(type_size * num_elem);
   obj->type_size = type_size;
   obj->size = 0;
   obj->buffer_size = num_elem;
}

/*****************************************************************************/
/* AL_APPEND() - Append an element to the end of an Array_List.              */
/*****************************************************************************/
void AL_append(Array_List* obj, void* to_append)
{
   /*------------------------------------------------------------------------*/
   /* If there is already space in the specified buffer for the new data,    */
   /* just append it to the end of the data that is already in the buffer.   */
   /*------------------------------------------------------------------------*/
   if (obj->size < obj->buffer_size)
      memcpy(((uint8_t*)obj->buf) + obj->type_size * ((obj->size)++), to_append,
               obj->type_size);

   /*------------------------------------------------------------------------*/
   /* Grow the buffer if we need more space to add the new data to it.       */
   /*------------------------------------------------------------------------*/
   else
   {
       void* old_buffer = obj->buf;
       obj->buffer_size *= 2;
       obj->buf = malloc(obj->buffer_size*obj->type_size);
       memcpy(obj->buf,old_buffer,obj->size*obj->type_size);
       free(old_buffer);
       memcpy(((uint8_t*)obj->buf) + obj->type_size *((obj->size)++), to_append,
               obj->type_size);
   }
}

/*****************************************************************************/
/* AL_SIZE() - Get the number of elements in an Array_List.                  */
/*****************************************************************************/
int32_t AL_size(Array_List* obj)
{
   return obj->size;
}

/*****************************************************************************/
/* AL_DESTROY() - Free up memory associated with an Array_List that is no    */
/*       longer in use.                                                         */
/*****************************************************************************/
void AL_destroy(Array_List* obj)
{
   free(obj->buf);
}
