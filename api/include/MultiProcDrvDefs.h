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
 *  @file   MultiProcDrvDefs.h
 *
 *  @brief      Definitions of NameServerDrv types and structures.
 *
 *  ============================================================================
 */


#ifndef MULTIPROC_DRVDEFS_H_0xf2ba
#define MULTIPROC_DRVDEFS_H_0xf2ba

/* Standard headers */
#include <ipc_ioctl.h>
#include <Std.h>

/* Utilities headers */
#include <MultiProc.h>

#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*  ----------------------------------------------------------------------------
 *  IOCTL command IDs for MultiProc
 *  ----------------------------------------------------------------------------
 */
enum CMD_MULTIPROC {
    MULTIPROC_SETUP = MULTIPROC_BASE_CMD,
    MULTIPROC_DESTROY,
    MULTIPROC_GETCONFIG,
    MULTIPROC_SETLOCALID
};

/*  ----------------------------------------------------------------------------
 *  IOCTL command IDs for MultiProc
 *  ----------------------------------------------------------------------------
 */

/*
 *  Command for multiproc_setup
 */
#define CMD_MULTIPROC_SETUP             \
                        _IOWR(IPC_IOC_MAGIC, MULTIPROC_SETUP,           \
                        struct MultiProcDrv_CmdArgs)

/*
 *  Command for multiproc_destroy
 */
#define CMD_MULTIPROC_DESTROY           \
                        _IOWR(IPC_IOC_MAGIC, MULTIPROC_DESTROY,         \
                        struct MultiProcDrv_CmdArgs)

/*
 *  Command for multiproc_get_config
 */
#define CMD_MULTIPROC_GETCONFIG         \
                        _IOWR(IPC_IOC_MAGIC, MULTIPROC_GETCONFIG,       \
                        struct MultiProcDrv_CmdArgs)

/*
 *  Command for multiproc_set_local_id
 */
#define CMD_MULTIPROC_SETLOCALID        \
                        _IOWR(IPC_IOC_MAGIC, MULTIPROC_SETLOCALID,      \
                        struct MultiProcDrv_CmdArgs)


/*  ----------------------------------------------------------------------------
 *  Command arguments for MultiProc
 *  ----------------------------------------------------------------------------
 */
/*!
 *  @brief  Command arguments for MultiProc
 */
typedef struct MultiProcDrv_CmdArgs {
    union {
        struct {
            MultiProc_Config * config;
        } getConfig;

        struct {
            MultiProc_Config * config;
        } setup;

        struct {
            UInt16 id;
        } setLocalId;

    } args;

    Int32 apiStatus;
} MultiProcDrv_CmdArgs;


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* MultiProc_DrvDefs_H_0xf2ba */
