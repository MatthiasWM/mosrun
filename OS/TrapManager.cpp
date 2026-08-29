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

#include "TrapManager.h"

extern "C" {
#include "../musashi331/m68k.h"
#include "../musashi331/m68kcpu.h"
#include "../musashi331/m68kops.h"
}


/**
 * Access the address of an Operating System routine, that is located in the Operating System trap dispatch table.
 * \param trap Operating System A-line instruction or a trap number. If you
 *      specify an Operating System A-line instruction, the function extracts the trap number for you.
 * \returns the address of the corresponding OS routine in the trap dispatch table.
 */
mosPtr GetOSTrapAddress(uint16_t trap)
{
    auto trapIndex = mosOSTrapIndex(trap);
    mosPtr addr = m68k_read_memory_32(gOSTrapTable + trapIndex*4);
    return addr;
}

/**
 * Access the address of an Toolbox routine, that is located in the Toolbox trap dispatch table.
 * \param trap Toolbox A-line instruction or a trap number. If you
 *      specify a Toolbox A-line instruction, the function extracts the trap number for you.
 * \returns the address of the corresponding Toolbox routine in the trap dispatch table.
 */
mosPtr GetToolboxTrapAddress(uint16_t trap)
{
    auto trapIndex = mosToolboxTrapIndex(trap);
    mosPtr addr = m68k_read_memory_32(gToolboxTrapTable + trapIndex*4);
    return addr;
}

/**
 * Retrieve the address of either an Operating System routine or a Toolbox routine.
 * \param trapNum The trap number to look up.
 * \param tTyp The type of trap (Operating System (0) or Toolbox (1)).
 * \returns The address of the corresponding routine in the trap dispatch table.
 */
mosPtr NGetTrapAddress(uint16_t trapNum, uint8_t tTyp)
{
    if (tTyp == 0) {
        return GetOSTrapAddress(trapNum);
    } else if (tTyp == 1) {
        return GetToolboxTrapAddress(trapNum);
    }
    return 0;
}

/**
 * Obsolete. Trap number $00 to $4F, $54, or $57 is drawn from the Operating
 * System dispatch table; any other software routine is taken from the
 * Toolbox dispatch table.
 * https://dev.os9.ca/techpubs/mac/OSUtilities/OSUtilities-185.html
 */
mosPtr GetTrapAddress(uint16_t trapNum)
{
    if (trapNum <= 0x4F || trapNum == 0x54 || trapNum == 0x57) {
        return GetOSTrapAddress(trapNum);
    } else {
        return GetToolboxTrapAddress(trapNum);
    }
}

/**
 * Install an Operating System patch address into an Operating System trap dispatch table.
 * \param[in] trapAddr The Operating System patch address.
 * \param[in] trapNum Operating System A-line instruction or a trap number.
 */
void SetOSTrapAddress(mosPtr trapAddr, uint16_t trapNum)
{
    auto trapIndex = mosOSTrapIndex(trapNum);
    m68k_write_memory_32(gOSTrapTable + trapIndex*4, trapAddr);
}

/**
 * Install an Toolbox patch address into a Toolbox trap dispatch table.
 * \param[in] trapAddr The Toolbox patch address.
 * \param[in] trapNum Toolbox A-line instruction or a trap number.
 */
void SetToolboxTrapAddress(mosPtr trapAddr, uint16_t trapNum)
{
    auto trapIndex = mosToolboxTrapIndex(trapNum);
    m68k_write_memory_32(gToolboxTrapTable + trapIndex*4, trapAddr);
}

/**
 * Install a patch address into either an Operating System trap dispatch table
 * or a Toolbox trap dispatch table.
 * \param[in] trapAddr The patch address.
 * \param[in] trapNum A-line instruction or a trap number.
 * \param tTyp The type of trap (Operating System (0) or Toolbox (1)).
 */
void SetTrapAddress(mosPtr trapAddr, uint16_t trapNum, uint8_t tTyp)
{
    if (tTyp == 0) {
        SetOSTrapAddress(trapAddr, trapNum);
    } else if (tTyp == 1) {
        SetToolboxTrapAddress(trapAddr, trapNum);
    }
}

/**
 * Obsolete. Trap number $00 to $4F, $54, or $57 is drawn from the Operating
 * System dispatch table; any other software routine is taken from the
 * Toolbox dispatch table.
 * https://dev.os9.ca/techpubs/mac/OSUtilities/OSUtilities-186.html
 */
void SetTrapAddress(mosPtr trapAddr, uint16_t trapNum)
{
    if (trapNum <= 0x4F || trapNum == 0x54 || trapNum == 0x57) {
        SetOSTrapAddress(trapAddr, trapNum);
    } else {
        SetToolboxTrapAddress(trapAddr, trapNum);
    }
}

// -------------------- trap to C ----------------------------------------------

/**
 * GetTrapAddress for an A-line trap word.
 *   { 0xA146, "_GetTrapAddress" },
 *   { 0xA346, "_GetOSTrapAddress" },
 *   { 0xA746, "_GetToolBoxTrapAddress" },
 * D0	An A-line trap word
 * A0	Return current address
 * When calling the _GetTrapAddress macro, you set bit 9 of the A-line trap
 * word to indicate a "new" system.
 * You use bit 10 to indicate whether the system software routine that is being
 * patched is a Toolbox routine (by setting bit 10 to 1) or an Operating System
 * routine (by setting bit 10 to 0).
 */
void trap_GetTrapAddress(unsigned short trap) {
    auto d0 = m68k_get_reg(0L, M68K_REG_D0);
    mosPtr trapAddr = 0;
    switch (mosOSTrapFlags(trap)) {
        case 0x0000:
            trapAddr = GetTrapAddress(d0);
            break;
        case 0x0200:
            trapAddr = GetToolboxTrapAddress(d0);
            break;
        default: // 0x0400, 0x0600
            trapAddr = GetToolboxTrapAddress(d0);
            break;
    }
    m68k_set_reg(M68K_REG_A0, trapAddr);
}

/**
 * SetTrapAddress for an A-line trap word.
 *  { 0xA047, "_SetTrapAddress" },
 *  { 0xA247, "_SetOSTrapAddress" },
 *  { 0xA647, "_SetToolBoxTrapAddress" },
 * D0	An A-line trap word
 * A0	Address of next routine in the daisy chain (a system software routine or a patch)
 * When calling the _SetTrapAddress macro, you set bit 9 of the A-line trap
 * word to indicate a "new" system.
 * You use bit 10 to indicate whether the system software routine that is being
 * patched is a Toolbox routine (by setting bit 10 to 1) or an Operating System
 * routine (by setting bit 10 to 0).
 */
void trap_SetTrapAddress(unsigned short trap) {
    auto a0 = m68k_get_reg(0L, M68K_REG_A0);
    auto d0 = m68k_get_reg(0L, M68K_REG_D0);
    switch (mosOSTrapFlags(trap)) {
        case 0x0000:
            SetTrapAddress(a0, d0);
            break;
        case 0x0200:
            SetToolboxTrapAddress(a0, d0);
            break;
        default: // 0x0400, 0x0600
            SetToolboxTrapAddress(a0, d0);
            break;
    }
}

void mosSetupTrapManager()
{
    // Initialize the trap manager by setting up the trap table
    createGlue(0xA146, trap_GetTrapAddress);
    createGlue(0xA047, trap_SetTrapAddress);
}