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


#include "windowmgr.h"

#include "traps.h"

extern "C" {
#include "musashi331/m68k.h"
}


// WindowList [0x09D6]: pointer to the first window in the Z-ordered window
// list, 0 if there are none yet. Backed here for real (see systemram.cpp)
// rather than left to the generic "unsupported RAM" fallback, since the
// Window Manager traps below will need to maintain it as real windows are
// created/disposed.
unsigned int gMosWindowList = 0;


/**
 * [A912] Initialize the Window Manager.
 *
 * PROCEDURE InitWindows;
 *
 * Parameterless. No real window server exists, so there is nothing to set
 * up beyond the WindowList global already starting out empty.
 */
void trapInitWindows(unsigned short )
{
    gMosWindowList = 0;
}


void mosSetupWindowMgrTraps()
{
    createGlue(0xA912, trapInitWindows);
}
