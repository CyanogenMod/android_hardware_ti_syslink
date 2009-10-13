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
/*============================================================================
 *  @file   NameServer.c
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
 *  ============================================================================
 */


/* Standard headers */
#include <Std.h>

/* Utilities & OSAL headers */
#include <Gate.h>
#include <Memory.h>
#include <Trace.h>
#include <String.h>

/* Module level headers */
#include <NameServer.h>
#include <NameServerDrv.h>
#include <NameServerDrvDefs.h>


#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 * Structures & Enums
 * =============================================================================
 */

/* Structure defining object for the Gate Peterson */
struct NameServer_Object {
    Ptr         knlObject;
    /*!< Pointer to the kernel-side ProcMgr object. */
};

/*!
 *  @brief  ProcMgr Module state object
 */
typedef struct NameServer_ModuleObject_tag {
    UInt32              setupRefCount;
    /*!< Reference count for number of times setup/destroy were called in this
         process. */
} NameServer_ModuleObject;


/* =============================================================================
 *  Globals
 * =============================================================================
 */
/*!
 *  @var    NameServer_state
 *
 *  @brief  ProcMgr state object variable
 */
#if !defined(SYSLINK_BUILD_DEBUG)
static
#endif /* if !defined(SYSLINK_BUILD_DEBUG) */
NameServer_ModuleObject NameServer_state =
{
    .setupRefCount = 0
};


/* =============================================================================
 * APIS
 * =============================================================================
 */
/*!
 *  @brief      Function to setup the NameServer module.
 *
 *  @sa         NameServer_destroy
 */
Int32
NameServer_setup (Void)
{
    Int                  status = NAMESERVER_SUCCESS;
    NameServerDrv_CmdArgs cmdArgs;

    GT_0trace (curTrace, GT_ENTER, "NameServer_setup");

    /* TBD: Protect from multiple threads. */
    NameServer_state.setupRefCount++;
    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */
    if (NameServer_state.setupRefCount > 1) {
        /*! @retval NAMESERVER_S_ALREADYSETUP Success: NameServer module has
                                          been already setup in this process */
        status = NAMESERVER_S_ALREADYSETUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "NameServer module has been already setup in this process.\n"
                   "    RefCount: [%d]\n",
                   NameServer_state.setupRefCount);
    }
    else {
        /* Open the driver handle. */
        status = NameServerDrv_open ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NameServer_setup",
                                 status,
                                 "Failed to open driver handle!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            status = NameServerDrv_ioctl (CMD_NAMESERVER_SETUP, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "NameServer_setup",
                                     status,
                                     "API (through IOCTL) failed on kernel-side!");
            }
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

    GT_1trace (curTrace, GT_LEAVE, "NameServer_setup", status);

    /*! @retval NAMESERVER_SUCCESS Operation successful */
    return (status);
}


/*!
 *  @brief      Function to destroy the NameServer module.
 *
 *  @sa         NameServer_setup
 */
Int32
NameServer_destroy (void)
{
    Int                     status = NAMESERVER_SUCCESS;
    NameServerDrv_CmdArgs    cmdArgs;

    GT_0trace (curTrace, GT_ENTER, "NameServer_destroy");

    /* TBD: Protect from multiple threads. */
    NameServer_state.setupRefCount--;
    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */
    if (NameServer_state.setupRefCount >= 1) {
        /*! @retval NAMESERVER_S_ALREADYSETUP Success: ProcMgr module has been
                                           already setup in this process */
        status = NAMESERVER_S_ALREADYSETUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "ProcMgr module has been already setup in this process.\n"
                   "    RefCount: [%d]\n",
                   NameServer_state.setupRefCount);
    }
    else {
        status = NameServerDrv_ioctl (CMD_NAMESERVER_DESTROY, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NameServer_destroy",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

        /* Close the driver handle. */
        NameServerDrv_close ();
    }

    GT_1trace (curTrace, GT_LEAVE, "NameServer_destroy", status);

    return status;
}


/*!
 *  @brief      Initialize this config-params structure with supplier-specified
 *              defaults before instance creation.
 *
 *  @param      params  Instance config-params structure.
 *
 *  @sa         NameServer_create
 */
