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
 *  @file   HeapBuf.c
 *
 *  @brief      Defines HeapBuf based memory allocator.
 *
 *  Heap implementation that manages fixed size buffers that can be used
 *  in a multiprocessor system with shared memory.<br>
 *  <br>
 *  The HeapBuf manager provides functions to allocate and free storage from a
 *  heap of type HeapBuf which inherits from Heap. HeapBuf manages a single
 *  fixed-size buffer, split into equally sized allocable blocks.<br>
 *  <br>
 *  The HeapBuf manager is intended as a very fast memory
 *  manager which can only allocate blocks of a single size. It is ideal for
 *  managing a heap that is only used for allocating a single type of object,
 *  or for objects that have very similar sizes.<br>
 *  <br>
 *  This module is instance based. Each instance requires shared memory
 *  (for the buffers and other internal state).  This is specified via the
 *  sharedAddr parameter to the create. The proper sharedAddrSize parameter
 *  can be determined via the #HeapBuf_sharedMemReq call. Note: the parameters
 *  to this function must be the same that will used to create the instance.<br>
 *  <br>
 *  The HeapBuf module uses a instance to store instance information
 *  when an instance is created and the name parameter is non-NULL.
 *  If a name is supplied, it must be unique for all HeapBuf instances.<br>
 *  <br>
 *  The create initializes the shared memory as needed. The shared memory must
 *  be initialized to 0 before the HeapBuf instance is created or
 *  opened.<br>
 *  <br>
 *  Once an instance is created, an open can be performed. The
 *  open is used to gain access to the same HeapBuf instance.
 *  Generally an instance is created on one processor and opened on the
 *  other processor(s).<br>
 *  <br>
 *  The open returns a HeapBuf instance handle like the create,
 *  however the open does not modify the shared memory.<br>
 *  <br>
 *  There are two options when opening the instance:<br>
 *  -Supply the same name as specified in the create. The HeapBuf module
 *  queries the NameServer to get the needed information.<br>
 *  -Supply the same sharedAddr value as specified in the create.<br>
 *  <br>
 *  If the open is called before the instance is created, open returns NULL.<br>
 *  <br>
 *  Constraints:<br>
 *  -Align parameter must be a power of 2.<br>
 *  -The buffer passed to dynamically create a HeapBuf must be aligned
 *   according to the alignment parameter, and must be large enough to account
 *   for the actual block size after it has been rounded up to a multiple of
 *   the alignment.
 *
 *  ============================================================================
 */



/* Standard headers */
#include <Std.h>

/* Utilities headers */
#include <Trace.h>
#include <Memory.h>
#include <String.h>
#include <string.h>

/* Module level headers */
#include <HeapBuf.h>
#include <_HeapBuf.h>
#include <HeapBufDrv.h>
#include <HeapBufDrvDefs.h>
#include <ListMPSharedMemory.h>
#include <SharedRegion.h>


#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 * Macros
 * =============================================================================
 */
/*!
 *  @brief  Cache size
 */
#define HEAPBUF_CACHESIZE              128u

/* =============================================================================
 * Structures & Enums
 * =============================================================================
 */
/*!
 *  @brief  Structure defining object for the HeapBuf
 */
typedef struct HeapBuf_Obj_tag {
    Ptr         knlObject;
    /*!< Pointer to the kernel-side HeapBuf object. */
} HeapBuf_Obj;

/*!
 *  @brief  Structure for HeapBuf module state
 */
typedef struct HeapBuf_ModuleObject_tag {
    UInt32              setupRefCount;
    /*!< Reference count for number of times setup/destroy were called in this
     *   process.
     */
} HeapBuf_ModuleObject;


/* =============================================================================
 *  Globals
 * =============================================================================
 */
/*!
 *  @var    HeapBuf_state
 *
 *  @brief  Heap Buf state object variable
 */
#if !defined(SYSLINK_BUILD_DEBUG)
static
#endif /* if !defined(SYSLINK_BUILD_DEBUG) */
HeapBuf_ModuleObject HeapBuf_state =
{
    .setupRefCount = 0
};


/* =============================================================================
 *  Internal functions
 * =============================================================================
 */
Int32
_HeapBuf_create (HeapBuf_Handle       * heapHandle,
                 HeapBufDrv_CmdArgs     cmdArgs,
                 Bool                   createFlag);



/* =============================================================================
 * APIS
 * =============================================================================
 */
/*!
 *  @brief      Get the default configuration for the HeapBuf module.
 *
 *              This function can be called by the application to get their
 *              configuration parameter to HeapBuf_setup filled in by
 *              the HeapBuf module with the default parameters. If the
 *              user does not wish to make any change in the default parameters,
 *              this API is not required to be called.
 *
 *  @param      cfgParams  Pointer to the HeapBuf module configuration
 *                         structure in which the default config is to be
 *                         returned.
 *
 *  @sa         HeapBuf_setup
 */
Void
HeapBuf_getConfig (HeapBuf_Config * cfgParams)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                status = HEAPBUF_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    HeapBufDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "HeapBuf_getConfig", cfgParams);

    GT_assert (curTrace, (cfgParams != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (cfgParams == NULL) {
        status = HEAPBUF_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_getConfig",
                             status,
                             "Argument of type (HeapBuf_Config *) passed "
                             "is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Temporarily open the handle to get the configuration. */
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        HeapBufDrv_open ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "HeapBuf_getConfig",
                                 status,
                                 "Failed to open driver handle!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            cmdArgs.args.getConfig.config = cfgParams;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            HeapBufDrv_ioctl (CMD_HEAPBUF_GETCONFIG, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                  GT_4CLASS,
                                  "HeapBuf_getConfig",
                                  status,
                                  "API (through IOCTL) failed on kernel-side!");

            }
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Close the driver handle. */
        HeapBufDrv_close ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "HeapBuf_getConfig");
}


/*!
 *  @brief      Setup the HeapBuf module.
 *
 *              This function sets up the HeapBuf module. This function
 *              must be called before any other instance-level APIs can be
 *              invoked.
 *              Module-level configuration needs to be provided to this
 *              function. If the user wishes to change some specific config
 *              parameters, then HeapBuf_getConfig can be called to get
 *              the configuration filled with the default values. After this,
 *              only the required configuration values can be changed. If the
 *              user does not wish to make any change in the default parameters,
 *              the application can simply call HeapBuf_setup with NULL
 *              parameters. The default parameters would get automatically used.
 *
 *  @sa        HeapBuf_destroy
 */
Int
HeapBuf_setup (const HeapBuf_Config * config)
{
    Int                status = HEAPBUF_SUCCESS;
    HeapBufDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "HeapBuf_setup", config);

    /* TBD: Protect from multiple threads. */
    HeapBuf_state.setupRefCount++;

    /* This is needed at runtime so should not be in
     * SYSLINK_BUILD_OPTIMIZE.
     */
    if (HeapBuf_state.setupRefCount > 1) {
        /*! @retval HEAPBUF_S_ALREADYSETUP Success: HeapBuf module has been
         *          already setup in this process
         */
        status = HEAPBUF_S_ALREADYSETUP;
        GT_1trace (curTrace,
                GT_1CLASS,
                "HeapBuf module has been already setup in this process.\n"
                " RefCount: [%d]\n",
                HeapBuf_state.setupRefCount);
    }
    else {
        /* Open the driver handle. */
        status = HeapBufDrv_open ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "HeapBuf_setup",
                                 status,
                                 "Failed to open driver handle!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            cmdArgs.args.setup.config = (HeapBuf_Config *) config;
            status = HeapBufDrv_ioctl (CMD_HEAPBUF_SETUP, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason
                       (curTrace,
                        GT_4CLASS,
                        "HeapBuf_setup",
                        status,
                        "API (through IOCTL) failed on kernel-side!");
            }
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

    GT_1trace (curTrace, GT_LEAVE, "HeapBuf_setup", status);

    /*! @retval HEAPBUF_SUCCESS Operation Successful */
    return status;
}


/*!
 *  @brief      Function to destroy the HeapBuf module.
 *
 *  @param      None
 *
 *  @sa         HeapBuf_create
 */
Int
HeapBuf_destroy (void)
{
    Int                status = HEAPBUF_SUCCESS;
    HeapBufDrv_CmdArgs    cmdArgs;

    GT_0trace (curTrace, GT_ENTER, "HeapBuf_destroy");

    /* TBD: Protect from multiple threads. */
    HeapBuf_state.setupRefCount--;

    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */
    if (HeapBuf_state.setupRefCount >= 1) {
        /*! @retval HEAPBUF_S_SETUP Success: HeapBuf module has been setup
                                             by other clients in this process */
        status = HEAPBUF_S_SETUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "HeapBuf module has been setup by other clients in this"
                   " process.\n"
                   "    RefCount: [%d]\n",
                   (HeapBuf_state.setupRefCount + 1));
    }
    else {
        status = HeapBufDrv_ioctl (CMD_HEAPBUF_DESTROY, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "HeapBuf_destroy",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

        /* Close the driver handle. */
        HeapBufDrv_close ();
    }

    GT_1trace (curTrace, GT_LEAVE, "HeapBuf_destroy", status);

    /*! @retval HEAPBUF_SUCCESS Operation Successful */
    return status;
}


