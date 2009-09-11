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
 *  @file   IPCManager.c
 *
 *  @brief  Source file for the IPC driver wrapper for Syslink modules.
 *  ============================================================================
*/

/*  ----------------------------------- Host OS */
#include <host_os.h>
#include <errno.h>
#include <Std.h>
#include <Trace.h>

#include <MultiProc.h>
#include <MultiProcDrvDefs.h>
#include <NameServer.h>
#include <NameServerDrvDefs.h>
#include <SharedRegion.h>
#include <SharedRegionDrvDefs.h>
#include <NameServerRemoteNotify.h>
#include <NameServerRemoteNotifyDrvDefs.h>
#include <GatePeterson.h>
#include <HeapBuf.h>
#include <MessageQ.h>
#include <MessageQTransportShm.h>
#include <ListMPSharedMemory.h>
#include <GatePetersonDrvDefs.h>
#include <HeapBufDrvDefs.h>
#include <MessageQDrvDefs.h>
#include <MessageQTransportShmDrvDefs.h>
#include <ListMPSharedMemoryDrvDefs.h>
#include <GateHWSpinLockDrvDefs.h>
#include <SysMgr.h>
#include <SysMgrDrvDefs.h>
#include <SysMemMgr.h>
#include <SysMemMgrDrvDefs.h>

#define IPC_SOK   0
#define IPC_EFAIL -1

/*
 * Driver name for complete IPC module
 */
#define IPC_DRIVER_NAME  "/dev/syslink_ipc"

enum IPC_MODULES_ID {
    SHAREDREGION = 0,
    GATEPETERSON,
    HEAPBUF,
    NAMESERVER,
    MESSAGEQ,
    MESSAGEQTRANSPORTSHM,
    LISTMPSHAREDMEMORY,
    NAMESERVERREMOTENOTIFY,
    MULTIPROC,
    SYSMGR,
    SYSMEMMGR,
    GATEHWSPINLOCK,
    MAX_IPC_MODULES
};
struct name_id_table {
    const char *module_name;
    enum IPC_MODULES_ID module_id;
    unsigned long ioctl_count;
};

struct name_id_table name_id_table[MAX_IPC_MODULES] = {
    { "/dev/syslinkipc/SharedRegion", SHAREDREGION, 0 },
    { "/dev/syslinkipc/GatePeterson", GATEPETERSON,  0},
    { "/dev/syslinkipc/HeapBuf", HEAPBUF, 0},
    { "/dev/syslinkipc/NameServer", NAMESERVER, 0},
    { "/dev/syslinkipc/MessageQ", MESSAGEQ, 0},
    { "/dev/syslinkipc/MessageQTransportShm", MESSAGEQTRANSPORTSHM, 0},
    { "/dev/syslinkipc/ListMPSharedMemory", LISTMPSHAREDMEMORY, 0},
    { "/dev/syslinkipc/NameServerRemoteNotify", NAMESERVERREMOTENOTIFY, 0},
    { "/dev/syslinkipc/MultiProc", MULTIPROC, 0},
    { "/dev/syslinkipc/SysMgr", SYSMGR, 0},
    { "/dev/syslinkipc/SysMemMgr", SYSMEMMGR, 0},
    { "/dev/syslinkipc/GateHWSpinLock", GATEHWSPINLOCK, 0}
};

/*  ----------------------------------- Globals */
Int                     ipc_handle = -1;         /* class driver handle */
static unsigned long    ipc_usage_count;
static sem_t            semOpenClose;
static bool             ipc_sem_initialized = false;


