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
 *  @file   ListMPSharedMemory.c
 *
 *  @brief      Defines for shared memory doubly linked list.
 *
 *              This module implements the ListMPSharedMemory.
 *              ListMPSharedMemory is a linked-list based module designed to be
 *              used in a multi-processor environment.  It is designed to
 *              provide a means of communication between different processors.
 *              processors.
 *              This module is instance based. Each instance requires a small
 *              piece of shared memory. This is specified via the #sharedAddr
 *              parameter to the create. The proper #sharedAddrSize parameter
 *              can be determined via the #sharedMemReq call. Note: the
 *              parameters to this function must be the same that will used to
 *              create or open the instance.
 *              The ListMPSharedMemory module uses a #NameServer instance
 *              to store instance information when an instance is created and
 *              the name parameter is non-NULL. If a name is supplied, it must
 *              be unique for all ListMPSharedMemory instances.
 *              The #create also initializes the shared memory as needed. The
 *              shared memory must be initialized to 0 or all ones
 *              (e.g. 0xFFFFFFFFF) before the ListMPSharedMemory instance
 *              is created.
 *              Once an instance is created, an open can be performed. The #open
 *              is used to gain access to the same ListMPSharedMemory instance.
 *              Generally an instance is created on one processor and opened
 *              on the other processor.
 *              The open returns a ListMPSharedMemory instance handle like the
 *              create, however the open does not modify the shared memory.
 *              Generally an instance is created on one processor and opened
 *              on the other processor.
 *              There are two options when opening the instance:
 *              @li Supply the same name as specified in the create. The
 *              ListMPSharedMemory module queries the NameServer to get the
 *              needed information.
 *              @li Supply the same #sharedAddr value as specified in the
 *              create.
 *              If the open is called before the instance is created, open
 *              returns NULL.
 *              There is currently a list of restrictions for the module:
 *              @li Both processors must have the same endianness. Endianness
 *              conversion may supported in a future version of
 *              ListMPSharedMemory.
 *              @li The module will be made a gated module
 *
 *  ============================================================================
 */



/* Standard headers */
#include <Std.h>

/* Osal And Utils  headers */
#include <String.h>
#include <Trace.h>
#include <Memory.h>
#include <string.h>

/* Module level headers */
#include <_ListMP.h>
#include <ListMP.h>
#include <ListMPSharedMemory.h>
#include <ListMPSharedMemoryDrv.h>
#include <ListMPSharedMemoryDrvDefs.h>


#if defined (__cplusplus)
extern "C" {
#endif

/* =============================================================================
 * Globals
 * =============================================================================
 */
/*!
 *  @brief  Cache size
 */
#define LISTMPSHAREDMEMORY_CACHESIZE   128u
/* =============================================================================
 * Structures & Enums
 * =============================================================================
 */

/* Structure defining object for the Gate Peterson */
typedef struct ListMPSharedMemory_Obj_tag {
    Ptr         knlObject;
    /*!< Pointer to the kernel-side ListMPSharedMemory object. */
} ListMPSharedMemory_Obj;

/*!
 *  @brief  ListMPSharedMemory Module state object
 */
typedef struct ListMPSharedMemory_ModuleObject_tag {
    UInt32                    setupRefCount;
    /*!< Reference count for number of times setup/destroy were called in this
         process. */
} ListMPSharedMemory_ModuleObject;


/* =============================================================================
 *  Globals
 * =============================================================================
 */
/*!
 *  @var    ListMPSharedMemory_state
 *
 *  @brief  ListMPSharedMemory state object variable
 */
#if !defined(SYSLINK_BUILD_DEBUG)
static
#endif /* if !defined(SYSLINK_BUILD_DEBUG) */
ListMPSharedMemory_ModuleObject ListMPSharedMemory_state =
{
    .setupRefCount = 0
};


/* =============================================================================
 *  Internal functions
 * =============================================================================
 */
Int32
 _ListMPSharedMemory_create(ListMPSharedMemory_Handle       * listMPHandle,
                             ListMPSharedMemoryDrv_CmdArgs    cmdArgs,
                             UInt16                           createFlag);


/* =============================================================================
 * APIS
 * =============================================================================
 */
/*!
 *  @brief      Function to get configuration parameters to setup
 *              the ListMPSharedMemory module.
 *
 *  @param      cfgParams   Configuration values.
 *
 *  @sa         ListMPSharedMemory_setup
 */
Void
ListMPSharedMemory_getConfig (ListMPSharedMemory_Config * cfgParams)
{
    Int                        status = LISTMPSHAREDMEMORY_SUCCESS;
    ListMPSharedMemoryDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "ListMPSharedMemory_getConfig", cfgParams);

    GT_assert (curTrace, (cfgParams != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (cfgParams == NULL) {
        /*! @retval LISTMPSHAREDMEMORY_E_INVALIDARG Argument of type
         *  (ListMPSharedMemory_Config *) passed is null
         */
        status = LISTMPSHAREDMEMORY_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_getConfig",
                             status,
                        "Argument of type (ListMPSharedMemory_Config *) passed "
                             "is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

        /* Temporarily open the handle to get the configuration. */
        status = ListMPSharedMemoryDrv_open ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ListMPSharedMemory_getConfig",
                                 status,
                                 "Failed to open driver handle!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            cmdArgs.args.getConfig.config = cfgParams;
            status = ListMPSharedMemoryDrv_ioctl
                             (CMD_LISTMPSHAREDMEMORY_GETCONFIG,
                              &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                  GT_4CLASS,
                                  "ListMPSharedMemory_getConfig",
                                  status,
                                  "API (through IOCTL) failed on kernel-side!");

            }
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Close the driver handle. */
        ListMPSharedMemoryDrv_close ();

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ListMPSharedMemory_getConfig", status);

    /*! @retval LISTMPSHAREDMEMORY_SUCCESS Operation Successful*/
}

/*!
 *  @brief      Function to setup the ListMPSharedMemory module.
 *
 *  @param      config Module configuration parameters
 *
 *  @sa         ListMPSharedMemory_destroy
 */
Int
ListMPSharedMemory_setup (ListMPSharedMemory_Config * config)
{
    Int                        status = LISTMPSHAREDMEMORY_SUCCESS;
    ListMPSharedMemoryDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "ListMPSharedMemory_setup", config);

    /* TBD: Protect from multiple threads. */
    ListMPSharedMemory_state.setupRefCount++;
    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */
    if (ListMPSharedMemory_state.setupRefCount > 1) {
        /*! @retval LISTMPSHAREDMEMORY_S_ALREADYSETUP Success:
         *           ListMPSharedMemory module has been already setup
         *           in this process
         */
        status = LISTMPSHAREDMEMORY_S_ALREADYSETUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "ListMPSharedMemory module has been already setup in this"
                   " process.\n RefCount: [%d]\n",
                   ListMPSharedMemory_state.setupRefCount);
    }
    else {
        /* Open the driver handle. */
        status = ListMPSharedMemoryDrv_open ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ListMPSharedMemory_setup",
                                 status,
                                 "Failed to open driver handle!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            cmdArgs.args.setup.config = (ListMPSharedMemory_Config *) config;
            status = ListMPSharedMemoryDrv_ioctl (CMD_LISTMPSHAREDMEMORY_SETUP,
                                                  &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "ListMPSharedMemory_setup",
                                     status,
                                     "API (through IOCTL) failed"
                                     " on kernel-side!");
            }
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

    GT_1trace (curTrace, GT_LEAVE, "ListMPSharedMemory_setup", status);

    /*! @retval LISTMPSHAREDMEMORY_SUCCESS Operation Successful*/
    return status;
}

