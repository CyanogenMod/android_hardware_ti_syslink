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
 * ipcioctl.h
 *
 * IPC IOCTL commands and data objects to be exchanged b/w api 
 * and kernel ipc layer
 */


/*
*  ======== ipc_ioctl.h ========
*  Purpose:
*      Contains structures and commands that are used for interaction
*      between the IPC API layer and IPC kernel layer.
*/


#ifndef IPC_IOCTL_
#define IPC_IOCTL_

#include <Std.h>
#include <NameServer.h>

typedef union {

	/* MultiProc Module */
	struct {
		UInt16 proc_id;
	} ARGS_MPROC_SET_ID;

	struct {
		UInt16 *proc_id;		
		char *name;
	} ARGS_MPROC_GET_ID;

	struct {
		UInt16 proc_id;
		char *name;
	} ARGS_MPROC_GET_NAME;

	struct {
		UInt16 *max_id;
	} ARGS_MPROC_GET_MAX_ID;

	/* Nameserver Module */

	struct {
		NameServer_Handle handle;
		NameServer_CreateParams *params;
	} ARGS_NMSVR_GET_PARAM;

	struct {
		String name;
		NameServer_CreateParams *params;
		NameServer_Handle *handle;
	} ARGS_NMSVR_CREATE;

	struct {
		NameServer_Handle *handle;
	} ARGS_NMSVR_DELETE;

	struct {
		NameServer_Handle handle;
		String name;
		Ptr buf;
		UInt len;
	} ARGS_NMSVR_ADD;

	struct {
		NameServer_Handle handle;
		String name;
		UInt32 value;
	} ARGS_NMSVR_ADD_U32;

	struct {
		NameServer_Handle handle;
		String name;
	} ARGS_NMSVR_REMOVE;

	struct {
		NameServer_Handle handle;
		String name;
		Ptr buf;
		UInt32 len;
		UInt16 *procId;		
	} ARGS_NMSVR_GET;

	struct {
		NameServer_Handle handle;
		String name;
		Ptr buf;
		UInt32 len;
	
	} ARGS_GET_LOCAL;

	struct {
		String name;
		NameServer_Handle *handle;
	} ARGS_NMSVR_GET_HANDLE;

	struct {
		NameServerRemote_Handle handle;
		UInt16 proc_id;
	} ARGS_NMSVR_REG_REMOTE_DRV;

	struct {
		UInt16 proc_id;
	} ARGS_NMSVR_UNREG_REMOTE_DRV;
} IPC_Trapped_Args;

#define IPC_CMD_BASE                      0

/* multiproc module offsets */
#define CMD_MPROC_BASE_OFFSET             IPC_CMD_BASE
#define CMD_MPROC_SET_ID_OFFSET           (CMD_MPROC_BASE_OFFSET + 0)
#define CMD_MPROC_GET_ID_OFFSET           (CMD_MPROC_BASE_OFFSET + 1)
#define CMD_MPROC_GET_NAME_OFFSET         (CMD_MPROC_BASE_OFFSET + 2)
#define CMD_MPROC_GET_MAX_ID_OFFSET       (CMD_MPROC_BASE_OFFSET + 3)
#define CMD_MPROC_END_OFFSET              CMD_MPROC_GET_MAX_ID_OFFSET

#define CMD_NMSVR_BASE_OFFSET             (CMD_MPROC_END_OFFSET + 1)
#define CMD_NMSVR_GET_PARAM_OFFSET        (CMD_NMSVR_BASE_OFFSET + 0)
#define CMD_NMSVR_GET_HANDLE_OFFSET       (CMD_NMSVR_BASE_OFFSET + 1)
#define CMD_NMSVR_CREATE_OFFSET           (CMD_NMSVR_BASE_OFFSET + 2)
#define CMD_NMSVR_DELETE_OFFSET           (CMD_NMSVR_BASE_OFFSET + 3)
#define CMD_NMSVR_ADD_OFFSET              (CMD_NMSVR_BASE_OFFSET + 4)
#define CMD_NMSVR_ADD_U32_OFFSET          (CMD_NMSVR_BASE_OFFSET + 5)
#define CMD_NMSVR_REMOVE_OFFSET           (CMD_NMSVR_BASE_OFFSET + 6)
#define CMD_NMSVR_GET_OFFSET              (CMD_NMSVR_BASE_OFFSET + 7)
#define CMD_NMSVR_GET_LOCAL_OFFSET        (CMD_NMSVR_BASE_OFFSET + 8)
#define CMD_NMSVR_REG_REMOTE_DRV_OFFSET   (CMD_NMSVR_BASE_OFFSET + 9)
#define CMD_NMSVR_UNREG_REMOTE_DRV_OFFSET (CMD_NMSVR_BASE_OFFSET + 9)
#define CMD_NMSVR_END_OFFSET              CMD_NMSVR_UNREG_REMOTE_DRV_OFFSET

#endif /* IPC_IOCTL_ */

