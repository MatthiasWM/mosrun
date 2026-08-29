/*
 mosrun - the MacOS MPW runtime emulator
 Copyright (C) 2026  Matthias Melcher

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


#ifndef mosrun_os_trap_manager_h
#define mosrun_os_trap_manager_h

#include "../traps.h"

mosPtr GetOSTrapAddress(uint16_t trap);
mosPtr GetToolboxTrapAddress(uint16_t trap);
mosPtr NGetTrapAddress(uint16_t trapNum, uint8_t tTyp);
mosPtr GetTrapAddress(uint16_t trapNum);

void SetOSTrapAddress(mosPtr trapAddr, uint16_t trapNum);
void SetToolboxTrapAddress(mosPtr trapAddr, uint16_t trapNum);
void SetTrapAddress(mosPtr trapAddr, uint16_t trapNum, uint8_t tTyp);
void SetTrapAddress(mosPtr trapAddr, uint16_t trapNum);

void mosSetupTrapManager();

#endif // mosrun_os_trap_manager_h
