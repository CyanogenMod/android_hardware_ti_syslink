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
 *  @file   MultiProc.c
 *
 *  @brief      Handles processor id management in multi processor systems.Used
 *              by all modules which need processor ids for their oprations.
 *
 *  ============================================================================
 */


/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <Trace.h>
#include <String.h>
#include <Memory.h>

/* Module level headers */
#include <MultiProc.h>
#include <MultiProcDrvDefs.h>
#include <MultiProcDrv.h>


#if defined (__cplusplus)
extern "C" {
#endif


/* =============================================================================
 *  Macros and types
 * =============================================================================
 */
/*!
 *  @brief  MultiProc Module state object
 */
typedef struct MultiProc_ModuleObject_tag {
    MultiProc_Config cfg;
    /*!< Notify configuration structure */
    MultiProc_Config defCfg;
    /*!< Default module configuration */
    UInt32           refCount;
    /* Reference count */
} MultiProc_ModuleObject;


/* =============================================================================
 *  Extern declarations
 * =============================================================================
 */
#if !defined(SYSLINK_BUILD_DEBUG)
static
#endif /* if !defined(SYSLINK_BUILD_DEBUG) */
MultiProc_ModuleObject MultiProc_state = { .refCount = 0, };


/* =============================================================================
 *  APIs
 * =============================================================================
 */

/*!
 *  @brief      Function to get MultiProc configuration
 *
 *  @param      cfg  MultiProc configuration give by system integrator
 *
 *  @sa         MultiProc_setId
 */
Void
MultiProc_getConfig (MultiProc_Config * cfg)
{
    Int                  status = MULTIPROC_SUCCESS;
    MultiProcDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "MultiProc_getConfig", cfg);

    GT_assert (curTrace, (cfg != NULL));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (cfg == NULL) {
        /*! @retval NULL Invalid NULL cfg pointer specified */
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MultiProc_getConfig",
                             MULTIPROC_E_FAIL,
                             "Argument of type (Notify_Config *) passed "
                             "is null!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        if (MultiProc_state.refCount == 0) {
            /* Temporarily open the handle to get the configuration. */
            status = MultiProcDrv_open ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "MultiProc_getConfig",
                                     status,
                                     "Failed to open driver handle!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                cmdArgs.args.getConfig.config = cfg;
                status = MultiProcDrv_ioctl (CMD_MULTIPROC_GETCONFIG,
                                             &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
                if (status < 0) {
                    GT_setFailureReason (curTrace,
                                  GT_4CLASS,
                                  "MultiProc_getConfig",
                                  status,
                                  "API (through IOCTL) failed on kernel-side!");

                }
            }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            /* Close the driver handle. */
            MultiProcDrv_close ();
        }
        else {
            Memory_copy (cfg, &MultiProc_state.cfg, sizeof (MultiProc_Config));
        }

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "MultiProc_getConfig");
}


/*!
 *  @brief      Function to set up module configuration
 *
 *  @param      cfg  MultiProc configuration give by system integrator
 *
 *  @sa         MultiProc_destroy
 */
Int32
MultiProc_setup (MultiProc_Config * cfg)
{
    Int32                status = MULTIPROC_SUCCESS;
    MultiProcDrv_CmdArgs cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "MultiProc_setup", cfg);

    if (MultiProc_state.refCount == 0) {
        /* Open the driver handle. */
        status = MultiProcDrv_open ();
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MultiProc_setup",
                                 status,
                                 "Failed to open driver handle!");
        }
        else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            cmdArgs.args.setup.config = (MultiProc_Config *) cfg;
            status = MultiProcDrv_ioctl (CMD_MULTIPROC_SETUP, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "MultiProc_setup",
                                     status,
                                     "API (through IOCTL) failed"
                                     " on kernel-side!");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                Memory_copy (&MultiProc_state.cfg,
                             cfg,
                             sizeof (MultiProc_Config));
                MultiProc_state.refCount++;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            }
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }
    else {
        MultiProc_state.refCount++;
    }

    GT_1trace (curTrace, GT_LEAVE, "MultiProc_setup", status);

    /*! @retval MULTIPROC_SUCCESS Operation successful */
    return (status);
}


/*!
 *  @brief      Function to destroy MultiProc module
 *
 *  @sa         MultiProc_setup
 */
