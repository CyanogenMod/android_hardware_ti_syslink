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
/* symtab.h                                                                  */
/*                                                                           */
/* Specification of functions used by the core loader to create, maintain,   */
/* and destroy internal symbol tables.                                       */
/*****************************************************************************/
#ifndef SYMTAB_H
#define SYMTAB_H

#include "ArrayList.h"
#include "dload.h"

/*****************************************************************************/
/* This is the top-level application file handle.  It should only be needed  */
/* under the Linux and DSBT models.                                          */ 
/*****************************************************************************/
extern int32_t DLIMP_application_handle;

/*---------------------------------------------------------------------------*/
/* Core Loader Symbol Table Management Functions                             */
/*---------------------------------------------------------------------------*/
BOOL DLSYM_canonical_lookup(int32_t sym_index,
                            DLIMP_Dynamic_Module *dyn_module, 
                            Elf32_Addr *sym_value);

BOOL DLSYM_global_lookup(const char *sym_name, 
                         DLIMP_Loaded_Module *pentry, 
                         Elf32_Addr *sym_value);

BOOL DLSYM_lookup_local_symtab(const char       *sym_name, 
                               struct Elf32_Sym *symtab, 
                               Elf32_Word        symnum,
                               Elf32_Addr       *sym_value);

void DLSYM_copy_globals(DLIMP_Dynamic_Module *dyn_module);

#endif
