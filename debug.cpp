/*
 mosrun - the MacOS MPW runtime emulator
 Copyright (C) 2013-2026  Matthias Melcher

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

#include "debug.h"

#include "memory.h"
#include "breakpoints.h"
#include "log.h"
#include "systemram.h"
#include "resourcefork.h"
#include "progress.h"

#include <string.h>

extern "C" {
    #include "musashi331/m68k.h"
    #include "musashi331/m68kcpu.h"
    #include "musashi331/m68kops.h"
}

/**
 * Print the current state of the CPU before running the current instruction.
 *
 * @param cpu Print disassembly at current pc (default is 1).
 * @param registers Whether to print the registers (default is 0).
 *        1: D and A registers. 2: also print flags.
 * @param stack Number of stack lines to print (default is 0).
 */
void mosDebugPrintCPUState(int cpu, int registers, int stack)
{
    uint32_t pc = m68k_get_reg(0L, M68K_REG_PC);
    uint32_t sp = m68k_get_reg(0L, M68K_REG_SP);
    if (registers) {
        for (int i = 0; i < 8; i++) {
            fprintf(stderr, " D%d:%08X", i, m68k_get_reg(0L, (m68k_register_t)(M68K_REG_D0 + i)));
        }
        fprintf(stderr, "\n");
        for (int i = 0; i < 8; i++) {
            fprintf(stderr, " A%d:%08X", i, m68k_get_reg(0L, (m68k_register_t)(M68K_REG_A0 + i)));
        }
        fprintf(stderr, "\n");
        if (registers == 2) {
            fprintf(stderr, " X:%d N:%d V:%d Z:%d C:%d\n", XFLAG_AS_1(), NFLAG_AS_1(), VFLAG_AS_1(), ZFLAG_AS_1(), CFLAG_AS_1());
            // x n v z c
        }
    }
    if (stack) {
        constexpr int stack_line_size = 16;
        for (int i = 0; i < stack; i++) {
            fprintf(stderr, " sp+%04X:", i * stack_line_size);
            for (int j = 0; j < stack_line_size; j++) {
                uint32_t sp_addr = sp + i * stack_line_size + j;
                fprintf(stderr, " %02X", m68k_read_memory_8(sp_addr));
                sp_addr += 4;
            }
            fprintf(stderr, ": '");
            for (int j = 0; j < stack_line_size; j++) {
                uint32_t sp_addr = sp + i * stack_line_size + j;
                uint8_t c = m68k_read_memory_8(sp_addr);
                fprintf(stderr, "%c", (c >= 32 && c <= 126) ? c : '.');
                sp_addr += 4;
            }
            fprintf(stderr, "'\n");
        }
    }
    if (cpu) {
        char buf[255];
        m68k_disassemble(buf, pc, M68K_CPU_TYPE_68020);
        fprintf(stderr, "0x%08X %s: %s\n", pc, mosDebugAddrToCodeOffsetStr(pc).c_str(), buf);
    }
}


void mosDebugPrintPCHistory(int max)
{
    mosProgressReport();
    mosTrace("PC history:\n");
    char buf[255];
    unsigned int pc = 0;
    int start = (max > 0) ? std::max(0, M68K_PC_HISTORY_SIZE - max) : 0;
    for (int i=start; i<M68K_PC_HISTORY_SIZE; i++) {
        pc = m68k_get_pc_history(M68K_PC_HISTORY_SIZE-i-1);
        if (pc==0) continue;
        if (mosCheckMemoryAccess(pc, 4, false)) {
            m68k_disassemble(buf, pc, M68K_CPU_TYPE_68020);
            mosDebug("  %4d: 0x%08X %s %s\n", M68K_PC_HISTORY_SIZE-i, pc, printAddr(pc), buf);
        } else {
            mosDebug("  %4d: 0x%08X ERR.RRANGE <invalid memory>\n", M68K_PC_HISTORY_SIZE-i, pc);
        }
    }
    pc = m68k_get_reg(0L, M68K_REG_PC);
    if (mosCheckMemoryAccess(pc, 4, false)) {
        m68k_disassemble(buf, pc, M68K_CPU_TYPE_68020);
        mosDebug("  %4d: 0x%08X %s %s\n", 0, pc, printAddr(pc), buf);
    } else {
        mosDebug("  %4d: 0x%08X ERR.RRANGE <invalid memory>\n", 0, pc);
    }
}

std::string mosStr255ToStr(mosPtr str255)
{
    uint8_t length = m68k_read_memory_8(str255);
    std::string result;
    for (uint8_t i = 0; i < length; i++) {
        result += static_cast<char>(m68k_read_memory_8(str255 + 1 + i));
    }
    return result;
}


std::string mosDebugAddrToCodeOffsetStr(uint32_t addr)
{
    return std::string(printAddr(addr));
}