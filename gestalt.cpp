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


#include "gestalt.h"

#include "traps.h"
#include "log.h"
#include "resourcefork.h"

extern "C" {
#include "musashi331/m68k.h"
}


/**
 * [A1AD] Report a system feature/capability.
 *
 * FUNCTION Gestalt (selector: OSType; VAR response: LongInt) : OSErr;
 *
 * d0       = selector
 * a1       = response (VAR, out)
 * d0[out]  = return value (OSErr)
 *
 * 1990s-era apps use this to detect available features before calling into
 * a Toolbox manager, so it needs to exist even before any of the managers
 * it asks about are actually implemented. Selectors this doesn't recognize
 * correctly return gestaltUndefSelectorErr, per Apple's own spec -- real
 * apps are written to expect and handle that, so this is not a fallback to
 * apologize for; it's the normal, correct answer for anything not listed
 * below.
 */
void trapGestalt(unsigned short )
{
    unsigned int responsePtr = m68k_get_reg(0L, M68K_REG_A1); // VAR response: LongInt
    unsigned int selector    = m68k_get_reg(0L, M68K_REG_D0); // OSType selector

    unsigned int response = 0;
    short err = 0; // noErr

    switch (selector) {
        case 'sysv': // gestaltSystemVersion
            response = 0x0766; // 7.6.6, matching this simulator's target OS version
            break;
        case 'qd  ': // gestaltQuickdrawVersion
            response = 0x0218; // gestalt32BitQD13, plausible for the System 7.6.6 era
            break;
        case 'os  ': // gestaltOSAttr
            response = 0; // no special attributes claimed
            break;
        case 'evnt': // gestaltAppleEventsAttr
            response = 1<<0; // gestaltAppleEventsPresent
            break;
        case 'proc': // gestaltProcessorType
            response = 3; // gestalt68020, matches main.cpp's M68K_CPU_TYPE_68020
            break;
        case 'fpu ': // gestaltFPUType
            response = 0; // gestaltNoFPU -- mosrun has no FPU emulation; steer apps away from FPU code paths
            break;
        case 'sysa': // gestaltSysArchitecture
            response = 1; // gestalt68k -- honest, since we're running the 68k side of this fat binary
            break;
        default:
            mosDebug("Gestalt: unknown selector '%c%c%c%c' (0x%08X), reporting gestaltUndefSelectorErr\n",
                     selector>>24, selector>>16, selector>>8, selector, selector);
            //printPCHistory();
            err = -5551; // gestaltUndefSelectorErr
            response = 0;
            break;
    }

    if (responsePtr) m68k_write_memory_32(responsePtr, response);
    m68k_set_reg(M68K_REG_D0, (unsigned int)(short)err);
}


void mosSetupGestaltTraps()
{
    createGlue(0xA1AD, trapGestalt);
}