/*!
 *  @brief      Initialize this config-params structure with supplier-specified
 *              defaults before instance creation.
 *
 *  @param      handle  Handle to previously created HeapBuf instance
 *  @param      params  Instance config-params structure.
 *
 *  @sa         HeapBuf_create
 */
Void
HeapBuf_Params_init (HeapBuf_Handle         handle,
                     HeapBuf_Params       * params)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int               status = HEAPBUF_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    HeapBufDrv_CmdArgs   cmdArgs;
    HeapBuf_Obj *        obj;

    GT_1trace (curTrace, GT_ENTER, "HeapBuf_Params_init", params);

    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (HeapBuf_state.setupRefCount == 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_Params_init",
                             HEAPBUF_E_INVALIDSTATE,
                             "Modules is not initialized!");
    }
    else if (params == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_Params_init",
                             HEAPBUF_E_INVALIDARG,
                             "Argument of type (HeapBuf_Params *) passed "
                             "is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        if (handle != NULL) {
            obj = (HeapBuf_Obj *) (((HeapBuf_Object *) handle)->obj);
            cmdArgs.args.ParamsInit.handle = obj->knlObject;
        }
        else {
            cmdArgs.args.ParamsInit.handle = handle;
        }
        cmdArgs.args.ParamsInit.params = params;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        HeapBufDrv_ioctl (CMD_HEAPBUF_PARAMS_INIT, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "HeapBuf_Params_init",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "HeapBuf_Params_init");
}


/*!
 *  @brief      Creates a new instance of HeapBuf module.
 *
 *  @param      params  Instance config-params structure.
 *
 *  @sa         HeapBuf_delete, HeapBuf_Params_init
 *              ListMPSharedMemory_sharedMemReq Gate_enter
 *              Gate_leave GateMutex_delete NameServer_addUInt32
 */
HeapBuf_Handle
HeapBuf_create (const HeapBuf_Params * params)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                  status = 0;
    UInt32               bufSize = 0;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    HeapBuf_Handle       handle = NULL;
    HeapBufDrv_CmdArgs   cmdArgs;
    Int32                index;

    GT_1trace (curTrace, GT_ENTER, "HeapBuf_create", params);

    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (HeapBuf_state.setupRefCount == 0) {
        /* @retval  NULL Module is not initialized */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_create",
                             HEAPBUF_E_INVALIDSTATE,
                             "Modules is not initialized!");
    }
    else if (params == NULL) {
        /* @retval  NULL Invalid NULL params pointer specified */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_create",
                             HEAPBUF_E_INVALIDARG,
                             "Invalid NULL params pointer specified!");
    }
    else if (   (params->sharedAddr == (UInt32) NULL)
             || (params->sharedBuf  == (UInt32) NULL)) {
        /* @retval  NULL Invalid NULL shared region address sharedAddr or
                         sharedBuf */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_create",
                             HEAPBUF_E_INVALIDARG,
                             "Invalid NULL shared region address sharedAddr or "
                             "sharedBuf!");
    }
    else if (  params->sharedAddrSize
             < HeapBuf_sharedMemReq (params,&bufSize)) {
        /* @retval  NULL params->sharedAddrSize less than module
         *          size requirements
         */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_create",
                             HEAPBUF_E_INVALIDARG,
                             "params->sharedAddrSize less than module"
                            " size requirements!");

    }
    else if (params->sharedBufSize < bufSize) {
        /*! @retval NULL if Shared memory size is less than required */
        status = HEAPBUF_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_create",
                             status,
                             "params->sharedBufSize is less than bufSize!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.create.params = (HeapBuf_Params *) params;

        /* Translate sharedAddr to SrPtr. */
        GT_1trace (curTrace,
                   GT_2CLASS,
                   "    HeapBuf_create: Shared addr [0x%x]\n",
                   params->sharedAddr);
        index = SharedRegion_getIndex (params->sharedAddr);
        cmdArgs.args.create.sharedAddrSrPtr = SharedRegion_getSRPtr (
                                                            params->sharedAddr,
                                                            index);
        GT_1trace (curTrace,
                   GT_2CLASS,
                   "    HeapBuf_create: Shared addr SrPtr [0x%x]\n",
                   cmdArgs.args.create.sharedAddrSrPtr);

        /* Translate sharedAddr to SrPtr. */
        GT_1trace (curTrace,
                   GT_2CLASS,
                   "    HeapBuf_create: Shared buffer addr [0x%x]\n",
                   params->sharedBuf);
        index = SharedRegion_getIndex (params->sharedBuf);
        cmdArgs.args.create.sharedBufSrPtr = SharedRegion_getSRPtr (
                                                            params->sharedBuf,
                                                            index);
        GT_1trace (curTrace,
                   GT_2CLASS,
                   "    HeapBuf_create: Shared buffer addr SrPtr [0x%x]\n",
                   cmdArgs.args.create.sharedBufSrPtr);
        if (params->name != NULL) {
            cmdArgs.args.create.nameLen = (strlen (params->name) + 1);
        }
        else {
            cmdArgs.args.create.nameLen = 0;
        }

        /* Translate Gate handle to kernel-side gate handle. */
        cmdArgs.args.create.knlGate = Gate_getKnlHandle (
                                                            params->gate);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        HeapBufDrv_ioctl (CMD_HEAPBUF_CREATE, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "HeapBuf_create",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            _HeapBuf_create (&handle, cmdArgs, TRUE);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "HeapBuf_create",
                                     status,
                                     "Heap creation failed on user-side!");
            }
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "HeapBuf_create", handle);

    /*! @retval valid-handle Operation Successful*/
    /*! @retval NULL Operation failed */
    return (HeapBuf_Handle) handle;
}


