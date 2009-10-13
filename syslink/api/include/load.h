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
#include "Std.h"
#include "dload_api.h"

extern Bool DLL_debug;
extern TARGET_ADDRESS DLModules_loc;
/*---------------------------------------------------------------------------*/
/* Management functions for DLL debug support data structures.               */
/*---------------------------------------------------------------------------*/
extern void DLDBG_add_host_record(const char *module_name);
extern void DLDBG_add_target_record(int handle);
extern void DLDBG_rm_target_record(int handle);
extern void DLDBG_add_segment_record(struct DLOAD_MEMORY_SEGMENT *obj_desc);
extern void DLDBG_dump_mirror_debug_list(void);
UInt32 load_executable(const char* file_name, int argc, char** argv);
