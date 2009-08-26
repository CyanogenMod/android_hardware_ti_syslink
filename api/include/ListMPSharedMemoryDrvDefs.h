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
 *  @file   ListMPSharedMemoryDrvDefs.h
 *
 *  @brief  Definitions of ListMPSharedMemoryDrv types and structures.
 *  ============================================================================
 */


#ifndef LISTMPSHAREDMEMORY_DRVDEFS_H_0x42d8
#define LISTMPSHAREDMEMORY_DRVDEFS_H_0x42d8


/* Utilities headers */
#include <SharedRegion.h>
#include <ListMPSharedMemory.h>
#include <ipc_ioctl.h>

#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*  ----------------------------------------------------------------------------
 *  IOCTL command IDs for ListMPSharedMemory
 *  ----------------------------------------------------------------------------
 */
/* Base command ID for listmp_sharedmemory */
#define LISTMP_SHAREDMEMORY_IOC_MAGIC        IPC_IOC_MAGIC
enum listmp_sharedmemory_drv_cmd {
    LISTMP_SHAREDMEMORY_GETCONFIG = LISTMPSHAREDMEMORY_BASE_CMD,
    LISTMP_SHAREDMEMORY_SETUP,
    LISTMP_SHAREDMEMORY_DESTROY,
    LISTMP_SHAREDMEMORY_PARAMS_INIT,
    LISTMP_SHAREDMEMORY_CREATE,
    LISTMP_SHAREDMEMORY_DELETE,
    LISTMP_SHAREDMEMORY_OPEN,
    LISTMP_SHAREDMEMORY_CLOSE,
    LISTMP_SHAREDMEMORY_ISEMPTY,
    LISTMP_SHAREDMEMORY_GETHEAD,
    LISTMP_SHAREDMEMORY_GETTAIL,
    LISTMP_SHAREDMEMORY_PUTHEAD,
    LISTMP_SHAREDMEMORY_PUTTAIL,
    LISTMP_SHAREDMEMORY_INSERT,
    LISTMP_SHAREDMEMORY_REMOVE,
    LISTMP_SHAREDMEMORY_NEXT,
    LISTMP_SHAREDMEMORY_PREV,
    LISTMP_SHAREDMEMORY_SHAREDMEMREQ
};

/*  ----------------------------------------------------------------------------
 *  IOCTL command IDs for listmp_sharedmemory
 *  ----------------------------------------------------------------------------
 */
/* Command for listmp_sharedmemory_get_config */
#define CMD_LISTMPSHAREDMEMORY_GETCONFIG \
    _IOWR(LISTMP_SHAREDMEMORY_IOC_MAGIC, LISTMP_SHAREDMEMORY_GETCONFIG, \
    struct ListMPSharedMemoryDrv_CmdArgs)

/* Command for listmp_sharedmemory_setup */
#define CMD_LISTMPSHAREDMEMORY_SETUP \
    _IOWR(LISTMP_SHAREDMEMORY_IOC_MAGIC, LISTMP_SHAREDMEMORY_SETUP, \
    struct ListMPSharedMemoryDrv_CmdArgs)

/* Command for listmp_sharedmemory_destroy */
#define CMD_LISTMPSHAREDMEMORY_DESTROY \
    _IOWR(LISTMP_SHAREDMEMORY_IOC_MAGIC, LISTMP_SHAREDMEMORY_DESTROY, \
    struct ListMPSharedMemoryDrv_CmdArgs)

/* Command for listmp_sharedmemory_params_init */
#define CMD_LISTMPSHAREDMEMORY_PARAMS_INIT \
    _IOWR(LISTMP_SHAREDMEMORY_IOC_MAGIC, LISTMP_SHAREDMEMORY_PARAMS_INIT, \
    struct ListMPSharedMemoryDrv_CmdArgs)

/* Command for listmp_sharedmemory_create */
#define CMD_LISTMPSHAREDMEMORY_CREATE \
    _IOWR(LISTMP_SHAREDMEMORY_IOC_MAGIC, LISTMP_SHAREDMEMORY_CREATE, \
    struct ListMPSharedMemoryDrv_CmdArgs)

/* Command for listmp_sharedmemory_delete */
#define CMD_LISTMPSHAREDMEMORY_DELETE \
    _IOWR(LISTMP_SHAREDMEMORY_IOC_MAGIC, LISTMP_SHAREDMEMORY_DELETE, \
    struct ListMPSharedMemoryDrv_CmdArgs)

/* Command for listmp_sharedmemory_open */
#define CMD_LISTMPSHAREDMEMORY_OPEN \
    _IOWR(LISTMP_SHAREDMEMORY_IOC_MAGIC, LISTMP_SHAREDMEMORY_OPEN, \
    struct ListMPSharedMemoryDrv_CmdArgs)

/* Command for listmp_sharedmemory_close */
#define CMD_LISTMPSHAREDMEMORY_CLOSE \
    _IOWR(LISTMP_SHAREDMEMORY_IOC_MAGIC, LISTMP_SHAREDMEMORY_CLOSE, \
    struct ListMPSharedMemoryDrv_CmdArgs)

/* Command for listmp_sharedmemory_is_empty */
#define CMD_LISTMPSHAREDMEMORY_ISEMPTY \
    _IOWR(LISTMP_SHAREDMEMORY_IOC_MAGIC, LISTMP_SHAREDMEMORY_ISEMPTY, \
    struct ListMPSharedMemoryDrv_CmdArgs)

