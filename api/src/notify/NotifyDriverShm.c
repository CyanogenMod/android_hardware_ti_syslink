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
 *  @file       NotifyDriverShm.c
 *
 *  @brief      Notify shared memory driver implementation
 *
 *  ============================================================================
 */


/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <Memory.h>
#include <Trace.h>
#include <String.h>

/* Module level headers */
#include <Notify.h>
#include <NotifyDriverShm.h>
#include <NotifyDriverShmDrvDefs.h>
#include <NotifyDriverShmDrvUsr.h>
#include <_NotifyDefs.h>


#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*!
 *  @brief  NotifyDriverShm Module state object
 */
typedef struct NotifyDriverShm_ModuleObject_tag {
    UInt32                   setupRefCount;
    /*!< Reference count for number of times setup/destroy were called in this
         process. */
    NotifyDriverShm_Handle   driverHandles [NOTIFY_MAX_DRIVERS];
    /*!< Loader handle array. */
} NotifyDriverShm_ModuleObject;

/*!
 *  @brief  Internal NotifyDriverShm instance object.
 */
typedef struct NotifyDriverShm_Object_tag {
    Notify_CommonObject  commonObj;
    /*!< Common object required to be the first field in the instance object
         structure. This is used by Notify to get the handle to the kernel
         object. */
    UInt32               openRefCount;
    /*!< Reference count for number of times open/close were called in this
         process. */
    Bool                 created;
    /*!< Indicates whether the object was created in this process. */
    Char                 driverName [NOTIFY_MAX_NAMELEN];
    /*!< Name of the NotifyDriverShm instance. */
    UInt16               slot;
    /*!< Driver slot in driver handles array */
} NotifyDriverShm_Object;


/* =============================================================================
 *  Globals
 * =============================================================================
 */
/*!
 *  @var    NotifyDriverShm_state
 *
 *  @brief  NotifyDriverShm state object variable
 */
#if !defined(SYSLINK_BUILD_DEBUG)
static
#endif /* if !defined(SYSLINK_BUILD_DEBUG) */
NotifyDriverShm_ModuleObject NotifyDriverShm_state =
{
    .setupRefCount = 0
};


/* =============================================================================
 * APIs
 * =============================================================================
 */
/*!
 *  @brief      Function to get the default configuration for the
 *              NotifyDriverShm module.
 *
 *              This function can be called by the application to get their
 *              configuration parameter to NotifyDriverShm_setup filled in by
 *              the NotifyDriverShm module with the default parameters. If the
 *              user does not wish to make any change in the default parameters,
 *              this API is not required to be called.
 *
 *  @param      cfg        Pointer to the NotifyDriverShm module configuration
 *                         structure in which the default config is to be
 *                         returned.
 *
 *  @sa         NotifyDriverShm_setup, NotifyDriverShmDrvUsr_open,
 *              NotifyDriverShmDrvUsr_ioctl, NotifyDriverShmDrvUsr_close
 */
Void
NotifyDriverShm_getConfig (NotifyDriverShm_Config * cfg)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                                 status = NOTIFYDRIVERSHM_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    NotifyDriverShm_CmdArgsGetConfig    cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "NotifyDriverShm_getConfig", cfg);

    GT_assert (curTrace, (cfg != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (cfg == NULL) {
        GT_setFailureReason (curTrace,
                           GT_4CLASS,
                           "NotifyDriverShm_getConfig",
                           NOTIFYDRIVERSHM_E_INVALIDARG,
                           "Argument of type (NotifyDriverShm_Config *) passed "
                           "is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Temporarily open the handle to get the configuration. */
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        NotifyDriverShmDrvUsr_open ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NotifyDriverShm_getConfig",
                                 status,
                                 "Failed to open driver handle!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            cmdArgs.cfg = cfg;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            NotifyDriverShmDrvUsr_ioctl (CMD_NOTIFYDRIVERSHM_GETCONFIG,
                                         &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                  GT_4CLASS,
                                  "NotifyDriverShm_getConfig",
                                  status,
                                  "API (through IOCTL) failed on kernel-side!");
            }
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Close the driver handle. */
        NotifyDriverShmDrvUsr_close ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_ENTER, "NotifyDriverShm_getConfig");
}