Int getHeapBufStatus(Int apiStatus)
{
    Int status = HEAPBUF_E_OSFAILURE;

    GT_1trace (curTrace, GT_2CLASS,
        "IPCMGR-getHeapBufStatus: apiStatus=0x%x\n", apiStatus);

    switch(apiStatus) {
        case 0:
            status = HEAPBUF_SUCCESS;
            break;

        case -EINVAL:
            status = HEAPBUF_E_INVALIDARG;
            break;

        case -ENOMEM:
            status = HEAPBUF_E_MEMORY;
            break;

        case -ENOENT:
            status = HEAPBUF_E_NOTFOUND;
            break;

        case -ENODEV:
            status = HEAPBUF_E_INVALIDSTATE;
            break;

        case -EPERM:
            status = HEAPBUF_E_NOTONWER;
            break;

        case -EBUSY:
            status = HEAPBUF_E_INUSE;
            break;

        case -EEXIST:
        case 1:
            status = HEAPBUF_S_ALREADYSETUP;
            break;

        default:
            status = HEAPBUF_E_FAIL;
            /* status =  HEAPBUF_E_OSFAILURE */
            break;
        }

        GT_1trace (curTrace, GT_2CLASS,
            "IPCMGR-getHeapBufStatus: status=0x%x\n", status);
        return status;
}


Int getGatePetersonStatus(Int apiStatus)
{
    Int status = GATEPETERSON_E_OSFAILURE;

    GT_1trace (curTrace, GT_2CLASS,
        "IPCMGR-getGatePetersonStatus: apiStatus=0x%x\n", apiStatus);

    switch(apiStatus) {
    case 0:
        status = GATEPETERSON_SUCCESS;
        break;

    case -EINVAL:
        status = GATEPETERSON_E_INVALIDARG;
        break;

    case -ENOMEM:
        status = GATEPETERSON_E_MEMORY;
        break;

    case -ENOENT:
        status = GATEPETERSON_E_NOTFOUND;
        break;

    case -ENXIO:
        status = GATEPETERSON_E_VERSION;
        break;

    case -EEXIST:
    case 1:
        status = GATEPETERSON_S_ALREADYSETUP;
        break;

    case -ENODEV:
        status = GATEPETERSON_E_INVALIDSTATE;
        break;

    case -EPERM:
        status = GATEPETERSON_E_NOTONWER;
        break;

    case -EBUSY:
        status = GATEPETERSON_E_INUSE;
        break;

    default:
        status = GATEPETERSON_E_FAIL;
/*        status =  */
        break;
    }

    GT_1trace (curTrace, GT_2CLASS,
        "IPCMGR-getGatePetersonStatus: status=0x%x\n", status);
    return status;
}


Int getSharedRegionStatus(Int apiStatus)
{
    Int status = SHAREDREGION_E_OSFAILURE;

    GT_1trace (curTrace, GT_2CLASS,
        "IPCMGR-getSharedRegionStatus: apiStatus=0x%x\n", apiStatus);

    switch(apiStatus) {
    case 0:
        status = SHAREDREGION_SUCCESS;
        break;

    case -EINVAL:
        status = SHAREDREGION_E_INVALIDARG;
        break;

    case -ENOMEM:
        status = SHAREDREGION_E_MEMORY;
        break;

    case -EBUSY:
        status = SHAREDREGION_E_BUSY;
        break;

    case -ENOENT:
        status = SHAREDREGION_E_NOTFOUND;
        break;

    case -EEXIST:
        status = SHAREDREGION_E_ALREADYEXIST;
        break;

    case -ENODEV:
        status = SHAREDREGION_E_INVALIDSTATE;
        break;

    case -EPERM:
        status = SHAREDREGION_E_OVERLAP;
        break;

    case 1:
        status = SHAREDREGION_S_ALREADYSETUP;
        break;

    default:
        status = SHAREDREGION_E_FAIL;
/*        status = SHAREDREGION_E_OSFAILURE; */
        break;
    }

    GT_1trace (curTrace, GT_2CLASS,
        "IPCMGR-getSharedRegionStatus: status=0x%x\n", status);
    return status;
}


