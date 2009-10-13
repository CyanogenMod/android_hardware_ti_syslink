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
 *  @file   SysMemMgr.h
 *
 *  @brief      Manager for the system memory. System level memory are allocated
 *              through this module.
 *
 *  ============================================================================
 */


#ifndef SYSTEMMEMORYMANAGER_H_0xb53d
#define SYSTEMMEMORYMANAGER_H_0xb53d


#if defined (__cplusplus)
extern "C" {
#endif


/*!
 *  @def    SYSMEMMGR_MODULEID
 *  @brief  Module identifier for System memory manager.
 */
#define SYSMEMMGR_MODULEID   (0xb53d)

/*!
 *  @def    SYSMEMMGR_STATUSCODEBASE
 *  @brief  Error code base for system memory manager module.
 */
#define SYSMEMMGR_STATUSCODEBASE         (SYSMEMMGR_MODULEID << 12u)

/*!
 *  @def    SYSMEMMGR_MAKE_ERROR
 *  @brief  Macro to make error code.
 */
#define SYSMEMMGR_MAKE_ERROR(x)         ((Int) (  0x80000000                   \
                                                + (SYSMEMMGR_STATUSCODEBASE    \
                                                + (x))))

/*!
 *  @def    SYSMEMMGR_MAKE_SUCCESS
 *  @brief  Macro to make success code.
 */
#define SYSMEMMGR_MAKE_SUCCESS(x)       (SYSMEMMGR_STATUSCODEBASE + (x))

/*!
 *  @def    SYSMEMMGR_E_CREATELOCK
 *  @brief  Mutex lock creation failed.
 */
#define SYSMEMMGR_E_CREATELOCK           SYSMEMMGR_MAKE_ERROR(0)

/*!
 *  @def    SYSMEMMGR_E_INVALIDSTATE
 *  @brief  Module is not initialized.
 */
#define SYSMEMMGR_E_INVALIDSTATE         SYSMEMMGR_MAKE_ERROR(1)

/*!
 *  @def    SYSMEMMGR_E_INVALIDARG
 *  @brief  Argument passed to a function is invalid.
 */
#define SYSMEMMGR_E_INVALIDARG           SYSMEMMGR_MAKE_ERROR(2)

/*!
 *  @def    SYSMEMMGR_E_MEMORY
 *  @brief  Memory allocation failed.
 */
#define SYSMEMMGR_E_MEMORY               SYSMEMMGR_MAKE_ERROR(3)

/*!
 *  @def    SYSMEMMGR_E_FAIL
 *  @brief  General failure.
 */
#define SYSMEMMGR_E_FAIL                 SYSMEMMGR_MAKE_ERROR(4)

/*!
 *  @def    SYSMEMMGR_E_ALREADYOPENED
 *  @brief  Internal OS Driver is already opened.
 */
#define SYSMEMMGR_E_ALREADYOPENED         SYSMEMMGR_MAKE_ERROR(5)

/*!
 *  @def    SYSMEMMGR_E_OSFAILURE
 *  @brief  OS Failure.
 */
#define SYSMEMMGR_E_OSFAILURE            SYSMEMMGR_MAKE_ERROR(6)

/*!
 *  @def    SYSMEMMGR_SUCCESS
 *  @brief  Operation successful.
 */
#define SYSMEMMGR_SUCCESS                SYSMEMMGR_MAKE_SUCCESS(0)

/*!
 *  @def    SYSMEMMGR_S_ALREADYSETUP
 *  @brief  Module already initialized.
 */
#define SYSMEMMGR_S_ALREADYSETUP         SYSMEMMGR_MAKE_SUCCESS(1)

/*!
 *  @def    SYSMEMMGR_S_DRVALREADYOPENED
 *  @brief  Internal OS Driver is already opened.
 */
#define SYSMEMMGR_S_DRVALREADYOPENED     SYSMEMMGR_MAKE_SUCCESS(2)


/*!
 *  @brief  Configuration data structure of system memory manager.
 */
typedef struct SysMemMgr_Config {
    UInt32      sizeOfValloc;
    /*!< Total size for virtual memory allocation */
    UInt32      sizeOfPalloc;
    /*!< Total size for physical memory allocation */
    UInt32      staticPhysBaseAddr;
    /*!< Physical address of static memory region */
    UInt32      staticVirtBaseAddr;
    /*!< Virtual  address of static memory region */
    UInt32      staticMemSize;
    /*!< size of static memory region */
    UInt32      pageSize;
    /*!< Page size */
    UInt32      eventNo;
    /*!< Event number to be used */
} SysMemMgr_Config;


/*!
 *  @brief  Flag used for allocating memory blocks.
 */
typedef enum SysMemMgr_AllocFlag {
    SysMemMgr_AllocFlag_Cached   = 0x0001u,
    /*!< Flag used for allocating cacheable block */
    SysMemMgr_AllocFlag_Uncached = 0x0002u,
    /*!< Flag used for allocating uncacheable block */
    SysMemMgr_AllocFlag_Physical = 0x0004u,
    /*!< Flag used for allocating physically contiguous block */
    SysMemMgr_AllocFlag_Virtual  = 0x0008u,
    /*!< Flag used for allocating virtually contiguous block */
    SysMemMgr_AllocFlag_Dma      = 0x0010u
    /*!< Flag used for allocating DMAable (physically contiguous) block */
} SysMemMgr_AllocFlag;

/*!
 *  @brief  Flag used for translating address.
 */
typedef enum SysMemMgr_XltFlag {
    SysMemMgr_XltFlag_Kvirt2Phys  = 0x0001u,
    /*!< Flag used for converting Kernel virtual address to physical address */
    SysMemMgr_XltFlag_Kvirt2Uvirt = 0x0002u,
    /*!< Flag used for converting Kernel virtual address to user virtual address */
    SysMemMgr_XltFlag_Uvirt2Phys  = 0x0004u,
    /*!< Flag used for converting user virtual address to physical address */
    SysMemMgr_XltFlag_Uvirt2Kvirt = 0x0008u,
    /*!< Flag used for converting user virtual address to Kernel virtual address */
    SysMemMgr_XltFlag_Phys2Kvirt  = 0x0010u,
    /*!< Flag used for converting physical address to user virtual address */
    SysMemMgr_XltFlag_Phys2Uvirt  = 0x0011u
    /*!< Flag used for converting physical address to Kernel virtual address */
} SysMemMgr_XltFlag;


/* Function prototypes */
Void SysMemMgr_getConfig (SysMemMgr_Config * params);
Int  SysMemMgr_setup     (SysMemMgr_Config * params);
Int  SysMemMgr_destroy   (Void);
Ptr  SysMemMgr_alloc     (UInt32 size, SysMemMgr_AllocFlag flag);
Int  SysMemMgr_free      (Ptr blk, UInt32 size, SysMemMgr_AllocFlag flag);
Ptr  SysMemMgr_translate (Ptr srcAddr, SysMemMgr_XltFlag flag);


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* SYSTEMMEMORYMANAGER_H_0xb53d */