/*!
 *  @brief      Function to destroy the ListMPSharedMemory module.
 *
 *  @param      None
 *
 *  @sa         ListMPSharedMemory_setup
 */
Int
ListMPSharedMemory_destroy (void)
{
    Int                           status = LISTMPSHAREDMEMORY_SUCCESS;
    ListMPSharedMemoryDrv_CmdArgs    cmdArgs;

    GT_0trace (curTrace, GT_ENTER, "ListMPSharedMemory_destroy");

    /* TBD: Protect from multiple threads. */
    ListMPSharedMemory_state.setupRefCount--;
    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */
    if (ListMPSharedMemory_state.setupRefCount >= 1) {
        /*! @retval LISTMPSHAREDMEMORY_S_ALREADYSETUP Success:
         *           ListMPSharedMemory module has been already setup
         *           in this process
         */
        status = LISTMPSHAREDMEMORY_S_ALREADYSETUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "ListMPSharedMemory module has been setup by other clients"
                   " in this process.\n"
                   "    RefCount: [%d]\n",
                   ListMPSharedMemory_state.setupRefCount);
    }
    else {
        status = ListMPSharedMemoryDrv_ioctl (CMD_LISTMPSHAREDMEMORY_DESTROY,
                                              &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ListMPSharedMemory_destroy",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

        /* Close the driver handle. */
        ListMPSharedMemoryDrv_close ();
    }

    GT_1trace (curTrace, GT_LEAVE, "ListMPSharedMemory_destroy", status);

    /*! @retval LISTMPSHAREDMEMORY_SUCCESS Operation Successful*/
    return status;
}

/*!
 *  @brief      Initialize this config-params structure with supplier-specified
 *              defaults before instance creation.
 *
 *  @param      handle Instance specific handle.
 *  @param      params Instance creation structure.
 *
 *  @sa         ListMPSharedMemory_create
 */
Void
ListMPSharedMemory_Params_init (ListMPSharedMemory_Handle         handle,
                                ListMPSharedMemory_Params       * params)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                           status = LISTMPSHAREDMEMORY_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    ListMPSharedMemoryDrv_CmdArgs    cmdArgs;

    GT_2trace (curTrace, GT_ENTER, "ListMPSharedMemory_Params_init",
               handle, params);

    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (ListMPSharedMemory_state.setupRefCount == 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_Params_init",
                             LISTMPSHAREDMEMORY_E_INVALIDSTATE,
                             "Module is not initialized!");
    }
    else if (params == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_Params_init",
                             LISTMPSHAREDMEMORY_E_INVALIDARG,
                             "Argument of type (ListMPSharedMemory_Params *) "
                             " passed is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        if (handle != NULL) {
            cmdArgs.args.ParamsInit.handle = ((ListMPSharedMemory_Obj *)
                      (((ListMPSharedMemory_Object *) handle)->obj))->knlObject;
        }
        else {
            cmdArgs.args.ParamsInit.handle = handle;
        }
        cmdArgs.args.ParamsInit.params = params;
#if !defined(SYSLINK_BUILD_OPTIMIZE)

        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        ListMPSharedMemoryDrv_ioctl (CMD_LISTMPSHAREDMEMORY_PARAMS_INIT,
                                     &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ListMPSharedMemory_Params_init",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "ListMPSharedMemory_Params_init");

    /* @retval None */
    return;
}
/*!
 *  @brief      Creates a new instance of ListMPSharedMemory module.
 *
 *  @param      params  Instance config-params structure.
 *
 *  @sa         ListMPSharedMemory_delete
 */