Int getNameServerRemoteNotifyStatus(Int apiStatus)
{
    Int status = NAMESERVERREMOTENOTIFY_E_OSFAILURE;

    GT_1trace (curTrace, GT_2CLASS,
        "IPCMGR-getNameServerRemoteNotifyStatus: apiStatus=0x%x\n", apiStatus);

    switch(apiStatus) {
    case 0:
        status = NAMESERVERREMOTENOTIFY_SUCCESS;
        break;

    case -EEXIST:
        status = NAMESERVERREMOTENOTIFY_E_ALREADYEXIST;
        break;


    case -EINVAL:
        status = NAMESERVERREMOTENOTIFY_E_INVALIDARG;
        break;

    case -ENOMEM:
        status = NAMESERVERREMOTENOTIFY_E_MEMORY;
        break;

    case -EBUSY:
        status = NAMESERVERREMOTENOTIFY_E_BUSY;
        break;

    case -ENOENT:
        status = NAMESERVERREMOTENOTIFY_E_NOTFOUND;
        break;

    case -ENODEV:
        status = NAMESERVERREMOTENOTIFY_E_INVALIDSTATE;
        break;

    case -EPERM:
        status = NAMESERVERREMOTENOTIFY_E_OVERLAP;
        break;

    case 1:
        status = NAMESERVERREMOTENOTIFY_S_ALREADYSETUP;
        break;

    default:
        status = NAMESERVERREMOTENOTIFY_E_FAIL;
        /* status = NAMESERVER_E_OSFAILURE; */
        break;
    }

    GT_1trace (curTrace, GT_2CLASS,
        "IPCMGR-getNameServerRemoteNotifyStatus: status=0x%x\n", status);
    return status;
}


Int getNameServerStatus(Int apiStatus)
{
    Int status = NAMESERVER_E_OSFAILURE;

    GT_1trace (curTrace, GT_2CLASS,
        "IPCMGR-getNameServerStatus: apiStatus=0x%x\n", apiStatus);

    switch(apiStatus) {
    case 0:
        status = NAMESERVER_SUCCESS;
        break;

    case -EINVAL:
        status = NAMESERVER_E_INVALIDARG;
        break;

    case -EEXIST:
    case 1:
        status = NAMESERVER_S_ALREADYSETUP;
        break;

    case -ENOMEM:
        status = NAMESERVER_E_MEMORY;
        break;

    case -EBUSY:
        status = NAMESERVER_E_BUSY;
        break;

    case -ENOENT:
        status = NAMESERVER_E_NOTFOUND;
        break;

    case -ENODEV:
        status = NAMESERVER_E_INVALIDSTATE;
        break;

    default:
        if(apiStatus < 0) {
            status = NAMESERVER_E_FAIL;
            /* status = NAMESERVER_E_OSFAILURE; */
        } else {
            status = apiStatus;
        }
        break;
    }

    GT_1trace (curTrace, GT_2CLASS,
        "IPCMGR-getNameServerStatus: status=0x%x\n", status);
    return status;
}


Int getMessageQStatus(Int apiStatus)
{
    Int status = MESSAGEQ_E_OSFAILURE;

    GT_1trace (curTrace, GT_2CLASS,
        "IPCMGR-getMessageQ: apiStatus=0x%x\n", apiStatus);

    switch(apiStatus) {
    case 0:
        status = MESSAGEQ_SUCCESS;
        break;

    case -EINVAL:
        status = MESSAGEQ_E_INVALIDARG;
        break;

    case -ENOMEM:
        status = MESSAGEQ_E_MEMORY;
        break;

    case -ENODEV:
        status = MESSAGEQ_E_INVALIDSTATE;
        break;

    case -EBUSY:
        status = MESSAGEQ_E_BUSY;
        break;

    case -ENOENT:
        status = MESSAGEQ_E_NOTFOUND;
        break;

    case -ETIME:
        status = MESSAGEQ_E_TIMEOUT;
        break;

    case 1:
        status = MESSAGEQ_S_ALREADYSETUP;
        break;

    /* All error codes are not converted to kernel error codes */
    default:
        status = apiStatus;
        /* status = MESSAGEQ_E_FAIL;
        status = MESSAGEQ_E_OSFAILURE; */
        break;
    }

    GT_1trace (curTrace, GT_2CLASS,
        "IPCMGR-getMessageQStatus: status=0x%x\n", status);
    return status;
}


