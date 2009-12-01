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
 *  @file   TilerTestApp.c
 *
 *  @brief  OS-specific sample application framework for RCM module
 *
 *  ============================================================================
 */


/* OS-specific headers */
#include <stdio.h>
#include <stdlib.h>

/* OSAL & Utils headers */
#include <OsalPrint.h>

/* RCM headers */
#include <RcmClient.h>
#include <RcmServer.h>

/* Sample headers */
#include <TilerSyslinkApp.h>

#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */

static const char SYSLINK_TUX_IMAGE[] =
"		               MMMMMMMMM                \n"
"		              MMMMMMMMMMM               \n"
"		             MMMMMMMMMMMMM              \n"
"		             MMMMMMMMMMMMMM             \n"
"		             M$  MMM   MMMM             \n"
"		             M M.MM MM. MMM             \n"
"		             M MN+++MM.IMMM             \n"
"		             MM++++++++MMMM.            \n"
"		             MM++++++=+MMMMM            \n"
"		             M. $++++.  MMMM            \n"
"		            NM.          MMMM.          \n"
"		           MM.           MMMMM.         \n"
"		          MMM             MMMMM7        \n"
"		         MMM              MMMMMMM       \n"
"		        ,MM.               MM.MMM.      \n"
"		        MMM                 MMMMMM      \n"
"		       MMM                  MMMMMM      \n"
"		      .MMM                  MMMMMMM     \n"
"		      MMM   S Y S L I N K   MMMMMMM     \n"
"		      MMM                   MM?MMMM     \n"
"		     .+++                 .+,MMMMMM     \n"
"		    .+++++.               ~+=MMMM++.    \n"
"		.++++++++++.              ,++++++++:    \n"
"		.+++++++++++              +++++++++++   \n"
"		.++++++++++++           .M+++++++++++++.\n"
"		 ++++++++++++=.       .MMM++++++++++++. \n"
"		.++++++++++++++MMMMMMMMMMM+++++++++.    \n"
"		  ..+++++++++++MMMMMMMMMMM+++++++.      \n"
"		        .+++++.          .+++++.        \n";

/*
 *  ======== main ========
 */
Int main (Int argc, Char * argv [])
{
    Int status = 0;
    Int testNo;
    Int subTestNo;
    Int procId;
    UInt numTrials;

    Osal_printf ("\n== Syslink Mem Utils Sample ==\n");

    if (argc < 2)
    {
        Osal_printf ("Usage: ./syslink_tilertest.out <Test#> <Sub Test#>\n"
                        "Options:\n");
        Osal_printf ("\t./syslink_tilertest.out 1: "
                        "\n\t\tSyslink Virt to Phys.\n");
        Osal_printf ("\t./syslink_tilertest.out 2: "
                        "\n\t\tSyslink Virt to Phys.Pages\n");
          Osal_printf ("\t./syslink_tilertest.out 3 1 [# trials]: "
                        "\n\t\tSyslink Use Tiler Buffer on SysM3\n");
          Osal_printf ("\t./syslink_tilertest.out 3 2 [# trials]: "
                        "\n\t\tSyslink Use Tiler Buffer on AppM3\n");
          Osal_printf ("\t./syslink_tilertest.out 4 1 [# trials]: "
                        "\n\t\tSyslink Use Malloc Buffer on SysM3\n");
          Osal_printf ("\t./syslink_tilertest.out 4 2 [# trials]: "
                        "\n\t\tSyslink Use Malloc Buffer on AppM3\n");
          Osal_printf ("\t./syslink_tilertest.out 5 [# trials]: "
                        "\n\t\tSyslink Map/UnMap test\n");
          Osal_printf ("\t[# trials] is optional, defaults to 1\n");
        goto exit;
    }

    testNo = atoi (argv[1]);

    switch(testNo) {
    case 3:
    case 4:
        if(argc > 2)
        {
            subTestNo = atoi (argv[2]);

            /* Determine proc ID based on subtest number */
            if(subTestNo == 1)
                procId = 2;   // SysM3
            if(subTestNo == 2)
                procId = 3;   // AppM3
        }

        if(argc > 3)
            numTrials = atoi (argv[3]);
        else
            numTrials = 1;
        break;

    case 5:
        if(argc > 2)
            numTrials = atoi (argv[2]);
        else
            numTrials = 1;
        break;
    }

    /* Run SyslinkVirtToPhysTest test */
    if(testNo == 1) {
        Osal_printf ("SyslinkVirtToPhysTest invoked.\n");
        status = SyslinkVirtToPhysTest();
        if (status < 0) {
            Osal_printf ("Error in SyslinkVirtToPhysTest test \n");
        }
    }

    /* Run SyslinkVirtToPhysPagesTesttest */
    if(testNo == 2) {
        Osal_printf ("SyslinkVirtToPhysPagesTest test invoked.\n");
        status = SyslinkVirtToPhysPagesTest ();
        if (status < 0)
            Osal_printf ("Error in SyslinkVirtToPhysPagesTest test \n");
    }

    if(testNo == 3) {
        Osal_printf ("SyslinkUseBufferTest with TILER invoked.\n");
        status = SyslinkUseBufferTest (procId, TRUE, numTrials);
        if (status < 0)
            Osal_printf ("Error in SyslinkUseBufferTest test \n");
    }

    if(testNo == 4) {
        Osal_printf ("SyslinkUseBufferTest with Malloc invoked.\n");
        status = SyslinkUseBufferTest (procId, FALSE, numTrials);
        if (status < 0)
            Osal_printf ("Error in SyslinkUseBufferTest test \n");
    }

    if(testNo == 5) {
        Osal_printf ("SyslinkMapUnMapTest with Malloc invoked.\n");
        status = SyslinkMapUnMapTest (numTrials);
        if (status < 0)
            Osal_printf ("Error in SyslinkMapUnMapTest test \n");
    }

    if(status < 0)
        Osal_printf("Exiting with status 0x%x\n", status);
exit:
    Osal_printf ("\n== Sample End ==\n");

    if(testNo == 3 && subTestNo == 2)
        Osal_printf("%s\n", SYSLINK_TUX_IMAGE);

    return status;
}

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


