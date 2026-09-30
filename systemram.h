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


#ifndef mosrun_systemram_h
#define mosrun_systemram_h


#include "main.h"

extern uint32_t gMosStackAllocation; // Start of the memory block that holds the stack.

extern unsigned int gMosCurrentA5;
extern uint32_t gMosCurStackBase;   // 0x0908: CurStackBase (start (top) of application stack)
extern uint32_t gMosApplLimit;      // 0x0130: ApplLimit: application memory limit
extern uint32_t gMosApplZone;       // 0x02AA: ApplZone application zone pointer
extern uint32_t gMosGZMoveHnd;      // 0x0330: GZMoveHnd: Grow Zone Move Handler
extern uint32_t gMosGZRootHnd;      // 0x0328: GZRootHnd: Grow Zone Root Handler
extern unsigned int gMosCurJTOffset;
extern uint8_t gMosResLoad;
extern unsigned int gMosSegHiEnable;
extern unsigned int gMosResErr;
extern unsigned int gMosMemErr;
extern unsigned int gMosMPWHandle;

extern uint16_t gMosCurApRefNum;    // 0x0900: Current application reference number
extern uint16_t gMosACount;         // 0x0A9A:
extern uint16_t gMosHWCfgFlags;     // 0x0B22: Basilisk test


extern "C" {
unsigned int m68k_read_memory_8(unsigned int address);
unsigned int m68k_read_memory_16(unsigned int address);
unsigned int m68k_read_memory_32(unsigned int address);
unsigned int m68k_read_disassembler_8(unsigned int address);
unsigned int m68k_read_disassembler_16(unsigned int address);
unsigned int m68k_read_disassembler_32(unsigned int address);
void m68k_write_memory_8(unsigned int address, unsigned int value);
void m68k_write_memory_16(unsigned int address, unsigned int value);
void m68k_write_memory_32(unsigned int address, unsigned int value);
}


#endif
