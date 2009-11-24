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
 *  @file   Config.h
 *
 *  @brief  Configuration flags to use during normal build.
 *
 *  ============================================================================
 */


#ifndef _CONFIG_H_
#define _CONFIG_H_


#if defined (__cplusplus)
extern "C" {
#endif


/*!
 * @def   HAVE_POSIX_MEMALIGN
 * @brief Use Posix Memory Aligned API
 */
#define HAVE_POSIX_MEMALIGN            1


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* ifndef _CONFIG_H_ */
