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
#ifndef _TEST_ARM_RELOC_H_
#define _TEST_ARM_RELOC_H_
#include "arm_elf32.h"
#include <cxxtest/TestSuite.h>

extern "C"
{
extern void unit_arm_reloc_do(ARM_RELOC_TYPE r_type, uint8_t* address, 
                     uint32_t addend, uint32_t symval, uint32_t pc, 
                     uint32_t base_pointer, int wrong_endian);

extern void unit_arm_rel_unpack_addend(ARM_RELOC_TYPE r_type, 
                                       uint8_t* address, 
                                       uint32_t* addend);

extern int unit_arm_rel_overflow(ARM_RELOC_TYPE r_type, int32_t reloc_value);

extern void unit_arm_rel_mask_for_group(ARM_RELOC_TYPE r_type, 
                                        int32_t* reloc_val);
}



class ARM_TestRelocDo : public CxxTest::TestSuite
{
  public:
    void test_R_ARM_PC24();
    void test_R_ARM_JUMP24();
    void test_R_ARM_ABS32();
    void test_R_ARM_ABS16();
    void test_R_ARM_ABS8();
    void test_R_ARM_THM_CALL();
    void test_R_ARM_THM_JUMP11();
    void test_R_ARM_THM_JUMP8();
    void test_R_ARM_THM_PC8();
    void test_R_ARM_THM_ABS5();
    void test_R_ARM_THM_JUMP6();
    void test_R_ARM_ABS12();
    void test_R_ARM_THM_JUMP19();
    void test_R_ARM_PREL31();
    void test_R_ARM_MOVW_ABS_NC();
    void test_R_ARM_MOVT_ABS();
    void test_R_ARM_MOVW_PREL_NC();
    void test_R_ARM_MOVT_PREL();
    void test_R_ARM_THM_MOVW_ABS_NC();
    void test_R_ARM_THM_MOVT_ABS();
    void test_R_ARM_THM_MOVW_PREL_NC();
    void test_R_ARM_THM_MOVT_PREL();
    void test_R_ARM_ABS32_NOI();
    void test_R_ARM_REL32_NOI();
    void test_R_ARM_THM_PC12();
    void test_R_ARM_ALU_PC_G0_NC();
    void test_R_ARM_LDR_PC_G0();
    void test_R_ARM_LDRS_PC_G0();
    void test_R_ARM_LDC_PC_G0();
};

class ARM_TestRelUnpackAddend : public CxxTest::TestSuite
{
  public:
    void test_R_ARM_PC24();
    void test_R_ARM_THM_JUMP19();
    void test_R_ARM_ABS32();
    void test_R_ARM_ABS16();
    void test_R_ARM_ABS8();
    void test_R_ARM_THM_JUMP11();
    void test_R_ARM_THM_JUMP8();
    void test_R_ARM_THM_ABS5();
    void test_R_ARM_THM_CALL();
    void test_R_ARM_THM_JUMP6();
    void test_R_ARM_THM_PC8();
    void test_R_ARM_THM_JUMP24();
    void test_R_ARM_PREL31();
    void test_R_ARM_MOVW_ABS_NC();
    void test_R_ARM_THM_MOVW_ABS_NC();
    void test_R_ARM_ABS12();
    void test_R_ARM_THM_PC12();
};

    
class ARM_TestRelOverflow : public CxxTest::TestSuite
{
  public:
    void test_R_ARM_PC24();
    void test_R_ARM_ABS12();
    void test_R_ARM_THM_CALL();
    void test_R_ARM_THM_JUMP19();
    void test_R_ARM_ABS16();
    void test_R_ARM_ABS8();
    void test_R_ARM_PREL31();
};

class ARM_TestGroupRelocations : public CxxTest::TestSuite
{
  public:
    void test_RelMaskForGroup();
};
#endif /* _TEST_ARM_RELOC_H_ */
