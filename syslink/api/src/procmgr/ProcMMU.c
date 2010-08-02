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
 *  @file   ProcMMU.c
 *
 *  @brief  Enabler for Ducati processor
 *
 *  ============================================================================
 */

/* OS-specific headers */
#include <host_os.h>
#include <sys/types.h>
#include <fcntl.h>

/* OSAL & Utils headers */
#include <Std.h>
#include <OsalPrint.h>
#include <Trace.h>
#include <UsrUtilsDrv.h>
#include <Memory.h>

/* Module level headers */
#include <ProcMMU.h>

#if defined (__cplusplus)
extern "C" {
#endif

/** ============================================================================
 *  Macros and types
 *  ============================================================================
 */
#define PROC_MMU_MPU_M3_DRIVER_NAME  "/dev/omap-iommu0"

#define PROC_MMU_DSP_DRIVER_NAME     "/dev/omap-iommu1"


/** ============================================================================
 *  Globals
 *  ============================================================================
 */
/*!
 *  @brief  Driver handle for ProcMgr in this process.
 */
static Int32 ProcMMU_MPU_M3_handle = -1;

/*!
 *  @brief  Driver handle for ProcMgr in this process.
 */
static Int32 ProcMMU_DSP_handle = -1;

/*!
 *  @brief  Reference count for the driver handle.
 */
static UInt32 ProcMMU_refCount = 0;

/* Attributes of L2 page tables for DSP MMU.*/
struct pageInfo {
    /* Number of valid PTEs in the L2 PT*/
    UInt32 num_entries;
};

enum pageType {
    SECTION         = 0,
    LARGE_PAGE      = 1,
    SMALL_PAGE      = 2,
    SUPER_SECTION   = 3
};


static UInt32 shmPhysAddr;
static UInt32 shmPhysAddrDsp;


/*
 *  @brief  Decides a TLB entry size
 *
 */
static Int32
ProcMMU_getEntrySize (UInt32 pa, UInt32 size, enum pageType *sizeTlb,
                     UInt32 *entrySize)
{
    Int32   status          = 0;
    Bool    pageAlign4Kb    = FALSE;
    Bool    pageAlign64Kb   = FALSE;
    Bool    pageAlign1Mb    = FALSE;
    Bool    pageAlign16Mb   = FALSE;
    UInt32  physAddr        = pa;

    GT_4trace (curTrace, GT_ENTER, "ProcMMU_getEntrySize", pa, size, sizeTlb,
                entrySize);

    /*  First check the page alignment*/
    if ((physAddr % PAGE_SIZE_4KB)  == 0)
        pageAlign4Kb  = TRUE;
    if ((physAddr % PAGE_SIZE_64KB) == 0)
        pageAlign64Kb = TRUE;
    if ((physAddr % PAGE_SIZE_1MB)  == 0)
        pageAlign1Mb  = TRUE;
    if ((physAddr % PAGE_SIZE_16MB) == 0)
        pageAlign16Mb = TRUE;

    if ((!pageAlign64Kb) && (!pageAlign1Mb)  && (!pageAlign4Kb)) {
        status = ProcMMU_E_INVALIDARG;
#if !defined(SYSLINK_BUILD_OPTIMIZE)
        GT_setFailureReason (curTrace,
                             GT_4CLASS,
                             "ProcMMU_getEntrySize",
                             status,
                             "Invalid page aligment");
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
    }

    /*  Now decide the entry size */
    if (status >= 0) {
        if (size >= PAGE_SIZE_16MB) {
            if (pageAlign16Mb) {
                *sizeTlb   = SUPER_SECTION;
                *entrySize = PAGE_SIZE_16MB;
            } else if (pageAlign1Mb) {
                *sizeTlb   = SECTION;
                *entrySize = PAGE_SIZE_1MB;
            } else if (pageAlign64Kb) {
                *sizeTlb   = LARGE_PAGE;
                *entrySize = PAGE_SIZE_64KB;
            } else if (pageAlign4Kb) {
                *sizeTlb   = SMALL_PAGE;
                *entrySize = PAGE_SIZE_4KB;
            } else {
                status = ProcMMU_E_INVALIDARG;
            }
        } else if (size >= PAGE_SIZE_1MB && size < PAGE_SIZE_16MB) {
            if (pageAlign1Mb) {
                *sizeTlb   = SECTION;
                *entrySize = PAGE_SIZE_1MB;
            } else if (pageAlign64Kb) {
                *sizeTlb   = LARGE_PAGE;
                *entrySize = PAGE_SIZE_64KB;
            } else if (pageAlign4Kb) {
                *sizeTlb   = SMALL_PAGE;
                *entrySize = PAGE_SIZE_4KB;
            } else {
                status = ProcMMU_E_INVALIDARG;
            }
        } else if (size > PAGE_SIZE_4KB &&
                size < PAGE_SIZE_1MB) {
            if (pageAlign64Kb) {
                *sizeTlb   = LARGE_PAGE;
                *entrySize = PAGE_SIZE_64KB;
            } else if (pageAlign4Kb) {
                *sizeTlb   = SMALL_PAGE;
                *entrySize = PAGE_SIZE_4KB;
            } else {
                status = ProcMMU_E_INVALIDARG;
            }
        } else if (size == PAGE_SIZE_4KB) {
            if (pageAlign4Kb) {
                *sizeTlb   = SMALL_PAGE;
                *entrySize = PAGE_SIZE_4KB;
            } else {
                status = ProcMMU_E_INVALIDARG;
            }
        } else {
            status = ProcMMU_E_INVALIDARG;
        }
    }

    GT_1trace (curTrace, GT_LEAVE, "ProcMMU_getEntrySize", status);

    return status;
}


/*!
 *  @brief  Add DSP MMU entries corresponding to given MPU-Physical address
 *          and DSP-virtual address
 *
 *  @sa     ProcMMU_getentrysize
 */
static Int32
ProcMMU_addEntry (UInt32  *physAddr, UInt32 *dspAddr, UInt32 size, UInt32 proc)
{
    UInt32              mappedSize  = 0;
    enum pageType       sizeTlb     = SECTION;
    UInt32              entrySize   = 0;
    Int32               status      = 0;
    struct Iotlb_entry  tlbEntry;

    GT_3trace (curTrace, GT_ENTER, "ProcMMU_addEntry", *physAddr, *dspAddr,
                size);

    while ((mappedSize < size) && (status == 0)) {
        status = ProcMMU_getEntrySize (*physAddr, (size - mappedSize),
                                        &sizeTlb, &entrySize);
        if (status < 0) {
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMMU_addEntry",
                                 status,
                                 "getEntrySize failed!");
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            break;
        }

        if (sizeTlb == SUPER_SECTION)
            tlbEntry.pgsz = MMU_CAM_PGSZ_16M;
        else if (sizeTlb == SECTION)
            tlbEntry.pgsz = MMU_CAM_PGSZ_1M;
        else if (sizeTlb == LARGE_PAGE)
            tlbEntry.pgsz = MMU_CAM_PGSZ_64K;
        else if (sizeTlb == SMALL_PAGE)
            tlbEntry.pgsz = MMU_CAM_PGSZ_4K;

        tlbEntry.elsz   = MMU_RAM_ELSZ_16;
        tlbEntry.endian = MMU_RAM_ENDIAN_LITTLE;
        tlbEntry.mixed  = MMU_RAM_MIXED;
        tlbEntry.prsvd  = MMU_CAM_P;
        tlbEntry.valid  = MMU_CAM_V;
        tlbEntry.da     = *dspAddr;
        tlbEntry.pa     = *physAddr;

        if (proc == 0)
            status = ioctl (ProcMMU_MPU_M3_handle, IOMMU_IOCSETTLBENT, &tlbEntry);
        else if (proc == 1)
            status = ioctl (ProcMMU_DSP_handle, IOMMU_IOCSETTLBENT, &tlbEntry);
        if (status < 0) {
#if !defined(SYSLINK_BUILD_OPTIMIZE)
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMMU_addEntry",
                                 status,
                                 "API (through IOCTL) failed on kernel-side!");
#endif /* if !defined(SYSLINK_BUILD_OPTIMIZE) */
            break;
        }

        mappedSize  += entrySize;
        *physAddr   += entrySize;
        *dspAddr   += entrySize;
    }

    GT_1trace (curTrace, GT_LEAVE, "ProcMMU_addEntry", status);

    return status;
}