Int32
MultiProc_destroy (Void)
{
    Int                  status = MULTIPROC_SUCCESS;
    MultiProcDrv_CmdArgs cmdArgs;

    GT_0trace (curTrace, GT_ENTER, "MultiProc_destroy");

    if (MultiProc_state.refCount == 1) {
        status = MultiProcDrv_ioctl (CMD_MULTIPROC_DESTROY, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MultiProc_destroy",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* Close the driver handle. */
        MultiProcDrv_close ();
        MultiProc_state.refCount--;
    }
    else {
        MultiProc_state.refCount--;
    }

    GT_1trace (curTrace, GT_LEAVE, "MultiProc_destroy", status);

    /*! @retval MULTIPROC_SUCCESS Operation successful */
    return (status);
}


/*!
 *  @brief      Function to set Local processor Id
 *
 *  @param      id  Processor id obtained for local processor
 *
 *  @sa         MultiProc_setId
 */
Int32
MultiProc_setLocalId (UInt16 id)
{
    Int                   status = MULTIPROC_SUCCESS;
    MultiProcDrv_CmdArgs  cmdArgs;

    GT_1trace (curTrace, GT_ENTER, "MultiProc_setLocalId", id);

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (MultiProc_state.refCount < 1) {
    /*! @retval MULTIPROC_E_INVALIDSTATE Module is in invalidstate */
        status = MULTIPROC_E_INVALIDSTATE;
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MultiProc_setLocalId",
                             status,
                             "Module is in invalidstate!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        cmdArgs.args.setLocalId.id = id;
        status = MultiProcDrv_ioctl (CMD_MULTIPROC_SETLOCALID, &cmdArgs);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "MultiProc_setLocalId",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
        }
        else{
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            MultiProc_state.cfg.id = id;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        }
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_1trace (curTrace, GT_LEAVE, "MultiProc_setLocalId", status);

    /*! @retval MULTIPROC_SUCCESS ID Operation successfully completed. */
    return (status);
}


/*!
 *  @brief      Function to get proccesor Id from proccessor name.
 *
 *  @param      name  Processor name for which id is to be retrieved
 *
 *  @sa         MultiProc_getMaxProcessors
 */
UInt16
MultiProc_getId (String name)
{
    Bool   found = FALSE;
    UInt32 id    = MULTIPROC_INVALIDID;
    Int    i;

    GT_1trace (curTrace, GT_ENTER, "MultiProc_getId", name);

    GT_assert(curTrace, (MultiProc_state.refCount > 0));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (MultiProc_state.refCount < 1) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MultiProc_getId",
                             MULTIPROC_E_INVALIDSTATE,
                             "Module is in invalidstate!");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        /* If the name is NULL, simply return the local id */
        if (name == NULL) {
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (MultiProc_state.cfg.id == MULTIPROC_INVALIDID){
                /*! @retval MULTIPROC_INVALIDID MultiProc_localId not set to
                                                proper value */
                GT_setFailureReason (curTrace,
                                   GT_4CLASS,
                                   "MultiProc_getId",
                                   MULTIPROC_E_INVALIDID,
                                   "MultiProc_localId not set to proper value");
            }
            else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
                GT_assert (curTrace,
                           (MultiProc_state.cfg.id != MULTIPROC_INVALIDID));
                /*! @retval ID If name is NULL, local ID */
                id = MultiProc_state.cfg.id;
                found = TRUE;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        }
        else {
            for (i = 0; i < MultiProc_state.cfg.maxProcessors ; i++) {
                if (   String_cmp (name, &MultiProc_state.cfg.nameList [i][0])
                    == 0) {
                    id = i;
                    found = TRUE;
                    break;
                }
            }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            if (!found) {
                /*! @retval MULTIPROC_INVALIDID Processor name does not exist */
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "MultiProc_getId",
                                     MULTIPROC_E_INVALIDNAME,
                                     "Processor name does not exist");
            }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        }
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_assert (curTrace, (found == TRUE));

    GT_1trace (curTrace, GT_1CLASS, "    MultiProc_getId [%d]", id);
    GT_1trace (curTrace, GT_LEAVE, "MultiProc_getId", id);

    /*! @retval ID Success: ID corresponding to the specified name */
    return (id);
}

/*!
 *  @brief      Function to get name from processor id.
 *
 *  @param      id  processor id for which proc name is to be retrieved
 *
 *  @sa
 */
String
MultiProc_getName (UInt16 id)
{
    String name = NULL;

    GT_1trace (curTrace, GT_ENTER, "MultiProc_getName", id);

    GT_assert (curTrace, (id < MULTIPROC_MAXPROCESSORS));

#if !defined(SYSLINK_BUILD_OPTIMIZE)
    if (MultiProc_state.refCount < 1) {
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "MultiProc_getName",
                             MULTIPROC_E_INVALIDSTATE,
                             "Module is in invalidstate!");
    }
    else if (id >= MULTIPROC_MAXPROCESSORS) {
        /*! @retval NULL Processor id is not less than MULTIPROC_MAXPROCESSORS*/
        GT_setFailureReason (curTrace,
                    GT_4CLASS,
                    "MultiProc_getName",
                    MULTIPROC_E_INVALIDID,
                    "Processor id is not less than MULTIPROC_MAXPROCESSORS");
    }
    else {
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
        name = MultiProc_state.cfg.nameList [id];
        GT_1trace (curTrace, GT_1CLASS, "    MultiProc_getName [%s]", name);
#if !defined(SYSLINK_BUILD_OPTIMIZE)
    }
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */

    GT_0trace (curTrace, GT_LEAVE, "MultiProc_getName");

    /*! @retval name Success: Processor name for given proc Id*/
    return (name);
}


/*!
 *  @brief      Function to get maximum proc Id in the system.
 *
 *  @sa         MultiProc_getId
 */
UInt16
MultiProc_getMaxProcessors (Void)
{
    /*! @retval MULTIPROC_MAXPROCESSORS Maximum processors in the system. */
    return (MultiProc_state.cfg.maxProcessors);
}




#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


