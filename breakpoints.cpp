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


#include "breakpoints.h"
#include "systemram.h"

#include <stdlib.h>


Breakpoint *gFirstBreakpoint = 0L;
Breakpoint *gPendingBreakpoint = 0L;


/**
 * Add a breakpoint before running the program.
 *
 * This does nothing once the corresponding segment is loaded.
 * All addresses and segment numbers can be taken right out of the code dump.
 *
 * Having two breakpoints at the same address will cause an infinite loop.
 */
void addBreakpoint(unsigned int segment, unsigned int address, const char *text)
{
  Breakpoint *bp = (Breakpoint*)calloc(1, sizeof(Breakpoint));
  bp->next = gFirstBreakpoint;
  gFirstBreakpoint = bp;
  bp->segment = segment;
  bp->segAddr = address;
  bp->text = text;
  // set the actual breakpoint when the segment is loaded
}


/**
 * Find a breakpoint description using a host-space address.
 */
Breakpoint *findBreakpoint(unsigned int pc)
{
  Breakpoint *bp = gFirstBreakpoint;
  while (bp) {
    if (bp->address==pc)
      break;
    bp = bp->next;
  }
  return bp;
}


/**
 * Install all breakpoints for a just loaded segment.
 */
void installBreakpoints(unsigned int segment, unsigned int segAddr)
{
  Breakpoint *bp = gFirstBreakpoint;
  while (bp) {
    if (bp->segment == segment) {
      unsigned int dst = segAddr+bp->segAddr;
      bp->address = dst;
      bp->originalCmd = m68k_read_memory_16(dst);
      m68k_write_memory_16(dst, 0xaffe);
    }
    bp = bp->next;
  }
}

