/*
 * Syslink-IPC for TI OMAP Processors
 *
 * Copyright (C) 2009 Texas Instruments, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation version 2.1 of the License.
 *
 * This program is distributed .as is. WITHOUT ANY WARRANTY of any kind,
 * whether express or implied; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */
/** ============================================================================
 *  @file   MemMgrServer_config.h
 *
 *  @brief  MemMgr Server configuration file
 *  ============================================================================
 */

#ifndef _MEMMGRSERVER_CONFIG_H_
#define _MEMMGRSERVER_CONFIG_H_

#if defined (__cplusplus)
extern "C" {
#endif

/* App defines */
#define NSRN_NOTIFY_EVENTNO         7
#define TRANSPORT_NOTIFY_EVENTNO    8
#define NSRN_NOTIFY_EVENTNO1        22
#define TRANSPORT_NOTIFY_EVENTNO1   23

#define RCMSERVER_NAME              "MEMALLOCMGRSERVER"
#define SYSM3_PROC_NAME             "SysM3"
#define APPM3_PROC_NAME             "AppM3"

//#define SYSM3_FILE_NAME             "./RCMClient_MPUSYS_Test_Core0.xem3"
#define SYSM3_FILE_NAME             "./RCMServer_MPUSYS_Test_Core0.xem3"
//#define SYSM3_FILE_NAME             "./Notify_MPUSYS_reroute_Test_Core0.xem3"
//#define APPM3_FILE_NAME             "./RCMClient_MPUAPP_Test_Core1.xem3"
#define APPM3_FILE_NAME             "./RCMServer_MPUAPP_Test_Core1.xem3"

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
#define SHAREDMEMNUMBER             0

/* Shared Memory Area for MPU - AppM3 */
#define SHAREDMEM1                  0xA0055000
#define SHAREDMEMSIZE1              0x55000
#define SHAREDMEMNUMBER1            1

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

#endif /* _MEMMGRSERVER_CONFIG_H_ */
