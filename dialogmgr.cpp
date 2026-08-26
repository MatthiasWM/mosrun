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
#include "log.h"

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


/**
 * [A985] Display an alert box and return the item the user picked.
 *
 * FUNCTION Alert (alertID: Integer; filterProc: ModalFilterProcPtr): Integer;
 *
 * Stack at trap entry:
 * sp+0.l  = return address
 * sp+4.l  = filterProc (pushed last, on top before the trap fires)
 * sp+8.w  = alertID
 * sp+10.w = reserved Integer function result
 *
 * No real alert is ever drawn and the filter proc is never called (per
 * explicit direction) -- this always reports item 1 (the default/OK
 * button) as a placeholder; picking the *correct* item number for a real
 * alert's actual button layout is left for later. Logged at warning level
 * (visible by default, not buried in trace output) since a popped alert is
 * effectively NTK's failure-reporting channel -- worth seeing even in a
 * normal run.
 *
 * Note: typedef CALLBACK_API( Boolean , ModalFilterProcPtr )(DialogRef theDialog, EventRecord *theEvent, DialogItemIndex *itemHit);
 *
 * Note: StopAlert ($A986), NoteAlert ($A987), and CautionAlert ($A988) are
 * the very next three trap numbers — same signature, same calling
 * convention, just a different icon
 *
 * (137): Your computer lacks the required hardware or system software to
 *        run this application.
 */
void trapAlert(unsigned short )
{
    unsigned int sp = m68k_get_reg(0L, M68K_REG_SP);

    unsigned int ret         = m68k_read_memory_32(sp); sp += 4;
    /*unsigned int filterProc =*/ m68k_read_memory_32(sp); sp += 4;
    unsigned int alertID     = m68k_read_memory_16(sp); sp += 2;

    mosWarning("Alert(%d) -- no real dialog shown, auto-answering item 1\n", (short)alertID);

    m68k_write_memory_16(sp, 1); // itemHit = 1 (default/OK), placeholder

    sp -= 4; m68k_write_memory_32(sp, ret);

    m68k_set_reg(M68K_REG_SP, sp);
    m68k_set_reg(M68K_REG_D0, 1);
}


void mosSetupDialogMgrTraps()
{
    createGlue(0xA97B, trapInitDialogs);
    createGlue(0xA985, trapAlert);
}