/*!
 *  @brief      Deletes an instance of HeapBuf module.
 *
 *  @param      hpHandle  Handle to previously created instance.
 *
 *  @sa         HeapBuf_create NameServer_remove Gate_enter
 *              Gate_leave GateMutex_delete ListMPSharedMemory_delete
 */
Int
HeapBuf_delete (HeapBuf_Handle * hpHandle)
{
    Int                status = HEAPBUF_SUCCESS;
    HeapBufDrv_CmdArgs cmdArgs;
    HeapBuf_Obj *      obj;

    GT_1trace (curTrace, GT_ENTER, "HeapBuf_delete", hpHandle);

    GT_assert (curTrace, (hpHandle != NULL));
    GT_assert (curTrace, ((hpHandle != NULL) && (*hpHandle != NULL)));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (HeapBuf_state.setupRefCount == 0) {
        /*! @retval HEAPBUF_E_INVALIDSTATE Modules is in an invalid state*/
        status = HEAPBUF_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_delete",
                             status,
                             "Modules is in an invalid state!");
    }
    else if (hpHandle == NULL) {
        /*! @retval HEAPBUF_E_INVALIDARG handle pointer passed is NULL*/
        status = HEAPBUF_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_delete",
                             status,
                             "handle pointer passed is NULL!");
    }
    else if (*hpHandle == NULL) {
        /*! @retval HEAPBUF_E_INVALIDARG handle passed is NULL*/
        status = HEAPBUF_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_delete",
                             status,
                             "*hpHandle passed is NULL!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        obj = (HeapBuf_Obj *) (((HeapBuf_Object *) (*hpHandle))->obj);
        cmdArgs.args.deleteInstance.handle = obj->knlObject;
        status = HeapBufDrv_ioctl (CMD_HEAPBUF_DELETE, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "HeapBuf_delete",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            Memory_free (NULL, obj, sizeof (HeapBuf_Obj));
            Memory_free (NULL, *hpHandle, sizeof (HeapBuf_Object));
            *hpHandle = NULL;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "HeapBuf_delete", status);

    /*! @retval HEAPBUF_SUCCESS Operation Successful */
    return status;
}


/*!
 *  @brief      Opens a created instance of HeapBuf module.
 *
 *  @param      hpHandle    Return value: HeapBuf handle
 *  @param      params      Parameters for opening the instance.
 *
 *  @sa         HeapBuf_create, HeapBuf_delete, HeapBuf_close
 */
