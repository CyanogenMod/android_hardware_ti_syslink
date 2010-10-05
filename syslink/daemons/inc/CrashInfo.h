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
 *  @file   CrashInfo.h
 *
 *  @brief  Crash Dump utility structures file
 *  ============================================================================
 */

#include <stdlib.h>

/* ThreadType */
typedef enum {
    BIOS_ThreadType_Hwi,
    BIOS_ThreadType_Swi,
    BIOS_ThreadType_Task,
    BIOS_ThreadType_Main
} BIOS_ThreadType;

/*!
 *  Exception Context - Register contents at the time of an exception.
 */
struct ExcContext {
    /* Thread Context */
    BIOS_ThreadType threadType; /* Type of thread executing at */
                                /* the time the exception occurred */
    Ptr     threadHandle;       /* Handle to thread executing at */
                                /* the time the exception occurred */
    Ptr     threadStack;        /* Address of stack contents of thread */
                                /* executing at the time the exception */
                                /* occurred */
    size_t   threadStackSize;    /* size of thread stack */

    /* Internal Registers */
    Ptr     r0;
    Ptr     r1;
    Ptr     r2;
    Ptr     r3;
    Ptr     r4;
    Ptr     r5;
    Ptr     r6;
    Ptr     r7;
    Ptr     r8;
    Ptr     r9;
    Ptr     r10;
    Ptr     r11;
    Ptr     r12;
    Ptr     sp;
    Ptr     lr;
    Ptr     pc;
    Ptr     psr;

    /* NVIC registers */
    Ptr     ICSR;
    Ptr     MMFSR;
    Ptr     BFSR;
    Ptr     UFSR;
    Ptr     HFSR;
    Ptr     DFSR;
    Ptr     MMAR;
    Ptr     BFAR;
    Ptr     AFSR;
};
