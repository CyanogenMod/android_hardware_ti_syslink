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
 *  @file   mmufault_listener.c
 *
 *  @brief  Demonstrate on how to register to mmu fault notification
 *
 *  ============================================================================
 */


/* OS-specific headers */
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>

/* OSAL & Utils headers */
#include <OsalPrint.h>

/* RCM headers */
#include <ProcMgr.h>

#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */

static pthread_t                mmu_fault_handle;
static sem_t                    sem_fault_wait;

static void mmu_fault_handler(void)
{
	int status;

	status = ProcMgr_waitForEvent(PROC_SYSM3, PROC_MMU_FAULT, -1);

	Osal_printf ("Received MMU fault notification from Ducati!!! "
			"status = 0x%x\n", status);

	/* Initiate cleanup */
	sem_post(&sem_fault_wait);
}

/*
 *  ======== main ========
 */
int main (int argc, char * argv [])
{
	/* Create the MMU fault handler thread */
	Osal_printf ("Create MMU fault handler thread.\n");
	sem_init(&sem_fault_wait, 0, 0);
	pthread_create (&mmu_fault_handle, NULL,
	                (void *)&mmu_fault_handler, NULL);

	sem_wait(&sem_fault_wait);

	sem_destroy(&sem_fault_wait);

	Osal_printf ("Exiting fault handler application application.\n");

	return 0;
}

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */
