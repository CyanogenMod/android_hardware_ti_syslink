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
 * IPCMultiProc.c
 *
 * IPC support functions for TI OMAP processors.
 */

/*
 * Handles processor id management in multi processor systems.Used
 * by all modules which need processor ids for their oprations.
 *
 */

#include <host_os.h>
#include <Std.h>
#include <IPCMultiProc.h>
#include <Trace.h>
#include <IPCManager.h>

#define MULTIPROC_HANDLE 0

typedef struct Module_state{
	char **proc_names;
	UInt16 local_id;
	UInt16 max_id;
	sem_t sem_cache;
	UInt16 initialized;
}MultiProc_cache;

MultiProc_cache cache = {
	NULL,
	MultiProc_INVALIDID,
	0,
	{ { 0 } },
	FALSE
};

/*
* ======== MultiProc_InitCache ========
* Purpose:
* This will create a local cache of the multiproc 
* to avoid system multiporc access
*/
static Int MultiProc_InitCache(void)
{
	struct multiproc_cmd_args args;
	UInt16 max_id = 0;
	Int i;
	Int j;
	Int status; /* IOCTL retrun is not used right now */
	Int len;
	char name[80];
	if (cache.initialized != TRUE) {
		if (sem_init(&cache.sem_cache, 0, 0) == -1) {
			GT_0trace (curTrace, GT_4CLASS, "MultiProc_InitCache"
			" init lock failed!\n");
			return FAIL;
		}
		cache.initialized = TRUE;
	}
	/* Get max id */
	args.cmd_arg.get_max_id.max_id = &max_id;
	status = ioctl(MULTIPROC_HANDLE, CMD_MULTIPROC_GETMAXID,&args);
	/* Allocate zero filled memory for the processor name pointer table */
	cache.proc_names = calloc(max_id, sizeof(char *));
	if(cache.proc_names == NULL) {
		GT_0trace (curTrace, GT_4CLASS, "MultiProc_InitCache"
			" memory allocation failed!\n");
		return FAIL;
	}

	args.cmd_arg.get_name.name = name;
	/* Create the proc name table */
	for(i = 0; i < max_id; i++) {
		args.cmd_arg.get_name.proc_id = i;
		status = ioctl(MULTIPROC_HANDLE, CMD_MULTIPROC_GETNAME,&args);
		len = strlen(args.cmd_arg.get_name.name) + 1;
		cache.proc_names[i] = malloc(len);
		if(cache.proc_names[i] != NULL) {
			strncpy(cache.proc_names[i], args.cmd_arg.get_name.name,
				len);
		} else {
			GT_0trace (curTrace, GT_4CLASS, "MultiProc_InitCache"
				" memory allocation failed!\n");
			/* Free memory allocated for successful proc names */
			for(j = 0; j < i; j++)
				free(cache.proc_names[j]);
			/* Free memory allocated for the proc name pointer table */
			free(cache.proc_names);
			return FAIL;/* Error for memory failure */
		}
		memset(args.cmd_arg.get_name.name, 0,len);
	}
	/* Get local processor id */
	args.cmd_arg.get_id.name = '\0';
	args.cmd_arg.get_id.name_len = 0;	
	status = ioctl(MULTIPROC_HANDLE, CMD_MULTIPROC_GETID, &args);
	cache.local_id = args.cmd_arg.get_id.proc_id;
	sem_post(&cache.sem_cache);

	return MULTIPROC_SUCCESS;

}

/*
* ======== MultiProc_FlushCache ========
* Purpose:
* This will flushes/destroy a local cache of the multiproc 
*/
static Int MultiProc_DeleteCache(void)
{
	Int i;
	if (cache.initialized != TRUE)
		return FAIL;

	sem_wait(&cache.sem_cache);
	for(i = 0; i < cache.max_id; i++)
		free(cache.proc_names[i]);
	free(cache.proc_names);
	cache.local_id = MultiProc_INVALIDID;
	cache.max_id = 0;
	cache.initialized = FALSE;
	if(sem_destroy(&cache.sem_cache))
		return FAIL;

	return MULTIPROC_SUCCESS;
}

/* ======== MultiProc_cache_getId ========
* Purpose:
* This will get proccesor Id from cache.
*/
static UInt16 MultiProc_cache_getId(String name) 
{
	int i;
	if(name == NULL) {
		return cache.local_id;
	} else {
		for(i = 0; i < cache.max_id; i++) {
			if(strcmp(name, cache.proc_names[i]) == 0)
				return i;			
		}
		GT_0trace (curTrace, GT_4CLASS, "MultiProc_cache_getId"
			" processor name not found!\n");
	}
	return MultiProc_INVALIDID;
}

/*
* ======== MultiProc_cache_getName ========
* Purpose:
* This will get get name from processor id from cache.
*/
static String MultiProc_cache_getName(UInt16 id)
{
	String name = cache.proc_names[id];
	return name;
}


/*
* ======== MultiProc_getId ========
* Purpose:
* This will get proccesor Id from proccessor name.
*/
UInt16 MultiProc_getId(String name)
{
	UInt16 id    = MultiProc_INVALIDID;
	Int status;

	GT_1trace (curTrace, GT_ENTER, "MultiProc_getId", name);
	/* Initialize the local cache if needed and take the id from it */
	if (cache.initialized != TRUE) {
		status = MultiProc_InitCache();
		if(status == MULTIPROC_SUCCESS) {
			id = MultiProc_cache_getId(name);
			return id;
		}
	} else {
		id = MultiProc_cache_getId(name);
		return id;
	}
	return id;
}

/*
* ======== MultiProc_getName ========
* Purpose:
* This will get get name from processor id.
*/
String MultiProc_getName(UInt16 id)
{
	String name;
	Int status;

	GT_1trace (curTrace, GT_ENTER, "MultiProc_getName", id);
	/* Initialize the local cache if needed and take the name from it */
	if (cache.initialized == TRUE) {
		if(id >= cache.max_id) 
			return NULL;
		name = MultiProc_cache_getName(id);
		return name;
	} else {
		status = MultiProc_InitCache();
		if(status == MULTIPROC_SUCCESS) {
			if(id >= cache.max_id)
				return NULL;
			name = MultiProc_cache_getName(id);
			return name;
		}
	}
	return NULL;
}

/* ======== MultiProc_getMaxId ========
* Purpose:
* This will get maximum proc Id in the system..
*/
UInt16 MultiProc_getMaxProcessors()
{
	Int max_id;
	Int status;

	/* Take from local cache and give it to user */
	if(cache.initialized == TRUE) {
		max_id = cache.max_id;
		return max_id;
	} else {
		status = MultiProc_InitCache();
		if(status == MULTIPROC_SUCCESS) {
			max_id = cache.max_id;
			return max_id;
		}
	}
}

