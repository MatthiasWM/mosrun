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


#ifndef __mosrun__progress__
#define __mosrun__progress__

#include "main.h"

#include <cstdint>


/**
 * Track how far the emulation has come.
 *
 * The emulator can run for a very long time before it gets stuck or
 * crashes, and when that happens, the only question that matters is
 * "did we actually make progress, or are we stuck spinning?". These
 * functions keep a running count of 68k instructions retired and native
 * trap calls dispatched, plus a short history of the most recently
 * called traps, so that a postmortem dump can answer that question.
 */

// Call once per 68k instruction that Musashi is about to execute.
void mosProgressInstruction();

// Call once per native trap dispatch (i.e. every time a 0xAFFF glue stub
// hands control to a native C trap handler).
void mosProgressTrap(uint16_t trap);

// Total number of 68k instructions retired since startup.
uint64_t mosProgressInstructionCount();

// Total number of native trap calls dispatched since startup.
uint64_t mosProgressTrapCount();

// Print a snapshot of where the emulation currently stands: totals,
// current PC, the most recently called traps, and the traps called most
// often overall (a tight loop shows up as one trap with a huge count).
void mosProgressReport();

#endif /* defined(__mosrun__progress__) */