Int32
HeapBuf_open (HeapBuf_Handle * hpHandle,
              HeapBuf_Params * params)
{
    Int32              status  = HEAPBUF_SUCCESS;
    Int32              index;
    HeapBufDrv_CmdArgs cmdArgs;

    GT_2trace (curTrace, GT_ENTER, "HeapBuf_open", hpHandle, params);

    GT_assert (curTrace, (params != NULL));
    GT_assert (curTrace, (hpHandle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (HeapBuf_state.setupRefCount == 0) {
        /*! @retval HEAPBUF_E_INVALIDSTATE Modules is in an invalid state */
        status = HEAPBUF_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_open",
                             status,
                             "Modules is in an invalid state!");
    }
    else if (params == NULL) {
        /*! @retval HEAPBUF_E_INVALIDARG params passed is NULL */
        status = HEAPBUF_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_open",
                             status,
                             "params passed is NULL!");
    }
    else if (hpHandle == NULL) {
        /*! @retval HEAPBUF_E_INVALIDARG hpHandle passed is NULL */
        status = HEAPBUF_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_open",
                             status,
                             "hpHandle passed is NULL!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.open.params = (HeapBuf_Params *) params;

        index = SharedRegion_getIndex (params->sharedAddr);
        cmdArgs.args.open.sharedAddrSrPtr = SharedRegion_getSRPtr (
                                                      params->sharedAddr,
                                                      index);

        if (params->name != NULL) {
            cmdArgs.args.open.nameLen = (strlen (params->name) + 1);
        }
        else {
            cmdArgs.args.open.nameLen = 0;
        }

        cmdArgs.args.open.knlGate =Gate_getKnlHandle (params->gate);

        status = HeapBufDrv_ioctl (CMD_HEAPBUF_OPEN, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "HeapBuf_open",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            status = _HeapBuf_create (hpHandle, cmdArgs, FALSE);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "HeapBuf_open",
                                     status,
                                   "_HeapBuf_create from HeapBuf_open failed!");
            }
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "HeapBuf_open", status);

    /*! @retval HEAPBUF_SUCCESS Operation Successful */
    return status;
}


/*!
 *  @brief      Closes previously opened instance of HeapBuf module.
 *
 *  @param      hpHandle  Pointer to handle to previously opened instance.
 *
 *  @sa         HeapBuf_create, HeapBuf_delete, HeapBuf_open
 */
Int32
HeapBuf_close (HeapBuf_Handle * hpHandle)
{
    Int32              status = HEAPBUF_SUCCESS;
    HeapBufDrv_CmdArgs cmdArgs;
    HeapBuf_Obj *      obj;

    GT_1trace (curTrace, GT_ENTER, "HeapBuf_close", hpHandle);

    GT_assert (curTrace, (hpHandle != NULL));
    GT_assert (curTrace, ((hpHandle != NULL)&& (*hpHandle != NULL)));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (HeapBuf_state.setupRefCount == 0) {
        /*! @retval HEAPBUF_E_INVALIDSTATE Module is not initialized */
        status = HEAPBUF_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_close",
                             status,
                             "Module is not initialized!");
    }
    else if (hpHandle == NULL) {
        /*! @retval HEAPBUF_E_INVALIDARG handle pointer passed is NULL*/
        status = HEAPBUF_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_close",
                             status,
                             "handle pointer passed is NULL!");
    }
    else if (*hpHandle == NULL) {
        /*! @retval HEAPBUF_E_INVALIDARG handle passed is NULL*/
        status = HEAPBUF_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_close",
                             status,
                             "*hpHandle passed is NULL!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        obj = (HeapBuf_Obj *) (((HeapBuf_Object *) (*hpHandle))->obj);
        cmdArgs.args.close.handle = obj->knlObject;
        status = HeapBufDrv_ioctl (CMD_HEAPBUF_CLOSE, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "HeapBuf_close",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            Memory_free (NULL, obj, sizeof (HeapBuf_Obj));
            Memory_free (NULL, *hpHandle, sizeof (HeapBuf_Object));
            *hpHandle = NULL;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "HeapBuf_close", status);

    /*! @retval HEAPBUF_SUCCESS Operation Successful */
    return status;
}


/*!
 *  @brief      Allocs a block
 *
 *  @param      hpHandle  Handle to previously created/opened instance.
 *  @param      size      Size to be allocated (in bytes)
 *  @param      align     Alignment for allocation (power of 2)
 *
 *  @sa         HeapBuf_alloc
 */