/*!
 *  @brief  Function to Program Processor MMU.
 *
 *  @sa     ProcMgrDrvUsr_open
 */
UInt32
ProcMMU_init (UInt32 aPhyAddr)
{
    UInt32 status           = 0;
    UInt32 physAddr         = 0;
    UInt32 numL4Entries;
    UInt32 i                = 0;
    UInt32 numL3MemEntries  = 0;
    UInt32 virtAddr         = 0;

    GT_1trace (curTrace, GT_ENTER, "ProcMMU_init", aPhyAddr);

    numL4Entries = (sizeof(L4Map) / sizeof(struct Mmu_entry));
    numL3MemEntries = sizeof(L3MemoryRegions) /
                      sizeof(struct Memory_entry);

    Osal_printf ("\n  Programming Ducati MMU using linear address \n");
    physAddr = DUCATI_BASEIMAGE_PHYSICAL_ADDRESS;

    Osal_printf ("  Programming Ducati memory regions\n");
    Osal_printf ("=========================================\n");
    for (i = 0; i < numL3MemEntries; i++) {
        Osal_printf ("VA = [0x%x] of size [0x%x] at PA = [0x%x]\n",
                L3MemoryRegions[i].virtAddr, L3MemoryRegions[i].size, physAddr);
        /* OMAP4430 SDC code */
        /* Adjust below logic if using cacheable shared memory */
        if (L3MemoryRegions[i].virtAddr == DUCATI_MEM_IPC_HEAP0_ADDR) {
            shmPhysAddr = physAddr;
        }

        virtAddr = L3MemoryRegions[i].virtAddr;
        status = ProcMMU_addEntry(&physAddr, &virtAddr,
                    (L3MemoryRegions[i].size), 0);
        if (status < 0) {
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMMU_init",
                                 status,
                                 "L3MemoryRegion addEntry failed!");
            break;
        }
    }

    if (status >= 0) {
        Osal_printf ("  Programming Ducati L4 peripherals\n");
        Osal_printf ("=========================================\n");
        for (i = 0; i < numL4Entries; i++) {
            Osal_printf ("PA [0x%x] VA [0x%x] size [0x%x]\n",
                    L4Map[i].physAddr, L4Map[i].virtAddr,
                    L4Map[i].size);
            virtAddr = L4Map[i].virtAddr;
            physAddr = L4Map[i].physAddr;
            status = ProcMMU_addEntry (&physAddr, &virtAddr, (L4Map[i].size), 0);
            if (status < 0) {
                Osal_printf ("**** Failed to map Peripheral ****");
                Osal_printf ("Phys addr [0x%x] Virt addr [0x%x] size [0x%x]",
                               L4Map[i].physAddr, L4Map[i].virtAddr,
                               L4Map[i].size);
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "ProcMMU_init",
                                     status,
                                     "L4Map addEntry failed!");
                break;
            }
        }
    }

    if (status >= 0) {
        numL4Entries = (sizeof(L4MapDsp) / sizeof(struct Mmu_entry));
        numL3MemEntries = sizeof(L3MemoryRegionsDsp) /
                          sizeof(struct Memory_entry);

        Osal_printf ("\n  Programming DSP MMU using linear address \n");
        physAddr = TESLA_BASEIMAGE_PHYSICAL_ADDRESS;

        Osal_printf("  Programming DSP memory regions\n");
        Osal_printf("=========================================\n");
        for (i = 0; i < numL3MemEntries; i++) {
            Osal_printf("VA = [0x%x] of size [0x%x] at PA = [0x%x]\n",
                        L3MemoryRegionsDsp[i].virtAddr,
                        L3MemoryRegionsDsp[i].size,
                        physAddr);
            /* OMAP4430 SDC code */
            /* Adjust below logic if using cacheable shared memory */
            if (L3MemoryRegionsDsp[i].virtAddr == TESLA_MEM_IPC_HEAP0_ADDR) {
                shmPhysAddrDsp = DUCATI_BASEIMAGE_PHYSICAL_ADDRESS;
            }

            virtAddr = L3MemoryRegionsDsp[i].virtAddr;
            status = ProcMMU_addEntry(&physAddr, &virtAddr,
                                      (L3MemoryRegionsDsp[i].size), 1);
            if (status < 0) {
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "ProcMMU_init",
                                     status,
                                     "L3MemoryRegionsDsp addEntry failed!");
                break;
            }
        }
    }

    if (status >= 0) {
        Osal_printf("  Programming DSP L4 peripherals\n");
        Osal_printf("=========================================\n");
        for (i = 0; i < numL4Entries; i++) {
            Osal_printf("PA [0x%x] VA [0x%x] size [0x%x]\n",
                    L4MapDsp[i].physAddr, L4MapDsp[i].virtAddr,
                    L4MapDsp[i].size);
            virtAddr = L4MapDsp[i].virtAddr;
            physAddr = L4MapDsp[i].physAddr;
            status = ProcMMU_addEntry(&physAddr,
                &virtAddr, (L4MapDsp[i].size), 1);
            if (status < 0) {
                Osal_printf ("**** Failed to map Peripheral ****");
                Osal_printf ("Phys addr [0x%x] Virt addr [0x%x] size [0x%x]",
                               L4MapDsp[i].physAddr, L4MapDsp[i].virtAddr,
                               L4MapDsp[i].size);
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "ProcMMU_init",
                                     status,
                                     "L4MapDsp addEntry failed!");
                break;
            }
        }
    }

    GT_1trace (curTrace, GT_LEAVE, "ProcMMU_init", status);

    return status;
}


