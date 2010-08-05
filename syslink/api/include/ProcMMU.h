/*
 *  Syslink-IPC for TI OMAP Processors
 *
 *  Copyright (c) 2008-2010, Texas Instruments Incorporated
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  *  Neither the name of Texas Instruments Incorporated nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 *  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*============================================================================
 *  @file   ProcMMU.h
 *
 *  @brief  Enabler for Ducati processor
 *
 *  ============================================================================
 */

#ifndef _PROCMMU_H_
#define _PROCMMU_H_

/* OSAL & Utils headers */
#include <Std.h>
#include <OsalPrint.h>

#if defined (__cplusplus)
extern "C" {
#endif

/*!
 *  @def    PROCMMU_MODULEID
 *  @brief  Module ID for ProcMgr.
 */
#define PROCMMU_MODULEID            (UInt16) 0xbabe

/* =============================================================================
 *  All success and failure codes for the module
 * =============================================================================
 */

/*!
 *  @def    PROCMMU_STATUSCODEBASE
 *  @brief  Error code base for ProcMgr.
 */
#define PROCMMU_STATUSCODEBASE      (PROCMMU_MODULEID << 12u)

/*!
 *  @def    PROCMMU_MAKE_FAILURE
 *  @brief  Macro to make failure code.
 */
#define PROCMMU_MAKE_FAILURE(x)     ((Int)(  0x80000000                        \
                                      | (PROCMMU_STATUSCODEBASE + (x))))

/*!
 *  @def    PROCMMU_MAKE_SUCCESS
 *  @brief  Macro to make success code.
 */
#define PROCMMU_MAKE_SUCCESS(x)     (PROCMMU_STATUSCODEBASE + (x))

/*!
 *  @def    ProcMMU_E_INVALIDARG
 *  @brief  Argument passed to a function is invalid.
 */
#define ProcMMU_E_INVALIDARG        PROCMMU_MAKE_FAILURE(1)

/*!
 *  @def    ProcMMU_E_MEMORY
 *  @brief  Memory allocation failed.
 */
#define ProcMMU_E_MEMORY            PROCMMU_MAKE_FAILURE(2)

/*!
 *  @def    ProcMMU_E_FAIL
 *  @brief  Generic failure.
 */
#define ProcMMU_E_FAIL              PROCMMU_MAKE_FAILURE(3)

/*!
 *  @def    ProcMMU_E_FAIL
 *  @brief  OS returned failure.
 */
#define ProcMMU_E_OSFAILURE         PROCMMU_MAKE_FAILURE(4)

/*!
 *  @def    PROCMMU_SUCCESS
 *  @brief  Operation successful.
 */
#define ProcMMU_S_SUCCESS           PROCMMU_MAKE_SUCCESS(0)

/*!
 *  @def    PROCMMU_S_ALREADYSETUP
 *  @brief  The ProcMgr module has already been setup in this process.
 */
#define ProcMMU_S_ALREADYSETUP      PROCMMU_MAKE_SUCCESS(1)

/*!
 *  @def    PROCMMU_S_OPENHANDLE
 *  @brief  Other ProcMgr clients have still setup the ProcMgr module.
 */
#define ProcMMU_S_SETUP             PROCMMU_MAKE_SUCCESS(2)

/*!
 *  @def    PROCMMU_S_OPENHANDLE
 *  @brief  Other ProcMgr handles are still open in this process.
 */
#define ProcMMU_S_OPENHANDLE        PROCMMU_MAKE_SUCCESS(3)

#define DUCATI_BASEIMAGE_PHYSICAL_ADDRESS    0x9CF00000
#define TESLA_BASEIMAGE_PHYSICAL_ADDRESS     0x9CC00000

#define PAGE_SIZE_4KB                   0x1000
#define PAGE_SIZE_64KB                  0x10000
#define PAGE_SIZE_1MB                   0x100000
#define PAGE_SIZE_16MB                  0x1000000

/* Define the Peripheral PAs and their Ducati VAs. */
#define L4_PERIPHERAL_MBOX              0x4A0F4000
#define DUCATI_PERIPHERAL_MBOX          0xAA0F4000

#define L4_PERIPHERAL_I2C1              0x48070000
#define DUCATI_PERIPHERAL_I2C1          0xA8070000
#define L4_PERIPHERAL_I2C2              0x48072000
#define DUCATI_PERIPHERAL_I2C2          0xA8072000
#define L4_PERIPHERAL_I2C3              0x48060000
#define DUCATI_PERIPHERAL_I2C3          0xA8060000

#define L4_PERIPHERAL_DMA               0x4A056000
#define DUCATI_PERIPHERAL_DMA           0xAA056000

