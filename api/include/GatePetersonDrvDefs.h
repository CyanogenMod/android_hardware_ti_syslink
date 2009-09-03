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
/** ============================================================================
 *  @file   GatePetersonDrvDefs.h
 *
 *  @brief  Definitions of GatePetersonDrv types and structures.
 *
 *  ============================================================================
 */


#ifndef GATEPETERSON_DRVDEFS_H_0xF414
#define GATEPETERSON_DRVDEFS_H_0xF414


/* Utilities headers */
#include <GatePeterson.h>
#include <SharedRegion.h>
#include <ipc_ioctl.h>

#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*  ----------------------------------------------------------------------------
 *  IOCTL command IDs for GatePeterson
 *  ----------------------------------------------------------------------------
 */
enum CMD_GATEPETERSON {
    GATEPETERSON_GETCONFIG = GATEPETERSON_BASE_CMD,
    GATEPETERSON_SETUP,
    GATEPETERSON_DESTROY,
    GATEPETERSON_PARAMS_INIT,
    GATEPETERSON_CREATE,
    GATEPETERSON_DELETE,
    GATEPETERSON_OPEN,
    GATEPETERSON_CLOSE,
    GATEPETERSON_ENTER,
    GATEPETERSON_LEAVE,
    GATEPETERSON_SHAREDMEMREQ
};

/*
 *  IOCTL command IDs for gatepeterson
 */

/*
 *  Command for gatepeterson_get_config
 */
#define CMD_GATEPETERSON_GETCONFIG      _IOWR(IPC_IOC_MAGIC,                   \
                                        GATEPETERSON_GETCONFIG,                \
                                        struct GatePetersonDrv_CmdArgs)

/*
 *  Command for gatepeterson_setup
 */
#define CMD_GATEPETERSON_SETUP          _IOWR(IPC_IOC_MAGIC,                   \
                                        GATEPETERSON_SETUP,                    \
                                        struct GatePetersonDrv_CmdArgs)

/*
 *  Command for gatepeterson_setup
 */
#define CMD_GATEPETERSON_DESTROY        _IOWR(IPC_IOC_MAGIC,                   \
                                        GATEPETERSON_DESTROY,                  \
                                        struct GatePetersonDrv_CmdArgs)

/*
 *  Command for gatepeterson_destroy
 */
#define CMD_GATEPETERSON_PARAMS_INIT    _IOWR(IPC_IOC_MAGIC,                   \
                                        GATEPETERSON_PARAMS_INIT,              \
                                        struct GatePetersonDrv_CmdArgs)

/*
 *  Command for gatepeterson_create
 */
#define CMD_GATEPETERSON_CREATE         _IOWR(IPC_IOC_MAGIC,                   \
                                        GATEPETERSON_CREATE,                   \
                                        struct GatePetersonDrv_CmdArgs)

/*
 *  Command for gatepeterson_delete
 */
#define CMD_GATEPETERSON_DELETE         _IOWR(IPC_IOC_MAGIC,                   \
                                        GATEPETERSON_DELETE,                   \
                                        struct GatePetersonDrv_CmdArgs)

/*
 *  Command for gatepeterson_open
 */
#define CMD_GATEPETERSON_OPEN           _IOWR(IPC_IOC_MAGIC,                   \
                                        GATEPETERSON_OPEN,                     \
                                        struct GatePetersonDrv_CmdArgs)

/*
 *  Command for gatepeterson_close
 */
#define CMD_GATEPETERSON_CLOSE          _IOWR(IPC_IOC_MAGIC,                   \
                                        GATEPETERSON_CLOSE,                    \
                                        struct GatePetersonDrv_CmdArgs)
/*
 *  Command for gatepeterson_enter
 */
#define CMD_GATEPETERSON_ENTER          _IOWR(IPC_IOC_MAGIC,                   \
                                        GATEPETERSON_ENTER,                    \
                                        struct GatePetersonDrv_CmdArgs)

/*
 *  Command for gatepeterson_leave
 */
#define CMD_GATEPETERSON_LEAVE          _IOWR(IPC_IOC_MAGIC,                   \
                                        GATEPETERSON_LEAVE,                    \
                                        struct GatePetersonDrv_CmdArgs)

/*
 *  Command for gatepeterson_shared_memreq
 */
#define CMD_GATEPETERSON_SHAREDMEMREQ   _IOWR(IPC_IOC_MAGIC,                   \
                                        GATEPETERSON_SHAREDMEMREQ,             \
                                        struct GatePetersonDrv_CmdArgs)


/*  ----------------------------------------------------------------------------
 *  Command arguments for GatePeterson
 *  ----------------------------------------------------------------------------
 */
/*!
 *  @brief  Command arguments for GatePeterson
 */
typedef struct GatePetersonDrv_CmdArgs {
    union {
        struct {
            Ptr                   handle;
            GatePeterson_Params * params;
        } ParamsInit;

        struct {
            GatePeterson_Config * config;
        } getConfig;

        struct {
            GatePeterson_Config * config;
        } setup;

        struct {
            Ptr                   handle;
            GatePeterson_Params * params;
            UInt32                nameLen;
            SharedRegion_SRPtr    sharedAddrSrPtr;
        } create;

        struct {
            Ptr                   handle;
        } deleteInstance;

        struct {
            Ptr                   handle;
            GatePeterson_Params * params;
            UInt32                nameLen;
            SharedRegion_SRPtr    sharedAddrSrPtr;
        } open;

        struct {
            Ptr                   handle;
        } close;

        struct {
            Ptr                   handle;
            UInt32                flags;
        } enter;

        struct {
            Ptr                   handle;
            UInt32                flags;
        } leave;

        struct {
            Ptr                   handle;
            GatePeterson_Params * params;
            UInt32                bytes;
        } sharedMemReq;

    } args;

    Int32 apiStatus;
} GatePetersonDrv_CmdArgs;


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* GATEPETERSON_DRVDEFS_H_0xF414 */