Int getMessageQTransportShmStatus(Int apiStatus)
{
    Int status = MESSAGEQTRANSPORTSHM_E_OSFAILURE;

    GT_1trace (curTrace, GT_2CLASS,
        "IPCMGR-getMessageQTransportShmStatus: apiStatus=0x%x\n", apiStatus);

    switch(apiStatus) {
    case 0:
        status = MESSAGEQTRANSPORTSHM_SUCCESS;
        break;

    case -EINVAL:
        status = MESSAGEQTRANSPORTSHM_E_INVALIDARG;
        break;

    case -ENOMEM:
        status = MESSAGEQTRANSPORTSHM_E_MEMORY;
        break;

    case 1:
        status = MESSAGEQTRANSPORTSHM_S_ALREADYSETUP;
        break;

    /* All error codes are not converted to kernel error codes */
    default:
        status = apiStatus;
        /* status = MESSAGEQTRANSPORTSHM_E_FAIL;
        status = MESSAGEQTRANSPORTSHM_E_OSFAILURE; */
        break;
    }

    GT_1trace (curTrace, GT_2CLASS,
        "IPCMGR-getMessageQTransportShmStatus: status=0x%x\n", status);
    return status;
}


Int getListMPSharedMemoryStatus(Int apiStatus)
{
    Int status = LISTMPSHAREDMEMORY_E_OSFAILURE;

    GT_1trace (curTrace, GT_2CLASS,
        "IPCMGR-getListMPSharedMemoryStatus: apiStatus=0x%x\n", apiStatus);

    switch(apiStatus) {
    case 0:
        status = LISTMPSHAREDMEMORY_SUCCESS;
        break;

    case -EINVAL:
        status = LISTMPSHAREDMEMORY_E_INVALIDARG;
        break;

    case -ENOMEM:
        status = LISTMPSHAREDMEMORY_E_MEMORY;
        break;

    case -ENODEV:
        status = LISTMPSHAREDMEMORY_E_INVALIDSTATE;
        break;

    case -EBUSY:
        /* LISTMPSHAREDMEMORY_E_REMOTEACTIVE is also converted to this. */
        status = LISTMPSHAREDMEMORY_E_INUSE;
        break;

    case 1:
        status = LISTMPSHAREDMEMORY_S_ALREADYSETUP;
        break;

    /* All error codes are not converted to kernel error codes */
    default:
        status = apiStatus;
        /* status = LISTMPSHAREDMEMORY_E_FAIL;
        status = LISTMPSHAREDMEMORY_E_OSFAILURE; */
        break;
    }

    GT_1trace (curTrace, GT_2CLASS,
        "IPCMGR-getListMPSharedMemory: status=0x%x\n", status);
    return status;
}


Int getMultiProcStatus(Int apiStatus)
{
    Int status = MULTIPROC_E_OSFAILURE;

    GT_1trace (curTrace, GT_2CLASS,
        "IPCMGR-getMultiProcStatus: apiStatus=0x%x\n", apiStatus);

    switch(apiStatus) {
    case 0:
        status = MULTIPROC_SUCCESS;
        break;

    case -ENOMEM:
        status = MULTIPROC_E_MEMORY;
        break;

    case -EEXIST:
    case 1:
        status = MULTIPROC_S_ALREADYSETUP;
        break;

    case -ENODEV:
        status = MULTIPROC_E_INVALIDSTATE;
        break;

    default:
        status = MULTIPROC_E_FAIL;
        break;
    }

    GT_1trace (curTrace, GT_2CLASS,
        "IPCMGR-getMultiProcStatus: status=0x%x\n", status);
    return status;
}