/*!
 *  @brief      Function to setup the NotifyDriverShm module.
 *
 *              This function sets up the NotifyDriverShm module. This function
 *              must be called before any other instance-level APIs can be
 *              invoked. Module-level configuration needs to be provided to this
 *              function. If the user wishes to change some specific config
 *              parameters, then NotifyDriverShm_getConfig can be called to get
 *              the configuration filled with the default values. After this,
 *              only the required configuration values can be changed. If the
 *              user does not wish to make any change in the default parameters,
 *              the application can simply call NotifyDriverShm_setup with NULL
 *              parameters. The default parameters would get automatically used.
 *
 *  @param      cfg   Optional NotifyDriverShm module configuration. If provided
 *                    as NULL, default configuration is used.
 *
 *  @sa         NotifyDriverShm_destroy, NotifyDriverShmDrvUsr_open,
 *              NotifyDriverShmDrvUsr_ioctl
 */
Int
NotifyDriverShm_setup (NotifyDriverShm_Config * cfg)
{
    Int                          status = NOTIFYDRIVERSHM_SUCCESS;
    NotifyDriverShm_CmdArgsSetup cmdArgs;
    UInt16                       i;

    /* TBD: Protect from multiple threads. */
    NotifyDriverShm_state.setupRefCount++;
    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */
    if (NotifyDriverShm_state.setupRefCount > 1) {
        /*! @retval NOTIFYDRIVERSHM_S_ALREADYSETUP Success: NotifyDriverShm
                               module has  been already setup in this process */
        status = NOTIFYDRIVERSHM_S_ALREADYSETUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "    NotifyDriverShm_setup: NotifyDriverShm module has been"
                   " already setup in this process.\n"
                   "        RefCount: [%d]\n",
                   (NotifyDriverShm_state.setupRefCount - 1));
    }
    else {
        /* Open the driver handle. */
        status = NotifyDriverShmDrvUsr_open ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NotifyDriverShm_setup",
                                 status,
                                 "Failed to open driver handle!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            cmdArgs.cfg = cfg;
            status = NotifyDriverShmDrvUsr_ioctl (CMD_NOTIFYDRIVERSHM_SETUP,
                                                  &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                  GT_4CLASS,
                                  "NotifyDriverShm_setup",
                                  status,
                                  "API (through IOCTL) failed on kernel-side!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                /* Initialize the driver handles. */
                for (i = 0 ; i < NOTIFY_MAX_DRIVERS ; i++) {
                    NotifyDriverShm_state.driverHandles [i] = NULL;
                }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            }
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

    GT_1trace (curTrace, GT_LEAVE, "NotifyDriverShm_setup", status);

    /*! @retval NOTIFYDRIVERSHM_SUCCESS Operation successful */
    return (status);
}


/*!
 *  @brief      Function to destroy the NotifyDriverShm module.
 *
 *              Once this function is called, other NotifyDriverShm module APIs,
 *              except for the NotifyDriverShm_getConfig API cannot be called
 *              anymore.
 *
 *  @sa         NotifyDriverShm_setup, NotifyDriverShmDrvUsr_ioctl
 */
