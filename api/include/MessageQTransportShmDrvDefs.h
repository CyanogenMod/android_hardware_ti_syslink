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
 *  @file   MessageQTransportShmDrvDefs.h
 *
 *  @brief  Definitions of MessageQTransportShmDrv types and structures.
 *
 *  ============================================================================
 */


#ifndef MESSAGEQTRANSPORTSHM_DRVDEFS_H_0x8afa
#define MESSAGEQTRANSPORTSHM_DRVDEFS_H_0x8afa


/* Module headers */
#include <MessageQTransportShm.h>
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
 *  IOCTL command IDs for MessageQTransportShm
 *  ----------------------------------------------------------------------------
 */
/* Base command ID for messageq_transportshm */
#define MESSAGEQ_TRANSPORTSHM_IOC_MAGIC        IPC_IOC_MAGIC
enum messageq_transportshm_drv_cmd {
    MESSAGEQ_TRANSPORTSHM_GETCONFIG = MESSAGEQTRANSPORTSHM_BASE_CMD,
    MESSAGEQ_TRANSPORTSHM_SETUP,
    MESSAGEQ_TRANSPORTSHM_DESTROY,
    MESSAGEQ_TRANSPORTSHM_PARAMS_INIT,
    MESSAGEQ_TRANSPORTSHM_CREATE,
    MESSAGEQ_TRANSPORTSHM_DELETE,
    MESSAGEQ_TRANSPORTSHM_PUT,
    MESSAGEQ_TRANSPORTSHM_SHAREDMEMREQ,
    MESSAGEQ_TRANSPORTSHM_GETSTATUS
};

/*  ----------------------------------------------------------------------------
 *  IOCTL command IDs for messageq_transportshm
 *  ----------------------------------------------------------------------------
 */

/* Command for messageq_transportshm_get_config */
#define CMD_MESSAGEQTRANSPORTSHM_GETCONFIG \
    _IOWR(MESSAGEQ_TRANSPORTSHM_IOC_MAGIC, \
    MESSAGEQ_TRANSPORTSHM_GETCONFIG, struct MessageQTransportShmDrv_CmdArgs)

/* Command for messageq_transportshm_setup */
#define CMD_MESSAGEQTRANSPORTSHM_SETUP \
    _IOWR(MESSAGEQ_TRANSPORTSHM_IOC_MAGIC, MESSAGEQ_TRANSPORTSHM_SETUP, \
    struct MessageQTransportShmDrv_CmdArgs)

/* Command for messageq_transportshm_setup */
#define CMD_MESSAGEQTRANSPORTSHM_DESTROY \
    _IOWR(MESSAGEQ_TRANSPORTSHM_IOC_MAGIC, MESSAGEQ_TRANSPORTSHM_DESTROY, \
    struct MessageQTransportShmDrv_CmdArgs)

/* Command for messageq_transportshm_destroy */
#define CMD_MESSAGEQTRANSPORTSHM_PARAMS_INIT \
    _IOWR(MESSAGEQ_TRANSPORTSHM_IOC_MAGIC, \
    MESSAGEQ_TRANSPORTSHM_PARAMS_INIT, \
    struct MessageQTransportShmDrv_CmdArgs)

/* Command for messageq_transportshm_create */
#define CMD_MESSAGEQTRANSPORTSHM_CREATE \
    _IOWR(MESSAGEQ_TRANSPORTSHM_IOC_MAGIC, MESSAGEQ_TRANSPORTSHM_CREATE, \
    struct MessageQTransportShmDrv_CmdArgs)

/* Command for messageq_transportshm_delete */
#define CMD_MESSAGEQTRANSPORTSHM_DELETE \
    _IOWR(MESSAGEQ_TRANSPORTSHM_IOC_MAGIC, MESSAGEQ_TRANSPORTSHM_DELETE, \
    struct MessageQTransportShmDrv_CmdArgs)

/* Command for messageq_transportshm_put */
#define CMD_MESSAGEQTRANSPORTSHM_PUT \
    _IOWR(MESSAGEQ_TRANSPORTSHM_IOC_MAGIC, MESSAGEQ_TRANSPORTSHM_PUT, \
    struct MessageQTransportShmDrv_CmdArgs)

/* Command for messageq_transportshm_shared_memreq */
#define CMD_MESSAGEQTRANSPORTSHM_SHAREDMEMREQ \
    _IOWR(MESSAGEQ_TRANSPORTSHM_IOC_MAGIC, \
    MESSAGEQ_TRANSPORTSHM_SHAREDMEMREQ, \
    struct MessageQTransportShmDrv_CmdArgs)

/* Command for messageq_transportshm_get_status */
#define CMD_MESSAGEQTRANSPORTSHM_GETSTATUS \
    _IOWR(MESSAGEQ_TRANSPORTSHM_IOC_MAGIC, \
    MESSAGEQ_TRANSPORTSHM_GETSTATUS, struct MessageQTransportShmDrv_CmdArgs)


/*  ----------------------------------------------------------------------------
 *  Command arguments for MessageQTransportShm
 *  ----------------------------------------------------------------------------
 */
/*!
 *  @brief  Command arguments for MessageQTransportShm
 */
typedef struct MessageQTransportShmDrv_CmdArgs {
    union {
        struct {
            MessageQTransportShm_Config * config;
        } getConfig;

        struct {
            MessageQTransportShm_Config * config;
        } setup;

        struct {
            Ptr                           handle;
            MessageQTransportShm_Params * params;
        } ParamsInit;

        struct {
            Ptr                           handle;
            UInt16                        procId;
            MessageQTransportShm_Params * params;
            SharedRegion_SRPtr            sharedAddrSrPtr;
            Ptr                           knlLockHandle;
            Ptr                           knlNotifyDriver;
        } create;

        struct {
            MessageQTransportShm_Handle     handle;
        } deleteTransport;

        struct {
            Ptr                     handle;
            SharedRegion_SRPtr      msgSrPtr;
        } put;

        struct {
            Ptr                         handle;
            MessageQTransportShm_Status status;
        } getStatus;

        struct {
            MessageQTransportShm_Params * params;
            UInt32                        bytes;
        } sharedMemReq;

    } args;

    Int32 apiStatus;
} MessageQTransportShmDrv_CmdArgs;


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* MESSAGEQTRANSPORTSHM_DRVDEFS_H_0x8afa */
