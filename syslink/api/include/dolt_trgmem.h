
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
/*****************************************************************************/
/* dolt_trgmem.h                                                             */
/*                                                                           */
/* Define internal data structures used by DOLT client side for target       */
/* memory management.                                                        */
/*****************************************************************************/
#ifndef DOLT_TRGMEM_H
#define DOLT_TRGMEM_H

#include "dload_api.h"

/*---------------------------------------------------------------------------*/
/* MIN_BLOCK is the minimum size of a packet.                                */
/*---------------------------------------------------------------------------*/
#define MIN_BLOCK       4

/*---------------------------------------------------------------------------*/
/* TRG_PACKET is the template for a data packet.  Packet size contains the   */
/* number of bytes allocated for the user.  Packets are always allocated     */
/* memory in MIN_BLOCK byte chunks.  The list is ordered by packet address   */
/* which refers to the target address associated with the first byte of the  */
/* packet.  The list itself is allocated out of host memory and is a doubly  */
/* linked list to help with easy splitting and merging of elements.          */
/*---------------------------------------------------------------------------*/
typedef struct _trg_packet
{
   /* Need to change this to TARGET_ADDRESS  packet_addr */
   uint32_t             packet_addr;   /* target address of packet          */
   uint32_t             packet_size;   /* number of bytes in this packet    */
   struct _trg_packet  *prev_packet;   /* prev packet in trg mem list       */
   struct _trg_packet  *next_packet;   /* next packet in trg mem list       */
   BOOL                 used_packet;   /* has packet been allocated?        */
} TRG_PACKET;

/*---------------------------------------------------------------------------*/
/* Interface into client's target memory manager.                            */
/*---------------------------------------------------------------------------*/
extern BOOL DLTMM_malloc(struct DLOAD_MEMORY_REQUEST *targ_req,
                         struct DLOAD_MEMORY_SEGMENT *obj_desc);
extern void DLTMM_free(TARGET_ADDRESS ptr);

extern void DLTMM_fwrite_trg_mem(FILE *fp);
extern void DLTMM_fread_trg_mem(FILE *fp);
extern void DLTMM_dump_trg_mem(uint32_t offset, uint32_t nbytes, 
                               FILE* fp);

#endif /* DOLT_TRGMEM_H */