Int
NotifyDriverShm_destroy (Void)
{
    Int                              status = NOTIFYDRIVERSHM_SUCCESS;
    NotifyDriverShm_CmdArgsDestroy   cmdArgs;
    UInt16                           i;

    GT_0trace (curTrace, GT_ENTER, "NotifyDriverShm_destroy");

    /* TBD: Protect from multiple threads. */
    NotifyDriverShm_state.setupRefCount--;
    /* This is needed at runtime so should not be in SYSLINK_BUILD_OPTIMIZE. */
    if (NotifyDriverShm_state.setupRefCount > 1) {
        /*! @retval NOTIFYDRIVERSHM_S_SETUP Success: NotifyDriverShm module has
                                  been setup by other clients in this process */
        status = NOTIFYDRIVERSHM_S_SETUP;
        GT_1trace (curTrace,
                   GT_1CLASS,
                   "NotifyDriverShm module has been setup by other clients in"
                   " this process.\n"
                   "    RefCount: [%d]\n",
                   (NotifyDriverShm_state.setupRefCount + 1));
    }
    else {
        status = NotifyDriverShmDrvUsr_ioctl (CMD_NOTIFYDRIVERSHM_DESTROY,
                                              &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NotifyDriverShm_destroy",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Check if any NotifyDriverShm instances have not been deleted so far.
         * If not, delete them.
         */
        for (i = 0 ; i < NOTIFY_MAX_DRIVERS ; i++) {
            GT_assert (curTrace,
                       (NotifyDriverShm_state.driverHandles [i] == NULL));
            if (NotifyDriverShm_state.driverHandles [i] != NULL) {
                NotifyDriverShm_delete (
                                   &(NotifyDriverShm_state.driverHandles [i]));
            }
        }

        /* Close the driver handle. */
        NotifyDriverShmDrvUsr_close ();
    }

    GT_1trace (curTrace, GT_LEAVE, "NotifyDriverShm_destroy", status);

    /*! @retval NOTIFYDRIVERSHM_SUCCESS Operation successful */
    return (status);
}


/*!
 *  @brief      Function to initialize the parameters for the NotifyDriverShm
 *              instance.
 *
 *              This function can be called by the application to get their
 *              configuration parameter to NotifyDriverShm_create filled in by
 *              the NotifyDriverShm module with the default parameters.
 *
 *  @param      handle   Handle to the NotifyDriverShm object. If specified as
 *                       NULL, the default global configuration values are
 *                       returned.
 *  @param      params   Pointer to the NotifyDriverShm instance params
 *                       structure in which the default params is to be returned
 *
 *  @sa         NotifyDriverShm_create, NotifyDriverShmDrvUsr_ioctl
 */
Void
NotifyDriverShm_Params_init (NotifyDriverShm_Handle   handle,
                             NotifyDriverShm_Params * params)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                      status = NOTIFYDRIVERSHM_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    NotifyDriverShm_Object * driverHandle = (NotifyDriverShm_Object *) handle;
    NotifyDriverShm_CmdArgsParamsInit cmdArgs;

    GT_2trace (curTrace, GT_ENTER, "NotifyDriverShm_Params_init",
               handle, params);

    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (params == NULL) {
        /* No retVal comment since this is a Void function. */
        status = NOTIFYDRIVERSHM_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                           GT_4CLASS,
                           "NotifyDriverShm_Params_init",
                           status,
                           "Argument of type (NotifyDriverShm_Params *) passed "
                           "is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Check if the handle is NULL and pass it in directly to kernel-side in
         * that case. Otherwise send the kernel object pointer.
         */
        if (handle == NULL) {
            cmdArgs.handle = handle;
        }
        else {
            cmdArgs.handle = driverHandle->commonObj.knlObject;
        }
        cmdArgs.params = params;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        NotifyDriverShmDrvUsr_ioctl (CMD_NOTIFYDRIVERSHM_PARAMS_INIT, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NotifyDriverShm_Params_init",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "NotifyDriverShm_Params_init");
}


/*!
 *  @brief      Function to create a NotifyDriverShm object for a specific slave
 *              processor.
 *
 *              This function creates an instance of the NotifyDriverShm module
 *              and returns an instance handle, which is used to access the
 *              specified slave processor. The processor ID specified here is
 *              the ID of the slave processor as configured with the MultiProc
 *              module.
 *              Instance-level configuration needs to be provided to this
 *              function. If the user wishes to change some specific config
 *              parameters, then NotifyDriverShm_Params_init can be called to
 *              get the configuration filled with the default values. After
 *              this, only the required configuration values can be changed.
 *
 *  @param      procId   Processor ID represented by this NotifyDriverShm
                         instance
 *  @param      params   NotifyDriverShm instance configuration parameters.
 *
 *  @sa         NotifyDriverShm_delete, Memory_calloc,
 *              NotifyDriverShmDrvUsr_ioctl
 */
NotifyDriverShm_Handle
NotifyDriverShm_create (      String                   driverName,
                        const NotifyDriverShm_Params * params)
{
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    Int                              status = NOTIFYDRIVERSHM_SUCCESS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    NotifyDriverShm_Object *         handle = NULL;
    Bool                             found  = FALSE;
    UInt32                           slot   = 0;
    /* TBD: UInt32                  key;*/
    NotifyDriverShm_CmdArgsCreate    cmdArgs;
    UInt32                           i;

    GT_2trace (curTrace, GT_ENTER, "NotifyDriverShm_create",
               driverName, params);

    GT_assert (curTrace, (driverName != NULL));
    GT_assert (curTrace, (params != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (driverName == NULL) {
        /*! @retval NULL Invalid NULL driverName pointer specified */
        status = NOTIFYDRIVERSHM_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NotifyDriverShm_create",
                             status,
                             "Invalid NULL driverName pointer specified");
    }
    else if (params == NULL) {
        /*! @retval NULL Invalid NULL params pointer specified */
        status = NOTIFYDRIVERSHM_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NotifyDriverShm_create",
                             status,
                             "Invalid NULL params pointer specified");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* TBD: Enter critical section protection. */
        /* key = Gate_enter (NotifyDriverShm_state.gateHandle); */
        /* Check if driver already exists. */
        for (i = 0 ; i < NOTIFY_MAX_DRIVERS ; i++) {
            if (NotifyDriverShm_state.driverHandles [i] != NULL) {
                handle = (NotifyDriverShm_Object *)
                                    NotifyDriverShm_state.driverHandles [i];
                if (String_cmp (driverName, handle->driverName) == 0) {
                    found = TRUE;
                }
            }

            if (NotifyDriverShm_state.driverHandles [i] == NULL) {
                /* Reserve a free slot.*/
                slot = i;
            }
        }

        if (found == TRUE) {
            /* If the object is already created/opened in this process, return
             * handle in the local array.
             */
            handle->openRefCount++;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            status = NOTIFYDRIVERSHM_S_ALREADYEXISTS;
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            GT_1trace (curTrace,
                       GT_2CLASS,
                       "    NotifyDriverShm_create: Instance already exists in"
                       " this process space"
                       "        RefCount [%d]\n",
                       (handle->openRefCount - 1));
        }
        else {
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (slot == NOTIFY_MAX_DRIVERS) {
                status = NOTIFY_E_MAXDRIVERS;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "NotifyDriverShm_create",
                                     status,
                                     "Maximum drivers already created.");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                handle = NULL;
                String_cpy (cmdArgs.driverName, driverName);
                Memory_copy (&(cmdArgs.params),
                             (Ptr) params,
                             sizeof (NotifyDriverShm_Params));
#if !defined(SYSLINK_BUILD_OPTIMIZE)
                status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                NotifyDriverShmDrvUsr_ioctl (CMD_NOTIFYDRIVERSHM_CREATE,
                                             &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
                if (status < 0) {
                    GT_setFailureReason (curTrace,
                                  GT_4CLASS,
                                  "NotifyDriverShm_create",
                                  status,
                                  "API (through IOCTL) failed on kernel-side!");
                }
                else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                    /* Allocate memory for the handle */
                    handle = (NotifyDriverShm_Object *) Memory_calloc (NULL,
                                                sizeof (NotifyDriverShm_Object),
                                                0);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
                    if (handle == NULL) {
                        /*! @retval NULL Memory allocation failed for handle */
                        status = NOTIFYDRIVERSHM_E_MEMORY;
                        GT_setFailureReason (curTrace,
                                        GT_4CLASS,
                                        "NotifyDriverShm_create",
                                        status,
                                        "Memory allocation failed for handle!");
                    }
                    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                        /* Set pointer to kernel object into the user handle. */
                        handle->commonObj.knlObject = cmdArgs.handle;
                        /* Indicate that the object was created in this
                         * process.
                         */
                        handle->created = TRUE;
                        String_cpy (handle->driverName, driverName);
                        handle->slot = slot;
                        /* Store the NotifyDriverShm handle in the local
                         * array.
                         */
                        NotifyDriverShm_state.driverHandles [slot] =
                                            (NotifyDriverShm_Handle) handle;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
                    }
                }
            }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        }
        /* TBD: Leave critical section protection. */
        /* Gate_leave (NotifyDriverShm_state.gateHandle, key); */
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "NotifyDriverShm_create", handle);

    /*! @retval Valid Handle Operation successful */
    return ((NotifyDriverShm_Handle) handle);
}