Void
NameServer_Params_init (NameServer_Params * params)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int32 status = NAMESERVER_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    NameServerDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "NameServer_Params_init", params);

    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (NameServer_state.setupRefCount == 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_create",
                             NAMESERVER_E_INVALIDSTATE,
                             "Modules is invalidstate!");
    }
    else if (params == NULL) {
        /* No retVal comment since this is a Void function. */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_Params_init",
                             NAMESERVER_E_INVALIDARG,
                             "Argument of type (NameServer_Params *) passed "
                             "is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.ParamsInit.params = params;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        NameServerDrv_ioctl (CMD_NAMESERVER_PARAMS_INIT, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NameServer_Params_init",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "NameServer_Params_init");
}


/*!
 *  @brief      Creates a new instance of NameServer module.
 *
 *  @param      params  Instance config-params structure.
 *
 *  @sa         NameServer_delete, NameServer_open, NameServer_close
 */
NameServer_Handle
NameServer_create (      String              name,
                   const NameServer_Params * params)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                  status = 0;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    NameServer_Handle  handle = NULL;
    NameServerDrv_CmdArgs cmdArgs;

    GT_2trace (curTrace, GT_ENTER, "NameServer_create", name, params);

    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (NameServer_state.setupRefCount == 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_create",
                             NAMESERVER_E_INVALIDSTATE,
                             "Modules is invalidstate!");
    }
    else if (params == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_create",
                             NAMESERVER_E_INVALIDARG,
                             "params passed is NULL!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.create.name   = name;
        cmdArgs.args.create.nameLen = String_len(name);
        cmdArgs.args.create.params = (NameServer_Params *) params;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        NameServerDrv_ioctl (CMD_NAMESERVER_CREATE, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NameServer_create",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            /* Allocate memory for the handle */
            handle = (NameServer_Handle) Memory_calloc (NULL,
                                                   sizeof (NameServer_Object),
                                                   0);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (handle == NULL) {
                /*! @retval NULL Memory allocation failed for handle */
                status = NAMESERVER_E_MEMORY;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "NameServer_create",
                                     status,
                                     "Memory allocation failed for handle!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                /* Set pointer to kernel object into the user handle. */
                handle->knlObject = cmdArgs.args.create.handle;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
             }
         }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "NameServer_create", handle);

    /*! @retval valid-handle Operation successful*/
    /*! @retval NULL Operation failed */
    return handle;
}


/*!
 *  @brief      Deletes a instance of NameServer module.
 *
 *  @param      gpHandle  Handle to previously created instance.
 *
 *  @sa         NameServer_create, NameServer_open, NameServer_close
 */
Int32
NameServer_delete (NameServer_Handle * handlePtr)
{
    Int32                status = NAMESERVER_SUCCESS;
    NameServerDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "NameServer_delete", handlePtr);

    GT_assert (curTrace, (handlePtr != NULL));
    GT_assert (curTrace, ((handlePtr != NULL) && (*handlePtr != NULL)));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (NameServer_state.setupRefCount == 0) {
        /*! @retval NAMESERVER_E_INVALIDSTATE Modules is invalidstate*/
        status = NAMESERVER_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_delete",
                             status,
                             "Modules is invalidstate!");
    }
    else if (handlePtr == NULL) {
        /*! @retval NAMESERVER_E_INVALIDARG handlePtr passed is NULL*/
        status = NAMESERVER_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_delete",
                             status,
                             "handlePtr passed is NULL!");
    }
    else if (*handlePtr == NULL) {
        /*! @retval NAMESERVER_E_INVALIDARG *handlePtr passed is NULL*/
        status = NAMESERVER_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_delete",
                             status,
                             "*handlePtr passed is NULL!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.delete.handle = (*handlePtr)->knlObject;
        status = NameServerDrv_ioctl (CMD_NAMESERVER_DELETE, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NameServer_Params_init",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "NameServer_delete", status);

    /*! @retval NAMESERVER_SUCCESS Operation successful */
    return status;
}


/*!
 *  @brief      Function to add an entry into the nameserver.
 *
 *  @param      nshandle  Handle to the nameserver.
 *  @param      name      Entry name.
 *  @param      buf       Pointer to the value.
 *  @param      len       Length of the value pointer.
 *
 *  @sa         NameServer_create
 */
Ptr
NameServer_add (NameServer_Handle handle, String name, Ptr buf, UInt len)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                status = NAMESERVER_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    Ptr                new_node = NULL;
    NameServerDrv_CmdArgs cmdArgs;

    GT_4trace (curTrace, GT_ENTER, "NameServer_add",
               handle, name, buf, len);

    GT_assert (curTrace, (handle != NULL));
    GT_assert (curTrace, (name     != NULL));
    GT_assert (curTrace, (buf      != NULL));
    GT_assert (curTrace, (len      != 0));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_add",
                             NAMESERVER_E_INVALIDARG,
                             "handle: Handle is null!");
    }
    else if (name == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_add",
                             NAMESERVER_E_INVALIDARG,
                             "name: name is null!");
    }
    else if (buf == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_add",
                             NAMESERVER_E_INVALIDARG,
                             "buf: buf is null!");
    }
    else if (len == 0u) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_add",
                             NAMESERVER_E_INVALIDARG,
                             "len: Length is zero!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.add.handle  = handle->knlObject;
        cmdArgs.args.add.name    = name;
        cmdArgs.args.add.nameLen = String_len(name);
        cmdArgs.args.add.buf     = buf;
        cmdArgs.args.add.len     = len;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        NameServerDrv_ioctl (CMD_NAMESERVER_ADD, &cmdArgs);
        new_node = cmdArgs.args.add.node;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NameServer_add",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "NameServer_add", new_node);

    /*! @retval NULL operation failed */
    /*! @retval valid-pointer Pointer to created entry if operation is
     * successful */
    return new_node;
}