ListMPSharedMemory_Handle
ListMPSharedMemory_create (ListMPSharedMemory_Params * params)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                           status = 0;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    ListMPSharedMemory_Handle     handle = NULL;
    ListMPSharedMemoryDrv_CmdArgs cmdArgs;
    Int32 index;

    GT_1trace (curTrace, GT_ENTER, "ListMPSharedMemory_create", params);

    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (ListMPSharedMemory_state.setupRefCount == 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_create",
                             LISTMPSHAREDMEMORY_E_INVALIDSTATE,
                             "Module is not initialized!");
    }
    else if (params == NULL) {
        /*! @retval NULL if Params passed is NULL */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_create",
                             LISTMPSHAREDMEMORY_E_INVALIDARG,
                             "Params passed is NULL!");
    }
    else if (    (params->sharedAddr == (UInt32)NULL)
              && (params->listType != ListMP_Type_FAST)){
        /*! @retval NULL if Invalid shared region address */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_create",
                             LISTMPSHAREDMEMORY_E_INVALIDARG,
                             "Please provide a valid shared region "
                             "address!");
    }
    else if (  params->sharedAddrSize
             < ListMPSharedMemory_sharedMemReq (params)) {
        /*! @retval NULL if Shared memory size is less
         *          than required
         */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_create",
                             LISTMPSHAREDMEMORY_E_INVALIDARG,
                             "Shared memory size is less than required!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.create.params = (ListMPSharedMemory_Params *) params;

        /* Translate sharedAddr to SrPtr. */
        GT_1trace (curTrace,
                   GT_2CLASS,
                   "    ListMPSharedMemory_create: Shared addr [0x%x]\n",
                   params->sharedAddr);
        index = SharedRegion_getIndex (params->sharedAddr);
        cmdArgs.args.create.sharedAddrSrPtr = SharedRegion_getSRPtr (
                                                            params->sharedAddr,
                                                            index);
        GT_1trace (curTrace,
                   GT_2CLASS,
                   "    ListMPSharedMemory_create: "
                   "Shared buffer addr SrPtr [0x%x]\n",
                   cmdArgs.args.create.params->sharedAddr);
        /* Translate Gate handle to kernel-side gate handle. */
        cmdArgs.args.create.knlGate =
                                        Gate_getKnlHandle (params->gate);
        if (params->name != NULL) {
            cmdArgs.args.create.nameLen = (String_len (params->name)+ 1u);
        }
        else {
            cmdArgs.args.create.nameLen = 0;
        }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        ListMPSharedMemoryDrv_ioctl (CMD_LISTMPSHAREDMEMORY_CREATE,
                                     &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ListMPSharedMemory_create",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            _ListMPSharedMemory_create (&handle, cmdArgs, TRUE);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                            GT_4CLASS,
                            "ListMPSharedMemory_create",
                            status,
                            "ListMPSharedMemory creation failed on user-side!");
            }
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ListMPSharedMemory_create", handle);

    /*! @retval Valid Handle Operation Successful */
    /*! @retval NULL         Operation failed */
    return handle;
}


/*!
 *  @brief      Deletes a instance of ListMPSharedMemory module.
 *
 *  @param      listMPHandle  Pointer to handle to previously created instance.
 *
 *  @sa         ListMPSharedMemory_create
 */
Int
ListMPSharedMemory_delete (ListMPSharedMemory_Handle * listMPHandle)
{
    Int                        status = LISTMPSHAREDMEMORY_SUCCESS;
    ListMPSharedMemoryDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "ListMPSharedMemory_delete", listMPHandle);

    GT_assert (curTrace, (listMPHandle != NULL));
    GT_assert (curTrace, ((listMPHandle != NULL) && (*listMPHandle != NULL)));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (ListMPSharedMemory_state.setupRefCount == 0) {
        /*! @retval LISTMPSHAREDMEMORY_E_INVALIDSTATE
         *          Module is not initialized
         */
        status = LISTMPSHAREDMEMORY_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_delete",
                             status,
                             "Module is not initialized!");
    }
    else if (listMPHandle == NULL) {
        /*! @retval LISTMPSHAREDMEMORY_E_INVALIDARG handle passed is NULL*/
        status = LISTMPSHAREDMEMORY_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_delete",
                             status,
                             "handle passed is NULL!");
    }
    else if (*listMPHandle == NULL) {
        /*! @retval LISTMPSHAREDMEMORY_E_INVALIDARG *handle passed is NULL*/
        status = LISTMPSHAREDMEMORY_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_delete",
                             status,
                             "*handle passed is NULL!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.deleteInstance.handle =                 (
                (ListMPSharedMemory_Obj *)
                (((ListMPSharedMemory_Object *)(*listMPHandle))->obj)
                )->knlObject;

        status = ListMPSharedMemoryDrv_ioctl (CMD_LISTMPSHAREDMEMORY_DELETE,
                                              &cmdArgs);

        Memory_free (NULL,
                     ((ListMPSharedMemory_Object *)(*listMPHandle))->obj,
                     sizeof (ListMPSharedMemory_Obj));

        Memory_free (NULL,
                      (*listMPHandle),
                      sizeof (ListMPSharedMemory_Object));
        *listMPHandle = NULL;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ListMPSharedMemory_delete", status);

    /*! @retval LISTMPSHAREDMEMORY_SUCCESS Operation Successful */
    return status;
}


/*!
 *  @brief      Amount of shared memory required for creation of each instance.
 *
 *  @param      listMPHandle  Handle to previously created/opened instance.
 *              One cache line for ListMP_Elem
 *              Second cache line for ListMP_Attrs
 *
 *  @sa         ListMPSharedMemory_enter
 */