Int getSysMgrStatus(Int apiStatus)
{
    Int status = SYSMGR_E_OSFAILURE;

    GT_1trace (curTrace, GT_2CLASS,
        "IPCMGR-getSysMgrStatus: apiStatus=0x%x\n", apiStatus);

    switch(apiStatus) {
    case 0:
        status = SYSMGR_SUCCESS;
        break;

    case -ENOMEM:
        status = SYSMGR_E_MEMORY;
        break;

    case SYSMGR_S_ALREADYSETUP:
    case 1:
        status = SYSMGR_S_ALREADYSETUP;
        break;

    case -ENODEV:
        status = SYSMGR_E_INVALIDSTATE;
        break;

    default:
        status = SYSMGR_E_FAIL;
        break;
    }

    GT_1trace (curTrace, GT_2CLASS,
        "IPCMGR-getSysMgrStatus: status=0x%x\n", status);
    return status;
}


Int getSysMemMgrStatus(Int apiStatus)
{
    Int status = SYSMEMMGR_E_OSFAILURE;

    GT_1trace (curTrace, GT_2CLASS,
        "IPCMGR-getSysMemMgrStatus: apiStatus=0x%x\n", apiStatus);

    switch(apiStatus) {
    case 0:
        status = SYSMEMMGR_SUCCESS;
        break;

    case -ENOMEM:
        status = SYSMEMMGR_E_MEMORY;
        break;

    case -EEXIST:
    case SYSMEMMGR_S_ALREADYSETUP:
        status = SYSMEMMGR_S_ALREADYSETUP;
        break;

    case -ENODEV:
        status = SYSMEMMGR_E_INVALIDSTATE;
        break;

    default:
        status = SYSMEMMGR_E_FAIL;
        break;
    }

    GT_1trace (curTrace, GT_2CLASS,
        "IPCMGR-getSysMemMgrStatus: status=0x%x\n", status);
    return status;
}


/* This function converts the os specific (Linux) error code to
 *  module specific error code
 */
