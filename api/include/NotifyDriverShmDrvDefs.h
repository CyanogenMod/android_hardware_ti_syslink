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
/*!
 *  @file       NotifyDriverShmDrvDefs.h
 *
 *  @brief      Definitions of NotifyDriverShmDrv types and structures.
 *
 *
 */


#ifndef NotifyDriverShmDrvDefs_H_0xb9d4
#define NotifyDriverShmDrvDefs_H_0xb9d4


/* Standard headers */
#include <Std.h>

/* Module headers */
#include <NotifyDriverShm.h>


#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*!
 *  @brief  Base structure for NotifyDriverShm command args. This needs to be
 *          the first field in all command args structures.
 */
typedef struct NotifyDriverShm_CmdArgs_tag {
    Int                 apiStatus;
    /*!< Status of the API being called. */
} NotifyDriverShm_CmdArgs;


/*  ----------------------------------------------------------------------------
 *  IOCTL command IDs for NotifyDriverShm
 *  ----------------------------------------------------------------------------
 */
/*!
 *  @brief  Base command ID for NotifyDriverShm
 */
#define NOTIFYDRIVERSHM_BASE_CMD                 0x100

/*!
 *  @brief  Command for NotifyDriverShm_getConfig
 */
#define CMD_NOTIFYDRIVERSHM_GETCONFIG           (NOTIFYDRIVERSHM_BASE_CMD + 1u)

/*!
 *  @brief  Command for NotifyDriverShm_setup
 */
#define CMD_NOTIFYDRIVERSHM_SETUP               (NOTIFYDRIVERSHM_BASE_CMD + 2u)

/*!
 *  @brief  Command for NotifyDriverShm_setup
 */
#define CMD_NOTIFYDRIVERSHM_DESTROY             (NOTIFYDRIVERSHM_BASE_CMD + 3u)

/*!
 *  @brief  Command for NotifyDriverShm_destroy
 */
#define CMD_NOTIFYDRIVERSHM_PARAMS_INIT         (NOTIFYDRIVERSHM_BASE_CMD + 4u)

/*!
 *  @brief  Command for NotifyDriverShm_create
 */
#define CMD_NOTIFYDRIVERSHM_CREATE              (NOTIFYDRIVERSHM_BASE_CMD + 5u)

/*!
 *  @brief  Command for NotifyDriverShm_delete
 */
#define CMD_NOTIFYDRIVERSHM_DELETE              (NOTIFYDRIVERSHM_BASE_CMD + 6u)

/*!
 *  @brief  Command for NotifyDriverShm_open
 */
#define CMD_NOTIFYDRIVERSHM_OPEN                (NOTIFYDRIVERSHM_BASE_CMD + 7u)

/*!
 *  @brief  Command for NotifyDriverShm_close
 */
#define CMD_NOTIFYDRIVERSHM_CLOSE               (NOTIFYDRIVERSHM_BASE_CMD + 8u)


/*  ----------------------------------------------------------------------------
 *  Command arguments for NotifyDriverShm
 *  ----------------------------------------------------------------------------
 */
/*!
 *  @brief  Command arguments for NotifyDriverShm_getConfig
 */
typedef struct NotifyDriverShm_CmdArgsGetConfig_tag {
    NotifyDriverShm_CmdArgs     commonArgs;
    /*!< Common command args */
    NotifyDriverShm_Config *    cfg;
    /*!< Pointer to the NotifyDriverShm module configuration structure in which
         the default config is to be returned. */
} NotifyDriverShm_CmdArgsGetConfig;

/*!
 *  @brief  Command arguments for NotifyDriverShm_setup
 */
typedef struct NotifyDriverShm_CmdArgsSetup_tag {
    NotifyDriverShm_CmdArgs     commonArgs;
    /*!< Common command args */
    NotifyDriverShm_Config *    cfg;
    /*!< Optional NotifyDriverShm module configuration. If provided as NULL,
         default configuration is used. */
} NotifyDriverShm_CmdArgsSetup;

/*!
 *  @brief  Command arguments for NotifyDriverShm_destroy
 */
typedef struct NotifyDriverShm_CmdArgsDestroy_tag {
    NotifyDriverShm_CmdArgs     commonArgs;
    /*!< Common command args */
} NotifyDriverShm_CmdArgsDestroy;

/*!
 *  @brief  Command arguments for NotifyDriverShm_Params_init
 */
typedef struct NotifyDriverShm_CmdArgsParamsInit_tag {
    NotifyDriverShm_CmdArgs     commonArgs;
    /*!< Common command args */
    NotifyDriverShm_Handle      handle;
    /*!< Handle to the NotifyDriverShm object. */
    NotifyDriverShm_Params *    params;
    /*!< Pointer to the NotifyDriverShm instance params structure in which the
         default params is to be returned. */
} NotifyDriverShm_CmdArgsParamsInit;

/*!
 *  @brief  Command arguments for NotifyDriverShm_create
 */
typedef struct NotifyDriverShm_CmdArgsCreate_tag {
    NotifyDriverShm_CmdArgs     commonArgs;
    /*!< Common command args */
    Char                        driverName [NOTIFY_MAX_NAMELEN];
    /*!< Name of the driver instance to be created. */
    NotifyDriverShm_Params      params;
    /*!< NotifyDriverShm instance configuration parameters. */
    NotifyDriverShm_Handle      handle;
    /*!< Handle to the created NotifyDriverShm object */
} NotifyDriverShm_CmdArgsCreate;

/*!
 *  @brief  Command arguments for NotifyDriverShm_delete
 */
typedef struct NotifyDriverShm_CmdArgsDelete_tag {
    NotifyDriverShm_CmdArgs     commonArgs;
    /*!< Common command args */
    NotifyDriverShm_Handle      handle;
    /*!< Pointer to Handle to the NotifyDriverShm object */
} NotifyDriverShm_CmdArgsDelete;

/*!
 *  @brief  Command arguments for NotifyDriverShm_open
 */
typedef struct NotifyDriverShm_CmdArgsOpen_tag {
    NotifyDriverShm_CmdArgs     commonArgs;
    /*!< Common command args */
    String                      driverName;
    /*!< Name of the driver instance to be created. */
    NotifyDriverShm_Handle      handle;
    /*!< Handle to the opened NotifyDriverShm object. */
} NotifyDriverShm_CmdArgsOpen;

/*!
 *  @brief  Command arguments for NotifyDriverShm_close
 */
typedef struct NotifyDriverShm_CmdArgsClose_tag {
    NotifyDriverShm_CmdArgs     commonArgs;
    /*!< Common command args */
    NotifyDriverShm_Handle      handle;
    /*!< Handle to the NotifyDriverShm object */
} NotifyDriverShm_CmdArgsClose;


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* NotifyDriverShmDrvDefs_H_0xb9d4 */
