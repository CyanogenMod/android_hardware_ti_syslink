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
 *  @file   NameServer.h
 *
 *  @brief      NameServer Manager
 *
 *              The NameServer module manages local name/value pairs that
 *              enables an application and other modules to store and retrieve
 *              values based on a name. The module supports different lengths of
 *              values. The add/get functions are for variable length values.
 *              The addUInt32 function is optimized for UInt32 variables and
 *              constants. Each NameServer instance manages a different
 *              name/value table. This allows each table to be customized to
 *              meet the requirements of user:
 *              @li Size differences: one table could allow long values
 *              (e.g. > 32 bits) while another table could be used to store
 *              integers. This customization enables better memory usage.
 *              @li Performance: improves search time when retrieving a
 *              name/value pair.
 *              @li Relax name uniqueness: names in a specific table must be
 *              unique, but the same name can be used in different tables.
 *              @li Critical region management: potentially different tables are
 *              used by different types of threads. The user can specify the
 *              type of critical region manager (i.e. xdc.runtime.IGateProvider)
 *              to be used for each instance.
 *              When adding a name/value pair, the name and value are copied
 *              into internal buffers in NameServer. To minimize runtime memory
 *              allocation these buffers can be allocated at creation time.
 *              The NameServer module can be used in a multiprocessor system.
 *              The module communicates to other processors via the RemoteProxy.
 *              The way the communication to the other processors is dependent
 *              on the RemoteProxy implementation.
 *              The NameServer module uses the MultiProc module for identifying
 *              the different processors. Which remote processors and the order
 *              they are quered is determined by the procId array in the get
 *              function.
 *              Currently there is no endian or wordsize conversion performed by
 *              the NameServer module.<br>
 *              Transport management:<br>
 *              #NameServer_setup API creates two NameServers internally. These
 *              NameServer are used for holding handles and names of other
 *              nameservers created by application or modules. This helps, when
 *              a remote processors wants to get data from a nameserver on this
 *              processor. In all modules, which can have instances, all created
 *              instances can be kept in a module specific nameserver. This
 *              reduces search operation if a single nameserver is used for all
 *              modules. Thus a module level nameserver helps.<br>
 *              When a module requires some information from a remote nameserver
 *              it passes a name in the following format:<br>
 *              "<module_name>:<instance_name>or<instance_info_name>"<br>
 *              For example: "GatePeterson:notifygate"<br>
 *              When transport gets this name it searches for <module_name> in
 *              the module nameserver (created by NameServer_setup). So, it gets
 *              the module specific NameServer handle, then it searchs for the
 *              <instance_name> or <instance_info_name> in the NameServer.
 *
 *
 *  ============================================================================
 */


#ifndef NAMESERVER_H_0XF414
#define NAMESERVER_H_0XF414

/* Utilities headers */
#include <List.h>
#include <Gate.h>
#include <NameServerRemote.h>

/* Module Headers */
#include <Heap.h>


#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 * Macros & Defines
 * =============================================================================
 */
/*!
 *  @def    NAMESERVER_MODULEID
 *  @brief  Unique module ID.
 */
#define NAMESERVER_MODULEID      (0xF414)

/*!
 *  @def    NAMESERVER_STATUSCODEBASE
 *  @brief  Error code base for buddy page allocator module.
 */
#define NAMESERVER_STATUSCODEBASE    (NAMESERVER_MODULEID << 12u)

/*!
 *  @def    NAMESERVER_MAKE_FAILURE
 *  @brief  Macro to make error code.
 */
#define NAMESERVER_MAKE_FAILURE(x)      ((Int)  (  0x80000000                  \
                                                 + NAMESERVER_STATUSCODEBASE   \
                                                 + (x)))

/*!
 *  @def    NAMESERVER_MAKE_SUCCESS
 *  @brief  Macro to make success code.
 */
#define NAMESERVER_MAKE_SUCCESS(x)  (NAMESERVER_STATUSCODEBASE + (x))

/*!
 *  @def    NAMESERVER_E_INVALIDARG
 *  @brief  Argument passed to a function is invalid.
 */
#define NAMESERVER_E_INVALIDARG     NAMESERVER_MAKE_FAILURE(1)

/*!
 *  @def    NAMESERVER_E_MEMORY
 *  @brief  Memory allocation failed.
 */
#define NAMESERVER_E_MEMORY         NAMESERVER_MAKE_FAILURE(2)

/*!
 *  @def    NAMESERVER_E_BUSY
 *  @brief  the name is already registered or not.
 */
#define NAMESERVER_E_BUSY           NAMESERVER_MAKE_FAILURE(3)

/*!
 *  @def    NAMESERVER_E_FAIL
 *  @brief  Generic failure.
 */
