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
 *  @file   RCMTest_config.h
 *
 *  @brief  RCM sample applications' configuration file
 *  ============================================================================
 */

#ifndef _RCMTEST_CONFIG_H_
#define _RCMTEST_CONFIG_H_

#if defined (__cplusplus)
extern "C" {
#endif

/* App defines */
#define MAX_CREATE_ATTEMPTS         0xFFFF
#define LOOP_COUNT                  4
#define JOB_COUNT                   4
#define MAX_NAME_LENGTH             32
#define MSGSIZE                     256
#define NSRN_NOTIFY_EVENTNO         7
#define TRANSPORT_NOTIFY_EVENTNO    8
#define NSRN_NOTIFY_EVENTNO1        22
#define TRANSPORT_NOTIFY_EVENTNO1   23
#define HEAPID                      0

#define HEAPNAME                    "RcmCli_Heap"
#define RCMSERVER_NAME              "RcmSvr_Mpu:0"
#define SYSM3_SERVER_NAME           "RcmSvr_SysM3:0"
#define APPM3_SERVER_NAME           "RcmSvr_AppM3:0"
#define MPU_PROC_NAME               "MPU"
#define SYSM3_PROC_NAME             "SysM3"
#define APPM3_PROC_NAME             "AppM3"

/*
 *  The shared memory is going to split between
 *  Notify:             0x98000000 - 0x98004000
 *  Gatepeterson:       0x98004000 - 0x98005000
 *  HEAPBUFMEM:         0x98005000 - 0x98009000
 *  NSRN_MEM:           0x98009000 - 0x9800A000
 *  transport:          0x9000A000 - 0x9000C000
 *  MESSAGEQ_NS_MEM:    0x9000C000 - 0x9000D000
 *  HEAPBUF_NS_MEM:     0x9000D000 - 0x9000E000
 *  HEAPBUFMEM1:        0x9000E000 - 0x90012000
 *  GATEPETERSONMEM1:   0x90012000 - 0x90013000
 *  HEAPMEM:            0x90013000 - 0x90014000
 *  HEAPMEM1:           0x90014000 - 0x90015000
 *  List:               0x98011000 - 0x98012000
 *  List1:              0x98012000 - 0x98013000
 *  DMMSHAREDMEM:       0x90017000 - 0x90018000
 *  DMMSHAREDMEM1:      0x90018000 - 0x90019000
 *  HEAPMBMEM_CTRL:     0x90019000 - 0x9001A000
 *   HEAPMBMEM_BUFS:    0x9001A000 - 0x9001D000
 */

/* Shared Memory Area for MPU - SysM3 */
#define SHAREDMEM                   0x98000000
#define SHAREDMEMSIZE               0x7F000

/* Shared Memory Area for MPU - AppM3 */
#define SHAREDMEM1                  0x98080000
#define SHAREDMEMSIZE1              0x7F000

/* Memory for the Notify Module */
#define NOTIFYMEM                   0x98000000
#define NOTIFYMEMSIZE               0x4000

/* Memory a GatePeterson instance */
#define GATEPETERSONMEM             (NOTIFYMEM + NOTIFYMEMSIZE)
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

/* Memory for second gatepeterson */
#define MESSAGEQ_NS_MEM             (TRANSPORTMEM + TRANSPORTMEMSIZE)
#define MESSAGEQ_NS_MEMSIZE         0x1000

/* Memory for HeapBuf's NameServer instance */
#define HEAPBUF_NS_MEM              (MESSAGEQ_NS_MEM + MESSAGEQ_NS_MEMSIZE)
#define HEAPBUF_NS_MEMSIZE          0x1000

/* Memory a HeapBuf instance for RCM TestCase*/
#define HEAPBUFMEM1                 (HEAPBUF_NS_MEM + HEAPBUF_NS_MEMSIZE)
#define HEAPBUFMEMSIZE1             0x4000

#define GATEPETERSONMEM1            (HEAPBUFMEM1 + HEAPBUFMEMSIZE1)
#define GATEPETERSONMEMSIZE1        0x1000

/* Memory for the Notify Module */
#define HEAPMEM                     (GATEPETERSONMEM1 + GATEPETERSONMEMSIZE1)
#define HEAPMEMSIZE                 0x1000

#define HEAPMEM1                    (HEAPMEM + HEAPMEMSIZE)
#define HEAPMEMSIZE1                0x1000

#define List                        (HEAPMEM1 + HEAPMEMSIZE1)
#define ListSIZE                    0x1000

#define List1                       (List + ListSIZE)
#define ListSIZE1                   0x1000

/* Memory a HeapMultiBuf instance */
#define HEAPMBMEM_CTRL              (List1 + ListSIZE1)
#define HEAPMBMEMSIZE_CTRL          0x1000

#define HEAPMBMEM_BUFS              (HEAPMBMEM_CTRL + HEAPMBMEMSIZE_CTRL)
#define HEAPMBMEMSIZE_BUFS          0x3000

#define HEAPMBMEM1_CTRL             (HEAPMBMEM_BUFS + HEAPMBMEMSIZE_BUFS)
#define HEAPMBMEMSIZE1_CTRL         0x1000

#define HEAPMBMEM1_BUFS             (HEAPMBMEM1_CTRL + HEAPMBMEMSIZE1_CTRL)
#define HEAPMBMEMSIZE1_BUFS         0x3000

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* _RCMTEST_CONFIG_H_ */
