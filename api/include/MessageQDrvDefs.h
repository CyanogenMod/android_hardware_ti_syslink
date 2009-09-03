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
 *  @file   MessageQDrvDefs.h
 *
 *  @brief  Definitions of MessageQDrv types and structures.
 *
 *  ============================================================================
 */


#ifndef MESSAGEQ_DRVDEFS_H_0xf653
#define MESSAGEQ_DRVDEFS_H_0xf653


/* Osal and utils headers */

/* Module headers */
#include <MessageQ.h>
#include <Heap.h>
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
 *  IOCTL command IDs for MessageQ
 *  ----------------------------------------------------------------------------
 */
#define MESSAGEQ_IOC_MAGIC        IPC_IOC_MAGIC
enum messageq_drv_cmd {
    MESSAGEQ_GETCONFIG = MESSAGEQ_BASE_CMD,
    MESSAGEQ_SETUP,
    MESSAGEQ_DESTROY,
    MESSAGEQ_PARAMS_INIT,
    MESSAGEQ_CREATE,
    MESSAGEQ_DELETE,
    MESSAGEQ_OPEN,
    MESSAGEQ_CLOSE,
    MESSAGEQ_COUNT,
    MESSAGEQ_ALLOC,
    MESSAGEQ_FREE,
    MESSAGEQ_PUT,
    MESSAGEQ_REGISTERHEAP,
    MESSAGEQ_UNREGISTERHEAP,
    MESSAGEQ_REGISTERTRANSPORT,
    MESSAGEQ_UNREGISTERTRANSPORT,
    MESSAGEQ_GET
};

/*  ----------------------------------------------------------------------------
 *  IOCTL command IDs for messageq
 *  ----------------------------------------------------------------------------
 */
/* Base command ID for messageq */
#define MESSAGEQ_BASE_CMD            0x0

/* Command for messageq_get_config */
#define CMD_MESSAGEQ_GETCONFIG \
                    _IOWR(MESSAGEQ_IOC_MAGIC, MESSAGEQ_GETCONFIG, \
                    struct MessageQDrv_CmdArgs)

/* Command for messageq_setup */
#define CMD_MESSAGEQ_SETUP \
                    _IOWR(MESSAGEQ_IOC_MAGIC, MESSAGEQ_SETUP, \
                    struct MessageQDrv_CmdArgs)

/* Command for messageq_destroy */
#define CMD_MESSAGEQ_DESTROY \
                    _IOWR(MESSAGEQ_IOC_MAGIC, MESSAGEQ_DESTROY, \
                    struct MessageQDrv_CmdArgs)

/* Command for messageq_params_init */
#define CMD_MESSAGEQ_PARAMS_INIT \
                    _IOWR(MESSAGEQ_IOC_MAGIC, MESSAGEQ_PARAMS_INIT, \
                    struct MessageQDrv_CmdArgs)

/* Command for messageq_create */
#define CMD_MESSAGEQ_CREATE \
                    _IOWR(MESSAGEQ_IOC_MAGIC, MESSAGEQ_CREATE, \
                    struct MessageQDrv_CmdArgs)

/* Command for messageq_delete */
#define CMD_MESSAGEQ_DELETE \
                    _IOWR(MESSAGEQ_IOC_MAGIC, MESSAGEQ_DELETE, \
                    struct MessageQDrv_CmdArgs)

/* Command for messageq_open */
#define CMD_MESSAGEQ_OPEN \
                    _IOWR(MESSAGEQ_IOC_MAGIC, MESSAGEQ_OPEN, \
                    struct MessageQDrv_CmdArgs)

/* Command for messageq_close */
#define CMD_MESSAGEQ_CLOSE \
                    _IOWR(MESSAGEQ_IOC_MAGIC, MESSAGEQ_CLOSE, \
                    struct MessageQDrv_CmdArgs)

/* Command for messageq_count */
#define CMD_MESSAGEQ_COUNT \
                    _IOWR(MESSAGEQ_IOC_MAGIC, MESSAGEQ_COUNT, \
                    struct MessageQDrv_CmdArgs)

