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
 *  @file   hal_platform.h
 *
 *
 *  @desc   Contains platform and Operating System specific definitions.
 *  ============================================================================
 */


#if !defined (HAL_PLATFORM_H)
#define HAL_PLATFORM_H


/*  ----------------------------------- OS Headers                  */
#include <linux/autoconf.h>
#include <linux/spinlock.h>
#include <asm/arch/io.h>
#include <asm/arch/hardware.h>


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


/*  ============================================================================
 *  @const  CORE_CM_BASE, MAILBOX_BASE
 *
 *  @desc   Mailbox interrupt related System configuration addresses
 *  ============================================================================
 */
#define CORE_CM_BASE           0x48004A00
#define MAILBOX_BASE           0x48094000

/*  ============================================================================
 *  @const  HAL_MAILBOX_BASE, HAL_CORECM_BASE
 *
 *  @desc   Mailbox interrupt related System configuration addresses as mapped
 *          in the Linux base port.
 *  ============================================================================
 */
#define HAL_CORECM_BASE               IO_ADDRESS(CORE_CM_BASE)
#define HAL_MAILBOX_BASE              IO_ADDRESS(MAILBOX_BASE)


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif  /* !defined (HAL_PLATFORM_H) */
