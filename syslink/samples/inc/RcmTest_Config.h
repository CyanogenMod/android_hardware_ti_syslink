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
#define HEAPID_SYSM3                0
#define HEAPNAME_SYSM3              "SysMgrHeap0"
#define HEAPID_APPM3                1
#define HEAPNAME_APPM3              "SysMgrHeap1"
#define RCMSERVER_NAME              "RcmSvr_Mpu:0"
#define SYSM3_SERVER_NAME           "RcmSvr_SysM3:0"
#define APPM3_SERVER_NAME           "RcmSvr_AppM3:0"
#define MPU_PROC_NAME               "MPU"
#define SYSM3_PROC_NAME             "SysM3"
#define APPM3_PROC_NAME             "AppM3"

/*
 *  The shared memory is going to split between
 *  Notify:             0xA0000000 - 0xA0004000
 *  Gatepeterson:       0xA0004000 - 0xA0005000
 *  HEAPBUFMEM:         0xA0005000 - 0xA0009000
 *  NSRN_MEM:           0xA0009000 - 0xA000A000
 *  transport:          0xA000A000 - 0xA000C000
 *  MESSAGEQ_NS_MEM:    0xA000C000 - 0xA000D000
 *  HEAPBUF_NS_MEM:     0xA000D000 - 0xA000E000
 *  HEAPBUFMEM1:        0xA000E000 - 0xA0012000
 *  GATEPETERSONMEM1:   0xA0012000 - 0xA0013000
 *  HEAPMEM:            0xA0013000 - 0xA0014000
 *  HEAPMEM1:           0xA0014000 - 0xA0015000
 *  List:               0xA0015000 - 0xA0016000
 *  List1:              0xA0016000 - 0xA0017000
 *  HEAPMBMEM_CTRL:     0xA0017000 - 0xA0018000
 *  HEAPMBMEM:          0xA0018000 - 0xA001B000
 *  HEAPMBMEM1_CTRL:    0xA001B000 - 0xA001C000
 *  HEAPMBMEM1:         0xA001C000 - 0xA001F000
 */

/* Shared Memory Area for MPU - SysM3 */
#define SHAREDMEM                   0xA0000000
#define SHAREDMEMSIZE               0x54000

/* Shared Memory Area for MPU - AppM3 */
#define SHAREDMEM1                  0xA0055000
#define SHAREDMEMSIZE1              0x54000

/* Memory for the Notify Module */
#define NOTIFYMEM                   0xA0000000
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
