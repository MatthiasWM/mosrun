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


#include "dialogmgr.h"

#include "traps.h"

extern "C" {
#include "musashi331/m68k.h"
}


/**
 * [A97B] Initialize the Dialog Manager.
 *
 * PROCEDURE InitDialogs (resumeProc: ProcPtr);
 *
 * sp+4.l  = resumeProc (almost always NIL in practice; used only for the
 *           classic "resume after system error" mechanism)
 * sp.l    = return address
 *
 * No real dialogs are ever shown, so the resume procedure is irrelevant
 * and simply discarded.
 */
void trapInitDialogs(unsigned short )
{
    unsigned int sp = m68k_get_reg(0L, M68K_REG_SP);

    unsigned int ret = m68k_read_memory_32(sp); sp += 4;
    /*unsigned int resumeProc =*/ m68k_read_memory_32(sp); sp += 4;

    sp -= 4; m68k_write_memory_32(sp, ret);

    m68k_set_reg(M68K_REG_SP, sp);
}


void mosSetupDialogMgrTraps()
{
    createGlue(0xA97B, trapInitDialogs);
}
