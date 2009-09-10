/** ============================================================================
 *  @file   TilerMgrServer_config.h
 *
 *  @brief  TILER Server configuration file
 *
 *  @ver    2.00.00.08
 *
 *  ============================================================================
 *
 *  Copyright (c) 2008-2009, Texas Instruments Incorporated
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
 *  Contact information for paper mail:
 *  Texas Instruments
 *  Post Office Box 655303
 *  Dallas, Texas 75265
 *  Contact information:
 *  http://www-k.ext.ti.com/sc/technical-support/product-information-centers.htm?
 *  DCMP=TIHomeTracking&HQS=Other+OT+home_d_contact
 *  ============================================================================
 */

#ifndef _TILERMGRSERVER_CONFIG_H_
#define _TILERMGRSERVER_CONFIG_H_

#if defined (__cplusplus)
extern "C" {
#endif

/* App defines */
#define MAX_CREATE_ATTEMPTS         0xFFFF
#define MSGSIZE                     256
#define HEAPID                      0

#define RCM_SERVER_NAME_SYSM3       "MemAllocServerSysM3"
#define RCM_SERVER_NAME_APPM3       "MemAllocServerAppM3"
#define SYSM3_PROC_NAME             "SysM3"
#define APPM3_PROC_NAME             "AppM3"

/*
 *  The shared memory is going to split between
 *
 *  MPU - SysM3 sample
 *  Notify:       0x98000000 - 0x98003FFF
 *  GatePeterson: 0x98004000 - 0x98004FFF
 *  HeapBuf:      0x98005000 - 0x98008FFF
 *  MessageQ NS:  0x98009000 - 0x98009FFF
 *  Transport:    0x9800A000 - 0x9800BFFF
 *
 *  MPU - AppM3 sample
 *  Notify:       0x98000000 - 0x98003FFF
 *  GatePeterson: 0x98080000 - 0x98080FFF
 *  HeapBuf:      0x98081000 - 0x98084FFF
 *  MessageQ NS:  0x98085000 - 0x98085FFF
 *  Transport:    0x98086000 - 0x98087FFF
 */

/* Shared Memory Area for MPU - SysM3 */
#define SHAREDMEM                   0x98000000
//#define SHAREDMEMSIZE               0xC000
#define SHAREDMEMSIZE               0x80000

/* Shared Memory Area for MPU - AppM3 */
#define SHAREDMEM1                  0x98080000
//#define SHAREDMEMSIZE1              0x1F000
#define SHAREDMEMSIZE1              0x80000

/* Memory for the Notify Module */
#define NOTIFYMEM                   SHAREDMEM
#define NOTIFYMEMSIZE               0x4000

/* Memory a GatePeterson instance */
//#define GATEPETERSONMEM             (NOTIFYMEM + NOTIFYMEMSIZE)
#define GATEPETERSONMEM             (NOTIFYMEM + NOTIFYMEMSIZE + 0x40000)
#define GATEPETERSONMEMSIZE         0x1000

/* Memory a HeapBuf instance */
#define HEAPBUFMEM                  (GATEPETERSONMEM + GATEPETERSONMEMSIZE)
#define HEAPBUFMEMSIZE              0x4000

/* Memory for NameServerRemoteNotify */
#define NSRN_MEM                    (HEAPBUFMEM + HEAPBUFMEMSIZE)
#define NSRN_MEMSIZE                0x1000

/* Memory a Transport instance */
#define TRANSPORTMEM                (NSRN_MEM + NSRN_MEMSIZE)
#define TRANSPORTMEMSIZE            0x2000

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* _TILERMGRSERVER_CONFIG_H_ */
