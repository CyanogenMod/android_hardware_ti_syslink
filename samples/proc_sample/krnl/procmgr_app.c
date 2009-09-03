/*============================================================================
 *  @file   procmgrapp.c
 *
 *  @brief  Sample application for procmgr module
 *
 *  @ver    2.00.00.08
 *
 *  ============================================================================
 *
 *  Copyright (c) 2008-2009, Texas Instruments Incorporated
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  *  Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 *  *  Neither the name of Texas Instruments Incorporated nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 *  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *  Contact information for paper mail:
 *  Texas Instruments
 *  Post Office Box 655303
 *  Dallas, Texas 75265
 *  Contact information:
 *  http://www-k.ext.ti.com/sc/technical-support/product-information-centers.htm?
 *  DCMP=TIHomeTracking&HQS=Other+OT+home_d_contact
 *  ============================================================================
 */
//#define SYSLINK_USE_SYSMGR

#include <linux/autoconf.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/pgtable.h>
#include <linux/types.h>

/* Module level headers */
#include <multiproc.h>
#include <platform_mem.h>
#include <procmgr.h>
#include <procmgr_drvdefs.h>
#if !defined (SYSLINK_USE_SYSMGR)
#include <proc4430.h>
#else
#include <sysmgr.h>
#endif /* if !defined (SYSLINK_USE_SYSMGR) */

/** ============================================================================
 *  Macros and types
 *  ============================================================================
 */
	
#if !defined (SYSLINK_USE_SYSMGR)
/*
 *  Number of slave memory entries for OMAP4430.
 */
#define NUM_MEM_ENTRIES 3

/*
 *  Position of reset vector memory region in the memEntries array
 */
#define RESET_VECTOR_ENTRY_ID 0

#endif /* if !defined (SYSLINK_USE_SYSMGR) */

/*
 *  handle to the procmgr instance used.
 */
void *procmgrapp_handle	   = NULL;


#if !defined (SYSLINK_USE_SYSMGR)
/*
 *  Array of memory entries for OMAP4430
 */
static struct proc4430_mem_entry mem_entries [NUM_MEM_ENTRIES] =
{
	{
		"DUCATI_SHM",	/* NAME	     : Name of the memory region */
		0x87B00000,	/* PHYSADDR	     : Physical address */
		0x98000000,	/* SLAVEVIRTADDR  : Slave virtual address */
		(u32) -1u,	/* MASTERVIRTADDR : Master virtual address (if known) */
		0x80000,	/* SIZE	     : Size of the memory region */
		true,		/* SHARE	     : Shared access memory? */
	},
	{
		"DUCATI_SHM1",	/* NAME	     : Name of the memory region */
		0x87B80000,	/* PHYSADDR	     : Physical address */
		0x98080000,	/* SLAVEVIRTADDR  : Slave virtual address */
		(u32) -1u,	/* MASTERVIRTADDR : Master virtual address (if known) */
		0x80000,	/* SIZE	     : Size of the memory region */
		true,		/* SHARE	     : Shared access memory? */
	},
	{
		"DUCATI_SWDMM",	/* NAME	     : Name of the memory region */
		0x87C00000,	/* PHYSADDR	     : Physical address */
		0x80000000,	/* SLAVEVIRTADDR  : Slave virtual address */
		(u32) -1u,	/* MASTERVIRTADDR : Master virtual address (if known) */
		0x400000,	/* SIZE	     : Size of the memory region */
		true,		/* SHARE	     : Shared access memory? */
	}
};


/*!
 *  @brief  handle to the Processor instance used.
 */
void *procmgr_app_proc_handle   = NULL;

/*!
 *  @brief  handle to the PwrMgr instance used.
 */
void *procmgrapp_pwrhandle	= NULL;

/*!
 *  @brief  handle to the Loader instance used.
 */
void *procmgrapp_loaderhandle = NULL;

/*!
 *  @brief  File ID of the file loaded.
 */
u32 procmgr_app_file_id	   = 0;

/** ============================================================================
 *  Forward declarations of internal functions
 *  ============================================================================
 */
/* Function to perform startup operations when SysMgr is not used. */
int _procmgrapp_sys_startup(void);

