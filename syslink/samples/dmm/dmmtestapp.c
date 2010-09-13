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

/*============================================================================
 *  @file   dmmtestapp.c
 *
 *  @brief  DMM test cases
 *
 *  ============================================================================
 */


 /* OS-specific headers */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

/* Standard headers */
#include <Std.h>

/* OSAL & Utils headers */
#include <OsalPrint.h>

/* RCM headers */
#include <RcmClient.h>
#include <RcmServer.h>

/* IPC headers */
#include <IpcUsr.h>
#include <ProcMgr.h>
#include <SysLinkMemUtils.h>

/* Tiler header file */
#include <tilermem.h>
#include <tilermgr.h>
#include <memmgr.h>

/* RCM headers */
#include <RcmClient.h>

/* Sample headers */
#include <MemAllocTest_Config.h>

#define maxCount	100
#define PAGE_SIZE	0x1000

enum {
	DMM_BUFFER = 1,
	USE_BUFFER = 2,
	MAP_BUFFER = 3,
	FLUSH_BUFFER_NEG = 4,
	MAP_NO_UNMAP = 5,
	MAP_IO_BUFFER = 6,
	DMM_TESTS_NUM = MAP_IO_BUFFER,
};
/*!
 *  @brief  Structure defining RCM remote function arguments
 */
typedef struct {
	uint32_t num_bytes;
	 /*!< Size of the Buffer */
	Ptr buf_ptr;
	 /*!< Buffer that is passed */
} RCM_Remote_FxnArgs;

uint32_t           fxn_buffer_test_idx;
uint32_t           fxn_exit_idx;
static RcmClient_Message  *return_msg;
static RcmClient_Handle   rcm_client_handle;
ProcMgr_Handle		proc_mgr_handle;
ProcMgr_Handle		proc_mgr_handle1;

/*
 *  ======== ipcSetup ========
 */