/*!
 *  @brief      Function to delete a NotifyDriverShm object for a specific slave
 *              processor.
 *
 *              Once this function is called, other NotifyDriverShm instance
 *              level APIs that require the instance handle cannot be called.
 *
 *  @param      handlePtr   Pointer to Handle to the NotifyDriverShm object
 *
 *  @sa         NotifyDriverShm_create, Memory_free, NotifyDriverShmDrvUsr_ioctl
 */
Int
NotifyDriverShm_delete (NotifyDriverShm_Handle * handlePtr)
{
    Int                          status    = NOTIFYDRIVERSHM_SUCCESS;
    Int                          tmpStatus = NOTIFYDRIVERSHM_SUCCESS;
    NotifyDriverShm_Object *     handle;
    /* TBD: UInt32          key;*/
    NotifyDriverShm_CmdArgsDelete cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "NotifyDriverShm_delete", handlePtr);

    GT_assert (curTrace, (handlePtr != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handlePtr == NULL) {
        /*! @retval NOTIFYDRIVERSHM_E_INVALIDARG Invalid NULL handle pointer
                                            specified*/
        status = NOTIFYDRIVERSHM_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NotifyDriverShm_delete",
                             status,
                             "Invalid NULL handle pointer specified");
    }
    else if (*handlePtr == NULL) {
        /*! @retval NOTIFYDRIVERSHM_E_HANDLE Invalid NULL handle specified */
        status = NOTIFYDRIVERSHM_E_HANDLE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NotifyDriverShm_delete",
                             status,
                             "Invalid NULL handle specified");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        handle = (NotifyDriverShm_Object *) (*handlePtr);
        /* TBD: Enter critical section protection. */
        /* key = Gate_enter (NotifyDriverShm_state.gateHandle); */

        if (handle->openRefCount != 0) {
            /* There are still some open handles to this NotifyDriverShm.
             * Give a warning, but still go ahead and delete the object.
             */
            status = NOTIFYDRIVERSHM_S_OPENHANDLE;
            GT_assert (curTrace, (handle->openRefCount != 0));
            GT_1trace (curTrace,
                       GT_1CLASS,
                       "    NotifyDriverShm_delete: Warning, some handles are"
                       " still open!\n"
                       "        RefCount: [%d]\n",
                       handle->openRefCount);
        }

        if (handle->created == FALSE) {
            /*! @retval NOTIFYDRIVERSHM_E_ACCESSDENIED The NotifyDriverShm
                                    object was not created in this process and
                                    access is denied to delete it. */
            status = NOTIFYDRIVERSHM_E_ACCESSDENIED;
            GT_setFailureReason (curTrace,
                            GT_4CLASS,
                            "NotifyDriverShm_delete",
                            status,
                            "The NotifyDriverShm object was not created in"
                            " this process and access is denied to delete it.");
        }

        if (status >= 0) {
            /* Only delete the object if it was created in this process. */
            cmdArgs.handle = handle->commonObj.knlObject;
            tmpStatus = NotifyDriverShmDrvUsr_ioctl (CMD_NOTIFYDRIVERSHM_DELETE,
                                                &cmdArgs);
            if (tmpStatus < 0) {
                /* Only override the status if kernel call failed. Otherwise
                 * we want the status from above to carry forward.
                 */
                status = tmpStatus;
                GT_setFailureReason (curTrace,
                                  GT_4CLASS,
                                  "NotifyDriverShm_delete",
                                  status,
                                  "API (through IOCTL) failed on kernel-side!");
            }
            else {
                /* Clear the NotifyDriverShm handle in the local array. */
                NotifyDriverShm_state.driverHandles [handle->slot] = NULL;

                Memory_free (NULL, handle, sizeof (NotifyDriverShm_Object));
                *handlePtr = NULL;
            }
        }

        /* TBD: Leave critical section protection. */
        /* Gate_leave (NotifyDriverShm_state.gateHandle, key); */
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "NotifyDriverShm_delete", status);

    /*! @retval NOTIFYDRIVERSHM_SUCCESS Operation successful */
    return (status);
}


