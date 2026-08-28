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
        // values taken from BasiliskII run:
        case 'os  ': response = 0x0000efff; break; // gestaltOSAttr
        case 'proc': response = 0x00000003; break; // gestaltProcessorType: gestalt68020
        case 'fpu ': response = 0x00000000; break;
        case 'vers': response = 0x00000006; break;
        case 'mach': response = 0x0000000b; break;
        case 'sysv': response = 0x00000755; break; // gestaltSystemVersion
        case 'qd  ': response = 0x00000230; break; // gestaltQuickdrawVersion
        case 'kbd ': response = 0x00000009; break;
        case 'atlk': response = 0x00000000; break;
        case 'hdwr': response = 0x0108489d; break;
        case 'scri': response = 0x00000750; break;
        case 'te  ': response = 0x00000005; break;
        case 'evnt': response = 0x00000003; break; // gestaltAppleEventsAttr
        case 'edtn': response = 0x00000003; break;
        case 'help': response = 0x80000001; break;
        case 'alis': response = 0x00000007; break;
        case 'fold': response = 0x00000001; break;
        case 'pop!': response = 0x00000001; break;
        case 'font': response = 0x00000007; break;
        case 'drag': response = 0x8000000b; break;
        case 'thds': response = 0x0000001b; break;
        case 'xlat': response = 0x0000001b; break;
        case 'stdf': response = 0x00000017; break;
        case 'qtim': response = 0x02108000; break;
        case 'tsmv': response = 0x00000001; break;
        case 'sysa': response = 0x00000001; break; // gestaltSysArchitecture

        default:
            mosDebug("Gestalt: unknown selector '%c%c%c%c' (0x%08X), reporting gestaltUndefSelectorErr\n",
                     selector>>24, selector>>16, selector>>8, selector, selector);
        // Known usupported selectors:
        case 'a/ux':
        case 'oceu':
        case 'grfx':
        case 'pmgr':
        case 'cfrg':
        case 'ttsc':
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

