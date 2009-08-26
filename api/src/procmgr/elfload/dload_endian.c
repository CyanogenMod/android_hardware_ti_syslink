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
/* dload_endian.c                                                            */
/*                                                                           */
/* Simple helper functions to assist core loader with endian-ness issues     */
/* when the host endian-ness may be opposite the endian-ness of the target.  */
/*****************************************************************************/
#include "dload_endian.h"

/*****************************************************************************/
/* DLIMP_GET_ENDIAN() - Determine endianness of the host.  Uses ELF          */
/*      endianness constants.                                                */
/*****************************************************************************/
int DLIMP_get_endian()
{
   int32_t x = 0x1;

   if (*((int16_t*)(&x))) return ELFDATA2LSB;

   return ELFDATA2MSB;
}

/*****************************************************************************/
/* DLIMP_CHANGE_ENDIAN32() - Swap endianness of a 32-bit integer.            */
/*****************************************************************************/
void DLIMP_change_endian32(int32_t* to_change)
{
   int32_t temp = 0;
   temp += (*to_change & 0x000000FF) << 24;
   temp += (*to_change & 0x0000FF00) << 8;
   temp += (*to_change & 0x00FF0000) >> 8;
   temp += (*to_change & 0xFF000000) >> 24;
   *to_change = temp;
}

/*****************************************************************************/
/* DLIMP_CHANGE_ENDIAN16() - Swap endianness of a 16-bit integer.            */
/*****************************************************************************/
void DLIMP_change_endian16(int16_t* to_change)
{
   int16_t temp = 0;
   temp += (*to_change & 0x00FF) << 8;
   temp += (*to_change & 0xFF00) >> 8;
   *to_change = temp;
}

/*****************************************************************************/
/* DLIMP_CHANGE_EHDR_ENDIAN() - Swap endianness of an ELF file header.       */
/*****************************************************************************/
void DLIMP_change_ehdr_endian(struct Elf32_Ehdr* ehdr)
{
   DLIMP_change_endian16((int16_t*)(&ehdr->e_type));
   DLIMP_change_endian16((int16_t*)(&ehdr->e_machine));
   DLIMP_change_endian32((int32_t*)(&ehdr->e_version));
   DLIMP_change_endian32((int32_t*)(&ehdr->e_entry));
   DLIMP_change_endian32((int32_t*)(&ehdr->e_phoff));
   DLIMP_change_endian32((int32_t*)(&ehdr->e_shoff));
   DLIMP_change_endian32((int32_t*)(&ehdr->e_flags));
   DLIMP_change_endian16((int16_t*)(&ehdr->e_ehsize));
   DLIMP_change_endian16((int16_t*)(&ehdr->e_phentsize));
   DLIMP_change_endian16((int16_t*)(&ehdr->e_phnum));
   DLIMP_change_endian16((int16_t*)(&ehdr->e_shentsize));
   DLIMP_change_endian16((int16_t*)(&ehdr->e_shnum));
   DLIMP_change_endian16((int16_t*)(&ehdr->e_shstrndx));
}

/*****************************************************************************/
/* DLIMP_CHANGE_PHDR_ENDIAN() - Swap endianness of an ELF program header.    */
/*****************************************************************************/
void DLIMP_change_phdr_endian(struct Elf32_Phdr* phdr)
{
   DLIMP_change_endian32((int32_t*)(&phdr->p_type));
   DLIMP_change_endian32((int32_t*)(&phdr->p_offset));
   DLIMP_change_endian32((int32_t*)(&phdr->p_vaddr));
   DLIMP_change_endian32((int32_t*)(&phdr->p_paddr));
   DLIMP_change_endian32((int32_t*)(&phdr->p_filesz));
   DLIMP_change_endian32((int32_t*)(&phdr->p_memsz));
   DLIMP_change_endian32((int32_t*)(&phdr->p_flags));
   DLIMP_change_endian32((int32_t*)(&phdr->p_align));
}

/*****************************************************************************/
/* DLIMP_CHANGE_DYNENT_ENDIAN() - Swap endianness of a dynamic table entry.  */
/*****************************************************************************/
void DLIMP_change_dynent_endian(struct Elf32_Dyn* dyn)
{
   DLIMP_change_endian32((int32_t*)(&dyn->d_tag));
   DLIMP_change_endian32((int32_t*)(&dyn->d_un.d_val));
}

/*****************************************************************************/
/* DLIMP_CHANGE_SYM_ENDIAN() - Swap endianness of an ELF symbol table entry. */
/*****************************************************************************/
void DLIMP_change_sym_endian(struct Elf32_Sym* sym)
{
   DLIMP_change_endian32((int32_t*)(&sym->st_name));
   DLIMP_change_endian32((int32_t*)(&sym->st_value));
   DLIMP_change_endian32((int32_t*)(&sym->st_size));
   DLIMP_change_endian16((int16_t*)(&sym->st_shndx));
}

/*****************************************************************************/
/* DLIMP_CHANGE_RELA_ENDIAN() - Swap endianness of a RELA-type relocation.   */
/*****************************************************************************/
void DLIMP_change_rela_endian(struct Elf32_Rela* ra)
{
   DLIMP_change_endian32((int32_t*)(&ra->r_offset));
   DLIMP_change_endian32((int32_t*)(&ra->r_info));
   DLIMP_change_endian32((int32_t*)(&ra->r_addend));
}

/*****************************************************************************/
/* DLIMP_CHANGE_REL_ENDIAN() - Swap endianness of a REL-type relocation.     */
/*****************************************************************************/
void DLIMP_change_rel_endian(struct Elf32_Rel* r)
{
   DLIMP_change_endian32((int32_t*)(&r->r_offset));
   DLIMP_change_endian32((int32_t*)(&r->r_info));
}
