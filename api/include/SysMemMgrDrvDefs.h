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
 *  @file   SysMemMgrDrvDefs.h
 *
 *  @brief      Definitions of SysMemMgrDrv types and structures.
 *
 *  ============================================================================
 */


#ifndef SYSMEMMGR_DRVDEFS_H_0xF414
#define SYSMEMMGR_DRVDEFS_H_0xF414


/* Utilities headers */
#include <SysMemMgr.h>
#include <ipc_ioctl.h>

#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*  ----------------------------------------------------------------------------
 *  IOCTL command IDs for SysMemMgr
 *  ----------------------------------------------------------------------------
 */
/*!
 *  @brief  IOC Magic Number for SysMemMgr
 */
#define SYSMEMMGR_IOC_MAGIC        IPC_IOC_MAGIC

/*!
 *  @brief  IOCTL command numbers for SysMgr
 */
enum SysMemMgrDrvCmd {
    SYSMEMMGR_GETCONFIG = SYSMEMMGR_BASE_CMD,
    SYSMEMMGR_SETUP,
    SYSMEMMGR_DESTROY,
    SYSMEMMGR_ALLOC,
    SYSMEMMGR_FREE,
    SYSMEMMGR_TRANSLATE
};

/*!
 *  @brief  Command for SysMemMgr_getConfig
 */
#define CMD_SYSMEMMGR_GETCONFIG \
                _IOWR(SYSMEMMGR_IOC_MAGIC, SYSMEMMGR_GETCONFIG, \
                struct SysMemMgrDrv_CmdArgs)

/*!
 *  @brief  Command for SysMemMgr_setup
 */
#define CMD_SYSMEMMGR_SETUP \
                _IOWR(SYSMEMMGR_IOC_MAGIC, SYSMEMMGR_SETUP, \
                struct SysMemMgrDrv_CmdArgs)

/*!
 *  @brief  Command for SysMemMgr_destroy
 */
#define CMD_SYSMEMMGR_DESTROY \
                _IOWR(SYSMEMMGR_IOC_MAGIC, SYSMEMMGR_DESTROY, \
                struct SysMemMgrDrv_CmdArgs)

/*!
 *  @brief  Command for SysMemMgr_alloc
 */
#define CMD_SYSMEMMGR_ALLOC \
                _IOWR(SYSMEMMGR_IOC_MAGIC, SYSMEMMGR_ALLOC, \
                struct SysMemMgrDrv_CmdArgs)

/*!
 *  @brief  Command for SysMemMgr_free
 */
#define CMD_SYSMEMMGR_FREE \
                _IOWR(SYSMEMMGR_IOC_MAGIC, SYSMEMMGR_FREE, \
                struct SysMemMgrDrv_CmdArgs)

/*!
 *  @brief  Command for SysMemMgr_translate
 */
#define CMD_SYSMEMMGR_TRANSLATE \
                _IOWR(SYSMEMMGR_IOC_MAGIC, SYSMEMMGR_TRANSLATE, \
                struct SysMemMgrDrv_CmdArgs)


/*  ----------------------------------------------------------------------------
 *  Command arguments for SysMemMgr
 *  ----------------------------------------------------------------------------
 */
/*!
 *  @brief  Command arguments for SysMemMgr
 */
typedef struct SysMemMgrDrv_CmdArgs {
    union {
        struct {
            SysMemMgr_Config * config;
        } getConfig;

        struct {
            SysMemMgr_Config * config;
        } setup;

        struct {
            UInt32              size;
            Ptr                 buf;
            Ptr                 phys;
            Ptr                 kbuf;
            SysMemMgr_AllocFlag flags;
        } alloc;

        struct {
            UInt32              size;
            Ptr                 buf;
            Ptr                 phys;
            Ptr                 kbuf;
            SysMemMgr_AllocFlag flags;
        } free;

        struct {
            Ptr                 buf;
            Ptr                 retPtr;
            SysMemMgr_XltFlag flags;
        } translate;

    } args;

    Int32 apiStatus;
} SysMemMgrDrv_CmdArgs;


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* SYSMEMMGR_DRVDEFS_H_0xF414 */