int ipcSetup(int proc_id)
{
	int			status = 0;
	Ipc_Config		config;
	RcmClient_Params	rcm_client_params;
	char			*remote_server_name;
	int			count = 0;
	ProcMgr_AttachParams	attachParams;
	ProcMgr_State		state;
	int			proc_sysm3 = 0;
	int			proc_appm3 = 0;

	Osal_printf("ipcSetup: Setup IPC componnets\n");

	Ipc_getConfig(&config);
	status = Ipc_setup(&config);
	if (status < 0) {
		Osal_printf("ipcSetup: Error in Ipc_setup [0x%x]\n", status);
		goto exit;
	}
	Osal_printf("ipcSetup: Ipc_setup status [0x%x]\n", status);

	if (proc_id == PROC_SYSM3)
		proc_sysm3 = 1;
	else if (proc_id == PROC_APPM3) {
		proc_sysm3 = 1;
		proc_appm3 = 1;
	} else {
		Osal_printf("ipcSetup: INVALID PROCID\n");
		goto exit;
	}

	if (proc_sysm3) {
		/* Open a handle to the ProcMgr instance. */
		status = ProcMgr_open(&proc_mgr_handle, PROC_SYSM3);
		if (status < 0) {
			Osal_printf("ipcSetup: Error in ProcMgr_open[0x%x]\n",
									status);
			goto exit;
		}


		Osal_printf ("ipcSetup: ProcMgr_open Status [0x%x]\n", status);
		ProcMgr_getAttachParams(NULL, &attachParams);
		/* Default params will be used if NULL is passed. */
		status = ProcMgr_attach (proc_mgr_handle, &attachParams);
		if (status < 0) {
			Osal_printf ("ipcSetup: ProcMgr_attach failed [0x%x]\n",
									status);
		} else {
			Osal_printf ("ipcSetup: ProcMgr_attach status: [0x%x]\n",
									status);
			state = ProcMgr_getState(proc_mgr_handle);
			Osal_printf ("ipcSetup: After attach: ProcMgr_getState\n"
			             "    state [0x%x]\n", state);
		}
	}

	if (proc_appm3) {
		/* Open a handle to the ProcMgr instance. */
		status = ProcMgr_open(&proc_mgr_handle1, PROC_APPM3);
		if (status < 0) {
			Osal_printf("ipcSetup: Error in ProcMgr_open[0x%x]\n",
									status);
			goto exit;
		}
		Osal_printf ("ipcSetup: ProcMgr_open Status [0x%x]\n", status);
		ProcMgr_getAttachParams(NULL, &attachParams);
		/* Default params will be used if NULL is passed. */
		status = ProcMgr_attach (proc_mgr_handle1, &attachParams);
		if (status < 0) {
			Osal_printf ("ipcSetup: ProcMgr_attach failed [0x%x]\n",
									status);
		} else {
			Osal_printf ("ipcSetup: ProcMgr_attach status: [0x%x]\n",
									status);
			state = ProcMgr_getState(proc_mgr_handle1);
			Osal_printf ("ipcSetup: After attach: ProcMgr_getState\n"
			             "    state [0x%x]\n", state);
		}
	}

	/* Set up RCM */

	/* Rcm client module init*/
	Osal_printf("RCM Client module init.\n");
	RcmClient_init();

	/* Rcm client module params init*/
	Osal_printf("RCM Client module params init.\n");
	status = RcmClient_Params_init(&rcm_client_params);
	if (status < 0) {
		Osal_printf("Error in RCM Client instance params init\n");
		goto exit;
	} else {
		Osal_printf("RCM Client instance params init passed\n");
	}

	if (proc_id == PROC_SYSM3)
		remote_server_name = RCM_SERVER_NAME_SYSM3;
	else if (proc_id == PROC_APPM3)
		remote_server_name = RCM_SERVER_NAME_APPM3;
	else
		remote_server_name = NULL;

	rcm_client_params.heapId = RCM_MSGQ_TILER_HEAPID;

	/* create an rcm client instance */
	Osal_printf("Creating RcmClient instance %s.\n", remote_server_name);
	rcm_client_params.callbackNotification = 0;

	while ((rcm_client_handle == NULL) && (count++ < MAX_CREATE_ATTEMPTS)) {
		status = RcmClient_create(remote_server_name,
				&rcm_client_params, &rcm_client_handle);
		if (status < 0) {
			if (status == RcmClient_E_SERVERNOTFOUND) {
				Osal_printf("Unable to open remote server"
						"%d time\n", count);
			} else {
				Osal_printf("Error in RCM Client create status"
							"= %d\n", status);
				goto exit;
			}
		} else {
			Osal_printf("RCM Client create passed\n");
		}
	}
	if (MAX_CREATE_ATTEMPTS <= count) {
		Osal_printf("Timeout... could not connect with remote"
								"server\n");
	}

	Osal_printf("\nQuerying server for fxnBufferTest() function index\n");

	status = RcmClient_getSymbolIndex(rcm_client_handle, "fxnBufferTest",
							&fxn_buffer_test_idx);
	if (status < 0)
		Osal_printf("Error getting symbol index [0x%x]\n", status);
	else {
		Osal_printf("fxnBufferTest() symbol index [0x%x]\n",
						fxn_buffer_test_idx);
	}

	Osal_printf("\nQuerying server for fxnExit() function index\n");

	status = RcmClient_getSymbolIndex(rcm_client_handle, "fxnExit",
							&fxn_exit_idx);
	if (status < 0)
		Osal_printf("Error getting symbol index [0x%x]\n", status);
	else
		Osal_printf("fxnExit() symbol index [0x%x]\n", fxn_exit_idx);

exit:
	Osal_printf("ipcSetup: Leaving ipcSetup()\n");
	return status;
}

/*!
 *  @brief  Function to demonstarate that flush fails on buffers that
 *          are not mapped to Device
 *
 *  @param  size    Size of the buffer to flush
 *
 *  @sa
 */
