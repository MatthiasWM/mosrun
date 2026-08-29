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


#ifndef mosrun_traps_h
#define mosrun_traps_h

#include "main.h"

extern uint16_t gCurrentTrap;
extern unsigned int gTrapDispatchALineStub;
extern unsigned int gTrapDispatchFLineStub;
extern unsigned int gTrapExitAppStub;

void trapGoNative(unsigned short instr);
void trapBreakpoint(unsigned short instr);
void trapALineDispatch(unsigned short);
void trapFLineDispatch(unsigned short);
mosPtr createGlue(unsigned short index, mosTrap trap);
void mosSetupTrapTable();

unsigned int mosTickCount();

inline bool mosIsTrapCommand(uint16_t cmd) { return (cmd & 0xF000) == 0xA000; }

inline bool mosIsToolboxTrap(uint16_t cmd) { return (cmd & 0xF800) == 0xA800; }
inline uint16_t mosToolboxTrapIndex(uint16_t cmd) { return (cmd & 0x03ff); }
inline bool mosToolboxTrapAutoPop(uint16_t cmd) { return (cmd & 0xFC00) == 0xAC00; }
extern mosPtr gToolboxTrapTable;
constexpr int kToolboxTrapTableSize = 0x400;

inline bool mosIsOSTrap(uint16_t cmd) { return (cmd & 0xF800) == 0xA000; }
inline uint16_t mosOSTrapIndex(uint16_t cmd) { return (cmd & 0x00ff); }
inline bool mosOSTrapPreserveA0(uint16_t cmd) { return (cmd & 0x0100) == 0x0100; }
inline bool mosOSTrapBit9(uint16_t cmd) { return (cmd & 0x0200) == 0x0200; }
inline bool mosOSTrapBit10(uint16_t cmd) { return (cmd & 0x0400) == 0x0400; }
inline uint16_t mosOSTrapFlags(uint16_t cmd) { return (cmd & 0x0600); }
extern mosPtr gOSTrapTable;
constexpr int kOSTrapTableSize = 0x100;

#endif
