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
/* relocate.h                                                                */
/*                                                                           */
/* Declare names and IDs of all C6x relocation types supported in the        */
/* dynamic loader.  Note that this list must be kept in synch with the       */
/* C6x relocation engine files in the other object development tools.        */
/*****************************************************************************/
#ifndef RELOCATE_H
#define RELOCATE_H

#include <inttypes.h>
#include "elf32.h"
#include "dload.h"
#include "dload_api.h"

/*---------------------------------------------------------------------------*/
/* Declare some globals that are used for internal debugging and profiling.  */
/*---------------------------------------------------------------------------*/
#if LOADER_DEBUG || LOADER_PROFILE
#include <time.h>
extern int DLREL_relocations;
extern time_t DLREL_total_reloc_time;
#endif


/*---------------------------------------------------------------------------*/
/* Landing point for core loader's relocation processor.                     */
/*---------------------------------------------------------------------------*/
void DLREL_relocate(LOADER_FILE_DESC *fd, DLIMP_Dynamic_Module *dyn_module);

#endif
