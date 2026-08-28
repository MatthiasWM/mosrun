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
#include "resourcefork.h"
#include "memory.h"
#include "debug.h"

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

    mosDebugPrintPCHistory();

    bool found = false;
    mosHandle hAlrt = GetResource('ALRT', alertID);
    if (hAlrt) {
        mosPtr pAlrt = mosHLock(hAlrt);
        printf("\n");
        uint16_t ditlID = m68k_read_memory_16(pAlrt+8);
        mosHandle hDitl = GetResource('DITL', ditlID);
        if (hDitl) {
            mosPtr pDitl = mosHLock(hDitl);
            uint16_t n = m68k_read_memory_16(pDitl);
            mosPtr src = pDitl+2;
            for (uint16_t i = 0; i < n; i++) {
                uint8_t type = m68k_read_memory_8(src+12);
                uint8_t length = m68k_read_memory_8(src+13);
                if ((type & 0x7f) == 8) { // static text
                    std::string buf(length, ' ');
                    for (uint16_t t = 0; t < length; t++) {
                        uint8_t c = m68k_read_memory_8(src+14+t);
                        buf[t] = c;
                    }
                    mosError("Alert(%d): %s\n", (short)alertID, buf.c_str());
                    found = true;
                }
                src += 14 + length;
            }
        }
    }

    if (!found) {
        mosWarning("Alert(%d) -- no static text found in dialog\n", (short)alertID);
    }

        // 0x02.w = 'vers'
        // 0x04.w = 'sysv' , test bit 0x600
        // 0x06.w = 0x00079386
        // 0x08.b =0x00079388
        // 0x0e = 'te  '
        // 0x12 -> jsr 0x00001368 -> 0x14
        // 0x14 , 0xA80B, jsr 0x00001368  ... _PopUpMenuSelect
        // 0x15 - 'scri'
        // 0x17 , 0xA803, jsr 0x00001368  ... _SndDoCommand
        // 0x18 , 0xA860, jsr 0x00001368  ... _WaitNextEvent
        // 0x19 = bit 7 in hdwr
        // 0x1b = a/ux
        // 0x1c = bit 4 in 'os  '
        // 0x1a = 0x400 bit in HWCfgFlags
        // 0x1e = bit 0 of evnt
        // 0x1f = bit 0 of edtn
        // 0x20 = bit 0 of help
        // 0x21 = bit 0 of alis
        // 0x22 = bit 0 of fold
        // 0x23 = [0x00079382] , bit 3 of 'os  ', *must* be set
        // 0x24 = bit 0 of pop!
        // 0x25 = bit 0 of font
        // 0x26 = [0x00079383] , ProcessInfoRec: [A7 + 0x26].b, bit 2
        // 0x27 = [0x00079384] , ProcessInfoRec: [A7 + 0x24].l mut not be 0
        // 0x28 = bit 0 of drag
        // 0x29 = bit 0 of thds
        // 0x2a = bit 0 of oceu
        // 0x2b = bit 0 of grfx
        // 0x2c = bit 0 of pmgr
        // 0x2d = always 0
        // 0x2e = bit 0 of cfrg
        // 0x2f = bit 0 of xlat
        // 0x30 = bit 0 of ttsc
        // 0x31 = bit 0 of stdf
        // 0x32 = qtim
        // 0x33 = tsmv

        // These bits are tested:
        // 12, 15, 14, 16, 18, 4>605, 9, 4>700, 1e, 1f, 20, 21, 22, 23, 1b==0
        //  1   1   0   1   1         1          0   0   0   0   0   1   1

        // 0x002B60DC
        // 1e, 1f, 20, 21, 22, 23,   12, 15, 14, 16, 18, 9
        // 4 == 605 (sge d0
        // 1b must be 0?
    for (int i = 0; i < 0x40; i++) {
        uint8_t d = m68k_read_memory_8(0x002B60DC + i);
        mosDebug("0x002B60DC + 0x%02X = 0x%02X\n", i, d);
    }

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
