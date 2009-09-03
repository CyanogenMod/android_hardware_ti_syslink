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
/*!
 *  @file       NameServerDrvDefs.h
 *
 *  @brief      Definitions of NameServerDrv types and structures.
 *
 */


#ifndef NAMESERVER_DRVDEFS_H_0xf2ba
#define NAMESERVER_DRVDEFS_H_0xf2ba


/* Standard headers */
#include <ipc_ioctl.h>
#include <Std.h>

/* Utilities headers */
#include <NameServer.h>


#if defined (__cplusplus)
extern "C" {
#endif

enum CMD_NAMESERVER {
	NAMESERVER_SETUP = NAMESERVER_BASE_CMD,
	NAMESERVER_DESTROY,
	NAMESERVER_PARAMS_INIT,
	NAMESERVER_CREATE,
	NAMESERVER_DELETE,
	NAMESERVER_ADD,
	NAMESERVER_ADDUINT32,
	NAMESERVER_GET,
	NAMESERVER_GETLOCAL,
	NAMESERVER_MATCH,
	NAMESERVER_REMOVE,
	NAMESERVER_REMOVEENTRY,
	NAMESERVER_GETHANDLE
};

/*
 *  IOCTL command IDs for nameserver
 *
 */
/*
 *  Command for nameserver_setup
 */
#define CMD_NAMESERVER_SETUP		_IOWR(IPC_IOC_MAGIC, NAMESERVER_SETUP, \
					struct NameServerDrv_CmdArgs)

/*
 *  Command for nameserver_destroy
 */
#define CMD_NAMESERVER_DESTROY		_IOWR(IPC_IOC_MAGIC,                   \
					NAMESERVER_DESTROY,                    \
					struct NameServerDrv_CmdArgs)

/*
 *  Command for nameserver_params_init
 */
#define CMD_NAMESERVER_PARAMS_INIT	_IOWR(IPC_IOC_MAGIC,		       \
					NAMESERVER_PARAMS_INIT,		       \
					struct NameServerDrv_CmdArgs)

/*
 *  Command for nameserver_create
 */
#define CMD_NAMESERVER_CREATE		_IOWR(IPC_IOC_MAGIC,                   \
					NAMESERVER_CREATE,                     \
					struct NameServerDrv_CmdArgs)

/*
 *  Command for nameserver_delete
 */
#define CMD_NAMESERVER_DELETE		_IOWR(IPC_IOC_MAGIC,                   \
					NAMESERVER_DELETE,                     \
					struct NameServerDrv_CmdArgs)

/*
 *  Command for nameserver_add
 */
#define CMD_NAMESERVER_ADD		_IOWR(IPC_IOC_MAGIC, NAMESERVER_ADD,   \
					struct NameServerDrv_CmdArgs)

/*
 *  Command for nameserver_addu32
 */
#define CMD_NAMESERVER_ADDUINT32	_IOWR(IPC_IOC_MAGIC,		       \
					NAMESERVER_ADDUINT32,		       \
					struct NameServerDrv_CmdArgs)
/*
 *  Command for nameserver_get
 */
#define CMD_NAMESERVER_GET		_IOWR(IPC_IOC_MAGIC, NAMESERVER_GET,   \
					struct NameServerDrv_CmdArgs)

/*
 *  Command for nameserver_get_local
 */
#define CMD_NAMESERVER_GETLOCAL		_IOWR(IPC_IOC_MAGIC,		       \
					NAMESERVER_GETLOCAL,		       \
					struct NameServerDrv_CmdArgs)

/*
 *  Command for nameserver_match
 */
#define CMD_NAMESERVER_MATCH		_IOWR(IPC_IOC_MAGIC, NAMESERVER_MATCH, \
					struct NameServerDrv_CmdArgs)

/*
 *  Command for nameserver_remove
 */
#define CMD_NAMESERVER_REMOVE		_IOWR(IPC_IOC_MAGIC, NAMESERVER_REMOVE,\
					struct NameServerDrv_CmdArgs)

/*
 *  Command for nameserver_remove_entry
 */
#define CMD_NAMESERVER_REMOVEENTRY 	_IOWR(IPC_IOC_MAGIC,		       \
					NAMESERVER_REMOVEENTRY,		       \
					struct NameServerDrv_CmdArgs)

/*
 *  Command for nameserver_get_handle
 */
#define CMD_NAMESERVER_GETHANDLE	_IOWR(IPC_IOC_MAGIC,		       \
					NAMESERVER_GETHANDLE,		       \
					struct NameServerDrv_CmdArgs)


/*  ----------------------------------------------------------------------------
 *  Command arguments for NameServer
 *  ----------------------------------------------------------------------------
 */
/*!
 *  @brief  Command arguments for NameServer
 */
typedef struct NameServerDrv_CmdArgs {
    union {
        struct {
            NameServer_Params * params;
        } ParamsInit;

        struct {
            NameServer_Handle   handle;
            String              name;
            UInt32              nameLen;
            NameServer_Params * params;
        } create;

        struct {
            NameServer_Object * obj;
            String              name;
            NameServer_Params * params;
        } construct;

        struct {
            NameServer_Handle   handle;
        } delete;

        struct {
            NameServer_Struct * object;
        } destruct;

        struct {
            NameServer_Handle   handle;
            String              name;
            UInt32              nameLen;
            Ptr                 buf;
            UInt                len;
            Ptr                 entry;
            NameServer_Entry *  node;
        } add;

        struct {
            NameServer_Handle   handle;
            String              name;
            UInt32              nameLen;
            UInt32              value;
            Ptr                 entry;
        } addUInt32;

        struct {
            NameServer_Handle   handle;
            String              name;
            UInt32              nameLen;
            Ptr                 buf;
            UInt32              len;
            UInt16 *            procId;
            UInt32              procLen;
            UInt32              count;
        } get;

        struct {
            NameServer_Handle   handle;
            String              name;
            UInt32              nameLen;
            Ptr                 buf;
            UInt32              len;
            UInt32              count;
        } getLocal;

        struct {
            NameServer_Handle   handle;
            String              name;
            UInt32              nameLen;
            UInt32            * value;
            UInt32              count;
        } match;

        struct {
            NameServer_Handle   handle;
            String              name;
            UInt32              nameLen;
        } remove;

        struct {
            NameServer_Handle   handle;
            Ptr                 entry;
        } removeEntry;

        struct {
            NameServer_Handle   handle;
            String              name;
            UInt32              nameLen;
        } getHandle;

    } args;

    Int32 apiStatus;
} NameServerDrv_CmdArgs;


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* NameServer_DrvDefs_H_0xf2ba */