/* Command for messageq_alloc */
#define CMD_MESSAGEQ_ALLOC \
                    _IOWR(MESSAGEQ_IOC_MAGIC, MESSAGEQ_ALLOC, \
                    struct MessageQDrv_CmdArgs)

/* Command for messageq_free */
#define CMD_MESSAGEQ_FREE \
                    _IOWR(MESSAGEQ_IOC_MAGIC, MESSAGEQ_FREE, \
                    struct MessageQDrv_CmdArgs)

/* Command for messageq_put */
#define CMD_MESSAGEQ_PUT \
                    _IOWR(MESSAGEQ_IOC_MAGIC, MESSAGEQ_PUT, \
                    struct MessageQDrv_CmdArgs)

/* Command for messageq_register_heap */
#define CMD_MESSAGEQ_REGISTERHEAP \
                    _IOWR(MESSAGEQ_IOC_MAGIC, MESSAGEQ_REGISTERHEAP, \
                    struct MessageQDrv_CmdArgs)

/* Command for messageq_unregister_heap */
#define CMD_MESSAGEQ_UNREGISTERHEAP \
                    _IOWR(MESSAGEQ_IOC_MAGIC, MESSAGEQ_UNREGISTERHEAP, \
                    struct MessageQDrv_CmdArgs)


/* Command for messageq_register_transport */
#define CMD_MESSAGEQ_REGISTERTRANSPORT \
                    _IOWR(MESSAGEQ_IOC_MAGIC, MESSAGEQ_REGISTERTRANSPORT, \
                    struct MessageQDrv_CmdArgs)


/* Command for messageq_unregister_transport */
#define CMD_MESSAGEQ_UNREGISTERTRANSPORT \
                    _IOWR(MESSAGEQ_IOC_MAGIC, MESSAGEQ_UNREGISTERTRANSPORT, \
                    struct MessageQDrv_CmdArgs)


/* Command for messageq_get */
#define CMD_MESSAGEQ_GET \
            _IOWR(MESSAGEQ_IOC_MAGIC, MESSAGEQ_GET, \
            struct MessageQDrv_CmdArgs)


/*  ----------------------------------------------------------------------------
 *  Command arguments for MessageQ
 *  ----------------------------------------------------------------------------
 */
/*!
 *  @brief  Command arguments for MessageQ
 */
typedef struct MessageQDrv_CmdArgs {
    union {
        struct {
            Ptr                   handle;
            MessageQ_Params     * params;
        } ParamsInit;

        struct {
            MessageQ_Config     * config;
        } getConfig;

        struct {
            MessageQ_Config     * config;
        } setup;

        struct {
            Ptr                   handle;
            String                name;
            MessageQ_Params     * params;
            UInt32                nameLen;
            MessageQ_QueueId      queueId;
        } create;

        struct {
            Ptr                  handle;
        } deleteMessageQ;

        struct {
            String               name;
            MessageQ_QueueId     queueId;
            UInt32               nameLen;
        } open;

        struct {
            MessageQ_QueueId     queueId;
        } close;

        struct {
            Ptr                   handle;
            UInt                  timeout;
            SharedRegion_SRPtr    msgSrPtr;
        } get;

        struct {
            Ptr                   handle;
            Int                   count;
        } count;

        struct {
            UInt16                heapId;
            UInt32                size;
            SharedRegion_SRPtr    msgSrPtr;
        } alloc;

        struct {
            SharedRegion_SRPtr    msgSrPtr;
        } free;

        struct {
            MessageQ_QueueId      queueId;
            SharedRegion_SRPtr    msgSrPtr;
        } put;

        struct {
            Heap_Handle           handle;
            UInt16                heapId;
        } registerHeap;

        struct {
            UInt16                heapId;
        } unregisterHeap;
    } args;

    Int32 apiStatus;
} MessageQDrv_CmdArgs;


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* MESSAGEQ_DRVDEFS_H_0xf653 */
