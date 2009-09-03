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
 *  @file   SharedRegionDrvDefs.h
 *
 *  @brief  Definitions of SharedRegionDrv types and structures.
 *  ============================================================================
 */


#ifndef SHAREDREGION_DRVDEFS_H_0xf2ba
#define SHAREDREGION_DRVDEFS_H_0xf2ba


/* Utilities headers */
#include <SharedRegion.h>
#include <ipc_ioctl.h>

#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
enum CMD_SHAREDREGION {
    SHAREDREGION_GETCONFIG = SHAREDREGION_BASE_CMD,
    SHAREDREGION_SETUP,
    SHAREDREGION_DESTROY,
    SHAREDREGION_ADD,
    SHAREDREGION_GETPTR,
    SHAREDREGION_GETSRPTR,
    SHAREDREGION_GETTABLEINFO,
    SHAREDREGION_REMOVE,
    SHAREDREGION_SETTABLEINFO,
    SHAREDREGION_GETINDEX,
};

/*
 *  IOCTL command IDs for sharedregion
 *
 */

/*
 *  Command for sharedregion_get_config
 */
#define CMD_SHAREDREGION_GETCONFIG      _IOWR(IPC_IOC_MAGIC,                   \
                                        SHAREDREGION_GETCONFIG,                \
                                        struct SharedRegionDrv_CmdArgs)

/*
 *  Command for sharedregion_setup
 */
#define CMD_SHAREDREGION_SETUP          _IOWR(IPC_IOC_MAGIC,                   \
                                        SHAREDREGION_SETUP,                    \
                                        struct SharedRegionDrv_CmdArgs)

/*
 *  Command for sharedregion_setup
 */
#define CMD_SHAREDREGION_DESTROY        _IOWR(IPC_IOC_MAGIC,                   \
                                        SHAREDREGION_DESTROY,                  \
                                        struct SharedRegionDrv_CmdArgs)

/*
 *  Command for sharedregion_ADD
 */
#define CMD_SHAREDREGION_ADD            _IOWR(IPC_IOC_MAGIC,                   \
                                        SHAREDREGION_ADD,                      \
                                        struct SharedRegionDrv_CmdArgs)

/*
 *  Command for sharedregion_get_ptr
 */
#define CMD_SHAREDREGION_GETPTR         _IOWR(IPC_IOC_MAGIC,                   \
                                        SHAREDREGION_GETPTR,                   \
                                        struct SharedRegionDrv_CmdArgs)

/*
 *  Command for sharedregion_get_srptr
 */
#define CMD_SHAREDREGION_GETSRPTR       _IOWR(IPC_IOC_MAGIC,                   \
                                        SHAREDREGION_GETSRPTR,                 \
                                        struct SharedRegionDrv_CmdArgs)

/*
 *  Command for sharedregion_get_table_info
 */
#define CMD_SHAREDREGION_GETTABLEINFO   _IOWR(IPC_IOC_MAGIC,                   \
                                        SHAREDREGION_GETTABLEINFO,             \
                                        struct SharedRegionDrv_CmdArgs)

/*
 *  Command for sharedregion_remove
 */
#define CMD_SHAREDREGION_REMOVE         _IOWR(IPC_IOC_MAGIC,                   \
                                        SHAREDREGION_REMOVE,               \
                                        struct SharedRegionDrv_CmdArgs)
/*
 *  Command for sharedregion_set_table_info
 */
#define CMD_SHAREDREGION_SETTABLEINFO   _IOWR(IPC_IOC_MAGIC,                   \
                                        SHAREDREGION_SETTABLEINFO,             \
                                        struct SharedRegionDrv_CmdArgs)

/*
 *  Command for sharedregion_get_index
 */
#define CMD_SHAREDREGION_GETINDEX       _IOWR(IPC_IOC_MAGIC,                   \
                                        SHAREDREGION_GETINDEX,                 \
                                        struct SharedRegionDrv_CmdArgs)


/*  ----------------------------------------------------------------------------
 *  Command arguments for SharedRegion
 *  ----------------------------------------------------------------------------
 */
/*!
 *  @brief  Command arguments for SharedRegion
 */
typedef struct SharedRegionDrv_CmdArgs {
    union {
        struct {
            SharedRegion_Config * config;
        } getConfig;

        struct {
            SharedRegion_Config * config;
            SharedRegion_Config * defaultCfg;
            SharedRegion_Info *   table;
        } setup;

        struct {
            UInt                  index;
            Ptr                   base;
            UInt32                len;
        } add;

        struct {
            Ptr                   addr;
            Int                   index;
        } getIndex;

        struct {
            SharedRegion_SRPtr    srptr;
            Ptr                   addr;
        } getPtr;

        struct {
            SharedRegion_SRPtr    srptr;
            Ptr                   addr;
            Int                   index;
        } getSRPtr;

        struct {
            UInt                  index;
            UInt16                procId;
            SharedRegion_Info *   info;
        } getTableInfo;

        struct {
            UInt                  index;
        } remove;

        struct {
            UInt                  index;
            UInt16                procId;
            SharedRegion_Info *   info;
        } setTableInfo;
    } args;

    Int32 apiStatus;

} SharedRegionDrv_CmdArgs;


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* SharedRegion_DrvDefs_H_0xf2ba */