/*!
 *  @brief      Function to add a UInt32 value into a name server.
 *
 *  @param      nshandle  Handle to the nameserver.
 *  @param      name      Entry name.
 *  @param      value     UInt32 value.
 *
 *  @sa         NameServer_create
 */
Ptr
NameServer_addUInt32 (NameServer_Handle handle, String name, UInt32 value)
{
    Ptr entry = NULL;

    GT_3trace (curTrace,
               GT_ENTER,
               "NameServer_addUInt32",
               handle,
               name,
               value);

    GT_assert (curTrace, (handle != NULL));
    GT_assert (curTrace, (name     != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_addUInt32",
                             NAMESERVER_E_INVALIDARG,
                             "handle: Handle is null!");
    }
    else if (name == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_addUInt32",
                             NAMESERVER_E_INVALIDARG,
                             "name: name is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        entry = NameServer_add (handle, name, &value, 4);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "NameServer_addUInt32", entry);

    /*! @retval NULL operation failed */
    /*! @retval valid-pointer Pointer to created entry if operation is
     * successful */
    return entry;
}


/*!
 *  @brief      Function to remove a name/value pair from a name server.
 *
 *  @param      nshandle  Handle to the nameserver.
 *  @param      name      Entry name.
 *
 *  @sa         NameServer_add,NameServer_addUInt32
 */
Void
NameServer_remove (NameServer_Handle handle, String name)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                status = NAMESERVER_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    NameServerDrv_CmdArgs cmdArgs;

    GT_2trace (curTrace, GT_ENTER, "NameServer_remove",
               handle, name);

    GT_assert (curTrace, (handle != NULL));
    GT_assert (curTrace, (name     != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_remove",
                             NAMESERVER_E_INVALIDARG,
                             "Handle is null!");
    }
    else if (name == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_remove",
                             NAMESERVER_E_INVALIDARG,
                             "name: name is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.remove.handle  = handle->knlObject;
        cmdArgs.args.remove.name    = name;
        cmdArgs.args.remove.nameLen = String_len(name);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        NameServerDrv_ioctl (CMD_NAMESERVER_REMOVE, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NameServer_remove",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "NameServer_remove");
}


/*!
 *  @brief      Function to remove a name/value pair from a name server.
 *
 *  @param      nshandle  Handle to the nameserver.
 *  @param      name      Entry name.
 *
 *  @sa         NameServer_add,NameServer_addUInt32
 */
Void
NameServer_removeEntry (NameServer_Handle handle, Ptr entry)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                status = NAMESERVER_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    NameServerDrv_CmdArgs  cmdArgs;

    GT_2trace (curTrace, GT_ENTER, "NameServer_removeEntrty",
               handle, entry);

    GT_assert (curTrace, (handle != NULL));
    GT_assert (curTrace, (entry  != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_removeEntrty",
                             NAMESERVER_E_INVALIDARG,
                             "Handle is null!");
    }
    else if (entry == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_removeEntrty",
                             NAMESERVER_E_INVALIDARG,
                             "entry: entry is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.removeEntry.handle  = handle->knlObject;
        cmdArgs.args.removeEntry.entry  = entry;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        NameServerDrv_ioctl (CMD_NAMESERVER_REMOVEENTRY, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NameServer_removeEntrty",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "NameServer_removeEntrty");
}


/*!
 *  @brief      Function to Retrieve the value portion of a name/value pair from local table.
 *
 *  @param      nshandle  Handle to the nameserver.
 *  @param      name      Entry name.
 *  @param      value     UInt32 value.
 *
 *  @sa         NameServer_create
 */
Int32
NameServer_get (NameServer_Handle handle,
                String            name,
                Ptr               buf,
                UInt32            len,
                UInt16            procId[])
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int    status = NAMESERVER_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    UInt32 count  = 0u;
    UInt32 procLen = 0;
    UInt32 i       = 0u;
    NameServerDrv_CmdArgs cmdArgs;

    GT_5trace (curTrace, GT_ENTER, "NameServer_getLocal",
               handle, name, buf, len, procId);

    GT_assert (curTrace, (handle != NULL));
    GT_assert (curTrace, (name     != NULL));
    GT_assert (curTrace, (buf      != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_getLocal",
                             NAMESERVER_E_INVALIDARG,
                             "handle: Handle is null!");
    }
    else if (name == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_getLocal",
                             NAMESERVER_E_INVALIDARG,
                             "name: name is null!");
    }
    else if (buf == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_getLocal",
                             NAMESERVER_E_INVALIDARG,
                             "buf: buf is null!");
    }
    else if (len == 0u) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_getLocal",
                             NAMESERVER_E_INVALIDARG,
                             "len: length is zero!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.get.handle  = handle->knlObject;
        cmdArgs.args.get.name    = name;
        cmdArgs.args.get.nameLen = String_len(name);
        cmdArgs.args.get.buf     = buf;
        cmdArgs.args.get.len     = len;
        cmdArgs.args.get.procId  = procId;
        while (procId[i] != 0xFFFF) { /* TBD */
            procLen++;
        }
        cmdArgs.args.get.procLen = procLen;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        NameServerDrv_ioctl (CMD_NAMESERVER_GET, &cmdArgs);
        count = cmdArgs.args.get.count;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NameServer_getLocal",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "NameServer_getLocal", count);

    /*! @retval >0 if the operation was successful */
    /*! @retval 0  if the operation was not successful */
    return count;
}


