/*
 *  Syslink-IPC for TI OMAP Processors
 *
 *  Copyright (c) 2008-2010, Texas Instruments Incorporated
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  *  Neither the name of Texas Instruments Incorporated nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
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
 */
/*
* rcmserver.h
*
* The RCM server module receives RCM messages from the RCM client,
* executes remote function and sends replies to the RCM client.
*/

#ifndef RCMTYPES_H_
#define RCMTYPES_H_

/* Standard headers */
#include <Std.h>

/* Utilities headers */
#include <ti/ipc/MessageQ.h>
#include <RcmClient.h>

/*
 * RCM message descriptors
 */
#define RcmClient_Desc_RCM_MSG          0x1 /*  RcmClient execution message */
#define RcmClient_Desc_DPC              0x2 /*  Deferred Procedure Call */
#define RcmClient_Desc_SYM_ADD          0x3 /*  Symbol add message */
#define RcmClient_Desc_SYM_IDX          0x4 /*  Query symbox index */
#define RcmClient_Desc_SHUTDOWN         0x5 /*  RcmServer shutdown message */
#define RcmClient_Desc_CONNECT          0x6 /*  RcmClient connected message */
#define RcmClient_Desc_RCM_NO_REPLY     0x7 /*  RcmClient No Reply */

#define RcmClient_Desc_TYPE_MASK  0x0F00    /* field mask */
#define RcmClient_Desc_TYPE_SHIFT 8         /* field shift width */

/* server status codes must be 0 - 15, it has to fit in a 4-bit field */
#define RcmServer_Status_SUCCESS ((UInt16)(0)) /* success */
#define RcmServer_Status_INVALID_FXN ((UInt16)(1)) /* invalid function index */
#define RcmServer_Status_SYMBOL_NOT_FOUND ((UInt16)(2)) /* symbol not found */
#define RcmServer_Status_INVALID_MSG_TYPE ((UInt16)(3)) /* invalid message type */
#define RcmServer_Status_MSG_FXN_ERR ((UInt16)(4)) /* message function error */
#define RcmServer_Status_ERROR ((UInt16)(5)) /* general failure */

/* =============================================================================
 * Structures & Enums
 * =============================================================================
 */

/*
 * RCM Client packet structure
 */
typedef struct RcmClient_Packet_tag{
    MessageQ_MsgHeader msgqHeader; /* MessageQ header */
    UInt16 desc; /* protocol version, descriptor, status */
    UInt16 msgId; /* message id */
    RcmClient_Message message; /* client message body (4 words) */
} RcmClient_Packet;


#endif /* RCMTYPES_H_ */