int test_flushfailuretest(int size)
{
	void	*buf_ptr;
	int	err;

	buf_ptr = (void *)malloc(size);
	err = ProcMgr_flushMemory(buf_ptr, size, PROC_SYSM3);
	if (err < 0) {
		Osal_printf("Flush memory failed for buffer 0x%x\n",
						(uint32_t)buf_ptr);
	} else {
		Osal_printf("Flush memory success for buffer 0x%x\n",
						(uint32_t)buf_ptr);
	}

	err = ProcMgr_invalidateMemory(buf_ptr, size, PROC_SYSM3);
	if (err < 0) {
		Osal_printf("Invalidate memory failed for buffer 0x%x\n",
						(uint32_t)buf_ptr);
	} else {
		Osal_printf("Invalidate memory success for buffer 0x%x\n",
						(uint32_t)buf_ptr);
	}

	free(buf_ptr);
	return 0;
}

/*!
 *  @brief  Function to stress test Mapping and Unmapping of DMM
 *          buffers.
 *
 *  @param  size             Size of the buffer to Map
 *  @param  num_of_buffers   Number of buffers to Map
 *  @param  unmap            Specify unmap as true if unmap needs
 *                           to be called in this test case. if unmap
 *                           is set to false, then it is used to validate
 *                           resource cleanup handling the unmapping part.
 *  @sa
 */
int test_mapbuffertest(int size, int num_of_buffers, bool unmap)
{
	int				status = 0;
	int				*buffers;
	int				i = 0;
	void				*buf_ptr;
	SyslinkMemUtils_MpuAddrToMap	mpuAddrList[1];
	uint32_t			mappedAddr;
	ProcMgr_MapType			mapType;

	Osal_printf("Running map buffer test\n");

	buffers = malloc(sizeof(int) * num_of_buffers);
	if (!buffers)
		return -1;
	memset(buffers, 0x0, sizeof(int) * num_of_buffers);

	while (i < num_of_buffers) {
		buf_ptr = (void *)malloc(size);
		if (buf_ptr == NULL) {
			Osal_printf("Error: malloc returned null.\n");
			free(buffers);
			return -1;
		} else {
			Osal_printf("malloc returned 0x%x.\n",
						(uint32_t)buf_ptr);
		}
		mapType = ProcMgr_MapType_Virt;

		mpuAddrList[0].mpuAddr = (uint32_t)buf_ptr;
		mpuAddrList[0].size = size;
		status = SysLinkMemUtils_map(mpuAddrList, 1, &mappedAddr,
						mapType, PROC_SYSM3);
		Osal_printf("MPU Address = 0x%x     Mapped Address = 0x%x,"
				"size = 0x%x\n", mpuAddrList[0].mpuAddr,
				mappedAddr, mpuAddrList[0].size);
		Osal_printf("Flushing mpuAddr 0x%x of size 0x%x\n",
				mpuAddrList[0].mpuAddr, mpuAddrList[0].size);
		ProcMgr_flushMemory(buf_ptr, size, PROC_SYSM3);
		buffers[i] = mappedAddr;
		i++;
	}

	if (unmap == false)
		goto exit;

	/* unmap the buffers */
	i = 0;
	while (i < num_of_buffers) {
		Osal_printf("Unmapping 0x%x\n", buffers[i]);
		SysLinkMemUtils_unmap(buffers[i], PROC_SYSM3);
		i++;
	}
exit:
	free(buffers);
	return 0;
}


/*!
 *  @brief  Function to validate use buffer functionality used with Tiler
 *
 *  @param  size             Size of the buffer to Map
 *  @param  iterations       Number of iterations to run the test
 *  @sa
 */
