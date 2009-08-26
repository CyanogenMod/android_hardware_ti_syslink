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
 *  @file   GateHWSpinLockDrvDefs.h
 *
 *  @brief      Definitions of GateHWSpinLockDrv types and structures.
 *
 *  ============================================================================
 */


#ifndef GATEHWSPINLOCK_DRVDEFS_H_0xF416
#define GATEHWSPINLOCK_DRVDEFS_H_0xF416

/* Utilities headers */
#include <GateHWSpinLock.h>

#include <ipc_ioctl.h>

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
enum CMD_GATEHWSPINLOCK {
    GATEHWSPINLOCK_GETCONFIG = GATEHWSPINLOCK_BASE_CMD,
    GATEHWSPINLOCK_SETUP,
    GATEHWSPINLOCK_DESTROY,
    GATEHWSPINLOCK_PARAMS_INIT,
    GATEHWSPINLOCK_CREATE,
    GATEHWSPINLOCK_DELETE,
    GATEHWSPINLOCK_OPEN,
    GATEHWSPINLOCK_CLOSE,
    GATEHWSPINLOCK_ENTER,
    GATEHWSPINLOCK_LEAVE
};

/*  ----------------------------------------------------------------------------
 *  IOCTL command IDs for MultiProc
 *  ----------------------------------------------------------------------------
 */

/*!
 *  @brief  Command for GateHWSpinLock_getConfig
 */
#define CMD_GATEHWSPINLOCK_GETCONFIG                \
                        _IOWR(IPC_IOC_MAGIC, GATEHWSPINLOCK_GETCONFIG,      \
                        struct GateHWSpinLockDrv_CmdArgs)

/*!
 *  @brief  Command for GateHWSpinLock_setup
 */
#define CMD_GATEHWSPINLOCK_SETUP                    \
                        _IOWR(IPC_IOC_MAGIC, GATEHWSPINLOCK_SETUP,          \
                        struct GateHWSpinLockDrv_CmdArgs)

/*!
 *  @brief  Command for GateHWSpinLock_setup
 */
#define CMD_GATEHWSPINLOCK_DESTROY                  \
                        _IOWR(IPC_IOC_MAGIC, GATEHWSPINLOCK_DESTROY,        \
                        struct GateHWSpinLockDrv_CmdArgs)

/*!
 *  @brief  Command for GateHWSpinLock_destroy
 */
#define CMD_GATEHWSPINLOCK_PARAMS_INIT              \
                        _IOWR(IPC_IOC_MAGIC, GATEHWSPINLOCK_PARAMS_INIT,    \
                        struct GateHWSpinLockDrv_CmdArgs)

/*!
 *  @brief  Command for GateHWSpinLock_create
 */
#define CMD_GATEHWSPINLOCK_CREATE                   \
                        _IOWR(IPC_IOC_MAGIC, GATEHWSPINLOCK_CREATE,         \
                        struct GateHWSpinLockDrv_CmdArgs)

/*!
 *  @brief  Command for GateHWSpinLock_delete
 */ 
#define CMD_GATEHWSPINLOCK_DELETE                   \
                        _IOWR(IPC_IOC_MAGIC, GATEHWSPINLOCK_DELETE,         \
                        struct GateHWSpinLockDrv_CmdArgs)

/*!
 *  @brief  Command for GateHWSpinLock_open
 */
#define CMD_GATEHWSPINLOCK_OPEN                     \
                        _IOWR(IPC_IOC_MAGIC, GATEHWSPINLOCK_OPEN,           \
                        struct GateHWSpinLockDrv_CmdArgs)

/*!
 *  @brief  Command for GateHWSpinLock_close
 */
#define CMD_GATEHWSPINLOCK_CLOSE                    \
                        _IOWR(IPC_IOC_MAGIC, GATEHWSPINLOCK_CLOSE,          \
                        struct GateHWSpinLockDrv_CmdArgs)

/*!
 *  @brief  Command for GateHWSpinLock_enter
 */
#define CMD_GATEHWSPINLOCK_ENTER                    \
                        _IOWR(IPC_IOC_MAGIC, GATEHWSPINLOCK_ENTER,          \
                        struct GateHWSpinLockDrv_CmdArgs)

/*!
 *  @brief  Command for GateHWSpinLock_leave
 */
#define CMD_GATEHWSPINLOCK_LEAVE                    \
                        _IOWR(IPC_IOC_MAGIC, GATEHWSPINLOCK_LEAVE,          \
                        struct GateHWSpinLockDrv_CmdArgs)

/*  ----------------------------------------------------------------------------
 *  Command arguments for GateHWSpinLock
 *  ----------------------------------------------------------------------------
 */
/*!
 *  @brief  Command arguments for GateHWSpinLock
 */
typedef struct GateHWSpinLockDrv_CmdArgs {
    union {
        struct {
            Ptr                   handle;
            GateHWSpinLock_Params * params;
        } ParamsInit;

        struct {
            GateHWSpinLock_Config * config;
        } getConfig;

        struct {
            GateHWSpinLock_Config * config;
        } setup;

        struct {
            Ptr                   handle;
            GateHWSpinLock_Params * params;
            UInt32                nameLen;
        } create;

        struct {
            Ptr                   handle;
        } deleteInstance;

        struct {
            Ptr                   handle;
            GateHWSpinLock_Params * params;
            UInt32                nameLen;
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

    } args;

    Int32 apiStatus;
} GateHWSpinLockDrv_CmdArgs;


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* GATEHWSPINLOCK_DRVDEFS_H_0xF416 */
