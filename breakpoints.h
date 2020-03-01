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

 This code is 64 bit safe.
*/

#ifndef __mosrun__breakpoints__
#define __mosrun__breakpoints__

#include "main.h"

/**
 Define a breakpoint in the m68k code of a Classic executable.
 */
typedef struct Breakpoint {
    // Manage breakpoints in a single linked list
    struct Breakpoint *next;
    // Classic executables can have multiple code segments
    uint32_t segment;
    // offset within the segment
    uint32_t offset;
    // address in mos memory
    mosPtr address;
    // this is the original m68k code that we replace with a bp trap
    uint16_t originalCmd;
    // this is a text describing the breakpoint
    const char *text;
} Breakpoint;

extern Breakpoint *gFirstBreakpoint;
extern Breakpoint *gPendingBreakpoint;

void addBreakpoint(uint32_t segment, uint32_t offset, const char *text="");
Breakpoint *findBreakpoint(mosPtr pc);
void installBreakpoints(uint32_t segment, mosPtr segAddr);


#endif /* defined(__mosrun__breakpoints__) */