/*!
 *  @brief      Function to Retrieve the value portion of a name/value pair from local table.
 *
 *  @param      nshandle  Handle to the nameserver.
 *  @param      name      Entry name.
 *  @param      value     UInt32 value.
 *
 *  @sa         NameServer_create
 */
Int32
NameServer_getLocal (NameServer_Handle handle,
                     String            name,
                     Ptr               buf,
                     UInt32            len)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                status = NAMESERVER_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    UInt32             count  = 0;
    NameServerDrv_CmdArgs cmdArgs;

    GT_4trace (curTrace, GT_ENTER, "NameServer_getLocal",
               handle, name, buf, len);

    GT_assert (curTrace, (handle != NULL));
    GT_assert (curTrace, (name     != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_getLocal",
                             NAMESERVER_E_INVALIDARG,
                             "handle: Handle is null!");
    }
    else if (name == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_getLocal",
                             NAMESERVER_E_INVALIDARG,
                             "name: name is null!");
    }
    else if (buf == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_getLocal",
                             NAMESERVER_E_INVALIDARG,
                             "buf: buf is null!");
    }
    else if (len == 0u) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_getLocal",
                             NAMESERVER_E_INVALIDARG,
                             "len: length is zero!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.getLocal.handle  = handle->knlObject;
        cmdArgs.args.getLocal.name    = name;
        cmdArgs.args.getLocal.nameLen = String_len(name);
        cmdArgs.args.getLocal.buf     = buf;
        cmdArgs.args.getLocal.len     = len;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        NameServerDrv_ioctl (CMD_NAMESERVER_GETLOCAL, &cmdArgs);
        count = cmdArgs.args.getLocal.count;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NameServer_getLocal",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "NameServer_getLocal", count);

    /*! @retval >0 if the operation was successful */
    /*! @retval 0  if the operation was not successful */
    return count;
}


