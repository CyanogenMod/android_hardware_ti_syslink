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

/*
 *  ipc_ioctl.h
 *
 *  Base file for all TI OMAP IPC ioctl's.
 *  Linux-OMAP IPC has allocated base 0xEE with a range of 0x00-0xFF.
 *  (need to get  the real one from open source maintainers)
 *
 */

#ifndef _IPC_IOCTL_H
#define _IPC_IOCTL_H

#include <linux/ioctl.h>

#define IPC_IOC_MAGIC       0xE0
#define IPC_IOC_BASE        2

enum ipc_command_count {
    MULTIPROC_CMD_NOS = 4,
    NAMESERVER_CMD_NOS = 13,
    HEAPBUF_CMD_NOS    = 13,
    SHAREDREGION_CMD_NOS = 10,
    GATEPETERSON_CMD_NOS = 11,
    LISTMPSHAREDMEMORY_CMD_NOS = 18,
    MESSAGEQ_CMD_NOS = 17,
    MESSAGEQTRANSPORTSHM_CMD_NOS = 9,
    NAMESERVERREMOTENOTIFY_CMD_NOS = 8,
    SYSMGR_CMD_NOS = 5,
    SYSMEMMGR_CMD_NOS = 6,
    GATEHWSPINLOCK_CMD_NOS = 10
};

enum ipc_command_ranges {
    MULTIPROC_BASE_CMD              = IPC_IOC_BASE,
    MULTIPROC_END_CMD               = (MULTIPROC_BASE_CMD + \
                                      MULTIPROC_CMD_NOS - 1),

    NAMESERVER_BASE_CMD             = 10,
    NAMESERVER_END_CMD              = (NAMESERVER_BASE_CMD + \
                                      NAMESERVER_CMD_NOS - 1),

    HEAPBUF_BASE_CMD                = 30,
    HEAPBUF_END_CMD                 = (HEAPBUF_BASE_CMD + \
                                      HEAPBUF_CMD_NOS - 1),

    SHAREDREGION_BASE_CMD           = 50,
    SHAREDREGION_END_CMD            = (SHAREDREGION_BASE_CMD + \
                                      SHAREDREGION_CMD_NOS - 1),

    GATEPETERSON_BASE_CMD           = 70,
    GATEPETERSON_END_CMD            = (GATEPETERSON_BASE_CMD + \
                                      GATEPETERSON_CMD_NOS - 1),

    LISTMPSHAREDMEMORY_BASE_CMD     = 90,
    LISTMPSHAREDMEMORY_END_CMD      = (LISTMPSHAREDMEMORY_BASE_CMD + \
                                      LISTMPSHAREDMEMORY_CMD_NOS - 1),

    MESSAGEQ_BASE_CMD               = 110,
    MESSAGEQ_END_CMD                = (MESSAGEQ_BASE_CMD + \
                                      MESSAGEQ_CMD_NOS - 1),

    MESSAGEQTRANSPORTSHM_BASE_CMD   = 130,
    MESSAGEQTRANSPORTSHM_END_CMD    = (MESSAGEQTRANSPORTSHM_BASE_CMD + \
                                      MESSAGEQTRANSPORTSHM_CMD_NOS - 1),

    NAMESERVERREMOTENOTIFY_BASE_CMD = 160,
    NAMESERVERREMOTENOTIFY_END_CMD  = (NAMESERVERREMOTENOTIFY_BASE_CMD + \
                                      NAMESERVERREMOTENOTIFY_CMD_NOS - 1),

    SYSMGR_BASE_CMD                 = 170,
    SYSMGR_END_CMD                  = (SYSMGR_BASE_CMD + \
                                      SYSMGR_CMD_NOS - 1),

    SYSMEMMGR_BASE_CMD              = 180,
    SYSMEMMGR_END_CMD               = (SYSMGR_BASE_CMD + \
                                      SYSMEMMGR_CMD_NOS - 1),

    GATEHWSPINLOCK_BASE_CMD         = 190,
    GATEHWSPINLOCK_END_CMD          = (GATEHWSPINLOCK_BASE_CMD + \
                                      GATEHWSPINLOCK_CMD_NOS - 1)
};

#endif /* _IPC_IOCTL_H */