/* Function to perform shutdown operations when SysMgr is not used. */
int  _procmgrapp_sys_shutdown(void);

#endif /* if !defined (SYSLINK_USE_SYSMGR) */

/*!
 *  @brief  handle to the Processor instance opned.
 */
void *procmgr_app_open_handle   = NULL;

/** ============================================================================
 *  Functions
 *  ============================================================================
 */
/*!
 *  @brief  Function to execute the startup for procmgrapp sample application
 */
int procmgrapp_startup(void)
{
	int status = 0;
	struct proc_mgr_attach_params attach_params;
	enum proc_mgr_state state;
#if defined (SYSLINK_USE_SYSMGR)
	u16 proc_id = multiproc_get_id("SysM3");
#endif /* if defined (SYSLINK_USE_SYSMGR) */
	printk(KERN_ERR "Entered ProcMgrApp_startup\n");

#if !defined (SYSLINK_USE_SYSMGR)
	status = _procmgrapp_sys_startup();
#else /* if !defined (SYSLINK_USE_SYSMGR) */
	status = proc_mgr_open(&procmgrapp_handle, proc_id);
	procmgr_app_open_handle = procmgrapp_handle;
#endif /* if !defined (SYSLINK_USE_SYSMGR) */
	if (status < 0)
		goto exit;

	proc_mgr_get_attach_params(NULL, &attach_params);
	/* Default params will be used if NULL is passed. */
	status = proc_mgr_attach(procmgrapp_handle, &attach_params);
	if (status < 0) {
		printk(KERN_ERR "proc_mgr_attach failed [0x%x]\n", status);
		status = -1;
		goto exit;
	}

	state = proc_mgr_get_state(procmgrapp_handle);
	printk(KERN_ERR "After attach: procmgr_getState state [0x%x]\n", state);
	printk(KERN_ERR "Leaving procmgrapp_startup with success\n");
	return 0;

exit:
	printk(KERN_ERR "**********Leaving procmgrapp_startup with failure*************\n");
	return status;
}

#if !defined (SYSLINK_USE_SYSMGR)
/*!
 *  @brief  Function to perform startup operations when SysMgr is not used.
 */
int _procmgrapp_sys_startup(void)
{
	int status = 0;
	struct proc_mgr_config config;
	struct proc4430_config proc_config;
	struct proc_mgr_params params;
	struct proc4430_params proc_params;
	struct multiproc_config	multiprocconfig;
	u16 proc_id;

	printk(KERN_ERR "Entered procmgrapp_sys_startup\n");
	/* Setup procmgr and its subsidiary modules */
	proc_mgr_get_config(&config);
	status = proc_mgr_setup(&config);
	if (status < 0) {
		printk(KERN_ERR "Error in procmgr_setup [0x%x]\n", status);
		goto exit;
	}
	printk(KERN_ERR"procmgr_setup status: [0x%x]\n", status);

	proc4430_get_config(&proc_config);
	status = proc4430_setup(&proc_config);
	if (status < 0) {
		printk(KERN_ERR "Error in proc4430_setup [0x%x]\n", status);
		goto exit;
	}

	printk(KERN_ERR"proc4430_setup status: [0x%x]\n", status);

	multiprocconfig.max_processors = 4;
	multiprocconfig.id             = 0;
	strcpy (multiprocconfig.name_list [0], "MPU");
	strcpy (multiprocconfig.name_list [1], "Tesla");
	strcpy (multiprocconfig.name_list [2], "SysM3");
	strcpy (multiprocconfig.name_list [3], "AppM3");

	multiproc_setup(&multiprocconfig);

	/* Get MultiProc ID by name. */
	proc_id = multiproc_get_id("SysM3");
	printk(KERN_ERR "MultiProc_getId proc_id: [0x%x]\n", proc_id);
	/* Create an instance of the Processor object for OMAP4430 */
	proc4430_params_init(NULL, &proc_params);
	proc_params.num_mem_entries = NUM_MEM_ENTRIES;
	proc_params.mem_entries  = (struct proc4430_mem_entry *)&mem_entries;
	proc_params.reset_vector_mem_entry = RESET_VECTOR_ENTRY_ID;


	procmgr_app_proc_handle = proc4430_create(proc_id, &proc_params);
	if (procmgr_app_proc_handle == NULL) {
		printk(KERN_ERR "Error in proc4430_create \n");
		status = -1;
		goto exit;
	}
		
	printk(KERN_ERR "proc4430_create procmgr_app_proc_handle: [0x%x]\n",
				(u32)procmgr_app_proc_handle);
	/* Initialize parameters */
	proc_mgr_params_init(NULL, &params);
	params.proc_handle = procmgr_app_proc_handle;
	procmgrapp_handle = proc_mgr_create(proc_id, &params);
	if (procmgrapp_handle == NULL) {
		printk(KERN_ERR"Error in procmgr_create \n");
		status = -1;
		goto exit;
	}
	printk(KERN_ERR "procmgr_create handle: [0x%x]\n",
					(unsigned int) procmgrapp_handle);

	printk(KERN_ERR "Leaving procmgrapp_sys_startup with success\n");
	return 0;

exit:
	printk(KERN_ERR "**********Leaving procmgrapp_sys_startup with failure*************\n");
	return status;

}
#endif

