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
 *  @file   UsrUtilsDrv.h
 *
 *  @brief      Initializes and finalizes user side utils
 *              Mainly created for future use. All setup/destroy APIs on user
 *              side will be call from this module. This allows syslink to work
 *              independent od system manager.
 *  ============================================================================
 */


#ifndef USRUTILS_H
#define USRUTILS_H


#if defined (__cplusplus)
extern "C" {
#endif


/* Function to initialize user side utils */
Void UsrUtilsDrv_setup (Void);
/* Function to finalize user side utils */
Void UsrUtilsDrv_destroy(Void);


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* USRUTILS_H */
