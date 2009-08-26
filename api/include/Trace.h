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
 *  @file   Trace.h
 *
 *  @brief      Kernel Trace enabling/disabling/application interface.
 *
 *              This will have the definitions for kernel side traces
 *              statements and also details of variable traces
 *              supported in existing implementation.
 *  ============================================================================
 */

#ifndef OSALTRACE_H_0xDA50
#define OSALTRACE_H_0xDA50

/* Standard headers */
#include <Std.h>

/* OSAL and utils headers */
#include <OsalPrint.h>


#if defined (__cplusplus)
extern "C" {
#endif


/*!
 *  @def    OSALTRACE_MODULEID
 *  @brief  Module ID for OsalTrace OSAL module.
 */
#define OSALTRACE_MODULEID                 (UInt16) 0xDA50


/*!
 *  @def    GT_TRACESTATE_MASK
 *  @brief  Trace state mask
 */
#define GT_TRACESTATE_MASK                 0x0000000F

/*!
 *  @def    GT_TRACESTATE_SHIFT
 *  @brief  Bit shift for trace state
 */
#define GT_TRACESTATE_SHIFT                0u

/*!
 *  @def    GT_TRACEENTER_MASK
 *  @brief  Trace enter mask
 */
#define GT_TRACEENTER_MASK                 0x000000F0

/*!
 *  @def    GT_TRACEENTER_SHIFT
 *  @brief  Bit shift for trace enter
 */
#define GT_TRACEENTER_SHIFT                4u

/*!
 *  @def    GT_TRACESETFAILURE_MASK
 *  @brief  Trace Set Failure Reason mask
 */
#define GT_TRACESETFAILURE_MASK            0x00000F00

/*!
 *  @def    GT_TRACESETFAILURE_SHIFT
 *  @brief  Bit shift for trace Set Failure Reason
 */
#define GT_TRACESETFAILURE_SHIFT           8u

/*!
 *  @def    GT_TRACECLASS_MASK
 *  @brief  GT class mask
 */
#define GT_TRACECLASS_MASK                 0x000F0000

/*!
 *  @def    GT_TRACECLASS_SHIFT
 *  @brief  Bit shift for GT class mask
 */
#define GT_TRACECLASS_SHIFT                16u

/*!
 *  @brief   Enumerates the types of states of trace (enable/disable)
 */
typedef enum {
    GT_TraceState_Disable       = 0x00000000,
    /*!< Disable trace */
    GT_TraceState_Enable        = 0x00000001,
    /*!< Enable trace */
    GT_TraceState_EndValue      = 0x00000002
    /*!< End delimiter indicating start of invalid values for this enum */
} GT_TraceState;

/*!
 *  @brief   Enumerates the states of enter/leave trace (enable/disable)
 */
typedef enum {
    GT_TraceEnter_Disable       = 0x00000000,
    /*!< Disable GT_ENTER trace prints */
    GT_TraceEnter_Enable        = 0x00000010,
    /*!< Enable GT_ENTER trace prints */
    GT_TraceEnter_EndValue      = 0x00000020
    /*!< End delimiter indicating start of invalid values for this enum */
} GT_TraceEnter;

/*!
 *  @brief   Enumerates the states of SetFailureReason trace (enable/disable)
 */
typedef enum {
    GT_TraceSetFailure_Disable       = 0x00000000,
    /*!< Disable Set Failure trace prints */
    GT_TraceSetFailure_Enable        = 0x00000100,
    /*!< Enable Set Failure trace prints */
    GT_TraceSetFailure_EndValue      = 0x00000200
    /*!< End delimiter indicating start of invalid values for this enum */
} GT_TraceSetFailure;

/*!
 *  @brief   Enumerates the types of trace classes
 */
typedef enum {
    GT_1CLASS                   = 0x00010000,
    /*!< Class 1 trace: Used for block level information */
    GT_2CLASS                   = 0x00020000,
    /*!< Class 2 trace: Used for critical information */
    GT_3CLASS                   = 0x00030000,
    /*!< Class 3 trace: Used for additional information */
    GT_4CLASS                   = 0x00040000,
    /*!< Class 4 trace: Used for errors/warnings */
    GT_ENTER                    = 0x00050000,
    /*!< Indicates a function entry class of trace */
    GT_LEAVE                    = 0x00060000
    /*!< Indicates a function leave class of trace */
} GT_TraceClass;

//#define SYSLINK_TRACE_ENABLE

#if defined(SYSLINK_BUILD_DEBUG)
/*!
 *  @brief   Prints assertion information when the specified condition is not
 *           met.
 */
#define GT_assert(x, y)                                                 \
do {                                                                    \
    if (!(y)) {                                                         \
        Osal_printf ("Assertion at Line no: %d in %s: %s : failed\n",   \
                     __LINE__, __FILE__, #y);                           \
    }                                                                   \
} while (0)
#else
#define GT_assert(x, y)
#endif /* if defined(SYSLINK_BUILD_DEBUG) */


#if defined (SYSLINK_TRACE_ENABLE)
/* The global trace variable containing current trace configuration. */
extern Int curTrace;

/* Function to report the syslink failure and log the trace. */
Void _GT_setFailureReason (Int enableMask,
                           Char * func,
                           Char * fileName,
                           UInt32 lineNo,
                           UInt32 status,
                           Char * msg);
#define GT_setFailureReason(mask, classId, func, status, msg)           \
       _GT_setFailureReason(mask, func,                                 \
                            __FILE__, __LINE__, status, (Char*) (msg"\n"))


/* Log the trace with zero parameters and information string. */
Void _GT_0trace (UInt32 maskType, GT_TraceClass classtype, Char* infoString);
#define GT_0trace(mask, classId, format) \
do {                                                            \
    if (classId == GT_ENTER) {                                  \
       _GT_0trace(mask, classId,                                \
                  "Entered "format"\n");                        \
    }                                                           \
    else if (classId == GT_LEAVE) {                             \
       _GT_0trace(mask, classId,                                \
                  "Leaving "format"\n");                        \
    }                                                           \
    else {                                                      \
       _GT_0trace(mask, classId,                                \
                  format"\n");                                  \
    }                                                           \
} while (0)


/* Function to log the trace with one additional parameter */
Void _GT_1trace (UInt32         maskType,
                 GT_TraceClass  classtype,
                 Char *         infoString,
                 UInt32         param);
#define GT_1trace(mask, classId, format, a)                     \
do {                                                            \
    if (classId == GT_ENTER) {                                  \
       _GT_1trace(mask, classId,                                \
                  "Entered "format"\n\t"#a"\t[0x%x]\n",         \
                  (UInt32) (a));                                \
    }                                                           \
    else if (classId == GT_LEAVE) {                             \
       _GT_1trace(mask, classId,                                \
                  "Leaving "format"\n\t"#a"\t[0x%x]\n",         \
                  (UInt32) (a));                                \
    }                                                           \
    else {                                                      \
       _GT_1trace(mask, classId,                                \
                  format"\n",                                   \
                  (UInt32) (a));                                \
    }                                                           \
} while (0)


/* Function to log the trace with two additional parameters */
Void _GT_2trace (UInt32         maskType,
                 GT_TraceClass  classtype,
                 Char *         infoString,
                 UInt32         param0,
                 UInt32         param1);
#define GT_2trace(mask, classId, format, a, b)                  \
do {                                                            \
    if (classId == GT_ENTER) {                                  \
       _GT_2trace(mask, classId,                                \
                  "Entered "format"\n\t"#a"\t[0x%x]\n"          \
                  "\t"#b"\t[0x%x]\n",                           \
                  (UInt32) (a),                                 \
                  (UInt32) (b));                                \
    }                                                           \
    else if (classId == GT_LEAVE) {                             \
       _GT_2trace(mask, classId,                                \
                  "Leaving "format"\n\t"#a"\t[0x%x]\n"          \
                  "\t"#b"\t[0x%x]\n",                           \
                  (UInt32) (a),                                 \
                  (UInt32) (b));                                \
    }                                                           \
    else {                                                      \
       _GT_2trace(mask, classId,                                \
                  format"\n",                                   \
                  (UInt32) (a),                                 \
                  (UInt32) (b));                                \
    }                                                           \
} while (0)


/* Function to log the trace with three parameters. */
Void _GT_3trace (UInt32         maskType,
                 GT_TraceClass  classtype,
                 Char*          infoString,
                 UInt32         param0,
                 UInt32         param1,
                 UInt32         param2);
#define GT_3trace(mask, classId, format, a, b, c)               \
do {                                                            \
    if (classId == GT_ENTER) {                                  \
       _GT_3trace(mask, classId,                                \
                  "Entered "format"\n\t"#a"\t[0x%x]\n"          \
                  "\t"#b"\t[0x%x]\n"                            \
                  "\t"#c"\t[0x%x]\n",                           \
                  (UInt32) (a),                                 \
                  (UInt32) (b),                                 \
                  (UInt32) (c));                                \
    }                                                           \
    else if (classId == GT_LEAVE) {                             \
       _GT_3trace(mask, classId,                                \
                  "Leaving "format"\n\t"#a"\t[0x%x]\n"          \
                  "\t"#b"\t[0x%x]\n"                            \
                  "\t"#c"\t[0x%x]\n",                           \
                  (UInt32) (a),                                 \
                  (UInt32) (b),                                 \
                  (UInt32) (c));                                \
    }                                                           \
    else {                                                      \
       _GT_3trace(mask, classId,                                \
                  format"\n",                                   \
                  (UInt32) (a),                                 \
                  (UInt32) (b),                                 \
                  (UInt32) (c));                                \
    }                                                           \
} while (0)


/* Function to log the trace with four parameters. */
Void _GT_4trace (UInt32         maskType,
                 GT_TraceClass  classtype,
                 Char*          infoString,
                 UInt32         param0,
                 UInt32         param1,
                 UInt32         param2,
                 UInt32         param3);
#define GT_4trace(mask, classId, format, a, b, c, d) \
do {                                                            \
    if (classId == GT_ENTER) {                                  \
       _GT_4trace(mask, classId,                                \
                  "Entered "format"\n\t"#a"\t[0x%x]\n"          \
                  "\t"#b"\t[0x%x]\n"                            \
                  "\t"#c"\t[0x%x]\n"                            \
                  "\t"#d"\t[0x%x]\n",                           \
                  (UInt32) (a),                                 \
                  (UInt32) (b),                                 \
                  (UInt32) (c),                                 \
                  (UInt32) (d));                                \
    }                                                           \
    else if (classId == GT_LEAVE) {                             \
       _GT_4trace(mask, classId,                                \
                  "Leaving "format"\n\t"#a"\t[0x%x]\n"          \
                  "\t"#b"\t[0x%x]\n"                            \
                  "\t"#c"\t[0x%x]\n"                            \
                  "\t"#d"\t[0x%x]\n",                           \
                  (UInt32) (a),                                 \
                  (UInt32) (b),                                 \
                  (UInt32) (c),                                 \
                  (UInt32) (d));                                \
    }                                                           \
    else {                                                      \
       _GT_4trace(mask, classId,                                \
                  format"\n",                                   \
                  (UInt32) (a),                                 \
                  (UInt32) (b),                                 \
                  (UInt32) (c),                                 \
                  (UInt32) (d));                                \
    }                                                           \
} while (0)


/* Function to log the trace with five parameters. */
Void _GT_5trace (UInt32         maskType,
                 GT_TraceClass  classtype,
                 Char*          infoString,
                 UInt32         param0,
                 UInt32         param1,
                 UInt32         param2,
                 UInt32         param3,
                 UInt32         param4);
#define GT_5trace(mask, classId, format, a, b, c, d, e) \
do {                                                            \
    if (classId == GT_ENTER) {                                  \
       _GT_5trace(mask, classId,                                \
                  "Entered "format"\n\t"#a"\t[0x%x]\n"          \
                  "\t"#b"\t[0x%x]\n"                            \
                  "\t"#c"\t[0x%x]\n"                            \
                  "\t"#d"\t[0x%x]\n"                            \
                  "\t"#e"\t[0x%x]\n",                           \
                  (UInt32) (a),                                 \
                  (UInt32) (b),                                 \
                  (UInt32) (c),                                 \
                  (UInt32) (d),                                 \
                  (UInt32) (e));                                \
    }                                                           \
    else if (classId == GT_LEAVE) {                             \
       _GT_5trace(mask, classId,                                \
                  "Leaving "format"\n\t"#a"\t[0x%x]\n"          \
                  "\t"#b"\t[0x%x]\n"                            \
                  "\t"#c"\t[0x%x]\n"                            \
                  "\t"#d"\t[0x%x]\n"                            \
                  "\t"#e"\t[0x%x]\n",                           \
                  (UInt32) (a),                                 \
                  (UInt32) (b),                                 \
                  (UInt32) (c),                                 \
                  (UInt32) (d),                                 \
                  (UInt32) (e));                                \
    }                                                           \
    else {                                                      \
       _GT_5trace(mask, classId,                                \
                  format"\n",                                   \
                  (UInt32) (a),                                 \
                  (UInt32) (b),                                 \
                  (UInt32) (c),                                 \
                  (UInt32) (d),                                 \
                  (UInt32) (e));                                \
    }                                                           \
} while (0)

#else /* if defined (SYSLINK_TRACE_ENABLE) */

#define GT_setFailureReason(mask, classId, func, status, msg)
#define GT_0trace(mask, classId, format)
#define GT_1trace(mask, classId, format, arg1)
#define GT_2trace(mask, classId, format, arg1, arg2)
#define GT_3trace(mask, classId, format, arg1, arg2, arg3)
#define GT_4trace(mask, classId, format, arg1, arg2, arg3, arg4)
#define GT_5trace(mask, classId, format, arg1, arg2, arg3, arg4, arg5)

#endif /* if defined (SYSLINK_TRACE_ENABLE) */


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* ifndef OSALTRACE_H_0xDA50 */