/*!
 *  @brief  Function to close the ProcMgr driver.
 *
 *  @sa     ProcMMU_open
 */
Int
ProcMMU_close (Void)
{
    Int                 status      = ProcMMU_S_SUCCESS;
    int                 osStatus    = 0;

    GT_0trace (curTrace, GT_ENTER, "ProcMMU_close");

    /* TBD: Protection for refCount. */
    ProcMMU_refCount--;
    if (ProcMMU_refCount == 0) {
        osStatus = close (ProcMMU_MPU_M3_handle);
        if (osStatus != 0) {
            perror ("ProcMMU_close: " PROC_MMU_MPU_M3_DRIVER_NAME);
            status = ProcMMU_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMMU_close",
                                 status,
                                 "Failed to close ProcMgr driver with OS!");
        }
        else {
            ProcMMU_MPU_M3_handle = 0;
        }
        osStatus = close (ProcMMU_DSP_handle);
        if (osStatus != 0) {
            perror ("ProcMMU_close: " PROC_MMU_DSP_DRIVER_NAME);
            status = ProcMMU_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMMU_close",
                                 status,
                                 "Failed to close ProcMgr driver with OS!");
        }
        else {
            ProcMMU_DSP_handle = 0;
        }
    }

    GT_1trace (curTrace, GT_LEAVE, "ProcMMU_close", status);

    return status;
}


