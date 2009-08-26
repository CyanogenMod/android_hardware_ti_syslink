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
 *  @file   MultiProcDrv.h
 *
 *  @brief      header file for MultiProcDrv on HLOS side
 *
 *  ============================================================================
 */


#ifndef MULTIPROC_DRV_H_0xf2ba
#define MULTIPROC_DRV_H_0xf2ba


#if defined (__cplusplus)
extern "C" {
#endif


/*Function to open MultiProc driver*/
Int MultiProcDrv_open (Void);

/*Function to close MultiProc driver*/
Int MultiProcDrv_close (Void);

/*Function to execute IOCTL operations*/
Int MultiProcDrv_ioctl (UInt32 cmd, Ptr args);


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* MULTIPROC_DRV_H_0xf2ba */
