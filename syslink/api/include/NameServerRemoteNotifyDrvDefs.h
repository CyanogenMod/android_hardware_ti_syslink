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
 *  @file       NameServerRemoteNotifyDrvDefs.h
 *
 *  @brief      Definitions of NameServerRemoteNotifyDrv types and structures.
 *
 */


#ifndef NAMESERVERREMOTENOTIFY_DRVDEFS_H_0X5711
#define NAMESERVERREMOTENOTIFY_DRVDEFS_H_0X5711


/* Standard headers */
#include <ipc_ioctl.h>
#include <Std.h>

/* Utilities headers */
#include <NameServerRemoteNotify.h>


#if defined (__cplusplus)
extern "C" {
#endif

enum CMD_NAMESERVERREMOTENOTIFY {
	NAMESERVERREMOTENOTIFY_GETCONFIG = NAMESERVERREMOTENOTIFY_BASE_CMD,
	NAMESERVERREMOTENOTIFY_SETUP,
	NAMESERVERREMOTENOTIFY_DESTROY,
	NAMESERVERREMOTENOTIFY_PARAMS_INIT,
	NAMESERVERREMOTENOTIFY_CREATE,
	NAMESERVERREMOTENOTIFY_DELETE,
	NAMESERVERREMOTENOTIFY_GET,
	NAMESERVERREMOTENOTIFY_SHAREDMEMREQ
};


/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*  ----------------------------------------------------------------------------
 *  IOCTL command IDs for NameServerRemoteNotify
 *  ----------------------------------------------------------------------------
 */

/*
 *  IOCTL command IDs for nameserver_remotenotify
 *
 */

/*
 *  Command for nameserver_remotenotify_get_config
 */
#define CMD_NAMESERVERREMOTENOTIFY_GETCONFIG 	_IOWR(IPC_IOC_MAGIC,	       \
					NAMESERVERREMOTENOTIFY_GETCONFIG,      \
					struct NameServerRemoteNotifyDrv_CmdArgs)
/*
 *  Command for nameserver_remotenotify_setup
 */
#define CMD_NAMESERVERREMOTENOTIFY_SETUP	_IOWR(IPC_IOC_MAGIC,	       \
					NAMESERVERREMOTENOTIFY_SETUP,	       \
					struct NameServerRemoteNotifyDrv_CmdArgs)

/*
 *  Command for nameserver_remotenotify_setup
 */
#define CMD_NAMESERVERREMOTENOTIFY_DESTROY	_IOWR(IPC_IOC_MAGIC,	       \
					NAMESERVERREMOTENOTIFY_DESTROY,	       \
					struct NameServerRemoteNotifyDrv_CmdArgs)

/*
 *  Command for nameserver_remotenotify_destroy
 */
#define CMD_NAMESERVERREMOTENOTIFY_PARAMS_INIT	_IOWR(IPC_IOC_MAGIC,	       \
					NAMESERVERREMOTENOTIFY_PARAMS_INIT,    \
					struct NameServerRemoteNotifyDrv_CmdArgs)

/*
 * Command for nameserver_remotenotify_create
 */
#define CMD_NAMESERVERREMOTENOTIFY_CREATE	_IOWR(IPC_IOC_MAGIC,	       \
					NAMESERVERREMOTENOTIFY_CREATE,	       \
					struct NameServerRemoteNotifyDrv_CmdArgs)

/*
 *  Command for nameserver_remotenotify_delete
 */
#define CMD_NAMESERVERREMOTENOTIFY_DELETE	_IOWR(IPC_IOC_MAGIC,	       \
					NAMESERVERREMOTENOTIFY_DELETE,	       \
					struct NameServerRemoteNotifyDrv_CmdArgs)

/*
 *  Command for nameserver_remotenotify_get
 */
#define CMD_NAMESERVERREMOTENOTIFY_GET 	_IOWR(IPC_IOC_MAGIC,		       \
					NAMESERVERREMOTENOTIFY_GET,	       \
					struct NameServerRemoteNotifyDrv_CmdArgs)

/*
 *  Command for nameserver_remotenotify_shared_memreq
 */
#define CMD_NAMESERVERREMOTENOTIFY_SHAREDMEMREQ	_IOWR(IPC_IOC_MAGIC,	       \
					NAMESERVERREMOTENOTIFY_SHAREDMEMREQ,   \
					struct NameServerRemoteNotifyDrv_CmdArgs)


/*  ----------------------------------------------------------------------------
 *  Command arguments for NameServerRemoteNotify
 *  ----------------------------------------------------------------------------
 */
/*!
 *  @brief  Command arguments for NameServerRemoteNotify
 */
typedef struct NameServerRemoteNotifyDrv_CmdArgs {
    union {
        struct {
            NameServerRemoteNotify_Config * config;
        } getConfig;

        struct {
            NameServerRemoteNotify_Config * config;
        } setup;

        struct {
            Ptr                             handle;
            NameServerRemoteNotify_Params * params;
        } ParamsInit;

        struct {
            Ptr                             handle;
            UInt16                          procId;
            NameServerRemoteNotify_Params * params;
        } create;

        struct {
            NameServerRemoteNotify_Handle   handle;
        } deleteInstance;

        struct {
            NameServerRemoteNotify_Handle   handle;
            String                          instanceName;
            UInt32                          instanceNameLen;
            String                          name;
            UInt32                          nameLen;
            UInt8 *                         value;
            Int                             valueLen;
            Ptr                             reserved;
            Int                             len;
        } get;

        struct {
            Ptr                             handle;
            NameServerRemoteNotify_Params * params;
            UInt32                          sharedMemSize;
        } sharedMemReq;

    } args;

    Int32 apiStatus;
} NameServerRemoteNotifyDrv_CmdArgs;


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* NAMESERVERREMOTENOTIFY_DRVDEFS_H_0X5711 */
