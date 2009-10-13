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
 *  @file   SysMgrDrvDefs.h
 *
 *  @brief      Definitions of SysMgrDrv types and structures.
 *
 *
 *  ============================================================================
 */


#ifndef SYSMGR_DRVDEFS_H_0xF414
#define SYSMGR_DRVDEFS_H_0xF414


/* Utilities headers */
#include <SysMgr.h>
#include <ipc_ioctl.h>

#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*  ----------------------------------------------------------------------------
 *  IOCTL command IDs for SysMgr
 *  ----------------------------------------------------------------------------
 */
/*!
 *  @brief  IOC Magic Number for SysMgr
 */
#define SYSMGR_IOC_MAGIC        IPC_IOC_MAGIC

/*!
 *  @brief  IOCTL command numbers for SysMgr
 */
enum SysMgrDrvCmd {
    SYSMGR_SETUP = SYSMGR_BASE_CMD,
    SYSMGR_DESTROY,
    SYSMGR_LOADCALLBACK,
    SYSMGR_STARTCALLBACK,
    SYSMGR_STOPCALLBACK
};

/*!
 *  @brief  Command for SysMgr_setup
 */
#define CMD_SYSMGR_SETUP \
                _IOWR(SYSMGR_IOC_MAGIC, SYSMGR_SETUP, \
                struct SysMgrDrv_CmdArgs)

/*!
 *  @brief  Command for SysMgr_destroy
 */
#define CMD_SYSMGR_DESTROY \
                _IOWR(SYSMGR_IOC_MAGIC, SYSMGR_DESTROY, \
                struct SysMgrDrv_CmdArgs)

/*!
 *  @brief  Command for Load callback
 */
#define CMD_SYSMGR_LOADCALLBACK \
                _IOWR(SYSMGR_IOC_MAGIC, SYSMGR_LOADCALLBACK, \
                struct SysMgrDrv_CmdArgs)
/*!
 *  @brief  Command for Start callback
 */
#define CMD_SYSMGR_STARTCALLBACK \
                _IOWR(SYSMGR_IOC_MAGIC, SYSMGR_STARTCALLBACK, \
                struct SysMgrDrv_CmdArgs)
/*!
 *  @brief  Command for Stop callback
 */
#define CMD_SYSMGR_STOPCALLBACK \
                _IOWR(SYSMGR_IOC_MAGIC, SYSMGR_STOPCALLBACK, \
                struct SysMgrDrv_CmdArgs)

/*  ----------------------------------------------------------------------------
 *  Command arguments for SysMgr
 *  ----------------------------------------------------------------------------
 */
/*!
 *  @brief  Command arguments for SysMgr
 */
typedef struct SysMgrDrv_CmdArgs {
    union {
        struct {
            SysMgr_Config * config;
        } setup;

        Int32 procId;
    } args;

    Int32 apiStatus;
} SysMgrDrv_CmdArgs;


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* SYSMGR_DRVDEFS_H_0xF414 */
