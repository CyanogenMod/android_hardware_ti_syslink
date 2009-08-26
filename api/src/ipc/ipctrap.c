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
 * ipctrap.c
 *
 * IPC support functions for TI OMAP processors.
 *
 */


/*
 *  ======== ipctrap.c ========
 *  Description:
 *      Source for trap hand-shaking
 */

/*  ----------------------------------- Host OS */
#include <host_os.h>
#include <ipctrap.h>
#include <Trace.h>

/*  ----------------------------------- Globals */
extern int ipc_handle;		/* ipc driver handle */

/*
 * ======== DSPTRAP_Trap ========
 */
int IPCTRAP_Trap(IPC_Trapped_Args *args, int cmd)
{
	int status = FAIL;

	if (ipc_handle >= 0)
		status = ioctl(ipc_handle, cmd, args);
	else
		/* To link against applications for testing with out kernel module support */
		return 0;			
		GT_0trace(curTrace, GT_LEAVE, "Invalid handle to driver!\n");

	return status;
}