#define L4_PERIPHERAL_GPIO1             0x4A310000
#define DUCATI_PERIPHERAL_GPIO1         0xAA310000
#define L4_PERIPHERAL_GPIO2             0x48055000
#define DUCATI_PERIPHERAL_GPIO2         0xA8055000
#define L4_PERIPHERAL_GPIO3             0x48057000
#define DUCATI_PERIPHERAL_GPIO3         0xA8057000

#define L4_PERIPHERAL_GPTIMER3          0x48034000
#define DUCATI_PERIPHERAL_GPTIMER3      0xA8034000
#define L4_PERIPHERAL_GPTIMER4          0x48036000
#define DUCATI_PERIPHERAL_GPTIMER4      0xA8036000
#define L4_PERIPHERAL_GPTIMER9          0x48040000
#define DUCATI_PERIPHERAL_GPTIMER9      0xA8040000
#define L4_PERIPHERAL_GPTIMER11         0x48088000
#define DUCATI_PERIPHERAL_GPTIMER11     0xA8088000

#define L4_PERIPHERAL_UART1             0x4806A000
#define DUCATI_PERIPHERAL_UART1         0xA806A000
#define L4_PERIPHERAL_UART2             0x4806C000
#define DUCATI_PERIPHERAL_UART2         0xA806C000
#define L4_PERIPHERAL_UART3             0x48020000
#define DUCATI_PERIPHERAL_UART3         0xA8020000
#define L4_PERIPHERAL_UART4             0x4806E000
#define DUCATI_PERIPHERAL_UART4         0xA806E000


#define L3_TILER_VIEW0_ADDR             0x60000000
#define DUCATIVA_TILER_VIEW0_ADDR       0x60000000
#define DUCATIVA_TILER_VIEW0_LEN        0x20000000


/* OMAP4430 SDC definitions */
#define L4_PERIPHERAL_L4CFG             0x4A000000
#define DUCATI_PERIPHERAL_L4CFG         0xAA000000
#define TESLA_PERIPHERAL_L4CFG          0x4A000000

#define L4_PERIPHERAL_L4PER             0x48000000
#define DUCATI_PERIPHERAL_L4PER         0xA8000000
#define TESLA_PERIPHERAL_L4PER          0x48000000

#define L4_PERIPHERAL_L4ABE             0x49000000
#define DUCATI_PERIPHERAL_L4ABE         0xA9000000
#define TESLA_PERIPHERAL_L4ABE          0x49000000

#define L3_IVAHD_CONFIG                 0x5A000000
#define DUCATI_IVAHD_CONFIG             0xBA000000
#define TESLA_IVAHD_CONFIG              0xBA000000

#define L3_IVAHD_SL2                    0x5B000000
#define DUCATI_IVAHD_SL2                0xBB000000
#define TELSA_IVAHD_SL2                 0xBB000000

#define L3_TILER_MODE0_1_ADDR           0x60000000
#define DUCATI_TILER_MODE0_1_ADDR       0x60000000
#define DUCATI_TILER_MODE0_1_LEN        0x10000000
#define TESLA_TILER_MODE0_1_ADDR        0x60000000
#define TESLA_TILER_MODE0_1_LEN         0x10000000

#define L3_TILER_MODE3_ADDR             0x78000000
#define DUCATI_TILER_MODE3_ADDR         0x78000000
#define DUCATI_TILER_MODE3_LEN          0x8000000
#define TESLA_TILER_MODE3_ADDR          0x78000000
#define TESLA_TILER_MODE3_LEN           0x8000000

#define DUCATI_BOOTVECS_UNUSED_ADDR     0x1000
#define DUCATI_BOOTVECS_UNUSED_LEN      0x3000

#define DUCATI_MEM_CODE_SYSM3_ADDR      0x4000
#define DUCATI_MEM_CODE_SYSM3_LEN       0xFC000

#define DUCATI_MEM_CODE_APPM3_ADDR      0x100000
#define DUCATI_MEM_CODE_APPM3_LEN       0x300000

#define DUCATI_MEM_CONST_SYSM3_ADDR     0x80000000
#define DUCATI_MEM_CONST_SYSM3_LEN      0x40000

#define DUCATI_MEM_HEAP_SYSM3_ADDR      0x80040000
#define DUCATI_MEM_HEAP_SYSM3_LEN       0xC0000

#define DUCATI_MEM_CONST_APPM3_ADDR     0x80100000
#define DUCATI_MEM_CONST_APPM3_LEN      0x180000

#define DUCATI_MEM_HEAP_APPM3_ADDR      0x80280000
#define DUCATI_MEM_HEAP_APPM3_LEN       0x1D60000

#define DUCATI_MEM_TRACEBUF_ADDR        0x81FE0000
#define DUCATI_MEM_TRACEBUF_LEN         0x20000