Int
ListMPSharedMemory_sharedMemReq (ListMPSharedMemory_Params * params)
{
    Int retVal = 0u;

    GT_1trace (curTrace, GT_ENTER, "ListMPSharedMemory_sharedMemReq", params);

    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
   if (params == NULL) {
        /*! @retval LISTMPSHAREDMEMORY_E_INVALIDARG handle passed is NULL*/
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_sharedMemReq",
                             LISTMPSHAREDMEMORY_E_INVALIDARG,
                             "handle passed is NULL!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        if (params->listType == ListMP_Type_SHARED) {
            if (params != NULL) {
                retVal = (LISTMPSHAREDMEMORY_CACHESIZE * 2u);
            }
        }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ListMPSharedMemory_sharedMemReq", retVal);

    /*! @retval     ((1u * sizeof(ListMP_Elem))
     *!            +  1u * sizeof(ListMP_Attrs)) if params is null */
    /*! @retval (2u * params->cacheSize) if params is provided */
    /*! @retval (0u) if hardware queue */

    return retVal;
}


/*!
 *  @brief      Function to open an ListMPSharedMemory instance
 *
 *  @param      params  Handle to listMP instance
 *
 *  @sa         ListMPSharedMemory_close
 */
Int ListMPSharedMemory_open (ListMPSharedMemory_Handle       * listMPHandle,
                             ListMPSharedMemory_Params       * params)
{
    Int                  status = LISTMPSHAREDMEMORY_SUCCESS;
    ListMPSharedMemoryDrv_CmdArgs cmdArgs;
    Int32                         index;

    GT_2trace (curTrace,
               GT_ENTER,
               "ListMPSharedMemory_open",
               listMPHandle,
               params);

    GT_assert (curTrace, (params != NULL));
    GT_assert (curTrace, (listMPHandle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (ListMPSharedMemory_state.setupRefCount == 0) {
        /*! @retval LISTMPSHAREDMEMORY_E_INVALIDSTATE
         *          Module is not initialized
         */
        status = LISTMPSHAREDMEMORY_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_open",
                             status,
                             "Module is not initialized!");
    }
    else if (params == NULL) {
        /*! @retval LISTMPSHAREDMEMORY_E_INVALIDARG params passed is NULL */
        status = LISTMPSHAREDMEMORY_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_open",
                             status,
                             "params passed is NULL!");
    }
    else if (listMPHandle == NULL) {
        /*! @retval LISTMPSHAREDMEMORY_E_INVALIDARG
         *          Invalid NULL listMPHandle pointer specified
         */
        status = LISTMPSHAREDMEMORY_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_open",
                             status,
                             "Invalid NULL listMPHandle pointer specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.open.params = (ListMPSharedMemory_Params *) params;
        index = SharedRegion_getIndex (params->sharedAddr);
        cmdArgs.args.open.sharedAddrSrPtr = SharedRegion_getSRPtr (
                                                      params->sharedAddr,
                                                      index);

        if (params->name != NULL) {
            cmdArgs.args.open.nameLen = (String_len (params->name)+ 1u);
        }
        else {
            cmdArgs.args.open.nameLen = 0;
        }
        cmdArgs.args.open.knlGate = Gate_getKnlHandle (params->gate);
        status = ListMPSharedMemoryDrv_ioctl (CMD_LISTMPSHAREDMEMORY_OPEN,
                                              &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            /* LISTMPSHAREDMEMORY_E_NOTFOUND is an expected run-time failure. */
            if (status != LISTMPSHAREDMEMORY_E_NOTFOUND) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "ListMPSharedMemory_open",
                                     status,
                                     "API (through IOCTL) failed on "
                                     "kernel-side!");
            }
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
           status = _ListMPSharedMemory_create (listMPHandle,cmdArgs,FALSE);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ListMPSharedMemory_open", status);

    /*! @retval LISTMPSHAREDMEMORY_SUCCESS Operation Successful */
    return status;
}


/*!
 *  @brief      Function to close a previously opened instance
 *
 *  @param      params  Handle to listMP instance
 *
 *  @sa         ListMPSharedMemory_open
 */
Int  ListMPSharedMemory_close (ListMPSharedMemory_Handle * listMPHandle)
{
    Int                        status = LISTMPSHAREDMEMORY_SUCCESS;
    ListMPSharedMemoryDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "ListMPSharedMemory_close", listMPHandle);

    GT_assert (curTrace, (*listMPHandle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (ListMPSharedMemory_state.setupRefCount == 0) {
        /* @retval LISTMPSHAREDMEMORY_E_INVALIDSTATE
         *         Module is not initialized
         */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_create",
                             LISTMPSHAREDMEMORY_E_INVALIDSTATE,
                             "Module is not initialized!");
    }
    else if (listMPHandle == NULL) {
        /*! @retval LISTMPSHAREDMEMORY_E_INVALIDARG handle passed is NULL*/
        status = LISTMPSHAREDMEMORY_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_close",
                             status,
                             "handle passed is NULL!");
    }
    else if (*listMPHandle == NULL) {
        /*! @retval LISTMPSHAREDMEMORY_E_INVALIDARG *handle passed is NULL*/
        status = LISTMPSHAREDMEMORY_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_close",
                             status,
                             "*handle passed is NULL!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.close.handle =                 (
                (ListMPSharedMemory_Obj *)
                (((ListMPSharedMemory_Object *)(*listMPHandle))->obj)
                )->knlObject;

        status = ListMPSharedMemoryDrv_ioctl (CMD_LISTMPSHAREDMEMORY_CLOSE,
                                              &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ListMPSharedMemory_close",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

        Memory_free (NULL,
                     ((ListMPSharedMemory_Object *)(*listMPHandle))->obj,
                     sizeof (ListMPSharedMemory_Obj));

        Memory_free (NULL,
                     (*listMPHandle),
                     sizeof (ListMPSharedMemory_Object));

        *listMPHandle = NULL;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ListMPSharedMemory_close", status);

    /*! @retval LISTMPSHAREDMEMORY_SUCCESS Operation Successful */
    return status;
}


