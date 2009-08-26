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
 *  @file   _drvdefs.h
 *
 *
 *  @desc   Defines internal data types and structures used by Linux Druver
 *  ============================================================================
 */


#if !defined (_DRVDEFS_H)
#define _DRVDEFS_H

/*  ----------------------------------- IPC Headers                 */
#include <gpptypes.h>
#include <ipctypes.h>
#include <dbc.h>

/*  ----------------------------------- IPC */
#include <ipctypes.h>

/*  ----------------------------------- NOTIFY Headers*/
#include <notify.h>

#if defined (__cplusplus)
EXTERN "C" {
#endif /* defined (__cplusplus) */


/*  ============================================================================
 *  @macro  CMD_NOTIFY_XXXX
 *
 *  @desc   Command ids for NOTIFY functions.
 *  ============================================================================
 */
#define NOTIFY_BASE_CMD                      (0x100)
#define NOTIFY_DRV_CMD_DRIVERINIT            (NOTIFY_BASE_CMD + 1)
#define NOTIFY_DRV_CMD_DRIVEREXIT            (NOTIFY_BASE_CMD + 2)
#define NOTIFY_DRV_CMD_REGISTEREVENT         (NOTIFY_BASE_CMD + 3)
#define NOTIFY_DRV_CMD_UNREGISTEREVENT       (NOTIFY_BASE_CMD + 4)
#define NOTIFY_DRV_CMD_SENDEVENT             (NOTIFY_BASE_CMD + 5)
#define NOTIFY_DRV_CMD_DISABLE               (NOTIFY_BASE_CMD + 6)
#define NOTIFY_DRV_CMD_RESTORE               (NOTIFY_BASE_CMD + 7)
#define NOTIFY_DRV_CMD_DISABLEEVENT          (NOTIFY_BASE_CMD + 8)
#define NOTIFY_DRV_CMD_ENABLEEVENT           (NOTIFY_BASE_CMD + 9)


/** ============================================================================
 *  @name   CMD_Args
 *
 *  @desc   Union defining arguments to be passed to ioctl calls. For the
 *          explanation of individual field please see the corresponding APIs.

 *  @field  apiStatus
 *              Status returned by this API.
 *          apiArgs
 *              Union representing arguments for different APIs.
 *  ============================================================================
 */
typedef struct Notify_CmdArgs_tag {
    Notify_Status apiStatus ;
    union {
        struct {
            Notify_Handle   handle ;
            Notify_Config * config ;
            Char8 *         driverName ;
        } driverInitArgs ;

        struct {
            Notify_Handle   handle ;
        } driverExitArgs ;

        struct {
            Notify_Handle   handle ;
            Uint32          eventNo ;
            Processor_Id     procId ;
            FnNotifyCbck    fnNotifyCbck ;
            Void *          cbckArg ;
        } unregisterEventArgs ;

        struct {
            Notify_Handle   handle ;
            Uint32          eventNo ;
            Processor_Id     procId ;
            FnNotifyCbck    fnNotifyCbck ;
            Void *          cbckArg ;
        } registerEventArgs ;

        struct {
            Notify_Handle   handle ;
            Uint32          eventNo ;
            Processor_Id    procId ;
            Uint32          payload;
            Bool            waitClear;
        } sendEventArgs ;

        struct {
            Void *          disableFlags ;
        } disableArgs ;

        struct {
            Void *          restoreFlags ;
        } restoreArgs ;

        struct {
            Notify_Handle   handle ;
            Uint32          eventNo ;
            Processor_Id     procId ;
        } disableEventArgs ;

        struct {
            Notify_Handle   handle ;
            Uint32          eventNo ;
            Processor_Id     procId ;
        } enableEventArgs ;
    } apiArgs ;
} Notify_CmdArgs ;

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif  /* !defined (_DRVDEFS_H) */
