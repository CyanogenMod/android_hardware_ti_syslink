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
 *  Notify:       0xA0000000 - 0xA0003FFF
 *  GatePeterson: 0xA0004000 - 0xA0004FFF
 *  HeapBuf:      0xA0005000 - 0xA0008FFF
 *  MessageQ NS:  0xA0009000 - 0xA0009FFF
 *  Transport:    0xA000A000 - 0xA000BFFF
 *
 *  MPU - AppM3 sample
 *  Notify:       0xA0000000 - 0xA0003FFF
 *  GatePeterson: 0xA0055000 - 0xA0055FFF
 *  HeapBuf:      0xA0056000 - 0xA0059FFF
 *  MessageQ NS:  0xA005A000 - 0xA005AFFF
 *  Transport:    0xA005B000 - 0xA005CFFF
 */

/* Shared Memory Area for MPU - SysM3 */
#define SHAREDMEM                   0xA0000000
#define SHAREDMEMSIZE               0x55000

/* Shared Memory Area for MPU - AppM3 */
#define SHAREDMEM1                  0xA0055000
#define SHAREDMEMSIZE1              0x55000

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