/*!
 *  @brief      Function to Retrieve the value portion of a name/value pair from
 *  local table.
 *
 *  Currently on 32-bit values are supported.
 *
 *  Returns the number of characters that matched with an entry.
 *  So if "abc" was an entry and you called match with "abcd", this
 *  function will have the "abc" entry. The return would be 3 since
 *  three characters matched.
 *
 *  @param handle    Handle to the nameserver
 *  @param name      Name in question
 *  @param value     Pointer in which the value is returned
 *
 */
Int
NameServer_match (NameServer_Handle handle,
                  String            name,
                  UInt32 *          value)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int         status = NAMESERVER_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    UInt32      foundLen = 0;
    NameServerDrv_CmdArgs cmdArgs;

    GT_3trace (curTrace, GT_ENTER, "NameServer_match", handle, name, value);

    GT_assert (curTrace, (name != NULL));
    GT_assert (curTrace, (handle != NULL));
    GT_assert (curTrace, (value != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (name == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_match",
                             NAMESERVER_E_INVALIDARG,
                             "name is null!");
    }
    else if (handle == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_match",
                             NAMESERVER_E_INVALIDARG,
                             "handle is null!");
    }
    else if (value == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_match",
                             NAMESERVER_E_INVALIDARG,
                             "value is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.match.handle  = handle->knlObject;
        cmdArgs.args.match.name    = name;
        cmdArgs.args.match.nameLen = String_len(name);
        cmdArgs.args.match.value   = value;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        NameServerDrv_ioctl (CMD_NAMESERVER_MATCH, &cmdArgs);
        foundLen = cmdArgs.args.match.count;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NameServer_match",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "NameServer_match", foundLen);

    /*! @retval >0 if the operation was successful */
    /*! @retval 0  if the operation was not successful */
    return foundLen;
}


/*!
 *  @brief      Function to retrive a NameServer handle from name.
 *
 *  @param      name Instance name.
 *
 *  @sa         NameServer_create
 */
NameServer_Handle
NameServer_getHandle (String name)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int         status = NAMESERVER_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    NameServer_Handle handle = NULL;
    NameServerDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "NameServer_getHandle", name);

    GT_assert (curTrace, (name != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (name == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NameServer_getHandle",
                             NAMESERVER_E_INVALIDARG,
                             "name is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.getHandle.name    = name;
        cmdArgs.args.getHandle.nameLen = String_len(name);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        NameServerDrv_ioctl (CMD_NAMESERVER_GETHANDLE, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NameServer_getHandle",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            /* Allocate memory for the handle */
            handle = (NameServer_Handle) Memory_calloc (NULL,
                                                   sizeof (NameServer_Object),
                                                   0);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (handle == NULL) {
                /*! @retval NULL Memory allocation failed for handle */
                status = NAMESERVER_E_MEMORY;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "NameServer_create",
                                     status,
                                     "Memory allocation failed for handle!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                /* Set pointer to kernel object into the user handle. */
                handle->knlObject = cmdArgs.args.getHandle.handle;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            }
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "NameServer_getHandle", handle);

    /*! @retval >0 if the operation was successful */
    /*! @retval 0  if the operation was not successful */
    return handle;
}


/*!
 *  @brief      Function to register a remote driver for a processor.
 *
 *  @param      handle  Handle to the nameserver remote driver.
 *  @param      procId  Processor identifier.
 *
 *  @sa         NameServer_unregisterRemoteDriver
 */

Int
NameServer_registerRemoteDriver (NameServerRemote_Handle handle,
                                 UInt16                  procId)
{
    Int status = NAMESERVER_SUCCESS;

    GT_2trace (curTrace,
               GT_ENTER,
               "NameServer_registerRemoteDriver",
               handle,
               procId);


    GT_1trace (curTrace, GT_LEAVE, "NameServer_registerRemoteDriver", status);

    /*! @retval NAMESERVER_SUCCESS Operation is successful */
    return status;
}


/*!
 *  @brief      Function to unregister a remote driver for a processor.
 *
 *  @param      procId  Processor identifier.
 *
 *  @sa         NameServer_unregisterRemoteDriver
 */
Int
NameServer_unregisterRemoteDriver (UInt16 procId)
{
    Int status = NAMESERVER_SUCCESS;

    GT_1trace (curTrace,
               GT_ENTER,
               "NameServer_unregisterRemoteDriver",
               procId);


    GT_1trace (curTrace, GT_LEAVE, "NameServer_unregisterRemoteDriver", status);

    /*! @retval NAMESERVER_SUCCESS Operation is successful */
    return status;
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
