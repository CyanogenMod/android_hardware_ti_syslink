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
 *  @file   MessageQApp_config1.h
 *
 *  @brief  MessageQ application configuration file
 *
 *  ============================================================================
 */


#ifndef _MESSAGEQAPP_CONFIG1_H_
#define _MESSAGEQAPP_CONFIG1_H_


#if defined (__cplusplus)
extern "C" {
#endif


/* App defines */
#define MSGSIZE                     256u
#define NSRN_NOTIFY_EVENTNO         22u
#define TRANSPORT_NOTIFY_EVENTNO    23u
#define HEAPID                      1u
#define HEAPNAME                    "SysMgrHeap1"
#define DIEMESSAGE                  0xFFFF

#define DSP_MESSAGEQNAME            "Q0"
#define ARM_MESSAGEQNAME            "Q1"
#define DUCATI_CORE0_MESSAGEQNAME   "Q2"
#define DUCATI_CORE1_MESSAGEQNAME   "Q3"

/*
 *  The shared memory is going to split between
 *  Notify:       0x98000000 - 0x98003FFF
 *  GatePeterson: 0x98080000 - 0x98080FFF
 *  HeapBuf:      0x98081000 - 0x98084FFF
 *  MessageQ NS:  0x98085000 - 0x98085FFF
 *  Transport:    0x98086000 - 0x98087FFF
 */

/* Memory for the MPU - SysM3 Shared Objects */
#define SHAREDMEM0               0x98000000
#define SHAREDMEMSIZE0           0x7F000

/* Memory for MPU - AppM3 Shared Objects */
#define SHAREDMEM               0x98080000
#define SHAREDMEMSIZE           0x7F000

/* Memory a GatePeterson instance */
#define GATEPETERSONMEM         (SHAREDMEM)
#define GATEPETERSONMEMSIZE     0x1000

/* Memory a HeapBuf instance */
#define HEAPBUFMEM              (GATEPETERSONMEM + GATEPETERSONMEMSIZE)
#define HEAPBUFMEMSIZE          0x4000

/* Memory for NameServerRemoteNotify */
#define NSRN_MEM                (HEAPBUFMEM + HEAPBUFMEMSIZE)
#define NSRN_MEMSIZE            0x1000

/* Memory a Transport instance */
#define TRANSPORTMEM            (NSRN_MEM + NSRN_MEMSIZE)
#define TRANSPORTMEMSIZE        0x2000

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* _MESSAGEQAPP_CONFIG1_H_ */
