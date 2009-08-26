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
/* arm_dynamic.h                                                             */
/*                                                                           */
/*   Interface into ARM specific dynamic loader functionality.               */
/*****************************************************************************/
#ifndef DLOAD_ARM_H
#define DLOAD_ARM_H

#include "dload.h"

BOOL DLDYN_arm_process_dynamic_tag(DLIMP_Dynamic_Module* dyn_module, int i);
BOOL DLDYN_arm_relocate_dynamic_tag_info(DLIMP_Dynamic_Module *dyn_module, 
                                         int32_t i);
BOOL DLDYN_arm_process_eiosabi(DLIMP_Dynamic_Module* dyn_module);

#endif
