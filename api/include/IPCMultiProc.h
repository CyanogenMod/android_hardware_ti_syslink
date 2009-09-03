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
 * IPCMultiProc.h
 *
 * IPC support functions for TI OMAP processors.
 *
 */

/*
 * Handles processor id management in multi processor systems.Used
 * by all modules which need processor ids for their oprations.
 *
 */

#ifndef IPCMULTIPROC_H_
#define IPCMULTIPROC_H_

#include <linux/types.h>
/* Standard headers */
#include <ipc_ioctl.h>
#include <Std.h>

#define FAIL -1

enum CMD_MULTIPROC {
	MULTIPROC_GETID = MULTIPROC_BASE_CMD,
	MULTIPROC_GETNAME,
	MULTIPROC_GETMAXID,
};

/*
 *  Command for multiproc_get_id
 */
#define CMD_MULTIPROC_GETID	_IOWR(IPC_IOC_MAGIC, MULTIPROC_GETID,          \
				struct multiproc_cmd_args)

/*
 *  Command for multiproc_get_name
 */
#define CMD_MULTIPROC_GETNAME	_IOWR(IPC_IOC_MAGIC, MULTIPROC_GETNAME,        \
				struct multiproc_cmd_args)

/*
 *  Command for multiproc_get_max_processors
 */
#define CMD_MULTIPROC_GETMAXID _IOWR(IPC_IOC_MAGIC, MULTIPROC_GETMAXID,        \
				struct multiproc_cmd_args)

/*
 *  Command arguments for multiproc
 */
union multiproc_arg {
	struct {
		UInt16 *proc_id;
		Char *name;
		UInt32 name_len;
	} get_id;

	struct {
		UInt16 proc_id;
		Char *name;
	} get_name;

	struct {
		UInt16 *max_id;
	} get_max_id;
};

/*
 *  Command arguments for multiproc
 */
struct multiproc_cmd_args {
	union multiproc_arg cmd_arg;
	Int32 api_status;
};

/*!
 *  MULTIPROC_MODULEID
 *  Unique module ID.
 */
#define MULTIPROC_MODULEID      (UInt16) 0xB522

/*
 *  MULTIPROC_STATUSCODEBASE
 *  Error code base for MultiProc module.
 */
#define MULTIPROC_STATUSCODEBASE  (MULTIPROC_MODULEID << 12u)

/*
 *  MULTIPROC_MAKE_FAILURE
 *  Macro to make failure code.
 */
#define MULTIPROC_MAKE_FAILURE(x) ((Int) (  0x80000000                         \
                                          + MULTIPROC_STATUSCODEBASE           \
                                          + (x)))

/*
 *  MULTIPROC_MAKE_SUCCESS
 *  Macro to make success code.
 */
#define MULTIPROC_MAKE_SUCCESS(x) (MULTIPROC_STATUSCODEBASE + (x))

/*
 *  MULTIPROC_E_INVALIDID
 *  Macro to EINVALIDID code.
 */

#define MULTIPROC_E_INVALIDID   MULTIPROC_MAKE_FAILURE(1)

/*
 *  MULTIPROC_EINVALIDNAME
 *  Macro to EINVALIDNAME code.
 */
#define MULTIPROC_E_INVALIDNAME MULTIPROC_MAKE_FAILURE(2)

/*
 *  MultiProc_INVALIDID
 *  Macro to define invalid processor id.
 */
#define MultiProc_INVALIDID (UInt16)0xFFFF

/*
 * MULTIPROC_SUCCESS
 * Operation was successful.
 */
#define MULTIPROC_SUCCESS           0


/* Function to get processor Id given proc name*/
UInt16 MultiProc_getId(String name);
/* Function to get processor name give proc Id */
String MultiProc_getName(UInt16 id);
/* Function to get maximum available proc Id */
UInt16 MultiProc_getMaxProcessors();


#endif /* IPCMULTIPROC_H_ */