#define DUCATI_MEM_IPC_HEAP0_ADDR       0xA0000000
#define DUCATI_MEM_IPC_HEAP0_LEN        0x54000
#define TESLA_MEM_IPC_HEAP0_ADDR        0x30000000

#define DUCATI_MEM_IPC_HEAP1_ADDR       0xA0054000
#define DUCATI_MEM_IPC_HEAP1_LEN        0xAC000
#define TESLA_MEM_IPC_HEAP1_ADDR        0x30054000

#define TESLA_MEM_CODE_ADDR             0x20000000
#define TESLA_MEM_CODE_LEN              0x80000

#define TESLA_MEM_CONST_ADDR            0x20080000
#define TESLA_MEM_CONST_LEN             0x80000

#define TESLA_MEM_HEAP_ADDR             0x20100000
#define TESLA_MEM_HEAP_LEN              0x1F0000

#define TESLA_MEM_EXT_RAM_ADDR          0x20000000
#define TESLA_MEM_EXT_RAM_LEN           0x300000

/* Types of mapping attributes */

/* MPU address is virtual and needs to be translated to physical addr */
#define DSP_MAPVIRTUALADDR              0x00000000
#define DSP_MAPPHYSICALADDR             0x00000001

/* Mapped data is big endian */
#define DSP_MAPBIGENDIAN                0x00000002
#define DSP_MAPLITTLEENDIAN             0x00000000

/* Element size is based on DSP r/w access size */
#define DSP_MAPMIXEDELEMSIZE            0x00000004

/*
 * Element size for MMU mapping (8, 16, 32, or 64 bit)
 * Ignored if DSP_MAPMIXEDELEMSIZE enabled
 */
#define DSP_MAPELEMSIZE8                0x00000008
#define DSP_MAPELEMSIZE16               0x00000010
#define DSP_MAPELEMSIZE32               0x00000020
#define DSP_MAPELEMSIZE64               0x00000040

#define DSP_MAPVMALLOCADDR              0x00000080
#define DSP_MAPTILERADDR                0x00000100


#define PG_MASK(pg_size) (~((pg_size)-1))
#define PG_ALIGN_LOW(addr, pg_size) ((addr) & PG_MASK(pg_size))
#define PG_ALIGN_HIGH(addr, pg_size) (((addr)+(pg_size)-1) & PG_MASK(pg_size))


struct Mmu_entry {
   UInt32 physAddr;
   UInt32 virtAddr;
   UInt32 size;
};

struct Memory_entry {
   UInt32 virtAddr;
   UInt32 size;
};

struct Iotlb_entry {
    UInt32 da;
    UInt32 pa;
    UInt32 pgsz, prsvd, valid;
    union {
        UInt16 ap;
        struct {
            UInt32 endian, elsz, mixed;
        };
    };
};

#define MMU_CAM_P                   (1 << 3)
#define MMU_CAM_V                   (1 << 2)
#define MMU_CAM_PGSZ_MASK           3
#define MMU_CAM_PGSZ_1M             (0 << 0)
#define MMU_CAM_PGSZ_64K            (1 << 0)
#define MMU_CAM_PGSZ_4K             (2 << 0)
#define MMU_CAM_PGSZ_16M            (3 << 0)

#define MMU_RAM_PADDR_SHIFT         12
#define MMU_RAM_PADDR_MASK \
                        ((~0UL >> MMU_RAM_PADDR_SHIFT) << MMU_RAM_PADDR_SHIFT)
#define MMU_RAM_ENDIAN_SHIFT        9
#define MMU_RAM_ENDIAN_MASK         (1 << MMU_RAM_ENDIAN_SHIFT)
#define MMU_RAM_ENDIAN_BIG          (1 << MMU_RAM_ENDIAN_SHIFT)
#define MMU_RAM_ENDIAN_LITTLE       (0 << MMU_RAM_ENDIAN_SHIFT)
#define MMU_RAM_ELSZ_SHIFT          7
#define MMU_RAM_ELSZ_MASK           (3 << MMU_RAM_ELSZ_SHIFT)
#define MMU_RAM_ELSZ_8              (0 << MMU_RAM_ELSZ_SHIFT)
#define MMU_RAM_ELSZ_16             (1 << MMU_RAM_ELSZ_SHIFT)
#define MMU_RAM_ELSZ_32             (2 << MMU_RAM_ELSZ_SHIFT)
#define MMU_RAM_ELSZ_NONE           (3 << MMU_RAM_ELSZ_SHIFT)
#define MMU_RAM_MIXED_SHIFT         6
#define MMU_RAM_MIXED_MASK          (1 << MMU_RAM_MIXED_SHIFT)
#define MMU_RAM_MIXED               MMU_RAM_MIXED_MASK

