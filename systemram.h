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

#ifndef mosrun_systemram_h
#define mosrun_systemram_h


#include "main.h"


extern unsigned int gMosCurrentA5;
extern unsigned int gMosCurrentStackBase;
extern unsigned int gMosCurJTOffset;
extern unsigned int gMosResLoad;
extern unsigned int gMosResErr;
extern unsigned int gMosMemErr;
extern unsigned int gMosMPWHandle;

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