int test_usebuffer(int size, int iterations)
{
	uint32_t			*buf_ptr;
	int				map_size;
	SyslinkMemUtils_MpuAddrToMap	mpuAddrList[1];
	uint32_t			mappedAddr;
	int				status;
	int				i;
	RcmClient_Message		*rcmMsg = NULL;
	int				rcmMsgSize;
	RCM_Remote_FxnArgs		*fxnArgs;
	int				count = 0;
	void				*tilVaPtr;
	MemAllocBlock			block;
	int				tiler_buf_length;
	uint32_t			*dataPtr;
	uint32_t			dummyMappedAddr;

	Osal_printf("Running Tiler use buffer test case\n");
	/* allocate a remote command message */
	Osal_printf("Allocating RCM message\n");
	rcmMsgSize = sizeof(RCM_Remote_FxnArgs);
	status = RcmClient_alloc(rcm_client_handle, rcmMsgSize, &rcmMsg);
	if (status < 0) {
		Osal_printf("Error allocating RCM message\n");
		goto exit;
	}

	tiler_buf_length = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
	map_size = tiler_buf_length + PAGE_SIZE - 1;

	while (iterations-- > 0) {
		buf_ptr = (void *)malloc(map_size);
		dataPtr = (void *)(((uint32_t)buf_ptr + PAGE_SIZE - 1)
							& ~(PAGE_SIZE - 1));

		Osal_printf("Calling malloc of size 0x%x.Iternation %d\n",
							map_size, iterations);
		if (buf_ptr == NULL) {
			Osal_printf("Error: malloc returned null.\n");
			goto exit_nomem;
		} else {
			Osal_printf("malloc returned 0x%x.Passed to Tlier"
				"0x%x\n", (uint32_t)buf_ptr, (uint32_t)dataPtr);
		}

		 /* allocate aligned buffer */
		memset(&block, 0, sizeof(block));
		block.pixelFormat = 4;
		block.dim.len = tiler_buf_length;
		block.stride = 0;
		block.ptr = dataPtr;
		tilVaPtr = MemMgr_Map(&block, 1);
		Osal_printf("Tiler VA Address = 0x%x, tiler_buf_length"
			"= 0x%x\n", (uint32_t)tilVaPtr, tiler_buf_length);
		if (tilVaPtr == NULL)
			goto exit_nomem;

		mpuAddrList[0].mpuAddr = (uint32_t)tilVaPtr;
		mpuAddrList[0].size = tiler_buf_length;
		status = SysLinkMemUtils_map(mpuAddrList, 1, &mappedAddr,
					ProcMgr_MapType_Tiler, PROC_SYSM3);
		Osal_printf("MPU Address = 0x%x     Mapped Address = 0x%x,"
				"size = 0x%x\n", mpuAddrList[0].mpuAddr,
				mappedAddr, mpuAddrList[0].size);


		/*
		* Doing a dummy mapping to allow iommu flush this buffer
		*/
		mpuAddrList[0].mpuAddr = (uint32_t)buf_ptr;
		mpuAddrList[0].size = map_size;
		status = SysLinkMemUtils_map(mpuAddrList, 1, &dummyMappedAddr,
					ProcMgr_MapType_Virt, PROC_SYSM3);
		/* Do actual test here */
		for (i = 0; i < tiler_buf_length/sizeof(uint32_t); i++) {
			dataPtr[i] = 0;
			dataPtr[i] = (0xbeef0000 | i);

			if (dataPtr[i] != (0xbeef0000 | i)) {
				Osal_printf("Readback failed at address 0x%x\n",
								&buf_ptr[i]);
				Osal_printf("\tExpected: [0x%x]\tActual:"
				"[0x%x]\n", (0xbeef0000 | i), buf_ptr[i]);
			}
		}
		ProcMgr_flushMemory(dataPtr, tiler_buf_length, PROC_SYSM3);

		/* fill in the remote command message */
		rcmMsg->fxnIdx = fxn_buffer_test_idx;
		fxnArgs = (RCM_Remote_FxnArgs *)(&rcmMsg->data);
		fxnArgs->num_bytes = tiler_buf_length;
		fxnArgs->buf_ptr   = (Ptr)mappedAddr;

		status = RcmClient_exec(rcm_client_handle, rcmMsg, &return_msg);
		if (status < 0) {
			Osal_printf(" RcmClient_execerror.\n");
		} else {
			/* Check the buffer data */
			Osal_printf("Testing data\n");
			count = 0;
			ProcMgr_invalidateMemory(dataPtr, tiler_buf_length,
								PROC_SYSM3);

			for (i = 0; i < tiler_buf_length/sizeof(uint32_t) &&
						count < maxCount; i++) {
				if (dataPtr[i] != ~(0xbeef0000 | i)) {
					printf("ERROR: Data mismatch at offset"
						"0x%x\n", i * sizeof(uint32_t));
					printf("\tExpected: [0x%x]\tActual:"
						"[0x%x]\n", ~(0xbeef0000 | i),
					dataPtr[i]);
					count++;
				}
			}
			if (count == 0)
				Osal_printf("Test passed!\n");
		}

		/* Set the memory to some other value to avoid a
		 * potential future false positive
		 */
		for (i = 0; i < tiler_buf_length/sizeof(uint32_t); i++)
			buf_ptr[i] = 0xdeadbeef;

		SysLinkMemUtils_unmap(mappedAddr, PROC_SYSM3);
		SysLinkMemUtils_unmap(dummyMappedAddr, PROC_SYSM3);
		MemMgr_UnMap(tilVaPtr);
		free(buf_ptr);
	}
exit_nomem:
	/* return message to the heap */
	Osal_printf("Calling RcmClient_free\n");
	RcmClient_free(rcm_client_handle, return_msg);
exit:
	return 0;
}