#define NAMESERVER_E_FAIL           NAMESERVER_MAKE_FAILURE(4)

/*!
 *  @def    NAMESERVER_E_NOTFOUND
 *  @brief  name not found in the nameserver.
 */
#define NAMESERVER_E_NOTFOUND       NAMESERVER_MAKE_FAILURE(5)

/*!
 *  @def    NAMESERVER_E_INVALIDSTATE
 *  @brief  Module is in wrong state.
 */
#define NAMESERVER_E_INVALIDSTATE   NAMESERVER_MAKE_FAILURE(6)

/*!
 *  @def    NAMESERVER_E_OSFAILURE
 *  @brief  Failure in OS call.
 */
#define NAMESERVER_E_OSFAILURE      NAMESERVER_MAKE_FAILURE(7)

/*!
 *  @def    NAMESERVER_SUCCESS
 *  @brief  Operation successful.
 */
#define NAMESERVER_SUCCESS          NAMESERVER_MAKE_SUCCESS(0)

/*!
 *  @def    NAMESERVER_S_ALREADYSETUP
 *  @brief  The NAMESERVER module has already been setup in this process.
 */
#define NAMESERVER_S_ALREADYSETUP   NAMESERVER_MAKE_SUCCESS(1)


/* =============================================================================
 * Struct & Enums
 * =============================================================================
 */
/*!
 *  @brief  Instance config-params object.
 */
typedef struct NameServer_Params {
    Gate_Handle gate;
    /*!< Lock used for critical region management of the table */
    UInt        maxRuntimeEntries;
    /*!< Maximum number of name/value pairs that can be dynamically created */
    Bool        checkExisting;
    /*!< Check if a name already exists in the name/value table */
    Heap_Handle tableHeap;
    /*!< Name/value table is placed into this section on dynamic creates */
    UInt        maxValueLen;
    /*!< Length, in MAUs, of the value field in the table */
    UInt16      maxNameLen;
    /*!< Length, in MAUs, of the name field in the table */
} NameServer_Params;

/* Forward declaration of Structure defining object for the NameServer. */
typedef struct NameServer_Object NameServer_Object;

/*!
 *  @brief  Structure defining object for the NameServer, use when embeddeding
 *          nameServer objects in user defined objects.
 */
typedef NameServer_Object NameServer_Struct;

/*!
 *  @brief  Handle to the NameServer.
 */
typedef NameServer_Object * NameServer_Handle;

/*!
 *  @brief Forward declaration of NameServer Entry
 */
typedef struct NameServer_Entry NameServer_Entry;


/* =============================================================================
 * APIs
 * =============================================================================
 */
/* Function to setup the nameserver module. */
Int
NameServer_setup (void);

/* Function to destroy the nameserver module. */
Int
NameServer_destroy (void);

/* Function to initialize the parameter structure */
void
NameServer_Params_init (NameServer_Params * params);

/* Function to create a name server */
NameServer_Handle
NameServer_create (      String              name,
                   const NameServer_Params * params);

/* Function to construct a name server */
void
NameServer_construct (      NameServer_Struct * object,
                            String              name,
                      const NameServer_Params * params);

/* Function to delete a name server */
Int
NameServer_delete (NameServer_Handle * handle);

/* Function to destruct a name server */
void
NameServer_destruct (NameServer_Struct * object);

/* Function to add a variable length value into the local table. */
Ptr
NameServer_add (NameServer_Handle handle, String name, Ptr buf, UInt len);

/* Function to add a 32 bit value into the local table. */
Ptr
NameServer_addUInt32 (NameServer_Handle handle, String name, UInt32 value);

/* Function to Retrieve the value portion of a name/value pair . */
Int32
NameServer_get (NameServer_Handle   handle,
                String              name,
                Ptr                 buf,
                UInt32              len,
                UInt16 *            procId);

/* Function to Retrieve the value portion of a name/value pair from local table.
*/
Int32
NameServer_getLocal (NameServer_Handle handle,
                     String            name,
                     Ptr               buf,
                     UInt32            len);

/* Function to Match the name. */
Int
NameServer_match (NameServer_Handle   handle,
                  String              name,
                  UInt32 *            value);

/* Function to removes a value/pair. */
Void
NameServer_remove (NameServer_Handle handle, String name);

/* Function to Remove an entry from the table. */
Void
NameServer_removeEntry (NameServer_Handle handle, Ptr entry);

/* Function to handle for  a name. */
NameServer_Handle
NameServer_getHandle (String name);

/* Function to register a remote driver.*/
Int
NameServer_registerRemoteDriver (NameServerRemote_Handle handle,
                                 UInt16                  procId);

/* Function to unregister a remote driver. */
Int
NameServer_unregisterRemoteDriver (UInt16 procId);


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* NAMESERVER_H_0X5B4D */
