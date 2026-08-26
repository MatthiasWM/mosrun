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


#include "quickdraw.h"

#include "traps.h"
#include "log.h"
#include "memory.h"
#include "resourcefork.h"

extern "C" {
#include "musashi331/m68k.h"
}


/**
 * [A86E] Initialize QuickDraw's global variables.
 *
 * PROCEDURE InitGraf (globalPtr: Ptr);
 *
 * sp+4.l  = globalPtr (points at thePort; the rest of the QDGlobals record
 *           lives at negative offsets from it, per Inside Macintosh)
 * sp.l    = return address
 *
 * mosrun does not implement real QuickDraw drawing, so this only needs to
 * leave QDGlobals in a state that later reads don't choke on: zero the
 * whole record (a safe default for randSeed/patterns/cursor/etc, none of
 * which matter without real rendering), set thePort itself to NIL, and
 * give screenBits.bounds a plausible non-zero size so any layout code that
 * centers something against "the screen" doesn't operate on a degenerate
 * zero-sized rect (which could trigger a real divide-by-zero exception
 * mosrun also doesn't handle). The exact byte offsets of the individual
 * QDGlobals fields are approximate best-effort, not verified against real
 * disassembly -- safe to be slightly off since everything in range is
 * already zeroed first, unlike a wrong pointer write into unrelated
 * memory.
 */
void trapInitGraf(unsigned short )
{
    unsigned int sp   = m68k_get_reg(0L, M68K_REG_SP);

    unsigned int ret       = m68k_read_memory_32(sp); sp += 4;
    unsigned int globalPtr = m68k_read_memory_32(sp); sp += 4;

    const int kQDGlobalsSpan = 108; // generous margin around the ~90-byte documented record
    int i;
    for (i = -kQDGlobalsSpan; i < 4; i += 4) {
        m68k_write_memory_32(globalPtr+i, 0);
    }

    // screenBits.bounds: a plausible fake screen size, so centering/layout
    // math doesn't divide by a zero screen width/height.
    m68k_write_memory_16(globalPtr-80, 0);     // top
    m68k_write_memory_16(globalPtr-78, 0);     // left
    m68k_write_memory_16(globalPtr-76, 480);   // bottom
    m68k_write_memory_16(globalPtr-74, 640);   // right
    m68k_write_memory_16(globalPtr-82, 640*2); // rowBytes (no real framebuffer behind this)

    sp -= 4; m68k_write_memory_32(sp, ret);

    m68k_set_reg(M68K_REG_SP, sp);
}


/**
 * [A850] Initialize (and show) the cursor.
 *
 * PROCEDURE InitCursor;
 *
 * Parameterless. No real cursor to draw, so this is a no-op.
 */
void trapInitCursor(unsigned short )
{
    // nothing to do here
}


/**
 * [A8FE] Initialize the Font Manager.
 *
 * PROCEDURE InitFonts;
 *
 * Parameterless. mosrun does not render text, so this is a no-op.
 */
void trapInitFonts(unsigned short )
{
    // nothing to do here
}


/**
 * [A9CC] Initialize TextEdit.
 *
 * PROCEDURE TEInit;
 *
 * Parameterless. mosrun does not render text, so this is a no-op.
 */
void trapTEInit(unsigned short )
{
    // nothing to do here
}


/**
 * [A9B9] Get a cursor resource.
 *
 * FUNCTION GetCursor (cursorID: INTEGER) : CursHandle;
 *
 * sp+6.l  = return value (CursHandle)
 * sp+4.w  = cursorID
 * sp.l    = return address
 *
 * No cursor is ever actually drawn, but callers may reasonably dereference
 * the returned handle (e.g. via SetCursor), so this returns a real, zeroed
 * Cursor-sized (68 byte) handle rather than NIL, regardless of which
 * cursorID was asked for.
 */
void trapGetCursor(unsigned short )
{
    unsigned int sp = m68k_get_reg(0L, M68K_REG_SP);

    unsigned int ret = m68k_read_memory_32(sp); sp += 4;
    /*unsigned int cursorID =*/ m68k_read_memory_16(sp); sp += 2;
    /* 4 bytes space on the stack for the result */

    unsigned int hdl = mosNewHandle(68); // sizeof(Cursor); never rendered, so zeroed contents are fine

    m68k_write_memory_32(sp, hdl);
    sp -= 4; m68k_write_memory_32(sp, ret);

    m68k_set_reg(M68K_REG_SP, sp);
    m68k_set_reg(M68K_REG_D0, 0);
}


/**
 * [A851] Set the cursor shape.
 *
 * At the trap level this takes a single 32-bit pointer to a 68-byte
 * QuickDraw Cursor record, passed on the stack -- not the record itself by
 * value (the documented Pascal signature, PROCEDURE SetCursor (crsr:
 * Cursor), reads that way but doesn't match the real calling convention),
 * and not a register-based no-op either (an earlier attempt at that left
 * this parameter unpopped, misaligning the stack for everything that ran
 * afterward). Standard Pascal convention: the trap itself pops its own
 * parameter off the stack. No cursor is ever drawn, so the pointer's
 * contents are irrelevant -- it's just read and discarded.
 */
void trapSetCursor(unsigned short )
{
    unsigned int sp = m68k_get_reg(0L, M68K_REG_SP);

    unsigned int ret = m68k_read_memory_32(sp); sp += 4;
    /*unsigned int cursorPtr =*/ m68k_read_memory_32(sp); sp += 4;

    sp -= 4; m68k_write_memory_32(sp, ret);

    m68k_set_reg(M68K_REG_SP, sp);
}


void mosSetupQuickDrawTraps()
{
    createGlue(0xA86E, trapInitGraf);
    createGlue(0xA850, trapInitCursor);
    createGlue(0xA8FE, trapInitFonts);
    createGlue(0xA9CC, trapTEInit);
    createGlue(0xA9B9, trapGetCursor);
    createGlue(0xA851, trapSetCursor);
}