void IPCManager_getModuleStatus(Int module_id, Ptr args)
{
    NameServerDrv_CmdArgs *ns_cmdargs = NULL;
    NameServerRemoteNotifyDrv_CmdArgs *nsrn_cmdargs = NULL;
    SharedRegionDrv_CmdArgs *shrn_cmdargs = NULL;
    GatePetersonDrv_CmdArgs *gp_cmdargs = NULL;
    HeapBufDrv_CmdArgs *hb_cmdargs = NULL;
    MessageQDrv_CmdArgs *mq_cmdargs = NULL;
    MessageQTransportShmDrv_CmdArgs *mqt_cmdargs = NULL;
    ListMPSharedMemoryDrv_CmdArgs *lstmp_cmdargs = NULL;
    MultiProcDrv_CmdArgs *multiproc_cmdargs = NULL;
    SysMgrDrv_CmdArgs *sysmgr_cmdargs = NULL;
    SysMemMgrDrv_CmdArgs *sysmemmgr_cmdargs = NULL;

    switch(module_id) {
    case MULTIPROC:
        multiproc_cmdargs = (MultiProcDrv_CmdArgs *)args;
        multiproc_cmdargs->apiStatus = getMultiProcStatus(multiproc_cmdargs->apiStatus);
        break;

    case NAMESERVER:
        ns_cmdargs = (NameServerDrv_CmdArgs *)args;
        ns_cmdargs->apiStatus = getNameServerStatus(ns_cmdargs->apiStatus);
        break;

    case NAMESERVERREMOTENOTIFY:
        nsrn_cmdargs = (NameServerRemoteNotifyDrv_CmdArgs *)args;
        nsrn_cmdargs->apiStatus = \
                getNameServerRemoteNotifyStatus(nsrn_cmdargs->apiStatus);
        break;

    case SHAREDREGION:
        shrn_cmdargs = (SharedRegionDrv_CmdArgs *)args;
        shrn_cmdargs->apiStatus = getSharedRegionStatus(shrn_cmdargs->apiStatus);
        break;

    case GATEPETERSON: /* Above id and this has same value */
        gp_cmdargs = (GatePetersonDrv_CmdArgs *)args;
        gp_cmdargs->apiStatus = getGatePetersonStatus(gp_cmdargs->apiStatus);
        break;

    case HEAPBUF:
        hb_cmdargs = (HeapBufDrv_CmdArgs *)args;
        hb_cmdargs->apiStatus = getHeapBufStatus(hb_cmdargs->apiStatus);
        break;

    case MESSAGEQ:
        mq_cmdargs = (MessageQDrv_CmdArgs *)args;
        mq_cmdargs->apiStatus = getMessageQStatus(mq_cmdargs->apiStatus);
        break;

    case MESSAGEQTRANSPORTSHM:
        mqt_cmdargs = (MessageQTransportShmDrv_CmdArgs *)args;
        mqt_cmdargs->apiStatus = \
                        getMessageQTransportShmStatus(mqt_cmdargs->apiStatus);
        break;

    case LISTMPSHAREDMEMORY:
        lstmp_cmdargs = (ListMPSharedMemoryDrv_CmdArgs *)args;
        lstmp_cmdargs->apiStatus = \
                        getListMPSharedMemoryStatus(lstmp_cmdargs->apiStatus);
        break;

    case SYSMGR:
        sysmgr_cmdargs = (SysMgrDrv_CmdArgs *)args;
        sysmgr_cmdargs->apiStatus = getSysMgrStatus(sysmgr_cmdargs->apiStatus);
        break;

    case SYSMEMMGR:
        sysmemmgr_cmdargs = (SysMemMgrDrv_CmdArgs *)args;
        sysmemmgr_cmdargs->apiStatus = \
                getSysMemMgrStatus(sysmemmgr_cmdargs->apiStatus);
        break;

    case GATEHWSPINLOCK:
        break;

    default:
        GT_0trace (curTrace, GT_2CLASS, "IPCMGR-Unknown module\n");
        break;
    }
}


/* This will increment individual module usage count */
Int createIpcModuleHandle(const char *name)
{
    Int module_found = 0;
    Int module_handle = 0;
    Int i;

    for (i = 0; i < MAX_IPC_MODULES; i++) {
        if (strcmp(name, name_id_table[i].module_name) == 0) {
            module_found = 1;
            break;
        }
    }

    if (module_found) {
        switch (name_id_table[i].module_id) {
        case MULTIPROC:
            module_handle = (name_id_table[i].module_id << 16) | ipc_handle;
            break;

        case SHAREDREGION:
            module_handle = (name_id_table[i].module_id << 16) | ipc_handle;
            break;

        case GATEPETERSON:
            module_handle = (name_id_table[i].module_id << 16) | ipc_handle;
            break;

        case HEAPBUF:
            module_handle = (name_id_table[i].module_id << 16) | ipc_handle;
            break;

        case NAMESERVER:
            module_handle = (name_id_table[i].module_id << 16) | ipc_handle;
            break;

        case MESSAGEQ:
            module_handle = (name_id_table[i].module_id << 16) | ipc_handle;
            break;

        case MESSAGEQTRANSPORTSHM:
            module_handle = (name_id_table[i].module_id << 16) | ipc_handle;
            break;

        case LISTMPSHAREDMEMORY:
            module_handle = (name_id_table[i].module_id << 16) | ipc_handle;
            break;

        case NAMESERVERREMOTENOTIFY:
            module_handle = (name_id_table[i].module_id << 16) | ipc_handle;
            break;

        case SYSMGR:
            module_handle = (name_id_table[i].module_id << 16) | ipc_handle;
            break;

        case SYSMEMMGR:
            module_handle = (name_id_table[i].module_id << 16) | ipc_handle;
            break;

        case GATEHWSPINLOCK:
            module_handle = (name_id_table[i].module_id << 16) | ipc_handle;
            break;

        default:
            break;
        }
    }

    if (module_found) {
        return module_handle;
    }
    else
        return -1;
}