Void *
HeapBuf_alloc (HeapBuf_Handle   hpHandle,
               UInt32           size,
               UInt32           align)
{
    Int32               status     = HEAPBUF_SUCCESS;
    Char           *    block      = NULL;
    SharedRegion_SRPtr  blockSrPtr = SHAREDREGION_INVALIDSRPTR;
    HeapBufDrv_CmdArgs  cmdArgs;

    GT_3trace (curTrace, GT_ENTER, "HeapBuf_alloc", hpHandle, size, align);

    GT_assert (curTrace, (hpHandle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (HeapBuf_state.setupRefCount == 0) {
        /*! @retval HEAPBUF_E_INVALIDSTATE Module is not initialized */
        status = HEAPBUF_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_alloc",
                             status,
                             "Module is not initialized!");
    }
    else if (hpHandle == NULL) {
        /*! @retval NULL Invalid NULL hpHandle pointer specified */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_alloc",
                             HEAPBUF_E_INVALIDARG,
                             "Invalid NULL hpHandle pointer specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.alloc.handle =
                ((HeapBuf_Obj *) ((HeapBuf_Object *) hpHandle)->obj)->knlObject;
        cmdArgs.args.alloc.size   = size;
        cmdArgs.args.alloc.align  = align;

        status = HeapBufDrv_ioctl (CMD_HEAPBUF_ALLOC, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "HeapBuf_alloc",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            blockSrPtr = cmdArgs.args.alloc.blockSrPtr;
            if (blockSrPtr != SHAREDREGION_INVALIDSRPTR) {
                block = SharedRegion_getPtr (blockSrPtr);
            }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "HeapBuf_alloc", block);

    /*! @retval Valid-block Operation Successful
     *! @retval NULL        Operation Failed
     */
    return block;
}


/*!
 *  @brief      Frees a block
 *
 *  @param      hpHandle  Handle to previously created/opened instance.
 *  @param      block     Block of memory to be freed.
 *  @param      size      Size to be freed (in bytes)
 *
 *  @sa         HeapBuf_alloc ListMP_putTail
 */
Int
HeapBuf_free (HeapBuf_Handle   hpHandle,
              Ptr              block,
              UInt32           size)
{
    Int                 status = HEAPBUF_SUCCESS;
    HeapBufDrv_CmdArgs  cmdArgs;
    Int32               index;

    GT_3trace (curTrace, GT_ENTER, "HeapBuf_free", hpHandle, block, size);

    GT_assert (curTrace, (hpHandle != NULL));
    GT_assert (curTrace, (block != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (HeapBuf_state.setupRefCount == 0) {
        /*! @retval HEAPBUF_E_INVALIDSTATE Module is not initialized */
        status = HEAPBUF_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_free",
                             status,
                             "Module is not initialized!");
    }
    else if (hpHandle == NULL) {
        /*! @retval HEAPBUF_E_INVALIDARG
         *          Invalid NULL hpHandle pointer specified
         */
        status = HEAPBUF_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_free",
                             status,
                             "Invalid NULL hpHandle pointer specified!");
    }
    else if (block == NULL) {
        /*! @retval HEAPBUF_E_INVALIDARG
         *          Invalid NULL block pointer specified
         */
        status = HEAPBUF_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_free",
                             status,
                             "Invalid NULL block pointer specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.free.handle =
                ((HeapBuf_Obj *) ((HeapBuf_Object *) hpHandle)->obj)->knlObject;
        cmdArgs.args.free.size   = size;

        /* Translate to SrPtr. */
        index = SharedRegion_getIndex (block);
        cmdArgs.args.free.blockSrPtr = SharedRegion_getSRPtr (block, index);

        status = HeapBufDrv_ioctl (CMD_HEAPBUF_FREE, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "HeapBuf_free",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "HeapBuf_free", status);

    /*! @retval HEAPBUF_SUCCESS Operation Successful */
    return status;
}


/*!
 *  @brief      Get memory statistics
 *
 *  @param      hpHandle  Handle to previously created/opened instance.
 *  @params     stats     Memory statistics
 *
 *  @sa         HeapBuf_getExtendedStats
 */
Int
HeapBuf_getStats (HeapBuf_Handle  hpHandle,
                  Memory_Stats  * stats)
{
    Int              status = HEAPBUF_SUCCESS;
    HeapBufDrv_CmdArgs  cmdArgs;

    GT_2trace (curTrace, GT_ENTER, "HeapBuf_getStats", hpHandle, stats);

    GT_assert (curTrace, (hpHandle != NULL));
    GT_assert (curTrace, (stats != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (HeapBuf_state.setupRefCount == 0) {
        /*! @retval HEAPBUF_E_INVALIDSTATE
         *          Module is not initialized
         */
        status = HEAPBUF_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_getStats",
                             status,
                             "Module is not initialized!");
    }
    else if (hpHandle == NULL) {
        /*! @retval HEAPBUF_E_INVALIDARG
         *          Invalid NULL hpHandle pointer specified
         */
        status = HEAPBUF_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_getStats",
                             status,
                             "Invalid NULL hpHandle pointer specified!");
    }
    else if (stats == NULL) {
        /*! @retval HEAPBUF_E_INVALIDARG
         *          Invalid NULL stats pointer specified
         */
        status = HEAPBUF_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_getStats",
                             status,
                             "Invalid NULL stats pointer specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.getStats.handle =
                ((HeapBuf_Obj *) ((HeapBuf_Object *) hpHandle)->obj)->knlObject;
        cmdArgs.args.getStats.stats  = (Memory_Stats*) stats;

        status = HeapBufDrv_ioctl (CMD_HEAPBUF_GETSTATS, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                    GT_4CLASS,
                    "HeapBuf_getStats",
                    status,
                    "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "HeapBuf_getStats", status);

    /*! @retval HEAPBUF_SUCCESS Operation Successful */
    return status;
}


/*!
 *  @brief      Indicate whether the heap may block during an alloc or free call
 *
 *  @param      handle    Handle to previously created/opened instance.
 */
Bool
HeapBuf_isBlocking (HeapBuf_Handle handle)
{
    Bool isBlocking = FALSE;

    GT_1trace (curTrace, GT_ENTER, "Heap_isBlocking", handle);

    GT_assert (curTrace, (handle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handle == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_isBlocking",
                             HEAPBUF_E_INVALIDARG,
                             "handle passed is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* TBD: Figure out how to determine whether the gate is blocking */
        isBlocking = TRUE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "HeapBuf_isBlocking", isBlocking);

    /*! @retval TRUE  Heap blocks during alloc/free calls */
    /*! @retval FALSE Heap does not block during alloc/free calls */
    return isBlocking;
}


/*!
 *  @brief      Get extended statistics
 *
 *  @param      hpHandle  Handle to previously created/opened instance.
 *  @params     stats     HeapBuf statistics
 *
 *  @sa         HeapBuf_getStats
 *
 */
Int
HeapBuf_getExtendedStats (HeapBuf_Handle       hpHandle,
                          HeapBuf_ExtendedStats * stats)
{
    Int              status = HEAPBUF_SUCCESS;
    HeapBufDrv_CmdArgs  cmdArgs;

    GT_2trace (curTrace, GT_ENTER, "HeapBuf_getExtendedStats", hpHandle, stats);

    GT_assert (curTrace, (hpHandle != NULL));
    GT_assert (curTrace, (stats != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (HeapBuf_state.setupRefCount == 0) {
        /*! @retval HEAPBUF_E_INVALIDSTATE
         *          Module is not initialized
         */
        status = HEAPBUF_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_getExtendedStats",
                             status,
                             "Module is not initialized!");
    }
    else if (hpHandle == NULL) {
        /*! @retval HEAPBUF_E_INVALIDARG Invalid NULL
         *          hpHandle pointer specified
         */
        status = HEAPBUF_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_getExtendedStats",
                             status,
                             "Invalid NULL hpHandle pointer specified!");
    }
    else if (stats == NULL) {
        /*! @retval HEAPBUF_E_INVALIDARG
         *          Invalid NULL stats pointer specified!;
         */

        status = HEAPBUF_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_getExtendedStats",
                             status,
                             "Invalid NULL stats pointer specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.getExtendedStats.handle =
                ((HeapBuf_Obj *) ((HeapBuf_Object *) hpHandle)->obj)->knlObject;
        cmdArgs.args.getExtendedStats.stats  = (HeapBuf_ExtendedStats *) stats;
        status = HeapBufDrv_ioctl (CMD_HEAPBUF_GETEXTENDEDSTATS, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                    GT_4CLASS,
                    "HeapBuf_getExtendedStats",
                    status,
                    "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "HeapBuf_getExtendedStats", status);

    /*! @retval HEAPBUF_SUCCESS Operation Successful */
    return status;
}


/*!
 *  @brief      Returns the shared memory size requirement for a single
 *              instance.
 *
 *  @param      params  Instance creation specific parameters
 *  @param      bufSize Return value: Size for heap buffers
 *
 *  @sa         None
 */
Int
HeapBuf_sharedMemReq (const HeapBuf_Params * params, UInt32 * bufSize)
{
    Int                 status = HEAPBUF_SUCCESS;
    Int                 stateSize = 0u;
    HeapBufDrv_CmdArgs  cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "HeapBuf_sharedMemReq", params);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (HeapBuf_state.setupRefCount == 0) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_sharedMemReq",
                             HEAPBUF_E_INVALIDSTATE,
                             "Module is not initialized!");
    }
    else if (params == NULL) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_sharedMemReq",
                             HEAPBUF_E_INVALIDARG,
                             "Invalid NULL params pointer specified!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.sharedMemReq.params  = (HeapBuf_Params *)params;

#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        HeapBufDrv_ioctl (CMD_HEAPBUF_SHAREDMEMREQ, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                    GT_4CLASS,
                    "HeapBuf_getExtendedStats",
                    status,
                    "API (through IOCTL) failed on kernel-side!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            *bufSize =  cmdArgs.args.sharedMemReq.bufSize;
            stateSize = cmdArgs.args.sharedMemReq.bytes;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "HeapBuf_sharedMemReq", stateSize);

    /*! @retval Amount-of-shared-memory-required On success */
    return (stateSize);
}


/*!
 *  @brief      Returns the HeapBuf kernel object pointer.
 *
 *  @param      handle  Handle to previousely created/opened instance.
 *
 */
Void *
HeapBuf_getKnlHandle (HeapBuf_Handle hpHandle)
{
    Ptr     objPtr = NULL;

    GT_1trace (curTrace, GT_ENTER, "HeapBuf_getKnlHandle", hpHandle);

    GT_assert (curTrace, (hpHandle != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (HeapBuf_state.setupRefCount == 0) {
        /*! @retval NULL Module is not initialized */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_getKnlHandle",
                             HEAPBUF_E_INVALIDSTATE,
                             "Module is not initialized!");
    }
    else if (hpHandle == NULL) {
        /*! @retval NULL handle passed is NULL */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_getKnlHandle",
                             HEAPBUF_E_INVALIDARG,
                             "hpHandle passed is NULL!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        objPtr = ((HeapBuf_Obj *)
                            (((HeapBuf_Object *) hpHandle)->obj))->knlObject;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (objPtr == NULL) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "HeapBuf_getKnlHandle",
                                 HEAPBUF_E_INVALIDSTATE,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "HeapBuf_getKnlHandle", objPtr);

    /*! @retval Kernel-Object-handle Operation successfully completed. */
    return (objPtr);
}


/* =============================================================================
 * Internal function
 * =============================================================================
 */
/*!
 *  @brief      Creates and populates handle and obj.
 *
 *  @param      heapHandle    Return value: Handle
 *  @param      cmdArgs       command areguments
 *  @param      createFlag    Indicates whether this is a create or open call.
 *
 *  @sa         ListMPSharedMemory_delete
 */
Int32
_HeapBuf_create (HeapBuf_Handle     * heapHandle,
                 HeapBufDrv_CmdArgs   cmdArgs,
                 Bool                 createFlag)
{
    Int32         status = HEAPBUF_SUCCESS;
    HeapBuf_Obj * obj;

    /* No need for parameter checks, since this is an internal function. */

    /* Allocate memory for the handle */
    *heapHandle = (HeapBuf_Handle) Memory_calloc (NULL,
                                                  sizeof (HeapBuf_Object),
                                                  0);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (*heapHandle == NULL) {
        /*! @retval HEAPBUF_E_MEMORY Memory allocation failed for handle */
        status = HEAPBUF_E_MEMORY;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "HeapBuf_create",
                             status,
                             "Memory allocation failed for handle!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Set pointer to kernel object into the user handle. */
        obj = (HeapBuf_Obj *) Memory_calloc (NULL,
                                             sizeof (HeapBuf_Obj),
                                             0);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (obj == NULL) {
            Memory_free (NULL, *heapHandle, sizeof (HeapBuf_Object));
            *heapHandle = NULL;
            /*! @retval HEAPBUF_E_MEMORY Memory allocation failed for obj */
            status = HEAPBUF_E_MEMORY;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "HeapBuf_create",
                                 status,
                                 "Memory allocation failed for obj!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            if (createFlag == TRUE){
                obj->knlObject = cmdArgs.args.create.handle;
            }
            else{
                obj->knlObject = cmdArgs.args.open.handle;
            }

            ((HeapBuf_Object *)(*heapHandle))->obj = obj;
            ((HeapBuf_Object *)(*heapHandle))->alloc        = &HeapBuf_alloc;
            ((HeapBuf_Object *)(*heapHandle))->free         = &HeapBuf_free;
            ((HeapBuf_Object *)(*heapHandle))->getStats     = &HeapBuf_getStats;
            ((HeapBuf_Object *)(*heapHandle))->getKnlHandle =
                                                          &HeapBuf_getKnlHandle;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    /*! @retval HEAPBUF_SUCCESS _HeapBuf_create successful */
    return (status);
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */




