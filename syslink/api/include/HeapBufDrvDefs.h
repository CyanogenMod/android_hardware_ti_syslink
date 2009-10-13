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
 *  @file   HeapBufDrvDefs.h
 *
 *  @brief  Definitions of HeapBufDrv types and structures.
 *  ============================================================================
 */


#ifndef HEAPBUF_DRVDEFS_H_0xb9a6
#define HEAPBUF_DRVDEFS_H_0xb9a6


/* Utilities headers */
#include <HeapBuf.h>
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
 *  IOCTL command IDs for HeapBuf
 *  ----------------------------------------------------------------------------
 */
enum CMD_HEAPBUF {
    HEAPBUF_GETCONFIG = HEAPBUF_BASE_CMD,
    HEAPBUF_SETUP,
    HEAPBUF_DESTROY,
    HEAPBUF_PARAMS_INIT,
    HEAPBUF_CREATE,
    HEAPBUF_DELETE,
    HEAPBUF_OPEN,
    HEAPBUF_CLOSE,
    HEAPBUF_ALLOC,
    HEAPBUF_FREE,
    HEAPBUF_SHAREDMEMREQ,
    HEAPBUF_GETSTATS,
    HEAPBUF_GETEXTENDEDSTATS
};

/*
 *  Command for heapbuf_get_config
 */
#define CMD_HEAPBUF_GETCONFIG           _IOWR(IPC_IOC_MAGIC, HEAPBUF_GETCONFIG,\
                                        struct HeapBufDrv_CmdArgs)

/*
 *  Command for heapbuf_setup
 */
#define CMD_HEAPBUF_SETUP               _IOWR(IPC_IOC_MAGIC, HEAPBUF_SETUP,    \
                                        struct HeapBufDrv_CmdArgs)
/*
 *  Command for heapbuf_destroy
 */
#define CMD_HEAPBUF_DESTROY             _IOWR(IPC_IOC_MAGIC, HEAPBUF_DESTROY,  \
                                        struct HeapBufDrv_CmdArgs)

/*
 *  Command for heapbuf_prams_init
 */
#define CMD_HEAPBUF_PARAMS_INIT         _IOWR(IPC_IOC_MAGIC,                   \
                                        HEAPBUF_PARAMS_INIT,                   \
                                        struct HeapBufDrv_CmdArgs)

/*
 *  Command for heapbuf_create
 */
#define CMD_HEAPBUF_CREATE              _IOWR(IPC_IOC_MAGIC, HEAPBUF_CREATE,   \
                                        struct HeapBufDrv_CmdArgs)

/*
 *  Command for heapbuf_delete
 */
#define CMD_HEAPBUF_DELETE              _IOWR(IPC_IOC_MAGIC, HEAPBUF_DELETE,   \
                                        struct HeapBufDrv_CmdArgs)

/*
 *  Command for heapbuf_open
 */
#define CMD_HEAPBUF_OPEN                _IOWR(IPC_IOC_MAGIC, HEAPBUF_OPEN,     \
                                        struct HeapBufDrv_CmdArgs)

/*
 *  Command for heapbuf_close
 */
#define CMD_HEAPBUF_CLOSE               _IOWR(IPC_IOC_MAGIC, HEAPBUF_CLOSE,    \
                                        struct HeapBufDrv_CmdArgs)

/*
 *  Command for heapbuf_alloc
 */
#define CMD_HEAPBUF_ALLOC               _IOWR(IPC_IOC_MAGIC, HEAPBUF_ALLOC,    \
                                        struct HeapBufDrv_CmdArgs)

/*
 *  Command for heapbuf_free
 */
#define CMD_HEAPBUF_FREE                _IOWR(IPC_IOC_MAGIC, HEAPBUF_FREE,     \
                                        struct HeapBufDrv_CmdArgs)

/*
 *  Command for heapbuf_shared_memreq
 */
#define CMD_HEAPBUF_SHAREDMEMREQ        _IOWR(IPC_IOC_MAGIC,                   \
                                        HEAPBUF_SHAREDMEMREQ,                  \
                                        struct HeapBufDrv_CmdArgs)

/*
 *  Command for heapbuf_get_stats
 */
#define CMD_HEAPBUF_GETSTATS            _IOWR(IPC_IOC_MAGIC,                   \
                                        HEAPBUF_GETSTATS,                      \
                                        struct HeapBufDrv_CmdArgs)

/*
 *  Command for heapbuf_get_extended_stats
 */
#define CMD_HEAPBUF_GETEXTENDEDSTATS    _IOWR(IPC_IOC_MAGIC,                   \
                                        HEAPBUF_GETEXTENDEDSTATS,              \
                                        struct HeapBufDrv_CmdArgs)


/*  ----------------------------------------------------------------------------
 *  Command arguments for HeapBuf
 *  ----------------------------------------------------------------------------
 */
/*!
 *  @brief  Command arguments for HeapBuf
 */
typedef struct HeapBufDrv_CmdArgs {
    union {
        struct {
            Ptr                  handle;
            HeapBuf_Params     * params;
        } ParamsInit;

        struct {
            HeapBuf_Config     * config;
        } getConfig;

        struct {
            HeapBuf_Config     * config;
        } setup;

        struct {
            Ptr                   handle;
            HeapBuf_Params      * params;
            UInt32                nameLen;
            SharedRegion_SRPtr    sharedAddrSrPtr;
            SharedRegion_SRPtr    sharedBufSrPtr;
            Ptr                   knlGate;
        } create;

        struct {
            Ptr                   handle;
        } deleteInstance;

        struct {
            Ptr                   handle;
            HeapBuf_Params      * params;
            UInt32                nameLen;
            SharedRegion_SRPtr    sharedAddrSrPtr;
            Ptr                   knlGate;
        } open;

        struct {
            Ptr                   handle;
        } close;

        struct {
            Ptr                   handle;
            UInt32                size;
            UInt32                align;
            SharedRegion_SRPtr    blockSrPtr;
        } alloc;

        struct {
            Ptr                   handle;
            SharedRegion_SRPtr    blockSrPtr;
            UInt32                size;
        } free;

        struct {
            Ptr                   handle;
            Memory_Stats       *  stats;
        } getStats;

        struct {
            Ptr                   handle;
            HeapBuf_ExtendedStats *  stats;
        } getExtendedStats;

        struct {
            Ptr                   handle;
            HeapBuf_Params      * params;
            UInt32                bufSize;
            UInt32                bytes;
        } sharedMemReq;

    } args;

    Int32 apiStatus;
} HeapBufDrv_CmdArgs;


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* HEAPBUF_DRVDEFS_H_0xb9a6 */
