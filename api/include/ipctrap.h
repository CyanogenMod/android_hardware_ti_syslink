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
/*
 * ipctrap.h
 *
 * IPC support functions for TI OMAP processors.
 */

/*
 *  ======== dsptrap.h ========
 *  Purpose:
 *      Handles interaction between user and driver layers.
 */

#ifndef IPCTRAP_
#define IPCTRAP_

#include <ipcioctl.h>

/* Function Prototypes */
extern int IPCTRAP_Trap(IPC_Trapped_Args *args, int cmd);

#endif	/* IPCTRAP_ */

