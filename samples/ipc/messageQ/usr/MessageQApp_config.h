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
 *  @file   MessageQApp_config.h
 *
 *  @brief  MessageQ application configuration file
 *
 *  ============================================================================
 */


#ifndef _MESSAGEQAPP_CONFIG_H_
#define _MESSAGEQAPP_CONFIG_H_


#if defined (__cplusplus)
extern "C" {
#endif


/* App defines */
#define MSGSIZE                  256u
#define NSRN_NOTIFY_EVENTNO        7u
#define TRANSPORT_NOTIFY_EVENTNO   8u
#define HEAPID                     0u
#define HEAPNAME             "SysMgrHeap0"
#define DIEMESSAGE             0xFFFF

#define DSP_MESSAGEQNAME            "Q0"
#define ARM_MESSAGEQNAME            "Q1"
#define DUCATI_CORE0_MESSAGEQNAME   "Q2"

/*
 *  The shared memory is going to split between
 *  Notify:       0x98000000 - 0x98003FFF
 *  GatePeterson: 0x98004000 - 0x98004FFF
 *  HeapBuf:      0x98005000 - 0x98008FFF
 *  MessageQ NS:  0x98009000 - 0x98009FFF
 *  Transport:    0x9800A000 - 0x9800BFFF
 */
#define SHAREDMEM               0x98000000
#define SHAREDMEMSIZE              0x7F000

/* Memory for the Notify Module */
#define NOTIFYMEM               0x98000000
#define NOTIFYMEMSIZE               0x4000

/* Memory a GatePeterson instance */
#define GATEPETERSONMEM         0x98004000
#define GATEPETERSONMEMSIZE         0x1000

/* Memory a HeapBuf instance */
#define HEAPBUFMEM              0x98005000
#define HEAPBUFMEMSIZE              0x4000

/* Memory for NameServerRemoteNotify */
#define NSRN_MEM                0x98009000
#define NSRN_MEMSIZE            0x1000

/* Memory a Transport instance */
#define TRANSPORTMEM            0x9800A000
#define TRANSPORTMEMSIZE            0x2000


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* _MESSAGEQAPP_CONFIG_H_ */