/* This will track ioctl trafic from different ipc modules */
void trackIoctlCommandFlow(Int fd)
{
    switch (fd >> 16) {
    case MULTIPROC:
        name_id_table[MULTIPROC].ioctl_count += 1;
        break;

    case SHAREDREGION:
        name_id_table[SHAREDREGION].ioctl_count += 1;
        break;

    case GATEPETERSON:
        name_id_table[GATEPETERSON].ioctl_count += 1;
        break;

    case HEAPBUF:
        name_id_table[HEAPBUF].ioctl_count += 1;
        break;

    case NAMESERVER:
        name_id_table[NAMESERVER].ioctl_count += 1;
        break;

    case MESSAGEQ:
        name_id_table[MESSAGEQ].ioctl_count += 1;
        break;

    case MESSAGEQTRANSPORTSHM:
        name_id_table[MESSAGEQTRANSPORTSHM].ioctl_count += 1;
        break;

    case LISTMPSHAREDMEMORY:
        name_id_table[LISTMPSHAREDMEMORY].ioctl_count += 1;
        break;

    case NAMESERVERREMOTENOTIFY:
        name_id_table[NAMESERVERREMOTENOTIFY].ioctl_count += 1;
        break;

    case SYSMGR:
        name_id_table[SYSMGR].ioctl_count += 1;
        break;

    case SYSMEMMGR:
        name_id_table[SYSMEMMGR].ioctl_count += 1;
        break;

    case GATEHWSPINLOCK:
        name_id_table[GATEHWSPINLOCK].ioctl_count += 1;
        break;

    default:
        break;
    }
}


Void displayIoctlInfo(Int fd, UInt32 cmd, Ptr args)
{
    UInt32 ioc_nr = _IOC_NR(cmd);

    switch (fd >> 16) {
    case MULTIPROC:
        GT_0trace (curTrace, GT_2CLASS,
            "IPCMGR: IOCTL command from MULTIPROC module\n");
        break;

    case SHAREDREGION:
        GT_0trace (curTrace, GT_2CLASS,
            "IPCMGR: IOCTL command from SHAREDREGION module\n");
        break;

    case GATEPETERSON:
        GT_0trace (curTrace, GT_2CLASS,
            "IPCMGR: IOCTL command from GATEPETERSON module\n");
        break;

    case HEAPBUF:
        GT_0trace (curTrace, GT_2CLASS,
            "IPCMGR: IOCTL command from HEAPBUF module\n");
        break;

    case NAMESERVER:
        GT_0trace (curTrace, GT_2CLASS,
            "IPCMGR: IOCTL command from NAMESERVER module\n");
        break;

    case MESSAGEQ:
        GT_0trace (curTrace, GT_2CLASS,
            "IPCMGR: IOCTL command from MESSAGEQ module\n");
        break;

    case MESSAGEQTRANSPORTSHM:
        GT_0trace (curTrace, GT_2CLASS,
            "IPCMGR: IOCTL command from MESSAGEQTRANSPORTSHM module\n");
        break;

    case LISTMPSHAREDMEMORY:
        GT_0trace (curTrace, GT_2CLASS,
            "IPCMGR: IOCTL command from LISTMPSHAREDMEMORY module\n");
        break;

    case NAMESERVERREMOTENOTIFY:
        GT_0trace (curTrace, GT_2CLASS,
            "IPCMGR: IOCTL command from NAMESERVERREMOTENOTIFY module\n");
        break;

    case SYSMGR:
        GT_0trace (curTrace, GT_2CLASS,
            "IPCMGR: IOCTL command from SYSMGR module\n");
        break;

    case SYSMEMMGR:
        GT_0trace (curTrace, GT_2CLASS,
            "IPCMGR: IOCTL command from SYSMEMMGR module\n");
        break;

   case GATEHWSPINLOCK:
        GT_0trace (curTrace, GT_2CLASS,
            "IPCMGR: IOCTL command from GATEHWSPINLOCK module\n");
        break;

    default:
        GT_0trace (curTrace, GT_2CLASS,
            "IPCMGR: IOCTL command from UNKNOWN module\n");
        break;
    }

    GT_4trace (curTrace, GT_2CLASS,
        "IOCTL cmd : %x, IOCTL ioc_nr: %x(%d) args: %p\n",
            cmd, ioc_nr, ioc_nr, args);
}


