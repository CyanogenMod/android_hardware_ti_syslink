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
/* Queue.h                                                                   */
/*                                                                           */
/*                       Interface to Linked List                            */
/*                                                                           */
/* This is an implementation of a type-independent linked list class for C.  */
/* It's basically a template class, but uses macros instead so that it can   */
/* be compiled with a C-only compiler.                                       */
/*                                                                           */
/* To define a linked list class:                                            */
/* #include "Queue.h"                                                        */
/* TYPE_QUEUE_DEFINITION(object_type,Class_Identifier)                       */
/*                                                                           */
/* In a separate C file:                                                     */
/* #include "Queue.h"                                                        */
/* TYPE_QUEUE_DEFINITION(object_type,Class_Identifier)                       */
/* TYPE_QUEUE_IMPLEMENTATION(object_type,Class_Identifier                    */
/*                                                                           */
/* Now, to create a list:                                                    */
/* Class_Identifier_Queue name;                                              */
/* Get it initialized to zero everywhere somehow, maybe like this:           */
/* Class_Identifier_initialize_queue(&name);                                 */
/*                                                                           */
/* To add to the list:                                                       */
/* Class_Identifier_enqueue(&name, object);                                  */
/*                                                                           */
/* To iterate over the list:                                                 */
/* Class_Identifier_Queue_Node* it = name.front;                             */
/* while(it) { do_something_to_(it->value); it=it->next; }                   */
/*                                                                           */
/* To delete from the list:                                                  */
/*  If it's the first node:                                                  */
/*   Class_Identifier_dequeue(&name);                                        */
/*  If it's not:                                                             */
/*   predecessor_node->next_ptr = deleted_node->next_ptr;                    */
/*   name.size--;                                                            */
/*****************************************************************************/
#ifndef QUEUE_H
#define QUEUE_H

#include <inttypes.h>
#include "dload_api.h"

/*****************************************************************************/
/* TYPE_QUEUE_DEFINITION() - Define structure specifications for a linked    */
/*       list of t_name objects.                                              */
/*****************************************************************************/
#define TYPE_QUEUE_DEFINITION(t, t_name)                                  \
struct t_name##_Queue_Node_                                               \
{                                                                         \
     t value;                                                             \
     struct t_name##_Queue_Node_* next_ptr;                               \
};                                                                        \
typedef struct t_name##_Queue_Node_ t_name##_Queue_Node;                  \
                                                                          \
typedef struct                                                            \
{                                                                         \
     t_name##_Queue_Node* front_ptr;                                      \
     t_name##_Queue_Node* back_ptr;                                       \
     int32_t size;                                                        \
} t_name##_Queue;                                                         \
                                                                          \
extern void t_name##_initialize_queue(t_name##_Queue* queue);             \
extern void t_name##_enqueue(t_name##_Queue* queue, t to_enqueue);        \
extern t    t_name##_dequeue(t_name##_Queue* queue);                      \
extern void t_name##_remove(t_name##_Queue* queue, t to_remove);


/*****************************************************************************/
/* TYPE_QUEUE_IMPLEMENTATION() - Define member functions of new linked list  */
/*       "class" of t_name objects.                                          */
/*                                                                           */
/* <type>_initialize_queue() - clears the queue                              */
/* <type>_enqueue() - adds a <t> type object to the end of the queue         */
/* <type>_dequeue() - remove a <t> type object from the front of the queue   */
/*       and provide access to it to the caller                              */
/* <type>_remove() - find and remove a <t> type object from the queue        */
/*****************************************************************************/
#define TYPE_QUEUE_IMPLEMENTATION(t, t_name)                             \
void t_name##_initialize_queue (t_name##_Queue* queue)                   \
{                                                                        \
     queue->front_ptr = queue->back_ptr = NULL;                          \
     queue->size = 0;                                                    \
}                                                                        \
void t_name##_enqueue(t_name##_Queue* queue, t to_enqueue)               \
{                                                                        \
     queue->size++;                                                      \
                                                                         \
     if(!queue->back_ptr)                                                \
         queue->back_ptr = queue->front_ptr =                            \
              (t_name##_Queue_Node*)                                     \
                (DLIF_malloc(sizeof(t_name##_Queue_Node)));              \
     else                                                                \
     {                                                                   \
         queue->back_ptr->next_ptr =                                     \
              (t_name##_Queue_Node*)(DLIF_malloc(                        \
                                 sizeof(t_name##_Queue_Node)));          \
         queue->back_ptr = queue->back_ptr->next_ptr;                    \
     }                                                                   \
                                                                         \
     queue->back_ptr->value = to_enqueue;                                \
     queue->back_ptr->next_ptr = NULL;                                   \
}                                                                        \
                                                                         \
t t_name##_dequeue(t_name##_Queue* queue)                                \
{                                                                        \
     t to_ret;                                                           \
     t_name##_Queue_Node* next_ptr = NULL;                               \
                                                                         \
     if (!queue->size) return NULL;                                      \
                                                                         \
     next_ptr = queue->front_ptr->next_ptr;                              \
     queue->size--;                                                      \
     to_ret = queue->front_ptr->value;                                   \
     DLIF_free((void*)(queue->front_ptr));                               \
                                                                         \
     if(!queue->size)                                                    \
         queue->front_ptr = queue->back_ptr = NULL;                      \
     else                                                                \
         queue->front_ptr = next_ptr;                                    \
                                                                         \
     return to_ret;                                                      \
}                                                                        \
                                                                         \
void t_name##_remove(t_name##_Queue* queue, t to_remove)                 \
{                                                                        \
     t_name##_Queue_Node* prev_ptr = NULL;                               \
     t_name##_Queue_Node* curr_ptr = queue->front_ptr;                   \
     t_name##_Queue_Node* next_ptr = NULL;                               \
                                                                         \
     for (; curr_ptr; curr_ptr = next_ptr)                               \
     {                                                                   \
	next_ptr = curr_ptr->next_ptr;                                   \
        if (curr_ptr->value == to_remove) break;                         \
	prev_ptr = curr_ptr;                                             \
     }                                                                   \
                                                                         \
     if (curr_ptr)                                                       \
     {                                                                   \
        if (prev_ptr) prev_ptr->next_ptr = next_ptr;                     \
	queue->size--;                                                   \
        DLIF_free((void*)(curr_ptr));                                    \
     }                                                                   \
                                                                         \
     if (!queue->size)                                                   \
        queue->front_ptr = queue->back_ptr = NULL;                       \
     else                                                                \
     {                                                                   \
        if (!prev_ptr) queue->front_ptr = next_ptr;                      \
        if (!next_ptr) queue->back_ptr = prev_ptr;                       \
     }                                                                   \
}


#endif