int procmgrapp_execute(void)
{
	int status = 0;
	int count = 0;
	int i;
	struct proc_mgr_cmd_args_open src_args;

	src_args.proc_id = multiproc_get_id("SysM3");
#if !defined (SYSLINK_USE_SYSMGR)
	status = proc_mgr_open(&(src_args.handle),
							src_args.proc_id);
	if (status < 0) {
		printk(KERN_ERR "proc_mgr_open failed [0x%x]\n", status);
		status = -1;
		goto exit;
	}
	procmgr_app_open_handle = src_args.handle;
#else /* if !defined (SYSLINK_USE_SYSMGR) */
	src_args.handle = procmgrapp_handle;
#endif /* if !defined (SYSLINK_USE_SYSMGR) */


	printk(KERN_ERR "procmgrapp_execute proc4430_handle [0x%x]\n", (u32)src_args.handle);
	status = proc_mgr_get_proc_info(src_args.handle,
			&(src_args.proc_info));
	if (status < 0) {
		printk(KERN_ERR "proc_mgr_get_proc_info failed [0x%x]\n", status);
		status = -1;
		goto exit;
	}

	printk(KERN_ERR "boot_mode  [0x%x]\n", src_args.proc_info.boot_mode);
	printk(KERN_ERR "num_mem_entries  [0x%x]\n", src_args.proc_info.num_mem_entries);
	count =  src_args.proc_info.num_mem_entries;
	for (i = 0; i < count; i++) {
		printk(KERN_ERR "is_init  [0x%x]\n",
					src_args.proc_info.mem_entries[i].is_init);
		printk(KERN_ERR "PROC_MGR_ADDRTYPE_MASTERKNLVIRT [0x%x]\n",
			src_args.proc_info.mem_entries[i].addr[PROC_MGR_ADDRTYPE_MASTERKNLVIRT]);
		printk(KERN_ERR "PROC_MGR_ADDRTYPE_MASTERUSRVIRT  [0x%x]\n",
			src_args.proc_info.mem_entries[i].addr[PROC_MGR_ADDRTYPE_MASTERUSRVIRT]);
		printk(KERN_ERR "PROC_MGR_ADDRTYPE_SLAVEVIRT  [0x%x]\n",
			src_args.proc_info.mem_entries[i].addr[PROC_MGR_ADDRTYPE_SLAVEVIRT]);
	}

	printk(KERN_ERR "size  [0x%x]\n",
					src_args.proc_info.mem_entries[i].size);
exit:
	return status;
}


/*!
 *  @brief  Function to execute the shutdown for procmgrapp sample application
 */
