/*
 *  Syslink-IPC for TI OMAP Processors
 *
 *  Copyright (c) 2008-2010, Texas Instruments Incorporated
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  *  Neither the name of Texas Instruments Incorporated nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 *  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/** ============================================================================
 *  @file   NotifyDrvDefs.h
 *
 *  @brief  NotifyDrvDefs header
 *  ============================================================================
 */


#if !defined (NOTIFYDRVDEFS_H_0x5f84)
#define NOTIFYDRVDEFS_H_0x5f84

#include <linux/ioctl.h>

/* OSAL and utils headers*/
#include <List.h>

/* Module headers*/
#include <ti/ipc/Notify.h>
#include <_Notify.h>


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
#define NOTIFYCMDBASE                           (0xE0)


/*!
 *  @brief  Base command ID for Notify
 */
#define NOTIFY_BASE_CMD                         (0x170)

/*!
 *  @brief  Command for Notify_getConfig
 */
#define CMD_NOTIFY_GETCONFIG                     _IOWR(NOTIFYCMDBASE,\
                                                 NOTIFY_BASE_CMD + 1u,\
                                                 Notify_CmdArgsGetConfig)

/*!
 *  @brief  Command for Notify_setup
 */
#define CMD_NOTIFY_SETUP                         _IOWR(NOTIFYCMDBASE,\
                                                 NOTIFY_BASE_CMD + 2u,\
                                                 Notify_CmdArgsSetup)

/*!
 *  @brief  Command for Notify_destroy
 */
#define CMD_NOTIFY_DESTROY                       _IOWR(NOTIFYCMDBASE,\
                                                 NOTIFY_BASE_CMD + 3u,\
                                                 Notify_CmdArgsDestroy)

/*!
 *  @brief  Command for Notify_registerEvent
 */
#define CMD_NOTIFY_REGISTEREVENT                 _IOWR(NOTIFYCMDBASE,\
                                                 NOTIFY_BASE_CMD + 4u,\
                                                 Notify_CmdArgsRegisterEvent)

/*!
 *  @brief  Command for Notify_unregisterEvent
 */
#define CMD_NOTIFY_UNREGISTEREVENT               _IOWR(NOTIFYCMDBASE,\
                                                 NOTIFY_BASE_CMD + 5u,\
                                                 Notify_CmdArgsUnregisterEvent)

/*!
 *  @brief  Command for Notify_sendEvent
 */
#define CMD_NOTIFY_SENDEVENT                     _IOWR(NOTIFYCMDBASE,\
                                                 NOTIFY_BASE_CMD + 6u,\
                                                 Notify_CmdArgsSendEvent)

/*!
 *  @brief  Command for Notify_disable
 */
#define CMD_NOTIFY_DISABLE                       _IOWR(NOTIFYCMDBASE,\
                                                 NOTIFY_BASE_CMD + 7u,\
                                                 Notify_CmdArgsDisable)

/*!
 *  @brief  Command for Notify_restore
 */
#define CMD_NOTIFY_RESTORE                       _IOWR(NOTIFYCMDBASE,\
                                                 NOTIFY_BASE_CMD + 8u,\
                                                 Notify_CmdArgsRestore)

/*!
 *  @brief  Command for Notify_disableEvent
 */
#define CMD_NOTIFY_DISABLEEVENT                  _IOWR(NOTIFYCMDBASE,\
                                                 NOTIFY_BASE_CMD + 9u,\
                                                 Notify_CmdArgsDisableEvent)

/*!
 *  @brief  Command for Notify_enableEvent
 */
#define CMD_NOTIFY_ENABLEEVENT                   _IOWR(NOTIFYCMDBASE,\
                                                 NOTIFY_BASE_CMD + 10u,\
                                                 Notify_CmdArgsEnableEvent)

/*!
 *  @brief  Command for Notify_attach
 */
#define CMD_NOTIFY_ATTACH                        _IOWR(NOTIFYCMDBASE,\
                                                 NOTIFY_BASE_CMD + 11u,\
                                                 Notify_CmdArgsAttach)

/*!
 *  @brief  Command for Notify_detach
 */
#define CMD_NOTIFY_DETACH                        _IOWR(NOTIFYCMDBASE,\
                                                 NOTIFY_BASE_CMD + 12u,\
                                                 Notify_CmdArgsDetach)

/*!
 *  @brief  Command for Notify_attach
 */
#define CMD_NOTIFY_THREADATTACH                  _IOWR(NOTIFYCMDBASE,\
                                                 NOTIFY_BASE_CMD + 13u,\
                                                 Notify_CmdArgs)

/*!
 *  @brief  Command for Notify_detach
 */
#define CMD_NOTIFY_THREADDETACH                  _IOWR(NOTIFYCMDBASE,\
                                                 NOTIFY_BASE_CMD + 14u,\
                                                 Notify_CmdArgs)

/*!
 *  @brief  Command for Notify_attach
 */
#define CMD_NOTIFY_ISREGISTERED                  _IOWR(NOTIFYCMDBASE,\
                                                 NOTIFY_BASE_CMD + 15u,\
                                                 Notify_CmdArgsIsRegistered)

