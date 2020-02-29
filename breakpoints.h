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

#ifndef __mosrun__breakpoints__
#define __mosrun__breakpoints__

#include "main.h"

typedef struct Breakpoint {
  struct Breakpoint *next;
  unsigned int segment;
  unsigned int segAddr;
  unsigned int address;
  unsigned short originalCmd;
  const char *text;
} Breakpoint;

extern Breakpoint *gFirstBreakpoint;
extern Breakpoint *gPendingBreakpoint;


void addBreakpoint(unsigned int segment, unsigned int address, const char *text="");
Breakpoint *findBreakpoint(unsigned int pc);
void installBreakpoints(unsigned int segment, unsigned int segAddr);


#endif /* defined(__mosrun__breakpoints__) */
