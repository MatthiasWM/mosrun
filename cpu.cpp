/*
 mosrun - the MacOS MPW runtime emulator
 Copyright (C) 2013  Matthias Melcher
 
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
 The latest source code can be found at http://code.google.com/p/dynee5/
 */

#include "cpu.h"

// Inlcude all the required system headers

#include <stdlib.h>

// Include our own interfaces

#include "cpu.h"
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


bool findFunctionName(unsigned int pc, char* outFunName);

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
    unsigned int pc = m68k_get_reg(0L, M68K_REG_PC);
    unsigned short instr = m68k_read_memory_16(pc);
    if (mosLogFile() && mosLogVerbosity()>=MOS_VERBOSITY_TRACE) {
      if (mosLogFile()!=stdout) {
        mosTrace("\n");
        bool gotFunctionName = findFunctionName(pc, functionName);
        if (gotFunctionName) {
          mosTrace("%s (%.8X)\n", functionName, pc);
        } else {
          mosTrace("? (%.8X)\n", pc);
        }
        mosTrace("D0:%08X D1:%08X D2:%08X D3:%08X D4:%08X D5:%08X D6:%08X D7:%08X\n",
                 m68k_get_reg(0L, M68K_REG_D0),
                 m68k_get_reg(0L, M68K_REG_D1),
                 m68k_get_reg(0L, M68K_REG_D2),
                 m68k_get_reg(0L, M68K_REG_D3),
                 m68k_get_reg(0L, M68K_REG_D4),
                 m68k_get_reg(0L, M68K_REG_D5),
                 m68k_get_reg(0L, M68K_REG_D6),
                 m68k_get_reg(0L, M68K_REG_D7)
                 );
        mosTrace("A0:%08X A1:%08X A2:%08X A3:%08X A4:%08X A5:%08X A6:%08X A7:%08X\n",
                 m68k_get_reg(0L, M68K_REG_A0),
                 m68k_get_reg(0L, M68K_REG_A1),
                 m68k_get_reg(0L, M68K_REG_A2),
                 m68k_get_reg(0L, M68K_REG_A3),
                 m68k_get_reg(0L, M68K_REG_A4),
                 m68k_get_reg(0L, M68K_REG_A5),
                 m68k_get_reg(0L, M68K_REG_A6),
                 m68k_get_reg(0L, M68K_REG_A7)
                 );
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

const unsigned short INSTR_RTS      = 0x4E75;
const unsigned short INSTR_RTE      = 0x4E73;
const unsigned short INSTR_RTD      = 0x4E74;
const unsigned short INSTR_JMP_A0 = 0x4ED0;
const unsigned short INSTR_ADDI_A7  = 0x0697;

bool
findEndOfFunction(unsigned int& pc) {
  unsigned int max_try = 0x1000;
  while (max_try) {
    unsigned short instr = m68k_read_memory_16(pc);
    pc+=2;
        
    if (
      (instr == INSTR_RTS && m68k_read_memory_16(pc - 6) != INSTR_ADDI_A7) // For CW jumps
      || instr == INSTR_RTE
      || instr == INSTR_JMP_A0
      || instr == INSTR_RTD) {
      return true;
    }
    max_try--;
  }
  return false;
}

bool
findFunctionName(unsigned int pc, char* outFunName) {
  if (findEndOfFunction(pc)) {
    unsigned char byte = m68k_read_memory_8(pc);
    unsigned int str_addr;
    unsigned int str_len;
    char* out = outFunName;
    if (byte < 0x20) {
      return false;
    } else if (byte == 0x80) {
      str_len = m68k_read_memory_8(pc + 1);
      str_addr = pc + 2;
    } else if (byte > 0x80 && byte < 0xA0) {
      str_len = byte - 0x80;
      str_addr = pc + 1;
    } else {
      *out = byte & 0x7F;
      out++;
      byte = m68k_read_memory_8(pc + 1);
      if (byte & 0x80) {
        *out = byte & 0x7F;
        out++;
        str_len = 14;   // 16 - 2
      } else {
        str_len = 6;    // 8 - 2
        *out = byte;
        out++;
      }
      str_addr = pc + 2;
    }
    while (str_len > 0) {
      byte = m68k_read_memory_8(str_addr);
      str_addr++;
      *out = byte;
      out++;
      str_len--;
    }
    *out = 0;
    return true;
  } else {
    return false;
  }
}
