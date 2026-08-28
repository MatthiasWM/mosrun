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


#include "eventmgr.h"

#include "traps.h"

extern "C" {
#include "musashi331/m68k.h"
}


/**
 * [A032] Remove events from the event queue.
 *
 * PROCEDURE FlushEvents (whichMask: INTEGER; stopMask: INTEGER);
 *
 * d0 low word  = whichMask
 * d0 high mask = stopMask
 * sp.l         = return address
 *
 * mosrun has no real event queue, so there is nothing to flush.
 */
void trapFlushEvents(unsigned short )
{
    // unsigned int stopMask  = m68k_get_reg(0L, M68K_REG_D0) >> 16; // high word
    // unsigned int whichMask = m68k_get_reg(0L, M68K_REG_D0) & 0xFFFF; // low word
}


/**
 * [A971] Peek at the next queued event without removing it.
 *
 * FUNCTION EventAvail (eventMask: Integer; VAR theEvent: EventRecord): Boolean;
 *
 * Stack layout at trap entry:
 * sp+0.l   = return address (pushed by the trap dispatch mechanism)
 * sp+4.l   = pointer to theEvent (VAR, out)
 * sp+8.w   = eventMask
 * sp+10.w  = reserved Boolean function result
 *
 * mosrun has no real event queue, so this always reports "no event
 * available": theEvent is filled in as a null event (what=nullEvent,
 * current tick count, no mouse position or modifiers tracked, since
 * neither is simulated), and the function result is FALSE.
 */
void trapEventAvail(unsigned short )
{
    unsigned int sp = m68k_get_reg(0L, M68K_REG_SP);

    unsigned int ret         = m68k_read_memory_32(sp); sp += 4;
    unsigned int theEventPtr = m68k_read_memory_32(sp); sp += 4;
    /*unsigned int eventMask  =*/ m68k_read_memory_16(sp); sp += 2;

    if (theEventPtr) {
        m68k_write_memory_16(theEventPtr+0, 0);              // what = nullEvent
        m68k_write_memory_32(theEventPtr+2, 0);               // message
        m68k_write_memory_32(theEventPtr+6, mosTickCount());  // when
        m68k_write_memory_32(theEventPtr+10, 0);              // where (v,h) = (0,0)
        m68k_write_memory_16(theEventPtr+14, 0);              // modifiers
    }

    m68k_write_memory_16(sp, 0); // FALSE -- no event available

    sp -= 4; m68k_write_memory_32(sp, ret);

    m68k_set_reg(M68K_REG_SP, sp);
    m68k_set_reg(M68K_REG_D0, 0);
}


void mosSetupEventMgrTraps()
{
    createGlue(0xA032, trapFlushEvents);
    createGlue(0xA971, trapEventAvail);
}
