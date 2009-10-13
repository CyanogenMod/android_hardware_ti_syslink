/*
 *  Copyright 2001-2009 Texas Instruments - http://www.ti.com/
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
/** ============================================================================
 *  @file   MemAllocTest_config.h
 *
 *  @brief  MemAllocTest sample configuration file
 *  ============================================================================
 */

#ifndef _MEMALLOCTEST_CONFIG_H_
#define _MEMALLOCTEST_CONFIG_H_

#if defined (__cplusplus)
extern "C" {
#endif

/* App defines */
#define MAX_CREATE_ATTEMPTS         0xFFFF
#define MSGSIZE                     256
#define HEAPID_SYSM3                0
#define HEAPID_APPM3                1

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

#endif /* _MEMALLOCTEST_CONFIG_H_ */
