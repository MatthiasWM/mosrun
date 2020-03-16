/*
 mosrun - the MacOS MPW runtime emulator
 Copyright (C) 2013-2020  Matthias Melcher

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 The author can be contacted at mosrun AT matthiasm DOT com.
 The latest source code can be found at https://github.com/MatthiasWM/mosrun
 */


#include "cpu.h"

// Inlcude all the required system headers

#include <stdlib.h>

// Include our own interfaces

#include "names.h"
#include "log.h"
#include "memory.h"
#include "resourcefork.h"
#include "breakpoints.h"
#include "traps.h"

// Inlcude Musahi's m68k emulator

extern "C" {
#include "musashi331/m68k.h"
#include "musashi331/m68kcpu.h"
#include "musashi331/m68kops.h"
}


bool findFunctionName(mosPtr pc, char* outFunName);

/**
 * Run a single 68020 command.
 *
 * This is where we handle break points and jump into host-native code.
 */
void m68k_instruction_hook()
{
    //  int i; unsigned int sp;
    char buf[2048];
    char functionName[257];
    for (;;) {
        gPendingBreakpoint = 0L;
    afterBreakpoint:
        mosPtr pc = m68k_get_reg(0L, M68K_REG_PC);
        uint16_t instr = m68k_read_memory_16(pc);
        if (mosLogFile() && mosLogVerbosity()>=MOS_VERBOSITY_TRACE) {
            if (mosLogFile()!=stdout) {
                mosTrace("\n");
                bool gotFunctionName = findFunctionName(pc, functionName);
                if (gotFunctionName) {
                    mosTrace("%s (%.8X)\n", functionName, pc);
                } else {
                    mosTrace("? (%.8X)\n", pc);
                }
                mosTraceRegisters();
            }
            m68k_disassemble(buf, pc, M68K_CPU_TYPE_68020);
            if ( (instr & 0xf000) == 0xa000 ) {
                mosTrace("0x%s: %s (%s)\n", printAddr(pc), buf, trapName(instr));
            } else {
                mosTrace("0x%s: %s\n", printAddr(pc), buf);
            } // if/else
        }
        // ---> space for command breakpoint ;-)
        if ( (instr & 0xf000) == 0xa000 ) {
            // p (char*)(m68k_get_reg(0, (m68k_register_t)8)) // gdnb dump string at (A0)
            // 1010.1a0x.xxxx.xxxx: Toolbox call: x = trap #, if a is set, pop the extra return address from the stack
            // 1010.0ffa.xxxx.xxxx: OS call: x = trap #, ff are extra flags that can be used by the traps
            // 01f3 = a9f3
            switch (instr) {
                case 0xaffc: {
                    // 0 Success
                    // 1 Command syntax error
                    // 2 Some error in processing
                    // 3 System error or insufficient resources
                    // –9 User abort
                    unsigned int mpwHandle = m68k_read_memory_32(0x0316);
                    unsigned int mpwMem = m68k_read_memory_32(mpwHandle+4);
                    unsigned int resultCode = m68k_read_memory_32(mpwMem+0x000E);
                    mosDebug("End Of Emulation (returns %d)\n", resultCode);
                    exit(resultCode); }
                case 0xaffd: trapDispatch(instr); break;
                case 0xaffe: trapBreakpoint(instr); goto afterBreakpoint;
                case 0xafff: trapGoNative(instr); break; // TODO: unverified
                default:
                    gCurrentTrap = instr;
                    return; // execute it!
            } // case
            // m68k_op_tst_16_d(); // tst.w d0
        } else {
            return;
        } // if/else
    } // for
}

const uint16_t INSTR_RTS        = 0x4E75;
const uint16_t INSTR_RTE        = 0x4E73;
const uint16_t INSTR_RTD        = 0x4E74;
const uint16_t INSTR_JMP_A0     = 0x4ED0;
const uint16_t INSTR_ADDI_A7    = 0x0697;

/**
 Find the return instruction in a m68k function.

 This is a very simple search function that does not understand much of the give code.
 Results must be taken with caution.

 \param[in,out] pc holds a start address on input and returns the end of that instruction sequence
 \return true, if the typical instruction sequnece for the ned of a function was found
 */
bool findEndOfFunction(mosPtr &pc)
{
    for (int max_try = 0x1000; max_try>0; --max_try) {
        uint16_t instr = m68k_read_memory_16(pc);
        pc+=2;
        if (
            (instr == INSTR_RTS && m68k_read_memory_16(pc - 6) != INSTR_ADDI_A7) // For CW jumps
            || instr == INSTR_RTE
            || instr == INSTR_JMP_A0
            || instr == INSTR_RTD) {
            return true;
        }
    }
    return false;
}


/**
 Try to find the name of a function for a pc somewhere iside the code.

 \param pc an address within a segment of m68k machine code
 \param[out] a buffer where the name of the function will be copied if found
 \return true, if a name for that function was found
 */
bool findFunctionName(mosPtr pc, char *outFunName)
{
    if (findEndOfFunction(pc)) {
        uint8_t d = m68k_read_memory_8(pc);
        mosPtr str_addr;
        uint32_t str_len;
        char *out = outFunName;
        if (d < 0x20) {
            return false;
        } else if (d == 0x80) {
            str_len = m68k_read_memory_8(pc + 1);
            str_addr = pc + 2;
        } else if (d > 0x80 && d < 0xA0) {
            str_len = d - 0x80;
            str_addr = pc + 1;
        } else {
            *out = d & 0x7F;
            out++;
            d = m68k_read_memory_8(pc + 1);
            if (d & 0x80) {
                *out = d & 0x7F;
                out++;
                str_len = 14;   // 16 - 2
            } else {
                str_len = 6;    // 8 - 2
                *out = d;
                out++;
            }
            str_addr = pc + 2;
        }
        while (str_len > 0) {
            d = m68k_read_memory_8(str_addr);
            str_addr++;
            *out = d;
            out++;
            str_len--;
        }
        *out = 0;
        return true;
    } else {
        return false;
    }
}
