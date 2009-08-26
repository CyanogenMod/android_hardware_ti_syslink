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
 *  @file   omap4430procDrvDefs.h
 *
 *  @brief      Definitions of omap4430procDrv types and structures.
 *
 *  ============================================================================
 */


#ifndef omap4430procDrvDefs_H_0xbbec
#define omap4430procDrvDefs_H_0xbbec


/* Standard headers */
#include <Std.h>

/* Module headers */
#include <ProcMgrDrvDefs.h>
#include <omap4430proc.h>


#if defined (__cplusplus)
extern "C" {
#endif

/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*  ----------------------------------------------------------------------------
 *  IOCTL command IDs for OMAP4430PROC
 *  ----------------------------------------------------------------------------
 */
/*!
 *  @brief  Base command ID for OMAP4430PROC
 */
#define PROC4430_BASE_CMD             0x200

/*!
 *  @brief  Command for OMAP4430PROC_getConfig
 */
#define CMD_PROC4430_GETCONFIG        (PROC4430_BASE_CMD + 1u)

/*!
 *  @brief  Command for OMAP4430PROC_setup
 */
#define CMD_PROC4430_SETUP            (PROC4430_BASE_CMD + 2u)

/*!
 *  @brief  Command for OMAP4430PROC_setup
 */
#define CMD_PROC4430_DESTROY          (PROC4430_BASE_CMD + 3u)

/*!
 *  @brief  Command for OMAP4430PROC_destroy
 */
#define CMD_PROC4430_PARAMS_INIT      (PROC4430_BASE_CMD + 4u)

/*!
 *  @brief  Command for OMAP4430PROC_create
 */
#define CMD_PROC4430_CREATE           (PROC4430_BASE_CMD + 5u)

/*!
 *  @brief  Command for OMAP4430PROC_delete
 */
#define CMD_PROC4430_DELETE           (PROC4430_BASE_CMD + 6u)

/*!
 *  @brief  Command for OMAP4430PROC_open
 */
#define CMD_PROC4430_OPEN             (PROC4430_BASE_CMD + 7u)

/*!
 *  @brief  Command for OMAP4430PROC_close
 */
#define CMD_PROC4430_CLOSE            (PROC4430_BASE_CMD + 8u)


/*  ----------------------------------------------------------------------------
 *  Command arguments for OMAP4430PROC
 *  ----------------------------------------------------------------------------
 */
/*!
 *  @brief  Command arguments for OMAP4430PROC_getConfig
 */
typedef struct OMAP4430PROC_CmdArgsArgsGetConfig_tag {
    ProcMgr_CmdArgs     commonArgs;
    /*!< Common command args */
    OMAP4430PROC_Config *   cfg;
    /*!< Pointer to the OMAP4430PROC module configuration structure in which the
         default config is to be returned. */
} OMAP4430PROC_CmdArgsGetConfig;

/*!
 *  @brief  Command arguments for OMAP4430PROC_setup
 */
typedef struct OMAP4430PROC_CmdArgsSetup_tag {
    ProcMgr_CmdArgs     commonArgs;
    /*!< Common command args */
    OMAP4430PROC_Config *   cfg;
    /*!< Optional OMAP4430PROC module configuration. If provided as NULL,
         default configuration is used. */
} OMAP4430PROC_CmdArgsSetup;

/*!
 *  @brief  Command arguments for OMAP4430PROC_destroy
 */
typedef struct OMAP4430PROC_CmdArgsDestroy_tag {
    ProcMgr_CmdArgs     commonArgs;
    /*!< Common command args */
} OMAP4430PROC_CmdArgsDestroy;

/*!
 *  @brief  Command arguments for OMAP4430PROC_Params_init
 */
typedef struct OMAP4430PROC_CmdArgsParamsInit_tag {
    ProcMgr_CmdArgs     commonArgs;
    /*!< Common command args */
    Handle                  handle;
    /*!< Handle to the processor instance. */
    OMAP4430PROC_Params *   params;
    /*!< Configuration parameters. */
} OMAP4430PROC_CmdArgsParamsInit;

/*!
 *  @brief  Command arguments for OMAP4430PROC_create
 */
typedef struct OMAP4430PROC_CmdArgsCreate_tag {
    ProcMgr_CmdArgs     commonArgs;
    /*!< Common command args */
    UInt16                  procId;
    /*!< Processor ID for which this processor instance is required. */
    OMAP4430PROC_Params *   params;
    /*!< Configuration parameters. */
    Handle                  handle;
    /*!< Handle to the created processor instance. */
} OMAP4430PROC_CmdArgsCreate;

/*!
 *  @brief  Command arguments for OMAP4430PROC_delete
 */
typedef struct OMAP4430PROC_CmdArgsDelete_tag {
    ProcMgr_CmdArgs     commonArgs;
    /*!< Common command args */
    Handle              handle;
    /*!< Pointer to handle to the processor instance */
} OMAP4430PROC_CmdArgsDelete;

/*!
 *  @brief  Command arguments for OMAP4430PROC_open
 */
typedef struct OMAP4430PROC_CmdArgsOpen_tag {
    ProcMgr_CmdArgs     commonArgs;
    /*!< Common command args */
    UInt16              procId;
    /*!< Processor ID addressed by this OMAP4430PROC instance. */
    Handle              handle;
    /*!< Return parameter: Handle to the processor instance */
} OMAP4430PROC_CmdArgsOpen;

/*!
 *  @brief  Command arguments for OMAP4430PROC_close
 */
typedef struct OMAP4430PROC_CmdArgsClose_tag {
    ProcMgr_CmdArgs     commonArgs;
    /*!< Common command args */
    Handle              handle;
    /*!< Handle to the processor instance */
} OMAP4430PROC_CmdArgsClose;


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* omap4430procDrvDefs_H_0xbbec */