/*!
 *  @brief  Function to open the ProcMMU driver.
 *
 *  @sa     ProcMMU_close
 */
Int
ProcMMU_open (Void)
{
    Int32               status          = ProcMMU_S_SUCCESS;
    Int32               osStatus        = 0;

    GT_0trace (curTrace, GT_ENTER, "ProcMMU_open");

    if (ProcMMU_refCount == 0) {
        Osal_printf ("%s %d\n", __func__, __LINE__);
        ProcMMU_MPU_M3_handle = open (PROC_MMU_MPU_M3_DRIVER_NAME,
                                        O_SYNC | O_RDWR);
        if (ProcMMU_MPU_M3_handle < 0) {
            perror ("ProcMgr driver open: " PROC_MMU_MPU_M3_DRIVER_NAME);
            status = ProcMMU_E_OSFAILURE;
            GT_setFailureReason (curTrace,
                                 GT_4CLASS,
                                 "ProcMMU_open",
                                 status,
                                 "Failed to open ProcMgr driver with OS!");
        }
        else {
            ProcMMU_DSP_handle = open (PROC_MMU_DSP_DRIVER_NAME,
                                        O_SYNC | O_RDWR);
            if (ProcMMU_DSP_handle < 0) {
                perror ("ProcMgr driver open: " PROC_MMU_DSP_DRIVER_NAME);
                /*! @retval PROCMGR_E_OSFAILURE Failed to open ProcMgr driver with
                            OS */
                status = ProcMMU_E_OSFAILURE;
                GT_setFailureReason (curTrace,
                                     GT_4CLASS,
                                     "ProcMMU_open",
                                     status,
                                     "Failed to open ProcMgr driver with OS!");
            }
            else {
                osStatus = fcntl (ProcMMU_MPU_M3_handle, F_SETFD, FD_CLOEXEC);
                if (osStatus != 0) {
                    /*! @retval PROCMGR_E_OSFAILURE Failed to set file descriptor
                                                    flags */
                    status = ProcMMU_E_OSFAILURE;
                    GT_setFailureReason (curTrace,
                                         GT_4CLASS,
                                         "ProcMMU_open",
                                         status,
                                         "Failed to set file descriptor flags!");
                }
                else{
                    /* TBD: Protection for refCount. */
                    ProcMMU_refCount++;
                }
            }
        }
    }
    else {
        ProcMMU_refCount++;
    }

    GT_1trace (curTrace, GT_LEAVE, "ProcMMU_open", status);

    return status;
}

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