/*!
 *  @brief Function to validate mapping io buffer functionality used with Tiler
 *
 *  @param  size             Size of the buffer to Map
 *  @param  iterations       Number of iterations to run the test
 *  @sa
 */
int test_iobuffertest(int size, int iterations)
{
	SyslinkMemUtils_MpuAddrToMap	mpuAddrList[1];
	uint32_t			mappedAddr;
	int				status;
	int				i;
	RcmClient_Message		*rcmMsg = NULL;
	int				rcmMsgSize;
	RCM_Remote_FxnArgs		*fxnArgs;
	int				count = 0;
	uint32_t			ssptr;
	uint32_t			*map_base;
	int				fd;

	Osal_printf("Running IO Map use buffer test case\n");
	Osal_printf("WARNING WARNING WARNING !\n");
	Osal_printf("This Test case is currently using Tiler as IO Address\n"
			"for dynamic mapping and unmapping. Restart the\n "
			"Syslink daemon if you want to run any Tiler related"
			"Tests after this test!\n");
	/* allocate a remote command message */
	Osal_printf("Allocating RCM message\n");
	rcmMsgSize = sizeof(RCM_Remote_FxnArgs);
	status = RcmClient_alloc(rcm_client_handle, rcmMsgSize, &rcmMsg);
	if (status < 0) {
		Osal_printf("Error allocating RCM message\n");
		goto exit;
	}

	while (iterations-- > 0) {

		TilerMgr_Open();
		Osal_printf("Calling tilerAlloc.\n");
		ssptr = TilerMgr_Alloc(PIXEL_FMT_8BIT, size, 1);
		if (ssptr == -1) {
			Osal_printf("Error: tilerAlloc returned null.\n");
			status = -1;
			goto exit_nomem;
		}

		fd = open("/dev/mem", O_RDWR|O_SYNC);
		if (fd) {
			map_base = mmap(0, size, PROT_READ | PROT_WRITE,
						MAP_SHARED, fd, ssptr);
			if (map_base == (void *) -1) {
				Osal_printf("Failed to do memory mapping\n");
				close(fd);
				goto exit_mmap_fail;
			}
		} else {
			printf("FAILED opening /dev/mem...continue\n");
			continue;
		}

		for (i = 0; i < size/sizeof(uint32_t); i++) {
			map_base[i] = 0;
			map_base[i] = (0xbeef0000 | i);

			if (map_base[i] != (0xbeef0000 | i)) {
				Osal_printf("Readback failed at address 0x%x\n",
								&map_base[i]);
				Osal_printf("\tExpected: [0x%x]\tActual:"
				"[0x%x]\n", (0xbeef0000 | i), map_base[i]);
			}
		}

		mpuAddrList[0].mpuAddr = (uint32_t)map_base;
		mpuAddrList[0].size = size;
		status = SysLinkMemUtils_map(mpuAddrList, 1, &mappedAddr,
					ProcMgr_MapType_Virt, PROC_SYSM3);
		Osal_printf("MPU Address = 0x%x     Mapped Address = 0x%x,"
					"size = 0x%x\n", mpuAddrList[0].mpuAddr,
					mappedAddr, mpuAddrList[0].size);

		/* fill in the remote command message */
		rcmMsg->fxnIdx = fxn_buffer_test_idx;
		fxnArgs = (RCM_Remote_FxnArgs *)(&rcmMsg->data);
		fxnArgs->num_bytes = size;
		fxnArgs->buf_ptr   = (void *)mappedAddr;

		status = RcmClient_exec(rcm_client_handle, rcmMsg, &return_msg);
		if (status < 0) {
			Osal_printf(" RcmClient_execerror.\n");
		} else {
			/* Check the buffer data */
			Osal_printf("Testing data\n");
			count = 0;
			for (i = 0; i < size/sizeof(uint32_t) &&
						count < maxCount; i++) {
				if (map_base[i] != ~(0xbeef0000 | i)) {
					printf("ERROR: Data mismatch at offset"
						"0x%x\n", i * sizeof(uint32_t));
					printf("\tExpected: [0x%x]\tActual:"
						"[0x%x]\n", ~(0xbeef0000 | i),
					map_base[i]);
					count++;
				}
			}
			if (count == 0)
				Osal_printf("Test passed!\n");
		}

		/* Set the memory to some other value to avoid a
		 * potential future false positive
		 */
		for (i = 0; i < size/sizeof(uint32_t); i++)
			map_base[i] = 0xdeadbeef;

		SysLinkMemUtils_unmap(mappedAddr, PROC_SYSM3);
		munmap(map_base, size);
exit_mmap_fail:
		close(fd);
		TilerMgr_Free(ssptr);
		TilerMgr_Close();
	}
exit_nomem:
	/* return message to the heap */
	Osal_printf("Calling RcmClient_free\n");
	RcmClient_free(rcm_client_handle, return_msg);
exit:
	return 0;
}

