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
/* dolt_debug.h                                                              */
/*                                                                           */
/* Define internal data structures used by DOLT client side for DLL debug.   */
/*****************************************************************************/
#ifndef DOLT_DEBUG_H
#define DOLT_DEBUG_H

#include "dload_api.h"
#include "Queue.h"
#include "Stack.h"

/*---------------------------------------------------------------------------*/
/* DLL Debug Support                                                         */
/*                                                                           */
/* A host copy of the DLL View debug support data structure that will be     */
/* written into target memory to assist the debugger with mapping symbol     */
/* definitions to their dynamically loaded location in target memory.        */
/*---------------------------------------------------------------------------*/
#define INIT_VERSION	2
#define VERIFICATION	0x79

/*---------------------------------------------------------------------------*/
/* DL_Debug_List_Header - Address of this structure in target memory is      */
/*	recorded in DLModules symbol.  Provides directions to first          */
/*	DL_Module_Record in the list.                                        */
/*---------------------------------------------------------------------------*/
typedef struct {
   uint32_t	first_module_ptr;
   uint16_t	first_module_size;
   uint16_t	update_flag;
} DL_Debug_List_Header;

/*---------------------------------------------------------------------------*/
/* DL_Segment - Debug information recorded about each segment in a module.   */ 
/*---------------------------------------------------------------------------*/
typedef struct {
   uint32_t	load_address;
   uint32_t	run_address;
} DL_Target_Segment;

typedef struct _DL_Host_Segment {
   uint32_t			 load_address;
   uint32_t			 run_address;
   struct _DL_Host_Segment	*next_segment;
} DL_Host_Segment;

/*---------------------------------------------------------------------------*/
/* DL_Module_Debug_Record - Debug information about each module that has     */
/*	been loaded.                                                         */
/*---------------------------------------------------------------------------*/
/* We have a host version of the debug record which is built up while the    */
/* module is being loaded, and a target version of the debug record which    */
/* is built after the load has completed and we know all of the information  */
/* that needs to be written to target memory for this module.                */
/*---------------------------------------------------------------------------*/
typedef struct {
   int			 handle;
   char			*module_name;
   TARGET_ADDRESS	 target_address;
   uint32_t		 next_module_ptr;
   uint16_t		 next_module_size;
   int			 num_segments;
   DL_Host_Segment	*segment_list_head;
   DL_Host_Segment	*segment_list_tail;
} DL_Host_Module_Debug;

typedef struct {
   uint32_t		next_module_ptr;
   uint16_t		next_module_size;
   uint16_t		tool_version;
   uint16_t		verification_word;
   uint16_t		num_segments;
   uint32_t		timestamp;
   DL_Target_Segment	segments[1];
} DL_Target_Module_Debug;

/*---------------------------------------------------------------------------*/
/* DLL Debug Support - context stack of module records.  We need to maintain */
/*	a stack of modules while creating the DLL module record list for     */
/*	debug support.  While we are building up a module record for one     */
/*	module, the loader may be asked to load dependent modules.  Note     */
/*	that we cannot emit a module record to target memory until loading   */
/*	of the module has been completed (need to know how many segments     */
/*	are in the module before we can allocate target memory for the       */
/*	module record).                                                      */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* dl_debug                                                                  */
/*                                                                           */
/* Define a LIFO linked list "class" of DL_Module_Debug_Record pointers.     */
/*---------------------------------------------------------------------------*/
TYPE_STACK_DEFINITION(DL_Host_Module_Debug*, dl_debug)
extern dl_debug_Stack	dl_debug_stk;

/*---------------------------------------------------------------------------*/
/* mirror_debug_ptr                                                          */
/*                                                                           */
/* Define a linked list "class" of DL_Host_Module_Debug pointers.            */
/*---------------------------------------------------------------------------*/
TYPE_QUEUE_DEFINITION(DL_Host_Module_Debug*, mirror_debug_ptr)
extern mirror_debug_ptr_Queue mirror_debug_list;

/*---------------------------------------------------------------------------*/
/* DLL debug support global data objects.                                    */
/*---------------------------------------------------------------------------*/
extern BOOL DLL_debug;
extern TARGET_ADDRESS DLModules_loc;

/*---------------------------------------------------------------------------*/
/* Management functions for DLL debug support data structures.               */
/*---------------------------------------------------------------------------*/
extern void	DLDBG_add_host_record(const char *module_name);
extern void	DLDBG_add_target_record(int handle);
extern void	DLDBG_rm_target_record(int handle);
extern void	DLDBG_add_segment_record(struct DLOAD_MEMORY_SEGMENT *obj_desc);
extern void	DLDBG_dump_mirror_debug_list(void);
#endif
