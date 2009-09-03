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
 *  @file   NameServerRemote.h
 *
 *  @brief      Defines for Remote NameServer transport.
 *
 *              This file contains defines used by specific implementations of
 *              remote NameServer transports.
 *
 *  ============================================================================
 */


#ifndef NAMESERVERREMOTE_H_0x9EC4
#define NAMESERVERREMOTE_H_0x9EC4


/* Osal & Utility headers */

/* Module headers */


#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 * Macros & Defines
 * =============================================================================
 */
/*!
 *  @def    NAMESERVERREMOTE_MODULEID
 *  @brief  Unique module ID.
 */
#define NAMESERVERREMOTE_MODULEID       (0x9EC4)

/*!
 *  @def    NAMESERVERREMOTE_STATUSCODEBASE
 *  @brief  Status code base for NameServerRemote.
 */
#define NAMESERVERREMOTE_STATUSCODEBASE     (NAMESERVERREMOTE_MODULEID << 12u)

/*!
 *  @def    NAMESERVERREMOTE_MAKE_FAILURE
 *  @brief  Macro to make error code.
 */
#define NAMESERVERREMOTE_MAKE_FAILURE(x) ((Int)  (  0x80000000                 \
                                           + (NAMESERVERREMOTE_STATUSCODEBASE  \
                                           + (x))))

/*!
 *  @def    NAMESERVERREMOTE_MAKE_SUCCESS
 *  @brief  Macro to make success code.
 */
#define NAMESERVERREMOTE_MAKE_SUCCESS(x) (NAMESERVERREMOTE_STATUSCODEBASE + (x))

/*!
 *  @def    NAMESERVERREMOTE_E_INVALIDARG
 *  @brief  Argument passed to a function is invalid.
 */
#define NAMESERVERREMOTE_E_INVALIDARG       NAMESERVERREMOTE_MAKE_FAILURE(1)

/*!
 *  @def    NAMESERVERREMOTE_E_MEMORY
 *  @brief  Memory allocation failed.
 */
#define NAMESERVERREMOTE_E_MEMORY           NAMESERVERREMOTE_MAKE_FAILURE(2)

/*!
 *  @def    NAMESERVERREMOTE_E_BUSY
 *  @brief  The name is already registered or not.
 */
#define NAMESERVERREMOTE_E_BUSY             NAMESERVERREMOTE_MAKE_FAILURE(3)

/*!
 *  @def    NAMESERVERREMOTE_E_FAIL
 *  @brief  Generic failure.
 */
#define NAMESERVERREMOTE_E_FAIL             NAMESERVERREMOTE_MAKE_FAILURE(4)



/*!
 *  @def    NAMESERVERREMOTE_SUCCESS
 *  @brief  Operation successful.
 */
#define NAMESERVERREMOTE_SUCCESS            NAMESERVERREMOTE_MAKE_SUCCESS(0)


/* =============================================================================
 * Struct & Enums
 * =============================================================================
 */
/*!
 *  @brief  Handle to the Generic NameServer remote driver.
 */
typedef struct NameServerRemote_Object * NameServerRemote_Handle;

/*!
 *  @brief  Object for Generic NameServer remote driver.
 */
typedef struct NameServerRemote_Object NameServerRemote_Object;

/*! @brief Type for function pointer to get data from remote nameserver
 *  @param      obj             Remote driver object.
 *  @param      instanceName    Nameserver instance name.
 *  @param      name            Name of the entry.
 *  @param      value           Pointer to the value.
 *  @param      valueLen        Length of value.
 *  @param      reserved        Reserved.
 */
typedef UInt32 (*NameServerRemote_GetFxn) (NameServerRemote_Object * obj,
                                           String                    instName,
                                           String                    name,
                                           Ptr                       value,
                                           Int                       valueLen,
                                           Ptr                       reserved);

/* =============================================================================
 * APIs
 * =============================================================================
 */
/* Function get data from remote name server */
Int
NameServerRemote_get (NameServerRemote_Object * obj,
                      String                    instanceName,
                      String                    name,
                      Ptr                       value,
                      Int                       valueLen);


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* NAMESERVERREMOTE_H_0x9EC4 */