/*!
 *  @brief      Function to open a handle to an existing NotifyDriverShm object
 *              handling the procId.
 *
 *              This function returns a handle to an existing NotifyDriverShm
 *              instance created for this procId. It enables other entities to
 *              access and use this NotifyDriverShm instance.
 *
 *  @param      handlePtr   Return Parameter: Handle to the NotifyDriverShm
 *                          instance
 *  @param      procId      Processor ID represented by this NotifyDriverShm
 *                          instance
 *
 *  @sa         NotifyDriverShm_close, Memory_calloc,
 *              NotifyDriverShmDrvUsr_ioctl
 */
Int
NotifyDriverShm_open (String                   driverName,
                      NotifyDriverShm_Handle * handlePtr)
{
    Int                             status = NOTIFYDRIVERSHM_SUCCESS;
    NotifyDriverShm_Object *        handle = NULL;
    Bool                            found  = FALSE;
    UInt32                          slot  = 0;
    /* UInt32           key; */
    NotifyDriverShm_CmdArgsOpen     cmdArgs;
    UInt32                          i;

    GT_2trace (curTrace, GT_ENTER, "NotifyDriverShm_open",
               driverName, handlePtr);

    GT_assert (curTrace, (driverName != NULL));
    GT_assert (curTrace, (handlePtr != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (driverName == NULL) {
        /*! @retval NOTIFYDRIVERSHM_E_INVALIDARG Invalid NULL driverName
                                            specified*/
        status = NOTIFYDRIVERSHM_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NotifyDriverShm_open",
                             status,
                             "Invalid NULL driverName specified");
    }
    else if (handlePtr == NULL) {
        /*! @retval NOTIFYDRIVERSHM_E_INVALIDARG Invalid NULL handle pointer
                                            specified*/
        status = NOTIFYDRIVERSHM_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NotifyDriverShm_open",
                             status,
                             "Invalid NULL handle pointer specified");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* TBD: Enter critical section protection. */
        /* key = Gate_enter (NotifyDriverShm_state.gateHandle); */
        /* Check if driver already exists. */
        for (i = 0 ; i < NOTIFY_MAX_DRIVERS ; i++) {
            if (NotifyDriverShm_state.driverHandles [i] != NULL) {
                handle = (NotifyDriverShm_Object *)
                                        NotifyDriverShm_state.driverHandles [i];
                if (String_cmp (driverName, handle->driverName) == 0) {
                    found = TRUE;
                    break;
                }
            }

            if (NotifyDriverShm_state.driverHandles [i] == NULL) {
                /* Reserve a free slot.*/
                slot = i;
            }
        }

        if (found == TRUE) {
            /* If the object is already created/opened in this process, return
             * handle in the local array.
             */
            handle->openRefCount++;
            status = NOTIFYDRIVERSHM_S_ALREADYEXISTS;
            GT_1trace (curTrace,
                       GT_1CLASS,
                       "    NotifyDriverShm_open: Instance already exists in"
                       " this process space"
                       "        RefCount [%d]\n",
                       (handle->openRefCount - 1));
        }
        else {
            if (slot == NOTIFY_MAX_DRIVERS) {
                status = NOTIFY_E_MAXDRIVERS;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "NotifyDriverShm_open",
                                     status,
                                     "Maximum drivers already created.");
            }
            else {
                /* The object was not created/opened in this process. Need to drop
                 * down to the kernel to get the object instance.
                 */
                String_cpy (cmdArgs.driverName, driverName);
                cmdArgs.handle = NULL; /* Return parameter */
#if !defined(SYSLINK_BUILD_OPTIMIZE)
                status =
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                NotifyDriverShmDrvUsr_ioctl (CMD_NOTIFYDRIVERSHM_OPEN,
                                             &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
                if (status < 0) {
                    GT_setFailureReason (curTrace,
                                      GT_4CLASS,
                                      "NotifyDriverShm_open",
                                      status,
                                      "API (through IOCTL) failed on kernel-side!");
                }
                else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                    /* Allocate memory for the handle */
                    handle = (NotifyDriverShm_Object *) Memory_calloc (NULL,
                                                sizeof (NotifyDriverShm_Object),
                                                0);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
                    if (handle == NULL) {
                        /*! @retval NOTIFYDRIVERSHM_E_MEMORY Memory allocation
                                                 failed for handle */
                        status = NOTIFYDRIVERSHM_E_MEMORY;
                        GT_setFailureReason (curTrace,
                                        GT_4CLASS,
                                        "NotifyDriverShm_open",
                                        status,
                                        "Memory allocation failed for handle!");
                    }
                    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                        /* Set pointer to kernel object into the user handle. */
                        handle->commonObj.knlObject = cmdArgs.handle;
                        handle->created = FALSE;
                        String_cpy (handle->driverName, driverName);
                        handle->slot = slot;
                        /* Store the NotifyDriverShm handle in the local
                         * array.
                         */
                        NotifyDriverShm_state.driverHandles [slot] =
                                                (NotifyDriverShm_Handle) handle;
                        handle->openRefCount = 1;

#if !defined(SYSLINK_BUILD_OPTIMIZE)
                    }
                }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            }
        }

        *handlePtr = (NotifyDriverShm_Handle) handle;
        /* TBD: Leave critical section protection. */
        /* Gate_leave (NotifyDriverShm_state.gateHandle, key); */
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "NotifyDriverShm_open", status);

    /*! @retval NOTIFYDRIVERSHM_SUCCESS Operation successful */
    return (status);
}


