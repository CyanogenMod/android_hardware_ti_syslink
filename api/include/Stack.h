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
/* Stack.h                                                                   */
/*                                                                           */
/*                       Interface to Stack                                  */
/*                                                                           */
/* This is an implementation of a type-independent stack implemented as      */
/* a signly linked list class for C.  It's basically a template class, but   */
/* uses macros instead so that it can be compiled with a C-only compiler.    */
/*                                                                           */
/* To define a Stack class:                                                  */
/* #include "Stack.h"                                                        */
/* TYPE_STACK_DEFINITION(object_type,Class_Identifier)                       */
/*                                                                           */
/* In a separate C file:                                                     */
/* #include "Stack.h"                                                        */
/* TYPE_STACK_DEFINITION(object_type,Class_Identifier)                       */
/* TYPE_STACK_IMPLEMENTATION(object_type,Class_Identifier                    */
/*                                                                           */
/* Now, to create a stack:                                                   */
/* struct Class_Identifier_Stack name;                                       */
/* Get it initialized to zero everywhere somehow, maybe like this:           */
/* initialize_stack_Class_Identifier(&name);                                 */
/*                                                                           */
/* To add to the stack:                                                      */
/* push_Class_Identifier(&name, object);                                     */
/*                                                                           */
/* To access the top of the stack:                                           */
/* Class_Identifier_Stack_Node* tos = name.top_ptr;                          */
/* do_something_to_(tos->value);                                             */
/*                                                                           */
/* To delete from the stack:                                                 */
/*   if (name.size > 0) pop_Class_Identifier(&name);                         */
/*                                                                           */
/*****************************************************************************/
#ifndef STACK_H
#define STACK_H

#include <inttypes.h>
#include "dload_api.h"

/*****************************************************************************/
/* TYPE_STACK_DEFINITION() - Define structure specifications for a last-in,  */
/*	first-out linked list of t_name objects.                             */
/*****************************************************************************/
#define TYPE_STACK_DEFINITION(t, t_name)                                      \
struct t_name##_Stack_Node_                                                   \
{                                                                             \
     t value;                                                                 \
     struct t_name##_Stack_Node_* next_ptr;                                   \
};                                                                            \
typedef struct t_name##_Stack_Node_ t_name##_Stack_Node;                      \
                                                                              \
typedef struct                                                                \
{                                                                             \
     t_name##_Stack_Node* top_ptr;                                            \
     t_name##_Stack_Node* bottom_ptr;                                         \
     int size;                                                                \
} t_name##_Stack;                                                             \
                                                                              \
extern void t_name##_initialize_stack(t_name##_Stack* stack);                 \
extern void t_name##_push(t_name##_Stack* stack, t to_push);                  \
extern t    t_name##_pop(t_name##_Stack* stack);

/*****************************************************************************/
/* TYPE_STACK_IMPLEMENTATION() - Define member functions of new LIFO linked  */
/*	list "class" of t_name objects.                                      */
/*                                                                           */
/* <type>_initialize_stack() - clears the stack                              */
/* <type>_push() - pushes a <t> type object to the top of the stack          */
/* <type>_pop() - pop a <t> type object from the top of the stack            */
/*	and provide access to it to the caller                               */
/*****************************************************************************/
#define TYPE_STACK_IMPLEMENTATION(t, t_name)                                  \
void t_name##_initialize_stack (t_name##_Stack* stack)                        \
{                                                                             \
     stack->top_ptr = stack->bottom_ptr = NULL;                               \
     stack->size = 0;                                                         \
}                                                                             \
void t_name##_push(t_name##_Stack* stack, t to_push)                          \
{                                                                             \
     stack->size++;                                                           \
                                                                              \
     if(!stack->top_ptr)                                                      \
     {                                                                        \
	  stack->bottom_ptr = stack->top_ptr =                                \
	    (t_name##_Stack_Node*)(DLIF_malloc(sizeof(t_name##_Stack_Node))); \
          stack->top_ptr->next_ptr = NULL;                                    \
     }                                                                        \
     else                                                                     \
     {                                                                        \
          t_name##_Stack_Node* next_ptr = stack->top_ptr;                     \
	  stack->top_ptr =                                                    \
	    (t_name##_Stack_Node*)(DLIF_malloc(sizeof(t_name##_Stack_Node))); \
	  stack->top_ptr->next_ptr = next_ptr;                                \
     }                                                                        \
                                                                              \
     stack->top_ptr->value = to_push;                                         \
}                                                                             \
                                                                              \
t t_name##_pop(t_name##_Stack* stack)                                         \
{                                                                             \
     t to_ret;                                                                \
     t_name##_Stack_Node* next_ptr = stack->top_ptr->next_ptr;                \
                                                                              \
     stack->size--;                                                           \
     to_ret = stack->top_ptr->value;                                          \
     DLIF_free((void*)(stack->top_ptr));                                      \
                                                                              \
     if(!stack->size)                                                         \
	  stack->top_ptr = stack->bottom_ptr = NULL;                          \
     else                                                                     \
	  stack->top_ptr = next_ptr;                                          \
                                                                              \
     return to_ret;                                                           \
}

#endif