/*!
 *  @brief      Function to check if list is empty
 *
 *  @param      listMPHandle Handle to listMP instance
 *
 *  @sa         none
 */
Bool
ListMPSharedMemory_empty (ListMPSharedMemory_Handle  listMPHandle)
{
    Bool                        isEmpty  = FALSE;
    Int                         status   = LISTMPSHAREDMEMORY_SUCCESS;
    ListMPSharedMemoryDrv_CmdArgs  cmdArgs;
    ListMPSharedMemory_Object *    handle = NULL;

    GT_1trace (curTrace, GT_ENTER, "ListMPSharedMemory_empty", listMPHandle);

    GT_assert (curTrace, (listMPHandle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (ListMPSharedMemory_state.setupRefCount == 0) {
        /* @retval FALSE
         *         Module is not initialized
         */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_create",
                             LISTMPSHAREDMEMORY_E_INVALIDSTATE,
                             "Module is not initialized!");
    }
    else if (listMPHandle == NULL) {
        /* @retval FALSE Invalid NULL listMPHandle
         *         pointer specified
         */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_empty",
                             LISTMPSHAREDMEMORY_E_INVALIDARG,
                             "Invalid NULL listMPHandle pointer specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

        handle = (ListMPSharedMemory_Object *)listMPHandle;
        cmdArgs.args.isEmpty.handle = ((ListMPSharedMemory_Obj *)
                                                      (handle->obj))->knlObject;
        status = ListMPSharedMemoryDrv_ioctl (CMD_LISTMPSHAREDMEMORY_ISEMPTY,
                                              &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ListMPSharedMemory_empty",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            isEmpty = cmdArgs.args.isEmpty.isEmpty ;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ListMPSharedMemory_empty", isEmpty);

    /*! @retval True if list is empty */
    /*! @retval False if list is not empty */
     return (isEmpty);
}


/*!
 *  @brief      Function to get head element from list
 *
 *  @param      listMPHandle Handle to listMP instance
 *
 *  @sa         ListMPSharedMemory_getTail
 */
Ptr ListMPSharedMemory_getHead (ListMPSharedMemory_Handle listMPHandle)
{
    Int                         status = LISTMPSHAREDMEMORY_SUCCESS;
    ListMP_Elem               * elem   = NULL;
    ListMPSharedMemoryDrv_CmdArgs  cmdArgs;
    ListMPSharedMemory_Object *    handle = NULL;

    GT_1trace (curTrace, GT_ENTER, "ListMPSharedMemory_getHead", listMPHandle);

    GT_assert (curTrace, (listMPHandle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (ListMPSharedMemory_state.setupRefCount == 0) {
        /* @retval NULL
         *         Module is not initialized
         */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_getHead",
                             LISTMPSHAREDMEMORY_E_INVALIDSTATE,
                             "Module is not initialized!");
    }
    else if (listMPHandle == NULL) {
        /* @retval NULL Invalid NULL listMPHandle
         *         pointer specified
         */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_getHead",
                             LISTMPSHAREDMEMORY_E_INVALIDARG,
                             "Invalid NULL listMPHandle pointer specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        handle = (ListMPSharedMemory_Object *)listMPHandle;
        cmdArgs.args.getHead.handle = ((ListMPSharedMemory_Obj *)
                                                      (handle->obj))->knlObject;
        status = ListMPSharedMemoryDrv_ioctl (CMD_LISTMPSHAREDMEMORY_GETHEAD,
                                              &cmdArgs);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ListMPSharedMemory_getHead",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            if (cmdArgs.args.getHead.elemSrPtr != SHAREDREGION_INVALIDSRPTR) {
                elem = (ListMP_Elem *) SharedRegion_getPtr(
                                                cmdArgs.args.getHead.elemSrPtr);
            }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ListMPSharedMemory_getHead", elem);

    /*! @retval Valid head element if Operation Successful */
    return elem;
}


/*!
 *  @brief      Function to get tail element from list
 *
 *  @param      listMPHandle Handle to listMP instance
 *
 *  @sa         ListMPSharedMemory_getHead
 */
Ptr ListMPSharedMemory_getTail (ListMPSharedMemory_Handle listMPHandle)
{
    Int                         status = LISTMPSHAREDMEMORY_SUCCESS;
    ListMP_Elem               * elem   = NULL;
    ListMPSharedMemoryDrv_CmdArgs  cmdArgs;
    ListMPSharedMemory_Object *    handle = NULL;

    GT_1trace (curTrace, GT_ENTER, "ListMPSharedMemory_getTail", listMPHandle);

    GT_assert (curTrace, (listMPHandle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (ListMPSharedMemory_state.setupRefCount == 0) {
        /* @retval NULL
         *         Module is not initialized
         */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_getTail",
                             LISTMPSHAREDMEMORY_E_INVALIDSTATE,
                             "Module is not initialized!");
    }
    else if (listMPHandle == NULL) {
        /* @retval NULL Invalid NULL listMPHandle
         *         pointer specified
         */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_getTail",
                             LISTMPSHAREDMEMORY_E_INVALIDARG,
                             "Invalid NULL listMPHandle pointer specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        handle = (ListMPSharedMemory_Object *)listMPHandle;
        cmdArgs.args.getTail.handle = ((ListMPSharedMemory_Obj *)
                                                      (handle->obj))->knlObject;
        status = ListMPSharedMemoryDrv_ioctl (CMD_LISTMPSHAREDMEMORY_GETTAIL,
                                              &cmdArgs);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ListMPSharedMemory_getTail",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            if (cmdArgs.args.getTail.elemSrPtr != SHAREDREGION_INVALIDSRPTR) {
                elem = (ListMP_Elem *) SharedRegion_getPtr(
                                                cmdArgs.args.getTail.elemSrPtr);
            }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ListMPSharedMemory_getTail", elem);

    /*! @retval Valid tail element if Operation Successful */
    return elem;
}


/*!
 *  @brief      Function to put head element into list
 *
 *  @param      params  Handle to listMP instance
 *
 *  @sa         ListMPSharedMemory_putTail
 */
Int ListMPSharedMemory_putHead (ListMPSharedMemory_Handle  listMPHandle,
                                ListMP_Elem              * elem)
{
    Int                  status = LISTMPSHAREDMEMORY_SUCCESS;
    ListMPSharedMemoryDrv_CmdArgs  cmdArgs;
    ListMPSharedMemory_Object *    handle = NULL;
    Int32                          index;


    GT_1trace (curTrace, GT_ENTER, "ListMPSharedMemory_putHead", listMPHandle);

    GT_assert (curTrace, (listMPHandle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (ListMPSharedMemory_state.setupRefCount == 0) {
        /*! @retval LISTMPSHAREDMEMORY_E_INVALIDSTATE Module
         *  was not initialized
         */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_putHead",
                             LISTMPSHAREDMEMORY_E_INVALIDSTATE,
                             "Module is not initialized!");
    }
    else if (listMPHandle == NULL) {
        /*! @retval LISTMPSHAREDMEMORY_E_INVALIDARG
         *          elem pointer passed is NULL
         */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_putHead",
                             LISTMPSHAREDMEMORY_E_INVALIDARG,
                             "Invalid NULL listMPHandle pointer specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        handle = (ListMPSharedMemory_Object *)listMPHandle;
        cmdArgs.args.putHead.handle = ((ListMPSharedMemory_Obj *)
                                                      (handle->obj))->knlObject;

        index = SharedRegion_getIndex (elem);
        cmdArgs.args.putHead.elemSrPtr =  SharedRegion_getSRPtr (elem,index);
        status = ListMPSharedMemoryDrv_ioctl (CMD_LISTMPSHAREDMEMORY_PUTHEAD,
                                              &cmdArgs);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ListMPSharedMemory_putHead",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ListMPSharedMemory_putHead", status);

    /*! @retval LISTMPSHAREDMEMORY_SUCCESS Operation Successful*/
    return status;
}


/*!
 *  @brief      Function to put tail element into list
 *
 *  @param      listMPHandle Handle to listMP instance
 *  @param      elem         ListMP_Elem element to be added at tail
 *
 *  @sa         ListMPSharedMemory_putHead
 */
Int ListMPSharedMemory_putTail (ListMPSharedMemory_Handle     listMPHandle,
                                ListMP_Elem                 * elem)
{
    Int                         status = LISTMPSHAREDMEMORY_SUCCESS;
    ListMPSharedMemoryDrv_CmdArgs  cmdArgs;
    Int32                          index;
    ListMPSharedMemory_Object *    handle = NULL;

    GT_1trace (curTrace, GT_ENTER, "ListMPSharedMemory_putTail", listMPHandle);

    GT_assert (curTrace, (listMPHandle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (ListMPSharedMemory_state.setupRefCount == 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_putTail",
                             LISTMPSHAREDMEMORY_E_INVALIDSTATE,
                             "Module is not initialized!");
    }
    else if (listMPHandle == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_putTail",
                             LISTMPSHAREDMEMORY_E_INVALIDARG,
                             "Invalid NULL listMPHandle pointer specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        handle = (ListMPSharedMemory_Object *)listMPHandle;
        cmdArgs.args.putTail.handle = ((ListMPSharedMemory_Obj *)
                                                      (handle->obj))->knlObject;
        index = SharedRegion_getIndex (elem);
        cmdArgs.args.putTail.elemSrPtr =  SharedRegion_getSRPtr (elem,index);

        status = ListMPSharedMemoryDrv_ioctl (CMD_LISTMPSHAREDMEMORY_PUTTAIL,
                                              &cmdArgs);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ListMPSharedMemory_putTail",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ListMPSharedMemory_putTail", status);

    /*! @retval LISTMPSHAREDMEMORY_SUCCESS Operation Successful*/
    return status;
}


/*!
 *  @brief      Function to insert element into list
 *
 *  @param      params  Handle to listMP instance
 *  @param      params  Element to be inserted
 *  @param      params  Current element before which element to be inserted
 *                      If element is NULL, head element is used.
 *
 *  @sa         ListMPSharedMemory_putHead
 */
Int ListMPSharedMemory_insert (ListMPSharedMemory_Handle  listMPHandle,
                               ListMP_Elem              * newElem,
                               ListMP_Elem              * curElem)
{
    Int                         status = LISTMPSHAREDMEMORY_SUCCESS;
    ListMPSharedMemoryDrv_CmdArgs  cmdArgs;
    ListMPSharedMemory_Object *    handle = NULL;
    Int32                          index;

    GT_1trace (curTrace, GT_ENTER, "ListMPSharedMemory_insert", listMPHandle);

    GT_assert (curTrace, (listMPHandle != NULL));
    GT_assert (curTrace, (newElem != NULL));
    GT_assert (curTrace, (curElem != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (ListMPSharedMemory_state.setupRefCount == 0) {
        /* @retval LISTMPSHAREDMEMORY_E_INVALIDSTATE
         *         Module is not initialized
         */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_insert",
                             LISTMPSHAREDMEMORY_E_INVALIDSTATE,
                             "Module is not initialized!");
    }
    else if (listMPHandle == NULL) {
        /* @retval LISTMPSHAREDMEMORY_E_INVALIDARG Invalid NULL listMPHandle
         *         pointer specified
         */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_insert",
                             LISTMPSHAREDMEMORY_E_INVALIDARG,
                             "Invalid NULL listMPHandle pointer specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        handle = (ListMPSharedMemory_Object *)listMPHandle;
        cmdArgs.args.insert.handle = ((ListMPSharedMemory_Obj *)
                                                      (handle->obj))->knlObject;

        index = SharedRegion_getIndex (newElem);
        cmdArgs.args.insert.newElemSrPtr =SharedRegion_getSRPtr (newElem,index);
        index = SharedRegion_getIndex (curElem);
        cmdArgs.args.insert.curElemSrPtr =SharedRegion_getSRPtr (curElem,index);
        status = ListMPSharedMemoryDrv_ioctl (CMD_LISTMPSHAREDMEMORY_INSERT,
                                              &cmdArgs);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ListMPSharedMemory_insert",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    /*! @retval LISTMPSHAREDMEMORY_SUCCESS Operation Successful*/
    return status;
}


/*!
 *  @brief      Function to remove element from list
 *
 *  @param      params  Handle to listMP instance
 *  @param      params  Element to be removed
 *
 *  @sa         ListMPSharedMemory_insert
 */
Int ListMPSharedMemory_remove (ListMPSharedMemory_Handle   listMPHandle,
                               ListMP_Elem               * elem)
{
    Int                         status = LISTMPSHAREDMEMORY_SUCCESS;
    ListMPSharedMemoryDrv_CmdArgs  cmdArgs;
    ListMPSharedMemory_Object *    handle = NULL;
    Int32                          index;

    GT_1trace (curTrace, GT_ENTER, "ListMPSharedMemory_remove", listMPHandle);

    GT_assert (curTrace, (listMPHandle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (ListMPSharedMemory_state.setupRefCount == 0) {
        /* @retval LISTMPSHAREDMEMORY_E_INVALIDSTATE
         *         Module is not initialized
         */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_remove",
                             LISTMPSHAREDMEMORY_E_INVALIDSTATE,
                             "Module is not initialized!");
    }
    else if (listMPHandle == NULL) {
        /* @retval LISTMPSHAREDMEMORY_E_INVALIDARG Invalid NULL listMPHandle
         *         pointer specified
         */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_remove",
                             LISTMPSHAREDMEMORY_E_INVALIDARG,
                             "Invalid NULL listMPHandle pointer specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        handle = (ListMPSharedMemory_Object *)listMPHandle;
        cmdArgs.args.remove.handle = ((ListMPSharedMemory_Obj *)
                                                      (handle->obj))->knlObject;

        index = SharedRegion_getIndex (elem);
        cmdArgs.args.remove.elemSrPtr = SharedRegion_getSRPtr (elem,index);

        status = ListMPSharedMemoryDrv_ioctl (CMD_LISTMPSHAREDMEMORY_REMOVE,
                                              &cmdArgs);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ListMPSharedMemory_remove",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ListMPSharedMemory_remove", status);

    /*! @retval LISTMPSHAREDMEMORY_SUCCESS Operation Successful*/
    return status;
}


/*!
 *  @brief      Function to traverse to next element in list
 *
 *  @param      listMPHandle Handle to listMP instance
 *  @param      elem         ListMP_Elem element whose next
 *                           is to be traversed. If NULL
 *                           traversal starts after head
 *
 *  @sa         ListMPSharedMemory_prev
 */
Ptr ListMPSharedMemory_next (ListMPSharedMemory_Handle     listMPHandle,
                             ListMP_Elem                 * elem)
{
    Int                 status = LISTMPSHAREDMEMORY_SUCCESS;
    Ptr                 next   = NULL;
    ListMPSharedMemoryDrv_CmdArgs  cmdArgs;
    ListMPSharedMemory_Object *    handle = NULL;
    Int32                          index;

    GT_1trace (curTrace, GT_ENTER, "ListMPSharedMemory_next", listMPHandle);

    GT_assert (curTrace, (listMPHandle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (ListMPSharedMemory_state.setupRefCount == 0) {
        /* @retval NULL Module is not initialized */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_next",
                             LISTMPSHAREDMEMORY_E_INVALIDSTATE,
                             "Module is not initialized!");
    }
    else if (listMPHandle == NULL) {
        /* @retval NULL Invalid NULL listMPHandle pointer specified */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_next",
                             LISTMPSHAREDMEMORY_E_INVALIDARG,
                             "Invalid NULL listMPHandle pointer specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        handle = (ListMPSharedMemory_Object *)listMPHandle;
        cmdArgs.args.next.handle = ((ListMPSharedMemory_Obj *)
                                                      (handle->obj))->knlObject;

        if (elem != NULL){
            index = SharedRegion_getIndex (elem);
            cmdArgs.args.next.elemSrPtr = SharedRegion_getSRPtr (elem,index);
        }
        else{
            cmdArgs.args.next.elemSrPtr = SHAREDREGION_INVALIDSRPTR;
        }

        status = ListMPSharedMemoryDrv_ioctl (CMD_LISTMPSHAREDMEMORY_NEXT,
                                              &cmdArgs);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ListMPSharedMemory_next",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            if (cmdArgs.args.next.nextElemSrPtr != SHAREDREGION_INVALIDSRPTR) {
                next = (ListMP_Elem *)SharedRegion_getPtr(
                                               cmdArgs.args.next.nextElemSrPtr);
            }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ListMPSharedMemory_next", next);

    /*! @retval Next-element if Operation Successful */
    return next;
}


/*!
 *  @brief      Function to traverse to prev element in list
 *
 *  @param      params  Handle to listMP instance
 *  @param      params  Element whose prev is to be retrieved
 *                      If NULL starts at head element
 *
 *  @sa         ListMPSharedMemory_next
 */
Ptr ListMPSharedMemory_prev (ListMPSharedMemory_Handle    listMPHandle,
                             ListMP_Elem                * elem)
{
    Int                 status = LISTMPSHAREDMEMORY_SUCCESS;
    Ptr                 prev   = NULL;
    ListMPSharedMemoryDrv_CmdArgs  cmdArgs;
    ListMPSharedMemory_Object *    handle = NULL;
    Int32                          index;

    GT_1trace (curTrace, GT_ENTER, "ListMPSharedMemory_prev", listMPHandle);

    GT_assert (curTrace, (listMPHandle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (ListMPSharedMemory_state.setupRefCount == 0) {
        /* @retval NULL Module is not initialized */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_prev",
                             LISTMPSHAREDMEMORY_E_INVALIDSTATE,
                             "Module is not initialized!");
    }
    else if (listMPHandle == NULL) {
        /* @retval NULL Invalid NULL listMPHandle pointer specified */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_prev",
                             LISTMPSHAREDMEMORY_E_INVALIDARG,
                             "Invalid NULL listMPHandle pointer specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        handle = (ListMPSharedMemory_Object *)listMPHandle;

        cmdArgs.args.prev.handle = ((ListMPSharedMemory_Obj *)
                                                      (handle->obj))->knlObject;
        if(elem != NULL){
            index = SharedRegion_getIndex (elem);
            cmdArgs.args.prev.elemSrPtr = SharedRegion_getSRPtr (elem,index);
        }
        else{
            cmdArgs.args.prev.elemSrPtr = SHAREDREGION_INVALIDSRPTR;
        }

        status = ListMPSharedMemoryDrv_ioctl (CMD_LISTMPSHAREDMEMORY_PREV,
                                              &cmdArgs);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ListMPSharedMemory_prev",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            if (cmdArgs.args.prev.prevElemSrPtr != SHAREDREGION_INVALIDSRPTR) {
                prev = (ListMP_Elem *)SharedRegion_getPtr(
                                               cmdArgs.args.prev.prevElemSrPtr);
            }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "ListMPSharedMemory_prev", prev);

    /*! @retval Previous-element if Operation Successful */
    return prev;
}


/*=============================================================================
    Internal functions
  =============================================================================
*/
/*!
 *  @brief      Createa and populates handle and obj.
 *
 *  @param      listMPHandle  pointer to ListMPSharedMemory_Handle
 *  @param      cmdArgs       command areguments
 *  @param      createFlag    Distiguistes the caller
 *
 *  @sa         ListMPSharedMemory_delete
 */
Int32
 _ListMPSharedMemory_create(ListMPSharedMemory_Handle     * listMPHandle,
                            ListMPSharedMemoryDrv_CmdArgs   cmdArgs,
                            UInt16                          createFlag)
{
    ListMPSharedMemory_Obj *       obj;
    Int32                          status = LISTMPSHAREDMEMORY_SUCCESS;

    /* Allocate memory for the handle */
    *listMPHandle = (ListMPSharedMemory_Handle)
                     Memory_calloc (NULL,
                                    sizeof (ListMPSharedMemory_Object),
                                    0);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (*listMPHandle == NULL) {
        /*! @retval LISTMPSHAREDMEMORY_E_MEMORY Memory allocation
         *          failed for handle
         */
        status = LISTMPSHAREDMEMORY_E_MEMORY;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ListMPSharedMemory_create",
                             status,
                             "Memory allocation failed for handle!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Set pointer to kernel object into the user handle. */
        obj = (ListMPSharedMemory_Obj *)Memory_calloc(NULL,
                                        sizeof (ListMPSharedMemory_Obj),
                                         0);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (obj == NULL) {
            /*! @retval NULL Memory allocation failed for handle */
            status = LISTMPSHAREDMEMORY_E_MEMORY;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ListMPSharedMemory_create",
                                 status,
                                 "Memory allocation failed for obj!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            if(createFlag == TRUE){
                obj->knlObject = cmdArgs.args.create.handle;
            }
            else{
                obj->knlObject = cmdArgs.args.open.handle;
            }


            ((ListMPSharedMemory_Object *)(*listMPHandle))->obj = obj;

            ((ListMPSharedMemory_Object *)(*listMPHandle))->listType =
                                                     ListMP_Type_SHARED;

            ((ListMPSharedMemory_Object *)(*listMPHandle))->empty   =
                                              &ListMPSharedMemory_empty;
            ((ListMPSharedMemory_Object *)(*listMPHandle))->getHead =
                                            &ListMPSharedMemory_getHead;
            ((ListMPSharedMemory_Object *)(*listMPHandle))->getTail =
                                            &ListMPSharedMemory_getTail;
            ((ListMPSharedMemory_Object *)(*listMPHandle))->putHead =
                                            &ListMPSharedMemory_putHead;
            ((ListMPSharedMemory_Object *)(*listMPHandle))->putTail =
                                            &ListMPSharedMemory_putTail;
            ((ListMPSharedMemory_Object *)(*listMPHandle))->insert  =
                                            &ListMPSharedMemory_insert;
            ((ListMPSharedMemory_Object *)(*listMPHandle))->remove  =
                                             &ListMPSharedMemory_remove;
            ((ListMPSharedMemory_Object *)(*listMPHandle))->next    =
                                               &ListMPSharedMemory_next;
            ((ListMPSharedMemory_Object *)(*listMPHandle))->prev    =
                                               &ListMPSharedMemory_prev;

#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    /*! @retval LISTMPSHAREDMEMORY_SUCCESS _ListMPSharedMemory_create successful
     */
    return(status);
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