#define IOMMU_IOC_MAGIC             'I'

#define IOMMU_IOCSETTLBENT          _IO(IOMMU_IOC_MAGIC, 0)

/* OMAP4430 SDC definitions */
static const struct Mmu_entry L4Map[] = {
    /* TILER 8-bit and 16-bit modes */
    {L3_TILER_MODE0_1_ADDR, DUCATI_TILER_MODE0_1_ADDR,
        (PAGE_SIZE_16MB * 16)},
    /* TILER: Pages-mode */
    {L3_TILER_MODE3_ADDR, DUCATI_TILER_MODE3_ADDR,
        (PAGE_SIZE_16MB * 8)},
    /*  L4_CFG: Covers all modules in L4_CFG 16MB*/
    {L4_PERIPHERAL_L4CFG, DUCATI_PERIPHERAL_L4CFG, PAGE_SIZE_16MB},
    /*  L4_PER: Covers all modules in L4_PER 16MB*/
    {L4_PERIPHERAL_L4PER, DUCATI_PERIPHERAL_L4PER, PAGE_SIZE_16MB},
    /* IVA_HD Config: Covers all modules in IVA_HD Config space 16MB */
    {L3_IVAHD_CONFIG, DUCATI_IVAHD_CONFIG, PAGE_SIZE_16MB},
    /* IVA_HD SL2: Covers all memory in IVA_HD SL2 space 16MB */
    {L3_IVAHD_SL2, DUCATI_IVAHD_SL2, PAGE_SIZE_16MB},
};

static const struct  Memory_entry L3MemoryRegions[] = {
    /*  MEM_IPC_HEAP0, MEM_IPC_HEAP1, MEM_IPC_HEAP2 */
    {DUCATI_MEM_IPC_HEAP0_ADDR, PAGE_SIZE_1MB},
    /*  MEM_INTVECS_SYSM3, MEM_INTVECS_APPM3, MEM_CODE_SYSM3,
       MEM_CODE_APPM3 */
    {0, PAGE_SIZE_16MB},
    /*  MEM_CONST_SYSM3, MEM_CONST_APPM3, MEM_HEAP_SYSM3, MEM_HEAP_APPM3,
       MEM_MPU_DUCATI_SHMEM, MEM_IPC_SHMEM */
    {DUCATI_MEM_CONST_SYSM3_ADDR, (PAGE_SIZE_16MB * 2)},
};

/* OMAP4430 SDC definitions */
static const struct Mmu_entry L4MapDsp[] = {
    /* TILER 8-bit and 16-bit modes */
    {L3_TILER_MODE0_1_ADDR, TESLA_TILER_MODE0_1_ADDR,
        (PAGE_SIZE_16MB * 16)},
    /* TILER: Pages-mode */
    {L3_TILER_MODE3_ADDR, TESLA_TILER_MODE3_ADDR,
        (PAGE_SIZE_16MB * 8)},
    /*  L4_CFG: Covers all modules in L4_CFG 16MB*/
    {L4_PERIPHERAL_L4CFG, TESLA_PERIPHERAL_L4CFG, PAGE_SIZE_16MB},
    /*  L4_PER: Covers all modules in L4_PER 16MB*/
    {L4_PERIPHERAL_L4PER, TESLA_PERIPHERAL_L4PER, PAGE_SIZE_16MB},
    /*  L4_ABE: Covers all modules in L4_ABE 16MB*/
    {L4_PERIPHERAL_L4ABE, TESLA_PERIPHERAL_L4ABE, PAGE_SIZE_16MB},
    /* IVA_HD Config: Covers all modules in IVA_HD Config space 16MB */
    {L3_IVAHD_CONFIG, TESLA_IVAHD_CONFIG, PAGE_SIZE_16MB},
#if 0 /* No room for below entry. Need to be added when PTE is enabled */
    /* IVA_HD SL2: Covers all memory in IVA_HD SL2 space 16MB */
    {L3_IVAHD_SL2, TELSA_IVAHD_SL2, PAGE_SIZE_16MB},
#endif
};

static const struct  Memory_entry L3MemoryRegionsDsp[] = {
    /*  MEM_EXT_RAM */
    {TESLA_MEM_CODE_ADDR, (PAGE_SIZE_1MB * 3)},
    /*  MEM_IPC_HEAP0, MEM_IPC_HEAP1 */
    {TESLA_MEM_IPC_HEAP0_ADDR, PAGE_SIZE_1MB},

};

Int32 ProcMMU_close (Int proc);
Int32 ProcMMU_open (Int proc);
UInt32 ProcMMU_init (UInt32 physAddr, Int proc);

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* _PROCMMU_H_*/