/* Command for listmp_sharedmemory_get_head */
#define CMD_LISTMPSHAREDMEMORY_GETHEAD \
    _IOWR(LISTMP_SHAREDMEMORY_IOC_MAGIC, LISTMP_SHAREDMEMORY_GETHEAD, \
    struct ListMPSharedMemoryDrv_CmdArgs)

/* Command for listmp_sharedmemory_get_tail */
#define CMD_LISTMPSHAREDMEMORY_GETTAIL \
    _IOWR(LISTMP_SHAREDMEMORY_IOC_MAGIC, LISTMP_SHAREDMEMORY_GETTAIL, \
    struct ListMPSharedMemoryDrv_CmdArgs)

/* Command for listmp_sharedmemory_put_head */
#define CMD_LISTMPSHAREDMEMORY_PUTHEAD \
    _IOWR(LISTMP_SHAREDMEMORY_IOC_MAGIC, LISTMP_SHAREDMEMORY_PUTHEAD, \
    struct ListMPSharedMemoryDrv_CmdArgs)

/* Command for listmp_sharedmemory_put_tail */
#define CMD_LISTMPSHAREDMEMORY_PUTTAIL \
    _IOWR(LISTMP_SHAREDMEMORY_IOC_MAGIC, LISTMP_SHAREDMEMORY_PUTTAIL, \
    struct ListMPSharedMemoryDrv_CmdArgs)

/* Command for listmp_sharedmemory_insert */
#define CMD_LISTMPSHAREDMEMORY_INSERT \
    _IOWR(LISTMP_SHAREDMEMORY_IOC_MAGIC, LISTMP_SHAREDMEMORY_INSERT, \
    struct ListMPSharedMemoryDrv_CmdArgs)

/* Command for listmp_sharedmemory_remove */
#define CMD_LISTMPSHAREDMEMORY_REMOVE \
    _IOWR(LISTMP_SHAREDMEMORY_IOC_MAGIC, LISTMP_SHAREDMEMORY_REMOVE, \
    struct ListMPSharedMemoryDrv_CmdArgs)

/* Command for listmp_sharedmemory_next */
#define CMD_LISTMPSHAREDMEMORY_NEXT \
    _IOWR(LISTMP_SHAREDMEMORY_IOC_MAGIC, LISTMP_SHAREDMEMORY_NEXT, \
    struct ListMPSharedMemoryDrv_CmdArgs)

/* Command for listmp_sharedmemory_prev */
#define CMD_LISTMPSHAREDMEMORY_PREV \
    _IOWR(LISTMP_SHAREDMEMORY_IOC_MAGIC, LISTMP_SHAREDMEMORY_PREV, \
    struct ListMPSharedMemoryDrv_CmdArgs)

/* Command for listmp_sharedmemory_shared_memreq */
#define CMD_LISTMPSHAREDMEMORY_SHAREDMEMREQ \
    _IOWR(LISTMP_SHAREDMEMORY_IOC_MAGIC, LISTMP_SHAREDMEMORY_SHAREDMEMREQ, \
    struct ListMPSharedMemoryDrv_CmdArgs)


/*  ----------------------------------------------------------------------------
 *  Command arguments for ListMPSharedMemory
 *  ----------------------------------------------------------------------------
 */
/*!
 *  @brief  Command arguments for ListMPSharedMemory
 */
typedef struct ListMPSharedMemoryDrv_CmdArgs {
    union {
        struct {
            Ptr                          handle;
            ListMPSharedMemory_Params *  params;
        } ParamsInit;

        struct {
            ListMPSharedMemory_Config * config;
        } getConfig;

        struct {
            ListMPSharedMemory_Config * config;
        } setup;

        struct {
            Ptr                         handle;
            ListMPSharedMemory_Params * params;
            UInt32                      nameLen;
            SharedRegion_SRPtr          sharedAddrSrPtr;
            Ptr                         knlGate;
        } create;

        struct {
            Ptr                   handle;
        } deleteInstance;

        struct {
            Ptr                         handle;
            ListMPSharedMemory_Params * params;
            UInt32                      nameLen;
            SharedRegion_SRPtr          sharedAddrSrPtr;
            Ptr                         knlGate;
        } open;

        struct {
            Ptr    handle;
        } close;

        struct {
            Ptr    handle;
            Bool   isEmpty;
        } isEmpty;

        struct {
            Ptr                   handle;
            SharedRegion_SRPtr    elemSrPtr;
        } getHead;

        struct {
            Ptr                   handle;
            SharedRegion_SRPtr    elemSrPtr;
        } getTail;

        struct {
            Ptr                   handle;
            SharedRegion_SRPtr    elemSrPtr ;
        } putHead;

        struct {
            Ptr                   handle;
            SharedRegion_SRPtr    elemSrPtr ;
        } putTail;

        struct {
            Ptr                   handle;
            SharedRegion_SRPtr    newElemSrPtr;
            SharedRegion_SRPtr    curElemSrPtr;
        } insert;

        struct {
            Ptr                   handle;
            SharedRegion_SRPtr    elemSrPtr ;
        } remove;

        struct {
            Ptr                   handle;
            SharedRegion_SRPtr    elemSrPtr ;
            SharedRegion_SRPtr    nextElemSrPtr ;
        } next;

        struct {
            Ptr                   handle;
            SharedRegion_SRPtr    elemSrPtr ;
            SharedRegion_SRPtr    prevElemSrPtr ;
        } prev;

        struct {
            Ptr                         handle;
            ListMPSharedMemory_Params * params;
            UInt32                      bytes;
        } sharedMemReq;

    } args;

    Int32 apiStatus;
} ListMPSharedMemoryDrv_CmdArgs;


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* LISTMPSHAREDMEMORY_DRVDEFS_H_0x42d8 */