/*!
 *  @brief  Function to validate DMM  buffer functionality.
 *
 *  @param  size             Size of the buffer to Map
 *  @param  iterations       Number of iterations to run the test
 *  @sa
 */
int test_dmmbuffer(int size, int iterations)
{
	uint32_t *buf_ptr;
	int map_size = size;
	ProcMgr_MapType mapType;
	SyslinkMemUtils_MpuAddrToMap    mpuAddrList[1];
	uint32_t mappedAddr;
	int status;
	int i;
	RcmClient_Message *rcmMsg = NULL;
	int rcmMsgSize;
	RCM_Remote_FxnArgs *fxnArgs;
	int count = 0;

	/* allocate a remote command message */
	Osal_printf("Allocating RCM message\n");
	rcmMsgSize = sizeof(RCM_Remote_FxnArgs);
	status = RcmClient_alloc(rcm_client_handle, rcmMsgSize, &rcmMsg);
	if (status < 0) {
		Osal_printf("Error allocating RCM message\n");
		goto exit;
	}

	while (iterations-- > 0) {
		Osal_printf("Calling malloc.Iternation %d\n", iterations);

		buf_ptr = (void *)malloc(map_size);
		if (buf_ptr == NULL) {
			Osal_printf("Error: malloc returned null.\n");
			goto exit_nomem;
		} else {
			Osal_printf("malloc returned 0x%x.\n",
						(uint32_t)buf_ptr);
		}

		mapType = ProcMgr_MapType_Virt;
		mpuAddrList[0].mpuAddr = (uint32_t)buf_ptr;
		mpuAddrList[0].size = map_size;
		status = SysLinkMemUtils_map(mpuAddrList, 1, &mappedAddr,
							mapType, PROC_SYSM3);
		Osal_printf("MPU Address = 0x%x     Mapped Address = 0x%x,"
					"size = 0x%x\n", mpuAddrList[0].mpuAddr,
					mappedAddr, mpuAddrList[0].size);

		/* Do actual test here */
		for (i = 0; i < map_size/sizeof(uint32_t); i++) {
			buf_ptr[i] = 0;
			buf_ptr[i] = (0xbeef0000 | i);

			if (buf_ptr[i] != (0xbeef0000 | i)) {
				Osal_printf("Readback failed at address 0x%x\n",
								&buf_ptr[i]);
				Osal_printf("\tExpected: [0x%x]\tActual:"
				"[0x%x]\n", (0xbeef0000 | i), buf_ptr[i]);
			}
		}
		ProcMgr_flushMemory(buf_ptr, map_size, PROC_SYSM3);

		/* fill in the remote command message */
		rcmMsg->fxnIdx = fxn_buffer_test_idx;
		fxnArgs = (RCM_Remote_FxnArgs *)(&rcmMsg->data);
		fxnArgs->num_bytes = map_size;
		fxnArgs->buf_ptr   = (Ptr)mappedAddr;

		status = RcmClient_exec(rcm_client_handle, rcmMsg, &return_msg);
		if (status < 0) {
			Osal_printf(" RcmClient_execerror.\n");
		} else {
			/* Check the buffer data */
			Osal_printf("Testing data\n");
			count = 0;
			ProcMgr_invalidateMemory(buf_ptr, map_size, PROC_SYSM3);

			for (i = 0; i < map_size/sizeof(uint32_t) &&
						count < maxCount; i++) {
				if (buf_ptr[i] != ~(0xbeef0000 | i)) {
					printf("ERROR: Data mismatch at offset"
						"0x%x\n", i * sizeof(uint32_t));
					printf("\tExpected: [0x%x]\tActual:"
						"[0x%x]\n", ~(0xbeef0000 | i),
						buf_ptr[i]);
					count++;
				}
			}
			if (count == 0)
				Osal_printf("Test passed!\n");
		}

		/* Set the memory to some other value to avoid a
		 * potential future false positive
		 */
		for (i = 0; i < map_size/sizeof(uint32_t); i++)
			buf_ptr[i] = 0xdeadbeef;

		SysLinkMemUtils_unmap(mappedAddr, PROC_SYSM3);
		free(buf_ptr);
	}
exit_nomem:
	/* return message to the heap */
	Osal_printf("Calling RcmClient_free\n");
	RcmClient_free(rcm_client_handle, return_msg);
exit:
	return 0;
}

