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
 *  @file   bin_loader.h
 *
 *  @path   $(DSPLINK)/gpp/src/gen/PrOS/
 *
 *  @desc   Defines Binary Loader interface.
 *          The binary loader loads a DSP executable, which is in the binary
 *          format. The loader expects a specific structure within the baseImage
 *          field of the loader object. This structure includes information such
 *          as the GPP address of the location of the binary file, the DSP load
 *          and run addresses, file size etc.
 *          Using this information, the loader directly writes the binary file
 *          contents in a single load into DSP memory at the specified DSP load
 *          address.
 */


#if !defined (BIN_LOADER_H)
#define BIN_LOADER_H


/*  ----------------------------------- DSP/BIOS Link               */
#include <dsplink.h>
#include <loaderdefs.h>


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/** ============================================================================
 *  @func   BINLOADER_init
 *
 *  @desc   Initializes the context object for loading a base image file or
 *          a section. This function is required to be called before any other
 *          function is called from this sub component.
 *
 *  @arg    procId
 *              Processor Identifier for the target DSP.
 *  @arg    baseImage
 *              Identifier for the file.
 *  @arg    args
 *              Arguments for initialization of the loader.
 *  @arg    objCtx
 *              Location to receive the context object for the loader.
 *
 *  @ret    DSP_SOK
 *              Operation successfully completed.
 *          DSP_EFILE
 *              File not found.
 *          DSP_EMEMORY
 *              Out of memory error.
 *
 *  @enter  procId must be valid.
 *          baseImage must not be NULL.
 *          contextObj must not be NULL.
 *
 *  @leave  None
 *
 *  @see    BINLOADER_exit ()
 *  ============================================================================
 */
NORMAL_API
DSP_STATUS
BINLOADER_init (IN  ProcessorId      procId,
                IN  Pstr             baseImage,
                IN  LoaderInitArgs * args,
                OUT Pvoid *          objCtx) ;


/** ============================================================================
 *  @func   BINLOADER_exit
 *
 *  @desc   Deallocates the object(s) created by BINLOADER_init function
 *          and releases the context.
 *
 *  @arg    objCtx
 *              The context object obtained through BINLOADER_init.
 *
 *  @ret    DSP_SOK
 *              Operation successfully completed.
 *          DSP_EFILE
 *              File is not open.
 *          DSP_EPOINTER
 *              Invalid file object.
 *
 *  @enter  objCtx must be valid.
 *
 *  @leave  None
 *
 *  @see    BINLOADER_init ()
 *  ============================================================================
 */
NORMAL_API
DSP_STATUS
BINLOADER_exit (IN  Pvoid objCtx) ;


/*  ============================================================================
 *  @func   BINLOADER_load
 *
 *  @desc   Loads the binary format file on the DSP. This is called by
 *          PMGR_PROC_load through the function pointer table.
 *
 *  @arg    procId
 *              Target DSP identifier where the base image is to be loaded.
 *  @arg    loaderObj
 *              This object is used to receive arguments from PMGR.
 *  @arg    argc
 *              Number of arguments to be passed to the base image upon start.
 *  @arg    argv
 *              Arguments to be passed to DSP main application.
 *  @arg    entryPt
 *              OUT argument for returning entry address for the executable.
 *
 *  @ret    DSP_SOK
 *              Operation successfully completed.
 *          DSP_EINVALIDARG
 *              Invalid ProcId argument.
 *          DSP_EMEMORY
 *              Memory Error.
 *          DSP_EFILE
 *              Invalid base image.
 *          DSP_EFAIL
 *              General Failure, unable to load image on DSP
 *
 *  @enter  procId must be valid.
 *          loaderObj must be a valid pointer.
 *          entryPt must be a valid pointer.
 *          If argc is 0 then argv must be NULL pointer.
 *          If argc is non-zero then argv must be a valid pointer.
 *
 *  @leave  None
 *
 *  @see    PMGR_PROC_load
 *  ============================================================================
 */
NORMAL_API
DSP_STATUS
BINLOADER_load (IN  ProcessorId     procId,
                IN  LoaderObject *  loaderObj,
                IN  Uint32          argc,
                IN  Char8 **        argv,
                OUT Uint32 *        entryPt) ;


/*  ============================================================================
 *  @func   BINLOADER_loadSection
 *
 *  @desc   Loads a section from the DSP executable onto the DSP. PMGR_PROC_load
 *          calls this through the function pointer table.
 *
 *  @arg    procId
 *              DSP identifier.
 *  @arg    loaderObj
 *              This object is used to receive arguments from PMGR.
 *  @arg    sectId
 *              Identifier for section to load.
 *
 *  @ret    DSP_ENOTIMPL
 *              Function not implemented.
 *
 *  @enter  procId must be valid.
 *          loaderObj must be a valid pointer.
 *          sectId must be a valid section identifier.
 *
 *  @leave  None
 *
 *  @see    PMGR_PROC_load
 *  ============================================================================
 */
NORMAL_API
DSP_STATUS
BINLOADER_loadSection (IN  ProcessorId     procId,
                       IN  LoaderObject *  loaderObj,
                       IN  Uint32          sectId) ;


/** ============================================================================
 *  @func   BINLOADER_getSymbolAddress
 *
 *  @desc   Gets the DSP address corresponding to a symbol within a DSP
 *          executable currently loaded on the DSP.
 *
 *  @arg    procId
 *              DSP identifier.
 *  @arg    objCtx
 *              Context of the loader object.
 *  @arg    symbolName
 *              Name of the symbol to be looked up.
 *  @arg    dspAddr
 *              Location to receive the DSP address for the specified symbol.
 *
 *  @ret    DSP_SOK
 *              Operation successfully completed.
 *          DSP_EINVALIDARG
 *              Invalid argument.
 *          DSP_ENOTSUPPORTED
 *              Symbol lookup feature is not supported by the loader.
 *          DSP_ENOTFOUND
 *              Symbol not found.
 *          DSP_EFAIL
 *              General failure.
 *
 *  @enter  procId must be valid.
 *          objCtx must be valid pointer.
 *          symbolName must be valid pointer.
 *          dspAddr must be valid pointer.
 *
 *  @leave  On success, dspAddr contains a valid DSP address.
 *          On failure, dspAddr is NULL.
 *
 *  @see    None
 *  ============================================================================
 */
NORMAL_API
DSP_STATUS
BINLOADER_getSymbolAddress (IN   ProcessorId     procId,
                            IN   Void *          objCtx,
                            IN   Char8 *         symbolName,
                            OUT  Uint32 *        dspAddr) ;


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* !defined (BIN_LOADER_H) */
