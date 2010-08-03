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
 *  @file   SLPMTESTApp_config.h
 *
 *  @brief  slpm application configuration file
 *
 *  ============================================================================
 */


#ifndef _SLPMTESTAPP_CONFIG_H_
#define _SLPMTESTAPP_CONFIG_H_


#if defined (__cplusplus)
extern "C" {
#endif


/* App defines */
#define MSGSIZE                     256u
#define HEAPID                      0u
#define HEAPNAME                    "Heap0"
#define DIEMESSAGE                  0xFFFF

#define DUCATI_CORE0_MESSAGEQNAME   "MsgQ0"
#define DUCATI_CORE1_MESSAGEQNAME   "MsgQ1"
#define DSP_MESSAGEQNAME            "MsgQ2"
#define ARM_MESSAGEQNAME            "MsgQ3"


#define NOVALID                                   -1

#define NOCMD                                      0
#define REQUEST                                  0x1
#define REGISTER                                 0x2
#define REQUEST_REGISTER                         0x3
#define VALIDATE                                 0x4
#define REQUEST_VALIDATE                         0x5
#define REGISTER_VALIDATE                        0x6
#define REQUEST_REGISTER_VALIDATE                0x7
#define DUMPRCB                                  0x8
#define SUSPEND                                 0x10
#define RESUME                                  0x20
#define UNREGISTER                              0x40
#define RELEASE                                 0x80
#define UNREGISTER_RELEASE                      0xC0

#define MULTITHREADS                           0x100
#define LIST                                   0x200
#define ENTER_I2C_SPINLOCK                     0x400
#define LEAVING_I2C_SPINLOCK                   0x800

#define MULTITHREADS_I2C                      0x1000
#define MULTI_TASK_I2C                        0x2000
#define NEWFEATURE                            0x4000
// Paul's Tests
#define IDLEWFION                             0x8000
#define IDLEWFIOFF                           0x10000
#define COUNTIDLES                           0x20000
#define PAUL_01                              0x40000
#define PAUL_02                              0x80000
#define PAUL_03                             0x100000
#define PAUL_04                             0x200000
#define TIMER                               0x400000
#define PWR_SUSPEND                         0x800000

#define NOTIFY_SYSM3                       0x1000000


#define EXIT                              0x80000000


#define REQUIRES_MESSAGEQ (REQUEST | REGISTER | VALIDATE | DUMPRCB | UNREGISTER \
    | RELEASE | MULTITHREADS | LIST | NEWFEATURE |ENTER_I2C_SPINLOCK \
    | LEAVING_I2C_SPINLOCK | MULTITHREADS_I2C | MULTI_TASK_I2C \
    | IDLEWFION | IDLEWFIOFF | COUNTIDLES | PAUL_01 | PAUL_02 | PAUL_03 \
    | PAUL_04 | TIMER | PWR_SUSPEND)

#define EXIT_OR_NOVALID (~(SUSPEND | RESUME | REQUIRES_MESSAGEQ))

#define REQUIRES_NOTIFY_SYSM3 NOTIFY_SYSM3


#define IVAHD           0
#define ISS             1
#define SDMA            2
#define GPTIMER         3
#define GPIO            4
#define I2C             5
#define REGULATOR       6
#define AUXCLK          7
#define SYS_M3_PROC     8
#define APP_M3_PROC     9
#define L3BUS           10

#define NORESOURCE      -1
//#define SYSTIMER    NORESOURCE


#define FIRSTRESOURCE   SDMA
#define LASTRESOURCE    REGULATOR

#define MAX_NUM_ARGS    10

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif /* _SLPMTESTAPP_CONFIG_H_ */