int ipc_shutdown(int proc_id)
{
	int status = 0;

	/* delete the rcm client */
	Osal_printf("Delete RCM client instance\n");
	status = RcmClient_delete(&rcm_client_handle);
	if (status < 0)
		Osal_printf("Error in RCM Client instance delete\n");

	/* rcm client module destroy*/
	Osal_printf("Destroy RCM client module\n");
	RcmClient_exit();

	if (proc_id == PROC_APPM3) {
		status =  ProcMgr_detach (proc_mgr_handle1);
		Osal_printf ("DMMTEST CASE: ProcMgr_detach status [0x%x]\n",
								status);
		status = ProcMgr_close (&proc_mgr_handle1);
		if (status < 0) {
		    Osal_printf ("DMMTEST: Error in ProcMgr_close [0x%x]\n",
		                    status);
		} else {
		    Osal_printf ("DMMTEST: ProcMgr_close status: [0x%x]\n",
		                    status);
		}
		ProcMgr_detach (proc_mgr_handle);
		Osal_printf ("DMMTEST: ProcMgr_detach status [0x%x]\n", status);

		status = ProcMgr_close (&proc_mgr_handle);
	}

	if (proc_id == PROC_SYSM3) {
		status =  ProcMgr_detach (proc_mgr_handle);
		Osal_printf ("DMMTEST CASE: ProcMgr_detach status [0x%x]\n",
								status);
		status = ProcMgr_close (&proc_mgr_handle);
		if (status < 0) {
		    Osal_printf ("DMMTEST: Error in ProcMgr_close [0x%x]\n",
		                    status);
		} else {
		    Osal_printf ("DMMTEST: ProcMgr_close status: [0x%x]\n",
		                    status);
		}
	}

	status = Ipc_destroy ();

	return status;
}