/*!
 *  @brief      Function to close this handle to the NotifyDriverShm instance.
 *
 *              This function closes the handle to the NotifyDriverShm instance
 *              obtained through NotifyDriverShm_open call made earlier.
 *
 *  @param      handle     Handle to the NotifyDriverShm object
 *
 *  @sa         NotifyDriverShm_open, Memory_free, NotifyDriverShmDrvUsr_ioctl
 */
Int
NotifyDriverShm_close (NotifyDriverShm_Handle * handlePtr)
{
    Int                          status = NOTIFYDRIVERSHM_SUCCESS;
    NotifyDriverShm_Object *     driverHandle;
    NotifyDriverShm_CmdArgsClose cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "NotifyDriverShm_close", handlePtr);

    GT_assert (curTrace, (handlePtr != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (handlePtr == NULL) {
        /*! @retval NOTIFYDRIVERSHM_E_INVALIDARG Invalid NULL handle pointer
                                            specified*/
        status = NOTIFYDRIVERSHM_E_INVALIDARG;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NotifyDriverShm_close",
                             status,
                             "Invalid NULL handle pointer specified");
    }
    else if (*handlePtr == NULL) {
        /*! @retval NOTIFYDRIVERSHM_E_HANDLE Invalid NULL handle specified */
        status = NOTIFYDRIVERSHM_E_HANDLE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "NotifyDriverShm_close",
                             status,
                             "Invalid NULL handle specified");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        driverHandle = (NotifyDriverShm_Object *) (*handlePtr);
        /* TBD: Enter critical section protection. */
        /* key = Gate_enter (NotifyDriverShm_state.gateHandle); */
        if (driverHandle->openRefCount == 0) {
            /*! @retval NOTIFYDRIVERSHM_E_ACCESSDENIED All open handles to this
                                   NotifyDriverShm object are already closed. */
            status = NOTIFYDRIVERSHM_E_ACCESSDENIED;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "NotifyDriverShm_close",
                                 status,
                                 "All open handles to this NotifyDriverShm"
                                 " object are already closed.");

        }
        else if (driverHandle->openRefCount > 1) {
            /* Simply reduce the reference count. There are other threads in
             * this process that have also opened handles to this
             * NotifyDriverShm instance.
             */
            driverHandle->openRefCount--;
            status = NOTIFYDRIVERSHM_S_OPENHANDLE;
            GT_1trace (curTrace,
                       GT_1CLASS,
                       "    NotifyDriverShm_close: Other handles to this"
                       " instance are still open\n"
                       "        RefCount: [%d]\n",
                       (driverHandle->openRefCount + 1));
        }
        else {
            /* The object can be closed now since all open handles are closed.*/
            cmdArgs.handle = driverHandle->commonObj.knlObject;
            status = NotifyDriverShmDrvUsr_ioctl (CMD_NOTIFYDRIVERSHM_CLOSE,
                                                  &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                  GT_4CLASS,
                                  "NotifyDriverShm_close",
                                  status,
                                  "API (through IOCTL) failed on kernel-side!");
            }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

            if (driverHandle->created == FALSE) {
                /* Clear the NotifyDriverShm handle in the local array. */
                NotifyDriverShm_state.driverHandles [driverHandle->slot] = NULL;
                /* Free memory for the handle only if it was not created in
                 * this process.
                 */
                Memory_free (NULL,
                             driverHandle,
                             sizeof (NotifyDriverShm_Object));
                *handlePtr = NULL;
            }
        }
        /* TBD: Leave critical section protection. */
        /* Gate_leave (NotifyDriverShm_state.gateHandle, key); */
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "NotifyDriverShm_close", status);

    /*! @retval NOTIFYDRIVERSHM_SUCCESS Operation successful */
    return (status);
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