int procmgrapp_shutdown(void)
{
	int	status = 0;
	enum proc_mgr_state state;

	printk(KERN_ERR "Entered procmgrapp_shutdown\n");

	if (procmgrapp_handle != NULL) {
		status = proc_mgr_detach(procmgrapp_handle);
		printk(KERN_ERR "proc_mgr_detach [0x%x]\n", status);

		state = proc_mgr_get_state(procmgrapp_handle);
		printk(KERN_ERR "After detach: procmgr_getState\n"
					 "	state [0x%x]\n", state);
	}

#if !defined (SYSLINK_USE_SYSMGR)
	status = _procmgrapp_sys_shutdown();
#else /* if !defined (SYSLINK_USE_SYSMGR) */
	if (procmgr_app_open_handle != NULL) {
		status = proc_mgr_close(&(procmgr_app_open_handle));
		printk(KERN_ERR "proc_mgr_close [0x%x]\n", status);
	}
#endif /* if !defined (SYSLINK_USE_SYSMGR) */

	printk(KERN_ERR "Leaving procmgrapp_shutdown [0x%x]\n", status);
	return status;
}

#if !defined (SYSLINK_USE_SYSMGR)
/*!
 *  @brief  Function to perform shutdown operations when SysMgr is not used.
 */
int _procmgrapp_sys_shutdown(void)
{
	int	status = 0;

	printk(KERN_ERR "Entered _procmgrapp__sys_shutdown\n");
	if (procmgr_app_proc_handle != NULL) {
		status = proc4430_delete(&procmgr_app_proc_handle);
		printk(KERN_ERR "proc4430_delete completed [0x%x]\n", status);
	}
	if (procmgrapp_handle != NULL) {
		status = proc_mgr_delete(&procmgrapp_handle);
		printk(KERN_ERR "procmgr_delete completed [0x%x]\n", status);
	}
	proc4430_destroy();
	printk(KERN_ERR "proc4430_destroy completed\n");
	proc_mgr_destroy();
	printk(KERN_ERR"procmgr_destroy completed\n");
	printk(KERN_ERR"Leaving _procmgrapp__sys_shutdown\n");
	multiproc_destroy();
	return status;
}
#endif

/*
 *  Module initialization  function for Linux driver
 */
u32 procmgr_app_run_sample_app = 0;
static int __init procmgrapp_initialize_module(void)
{
	int result = 0;
#if defined (SYSLINK_USE_SYSMGR)
	struct sysmgr_config config;
#endif
	/* Display the version info and created date/time */
	printk(KERN_ERR "ProcMgr sample module created on Date:%s Time:%s\n",
			__DATE__,
			__TIME__);

	printk(KERN_ERR "Entered ProcMgrApp_initializeModule\n");

	if (procmgr_app_run_sample_app != 0) {
		printk(KERN_ERR "procmgrapp already running\n");
	}

#if defined (SYSLINK_USE_SYSMGR)
	sysmgr_get_config(&config);
	printk(KERN_ERR "sysmgr_getconfig done [0x%x]\n");

	result = sysmgr_setup(&config);
	if (result < 0)
		printk(KERN_ERR "sysmgr_setup failed [0x%x]\n", result);
#else /* if defined (SYSLINK_USE_SYSMGR) */
	platform_mem_setup();
#endif /* if defined (SYSLINK_USE_SYSMGR) */

	result = procmgrapp_startup();
	if (result < 0) {
		printk(KERN_ERR "procmgrapp_startup failed\n");
		goto exit;
	}

	result = procmgrapp_execute();
	if (result < 0) {
		printk(KERN_ERR "procmgrapp_execute failed status [0x%x]\n", result);
	}

	printk(KERN_ERR "Leaving procmgrapp_initialize_module [0x%x]\n", result);

exit:
    return result;
}


/*
 *  Linux driver function to finalize the driver module.
 */
static void __exit procmgrapp_finalize_module (void)
{
	int result = 0;

	printk(KERN_ERR "Entered procmgrapp_finalize_module\n");
	result = procmgrapp_shutdown ();
#if defined (SYSLINK_USE_SYSMGR)
	sysmgr_destroy();
#else /* if defined (SYSLINK_USE_SYSMGR) */
	platform_mem_destroy();
#endif /* if defined (SYSLINK_USE_SYSMGR) */
	printk(KERN_ERR "Leaving procmgrapp_finalize_module status:%x\n",
			result);

}

/*
 *  Macro calls that indicate initialization and finalization functions
 *  to the kernel.
 */
MODULE_LICENSE ("GPL v2");
module_init (procmgrapp_initialize_module);
module_exit (procmgrapp_finalize_module);