/*
 *  ======== main ========
 */
int main(int argc, char *argv[])
{
	int status = 0;
	int testNo;
	int subTestNo;
	int proc_id;
	Bool validArgs = TRUE;

	Osal_printf("\n== Syslink Mem Utils Sample ==\n");

	if (argc < 2) {
		validArgs = FALSE;
		goto exit;
	}

	testNo = atoi(argv[1]);
	if (testNo < 1 || testNo > DMM_TESTS_NUM) {
		validArgs = FALSE;
		goto exit;
	}

	if (argc > 2) {
		subTestNo = atoi(argv[2]);

		/* Determine proc ID based on subtest number */
		if (subTestNo == 1)
			proc_id = PROC_SYSM3;
		else if (subTestNo == 2)
			proc_id = PROC_APPM3;
		else if (subTestNo == 3)
			proc_id = PROC_TESLA;
		else {
			validArgs = FALSE;
			goto exit;
		}
	} else {
		validArgs = FALSE;
		goto exit;
	}

	status = ipcSetup(proc_id);
	if (status != RcmClient_S_SUCCESS) {
		Osal_printf("IPC SETUP FAILED\n");
		goto exit;
	}

	Osal_printf("Running Test case %d\n", testNo);
	switch (testNo) {
	case DMM_BUFFER:
		test_dmmbuffer(atoi(argv[3]), atoi(argv[4]));
		break;
	case USE_BUFFER:
		test_usebuffer(atoi(argv[3]), atoi(argv[4]));
		break;
	case MAP_BUFFER:
		test_mapbuffertest(atoi(argv[3]), atoi(argv[4]), 1);
		break;
	case FLUSH_BUFFER_NEG:
		test_flushfailuretest(atoi(argv[3]));
		break;
	case MAP_NO_UNMAP:
		test_mapbuffertest(atoi(argv[3]), atoi(argv[4]), 0);
		break;
	case MAP_IO_BUFFER:
		test_iobuffertest(atoi(argv[3]), atoi(argv[4]));
		break;
	default:
		printf("Invalid Test case number\n");
	}

	ipc_shutdown(proc_id);

exit:
	if (!validArgs) {
		Osal_printf("Test 1 - DMM buffer\n"
				"\tUsage: ./dmm_daemontest.out 1 <Proc#> "
				"<Buffer Size> <NumIterations>:\n");
		Osal_printf("Test 2 - Tiler use buffer\n"
				"\tUsage: ./dmm_daemontest.out 2 <Proc#> "
				"<Buffer Size> <NumIterations>:\n");
		Osal_printf("Test 3 - Map Test:\n"
				"\tUsage: ./dmm_daemontest.out 3 <Proc#> "
				 "<Buffer Size> <NumIterations>:\n");
		Osal_printf("Test 4 - Flush memory Negetive Test:\n"
				"\tUsage: ./dmm_daemontest.out 4 <Proc#> "
				"<Buffer Size>\n");
		Osal_printf("Test 5 - Map with no Unmap Test:\n"
				"\tUsage: ./dmm_daemontest.out 5 <Proc#> "
				"<Buffer Size> <num_of_buffers>\n");
		Osal_printf("Test 6 - Map IO buffer Test:\n"
				"\tUsage: ./dmm_daemontest.out 6 <Proc#> "
				"<Buffer Size> <num_of_buffers>\n");
	}
	return status;
}