/*!
 *  @brief  Command for Notify_detach
 */
#define CMD_NOTIFY_SHAREDMEMREQ                  _IOWR(NOTIFYCMDBASE,\
                                                 NOTIFY_BASE_CMD + 16u,\
                                                 Notify_CmdArgsSharedMemReq)
/*!
 *  @brief  Command for Notify_detach
 */
#define CMD_NOTIFY_REGISTEREVENTSINGLE           _IOWR(NOTIFYCMDBASE,\
                                                 NOTIFY_BASE_CMD + 17u,\
                                                 Notify_CmdArgsRegisterEvent)

/*!
 *  @brief  Command for Notify_detach
 */
#define CMD_NOTIFY_UNREGISTEREVENTSINGLE         _IOWR(NOTIFYCMDBASE,\
                                                 NOTIFY_BASE_CMD + 18u,\
                                                 Notify_CmdArgsUnregisterEvent)

/*!
 *  @brief  Structure of Event Packet read from notify kernel-side.
 */
typedef struct NotifyDrv_EventPacket_tag {
    List_Elem          element;
    /*!< List element header */
    UInt32             pid;
    /* Processor identifier */
    UInt16             procId;
    /*!< Processor identifier */
    UInt32             eventId;
    /*!< Event number used for the registration */
    UInt16             lineId;
    /*!< Event number used for the registration */
    UInt32             data;
    /*!< Data associated with event. */
    Notify_FnNotifyCbck func;
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
 *  @brief  Command arguments for Notify_destroy
 */
typedef struct Notify_CmdArgsAttach_tag {
    Notify_CmdArgs     commonArgs;
    UInt16             procId;
    Ptr                sharedAddr;
} Notify_CmdArgsAttach;

/*!
 *  @brief  Command arguments for Notify_destroy
 */
typedef struct Notify_CmdArgsDetach_tag {
    Notify_CmdArgs     commonArgs;
    UInt16             procId;
} Notify_CmdArgsDetach;

/*!
 *  @brief  Command arguments for Notify_destroy
 */
typedef struct Notify_CmdArgsSharedMemReq_tag {
    Notify_CmdArgs     commonArgs;
    UInt16             procId;
    Ptr                sharedAddr;
    SizeT              sharedMemSize;
} Notify_CmdArgsSharedMemReq;

/*!
 *  @brief  Command arguments for Notify_destroy
 */
typedef struct Notify_CmdArgsIsRegistered_tag {
    Notify_CmdArgs     commonArgs;
    UInt16             procId;
    UInt16             lineId;
    Bool               isRegistered;
} Notify_CmdArgsIsRegistered;

/*!
 *  @brief  Command arguments for Notify_registerEvent
 */
typedef struct Notify_CmdArgsRegisterEvent_tag {
    Notify_CmdArgs        commonArgs;
    UInt16                procId;
    UInt16                lineId;
    UInt32                eventId;
    Notify_FnNotifyCbck   fnNotifyCbck;
    UArg                  cbckArg;
    UInt32                pid;
} Notify_CmdArgsRegisterEvent;

/*!
 *  @brief  Command arguments for Notify_unregisterEvent
 */
typedef struct Notify_CmdArgsUnregisterEvent_tag {
    Notify_CmdArgs        commonArgs;
    UInt16                procId;
    UInt16                lineId;
    UInt32                eventId;
    Notify_FnNotifyCbck   fnNotifyCbck;
    UArg                  cbckArg;
    UInt32                pid;
} Notify_CmdArgsUnregisterEvent;

/*!
 *  @brief  Command arguments for Notify_sendEvent
 */
typedef struct Notify_CmdArgsSendEvent_tag {
    Notify_CmdArgs        commonArgs;
    UInt16                procId;
    UInt16                lineId;
    UInt32                eventId;
    UInt32                payload;
    Bool                  waitClear;
} Notify_CmdArgsSendEvent;

/*!
 *  @brief  Command arguments for Notify_disable
 */
typedef struct Notify_CmdArgsDisable_tag {
    Notify_CmdArgs        commonArgs;
    UInt16                procId;
    UInt16                lineId;
    UInt32                flags;
} Notify_CmdArgsDisable;

/*!
 *  @brief  Command arguments for Notify_restore
 */
typedef struct Notify_CmdArgsRestore_tag {
    Notify_CmdArgs        commonArgs;
    UInt32                key;
    UInt16                procId;
    UInt16                lineId;
} Notify_CmdArgsRestore;

/*!
 *  @brief  Command arguments for Notify_disableEvent
 */
typedef struct Notify_CmdArgsDisableEvent_tag {
    Notify_CmdArgs        commonArgs;
    UInt16                procId;
    UInt16                lineId;
    UInt32                eventId;
} Notify_CmdArgsDisableEvent;

/*!
 *  @brief  Command arguments for Notify_enableEvent
 */
typedef struct Notify_CmdArgsEnableEvent_tag {
    Notify_CmdArgs        commonArgs;
    UInt16                procId;
    UInt16                lineId;
    UInt32                eventId;
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
