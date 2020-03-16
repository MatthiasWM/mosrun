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


#include "breakpoints.h"
#include "systemram.h"

#include <stdlib.h>


Breakpoint *gFirstBreakpoint = 0L;
Breakpoint *gPendingBreakpoint = 0L;


/**
 * Add a breakpoint before running the program.
 *
 * Manage breakpoints in a list until the corresponding segment is loaded. Once the segment
 * is activated, the corresponding address is patched into a Breakpoint Trap command. The
 * original opcode is stored and can be restored later. This allows the program to continue
 * after the breakpoint was hit.
 *
 * \note Breakpoints must be added before a segment is loaded and activated.
 *
 * \note Having two breakpoints at the same address will cause an infinite loop.
 *
 * \param segment index number of the given segment
 * \param offset byte offset within segment
 * \param text description of breakpoint
 */
void addBreakpoint(uint32_t segment, uint32_t offset, const char *text)
{
    Breakpoint *bp = (Breakpoint*)calloc(1, sizeof(Breakpoint));
    bp->next = gFirstBreakpoint;
    gFirstBreakpoint = bp;
    bp->segment = segment;
    bp->offset = offset;
    bp->text = text;
}


/**
 * Find a breakpoint.
 *
 * Search the list for a breakpoint for the given mos address.
 *
 * \param pc find the breakpoint for this address in mos memory.
 * \return the address of the breakpoint, or a nullptr
 */
Breakpoint *findBreakpoint(mosPtr pc)
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
 *
 * \param segmentIndex the index of the segment that we just installed
 * \param segmentBaseAddress the adress in mos space where the segment was installed
 */
void installBreakpoints(uint32_t segmentIndex, mosPtr segmentBaseAddress)
{
    Breakpoint *bp = gFirstBreakpoint;
    while (bp) {
        if (bp->segment == segmentIndex) {
            mosPtr dst = segmentBaseAddress + bp->offset;
            bp->address = dst;
            bp->originalCmd = m68k_read_memory_16(dst);
            m68k_write_memory_16(dst, 0xaffe); // breakpoint trap code
        }
        bp = bp->next;
    }
}

