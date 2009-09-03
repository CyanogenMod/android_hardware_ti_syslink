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
/* arm_dynamic.c                                                             */
/*                                                                           */
/* ARM specific dynamic loader functionality.                                */
/*****************************************************************************/

#ifdef ARM_TARGET

#include "arm_elf32.h"
#include "dload.h"

/*****************************************************************************/
/* process_arm_dynamic_tag()                                                 */
/*                                                                           */
/* Process ARM specific dynamic tags                                         */
/*****************************************************************************/  
BOOL DLDYN_arm_process_dynamic_tag(DLIMP_Dynamic_Module* dyn_module, int i)
{
   switch (dyn_module->dyntab[i].d_tag)
   {
      case DT_ARM_SYMTABSZ:
      {
         dyn_module->symnum = dyn_module->dyntab[i].d_un.d_val;
#if LOADER_DEBUG
         if (debugging_on)
            printf("Found symbol table size: %d\n",
                   dyn_module->symnum);
#endif
         return TRUE;
      }
   }

   return FALSE;
}

/*****************************************************************************/
/* DLDYN_arm_relocate_dynamic_tag_info()                                     */
/*                                                                           */
/*    Update any target specific dynamic tag values that are associated with */
/*    a section address. Return TRUE if the tag value is successfully        */
/*    updated or if the tag is not associated with a section address, and    */
/*    FALSE if we can't find the sectoin associated with the tag or if the   */
/*    tag type is not recognized.                                            */
/*                                                                           */
/*****************************************************************************/
BOOL DLDYN_arm_relocate_dynamic_tag_info(DLIMP_Dynamic_Module *dyn_module, 
                                         int32_t i)
{
   switch (dyn_module->dyntab[i].d_tag)
   {
      /*---------------------------------------------------------------------*/
      /* These tags do not point to sections.                                */
      /*---------------------------------------------------------------------*/
      case DT_ARM_RESERVED1:
      case DT_ARM_SYMTABSZ:
      case DT_ARM_PREEMPTMAP:
      case DT_ARM_RESERVED2:
         return TRUE;
   }

   DLIF_error(DLET_MISC, "Invalid dynamic tag encountered, %d\n", 
                         (int)dyn_module->dyntab[i].d_tag);
   return FALSE;
}

/*****************************************************************************/
/* arm_process_eiosabi()                                                     */
/*                                                                           */
/*   Process the EI_OSABI value. Verify that the OSABI is supported and set  */
/*   any variables which depend on the OSABI.                                */
/*****************************************************************************/
BOOL DLDYN_arm_process_eiosabi(DLIMP_Dynamic_Module* dyn_module)
{
   /*------------------------------------------------------------------------*/
   /* For ARM, the EI_OSABI value must not be set.  The only ARM specific    */
   /* value is ELFOSABI_ARM_AEABI is for objects which contain symbol        */
   /* versioning, which we do not support for Baremetal ABI.                 */
   /*------------------------------------------------------------------------*/
   if (dyn_module->fhdr.e_ident[EI_OSABI] != ELFOSABI_NONE)
      return FALSE;

   return TRUE;
}

#endif /* ARM_TARGET */
