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
/* endian.h                                                                  */
/*                                                                           */
/* Specification of functions used to assist loader with endian-ness issues. */
/*****************************************************************************/
#ifndef DLOAD_ENDIAN_H
#define DLOAD_ENDIAN_H

#include "elf32.h"

/*---------------------------------------------------------------------------*/
/* Prototypes for ELF file object reader endianness swap routines.           */
/*---------------------------------------------------------------------------*/

int     DLIMP_get_endian();
void    DLIMP_change_endian32(int32_t* to_change);
void    DLIMP_change_endian16(int16_t* to_change);
void    DLIMP_change_ehdr_endian(struct Elf32_Ehdr* to_change);
void    DLIMP_change_phdr_endian(struct Elf32_Phdr* to_change);
void    DLIMP_change_dynent_endian(struct Elf32_Dyn* to_change);
void    DLIMP_change_sym_endian(struct Elf32_Sym* to_change);
void    DLIMP_change_rela_endian(struct Elf32_Rela* to_change);
void    DLIMP_change_rel_endian(struct Elf32_Rel* to_change);

#endif
