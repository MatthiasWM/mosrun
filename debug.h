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


#ifndef __mosrun__debug__
#define __mosrun__debug__

#include "main.h"

#include <vector>

void mosDebugPrintCPUState(int cpu=1, int registers=0, int stack=0);
void mosDebugPrintPCHistory();

std::string mosStr255ToStr(mosPtr str255);
std::string mosDebugAddrToCodeOffsetStr(uint32_t addr);

#endif /* defined(__mosrun__debug__) */