/*
 *  ======== IPCManager_open ========
 *  Purpose:
 *      Open handle to the IPC driver
 */
Int IPCManager_open(const char *name, Int flags)
{
    Int module_handle = 0;
    Int retval = -1;

    GT_2trace (curTrace, GT_1CLASS,
        "IPCManager_open: Module Name:%s Flags:0x%x\n", name, flags);

    if (!ipc_sem_initialized) {
        if (sem_init(&semOpenClose, 0, 1) == -1) {
            GT_0trace (curTrace, GT_4CLASS,
                "MGR: Failed to Initialize"
                "the ipc semaphore\n");
            goto exit;
        } else
            ipc_sem_initialized = true;
    }

    sem_wait(&semOpenClose);
    if (ipc_usage_count == 0) {
        retval = open(IPC_DRIVER_NAME, flags);
        if (retval < 0) {
            GT_0trace(curTrace, GT_4CLASS,
                "IPCManager_open: Failed to open the ipc driver\n");
            goto error;
        }
        ipc_handle = retval;
    }

    /* Success in opening handle to ipc driver */
    module_handle = createIpcModuleHandle(name);
    if (module_handle < 0) {
        GT_0trace(curTrace, GT_4CLASS,
            "IPCManager_open: Failed to creat module handle\n");
        retval = -1;
        goto error;
    }

    ipc_usage_count++;
    sem_post(&semOpenClose);
    return module_handle;

error:
    GT_1trace (curTrace, GT_LEAVE, "IPCManager_open", retval);
    sem_post(&semOpenClose);

exit:
    return retval;
}


/*
 *  ======== IPCManager_Close ========
 *  Purpose:   Close handle to the IPC driver
 */
Int IPCManager_close(Int fd)
{
    Int retval = 0;

    sem_wait(&semOpenClose);
    if (ipc_handle < 0)
        goto exit;

    ipc_usage_count--;
    if (ipc_usage_count == 0) {
        retval = close(ipc_handle);
        if (retval < 0)
            goto exit;

        ipc_handle = -1;
    }

exit:
    sem_post(&semOpenClose);
    return retval;
}


/*
 * ======== IPCTRAP_Trap ========
 */
Int IPCManager_ioctl(Int fd, UInt32 cmd, Ptr args)
{
    Int status = -1;

    displayIoctlInfo(fd, cmd, args);
    if (ipc_handle >= 0)
        status = ioctl(ipc_handle, cmd, args);

    IPCManager_getModuleStatus(fd >> 16, args);
    /* This is to track the command flow */
    trackIoctlCommandFlow(fd);

    GT_1trace (curTrace, GT_LEAVE, "IPCManager_ioctl", status);
    return status;
}


/*
 * ======== IPCTRAP_fcntl========
 */
Int IPCManager_fcntl(Int fd, Int cmd, long arg)
{
    Int retval = -1;

    retval = fcntl(ipc_handle, cmd, arg);

    return retval;
}
