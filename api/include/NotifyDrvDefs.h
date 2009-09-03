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
 *  @file   NotifyDrvDefs.h
 *
 *  @brief  NotifyDrvDefs header
 *
 *  ============================================================================
 */


#if !defined (NOTIFYDRVDEFS_H_0x5f84)
#define NOTIFYDRVDEFS_H_0x5f84


/* Standard headers*/
#include <Std.h>

/* OSAL and utils headers*/
#include <List.h>

/* Module headers*/
#include <Notify.h>
#include <NotifyDriverShm.h>


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*  ----------------------------------------------------------------------------
 *  IOCTL command IDs for Notify
 *  ----------------------------------------------------------------------------
 */
/*!
 *  @brief  Base command ID for Notify
 */
#define NOTIFY_BASE_CMD                      (0x100)

/*!
 *  @brief  Command for Notify_getConfig
 */
#define CMD_NOTIFY_GETCONFIG                 (NOTIFY_BASE_CMD + 1u)

/*!
 *  @brief  Command for Notify_setup
 */
#define CMD_NOTIFY_SETUP                     (NOTIFY_BASE_CMD + 2u)

/*!
 *  @brief  Command for Notify_destroy
 */
#define CMD_NOTIFY_DESTROY                   (NOTIFY_BASE_CMD + 3u)

/*!
 *  @brief  Command for Notify_registerEvent
 */
#define CMD_NOTIFY_REGISTEREVENT             (NOTIFY_BASE_CMD + 4u)

/*!
 *  @brief  Command for Notify_unregisterEvent
 */
#define CMD_NOTIFY_UNREGISTEREVENT           (NOTIFY_BASE_CMD + 5u)

/*!
 *  @brief  Command for Notify_sendEvent
 */
#define CMD_NOTIFY_SENDEVENT                 (NOTIFY_BASE_CMD + 6u)

/*!
 *  @brief  Command for Notify_disable
 */
#define CMD_NOTIFY_DISABLE                   (NOTIFY_BASE_CMD + 7u)

/*!
 *  @brief  Command for Notify_restore
 */
#define CMD_NOTIFY_RESTORE                   (NOTIFY_BASE_CMD + 8u)

/*!
 *  @brief  Command for Notify_disableEvent
 */
#define CMD_NOTIFY_DISABLEEVENT              (NOTIFY_BASE_CMD + 9u)

/*!
 *  @brief  Command for Notify_enableEvent
 */
#define CMD_NOTIFY_ENABLEEVENT               (NOTIFY_BASE_CMD + 10u)

/*!
 *  @brief  Command for Notify_attach
 */
#define CMD_NOTIFY_ATTACH                    (NOTIFY_BASE_CMD + 11u)

/*!
 *  @brief  Command for Notify_detach
 */
#define CMD_NOTIFY_DETACH                    (NOTIFY_BASE_CMD + 12u)


/*!
 *  @brief  Structure of Event Packet read from notify kernel-side.
 */
typedef struct NotifyDrv_EventPacket_tag {
    List_Elem          element;
    /*!< List element header */
    UInt32             pid;
    /* Process identifier */
    UInt16             procId;
    /*!< Processor identifier */
    UInt32             eventNo;
    /*!< Event number used for the registration */
    UInt32             data;
    /*!< Data associated with event. */
    Notify_CallbackFxn func;
    /*!< User callback function. */
    Ptr                param;
    /*!< User callback argument. */
    Bool               isExit;
    /*!< Indicates whether this is an exit packet */
} NotifyDrv_EventPacket ;

/*  ----------------------------------------------------------------------------
 *  Command arguments for Notify
 *  ----------------------------------------------------------------------------
 */
/*!
 *  @brief  Base structure for Notify command args. This needs to be the first
 *          field in all command args structures.
 */
typedef struct Notify_CmdArgs_tag {
    Int                 apiStatus;
    /*!< Status of the API being called. */
} Notify_CmdArgs;

/*!
 *  @brief  Command arguments for Notify_getConfig
 */
typedef struct Notify_CmdArgsArgsGetConfig_tag {
    Notify_CmdArgs     commonArgs;
    Notify_Config *    cfg;
} Notify_CmdArgsGetConfig;

/*!
 *  @brief  Command arguments for Notify_setup
 */
typedef struct Notify_CmdArgsSetup_tag {
    Notify_CmdArgs     commonArgs;
    Notify_Config *    cfg;
} Notify_CmdArgsSetup;

/*!
 *  @brief  Command arguments for Notify_destroy
 */
typedef struct Notify_CmdArgsDestroy_tag {
    Notify_CmdArgs     commonArgs;
} Notify_CmdArgsDestroy;

/*!
 *  @brief  Command arguments for Notify_registerEvent
 */
typedef struct Notify_CmdArgsRegisterEvent_tag {
    Notify_CmdArgs        commonArgs;
    NotifyDriver_Handle   handle;
    UInt16                procId;
    UInt32                eventNo;
    Notify_CallbackFxn    fnNotifyCbck;
    Ptr                   cbckArg;
    UInt32                pid;
} Notify_CmdArgsRegisterEvent;

/*!
 *  @brief  Command arguments for Notify_unregisterEvent
 */
typedef struct Notify_CmdArgsUnregisterEvent_tag {
    Notify_CmdArgs        commonArgs;
    NotifyDriver_Handle   handle;
    UInt16                procId;
    UInt32                eventNo;
    Notify_CallbackFxn    fnNotifyCbck;
    Ptr                   cbckArg;
    UInt32                pid;
} Notify_CmdArgsUnregisterEvent;

/*!
 *  @brief  Command arguments for Notify_sendEvent
 */
typedef struct Notify_CmdArgsSendEvent_tag {
    Notify_CmdArgs        commonArgs;
    NotifyDriver_Handle   handle;
    UInt16                procId;
    UInt32                eventNo;
    UInt32                payload;
    Bool                  waitClear;
} Notify_CmdArgsSendEvent;

/*!
 *  @brief  Command arguments for Notify_disable
 */
typedef struct Notify_CmdArgsDisable_tag {
    Notify_CmdArgs        commonArgs;
    UInt16                procId;
    UInt32                flags;
} Notify_CmdArgsDisable;

/*!
 *  @brief  Command arguments for Notify_restore
 */
typedef struct Notify_CmdArgsRestore_tag {
    Notify_CmdArgs        commonArgs;
    UInt32                key;
    UInt16                procId;
} Notify_CmdArgsRestore;

/*!
 *  @brief  Command arguments for Notify_disableEvent
 */
typedef struct Notify_CmdArgsDisableEvent_tag {
    Notify_CmdArgs        commonArgs;
    NotifyDriver_Handle   handle;
    UInt16                procId;
    UInt32                eventNo;
} Notify_CmdArgsDisableEvent;

/*!
 *  @brief  Command arguments for Notify_enableEvent
 */
typedef struct Notify_CmdArgsEnableEvent_tag {
    Notify_CmdArgs        commonArgs;
    NotifyDriver_Handle   handle;
    UInt16                procId;
    UInt32                eventNo;
} Notify_CmdArgsEnableEvent;

/*!
 *  @brief  Command arguments for Notify_exit
 */
typedef struct Notify_CmdArgsExit_tag {
    Notify_CmdArgs     commonArgs;
} Notify_CmdArgsExit;


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif  /* !defined (NOTIFYDRVDEFS_H_0x5f84) */
