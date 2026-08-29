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

#include "progress.h"

#include "log.h"
#include "names.h"
#include "resourcefork.h"

#include <algorithm>
#include <unordered_map>
#include <vector>

extern "C" {
#include "musashi331/m68k.h"
}


// Print a heartbeat every this many retired instructions, so a run that is
// actually progressing shows an ever-advancing PC, while a run stuck in a
// tight loop shows the same handful of addresses over and over.
static const uint64_t kHeartbeatInterval = 1'000'000;

// How many of the most recently dispatched traps to remember for the
// postmortem report.
static const int kRecentTrapSlots = 16;

struct RecentTrap {
    uint64_t atInstruction = 0;
    uint16_t trap = 0;
    mosPtr callerPC = 0;
};

static uint64_t gInstructionCount = 0;
static uint64_t gTrapCount = 0;
static RecentTrap gRecentTraps[kRecentTrapSlots];
static int gRecentTrapNext = 0;
static std::unordered_map<uint16_t, uint32_t> gTrapFrequency;


void mosProgressInstruction()
{
    gInstructionCount++;
    if (gInstructionCount % kHeartbeatInterval == 0) {
        mosPtr pc = m68k_get_reg(0L, M68K_REG_PC);
        mosLog("progress: %llu instructions, %llu trap calls, pc=0x%08X (%s)\n",
               (unsigned long long)gInstructionCount,
               (unsigned long long)gTrapCount,
               pc, printAddr(pc));
    }
}


void mosProgressTrap(uint16_t trap)
{
    gTrapCount++;
    gTrapFrequency[trap]++;

    RecentTrap &slot = gRecentTraps[gRecentTrapNext];
    slot.atInstruction = gInstructionCount;
    slot.trap = trap;
    slot.callerPC = m68k_get_reg(0L, M68K_REG_PC);
    gRecentTrapNext = (gRecentTrapNext + 1) % kRecentTrapSlots;
}


uint64_t mosProgressInstructionCount()
{
    return gInstructionCount;
}


uint64_t mosProgressTrapCount()
{
    return gTrapCount;
}


void mosProgressReport()
{
    mosPtr pc = m68k_get_reg(0L, M68K_REG_PC);
    mosLog("--- progress report ---\n");
    mosLog("%llu instructions executed, %llu trap calls dispatched, pc=0x%08X (%s)\n",
           (unsigned long long)gInstructionCount,
           (unsigned long long)gTrapCount,
           pc, printAddr(pc));

    mosLog("recently called traps (oldest first):\n");
    for (int i = 0; i < kRecentTrapSlots; i++) {
        const RecentTrap &slot = gRecentTraps[(gRecentTrapNext + i) % kRecentTrapSlots];
        if (slot.atInstruction == 0 && slot.trap == 0 && slot.callerPC == 0)
            continue;
        mosLog("  instr #%llu: 0x%04X %s, called from %s\n",
               (unsigned long long)slot.atInstruction,
               slot.trap, trapName(slot.trap), printAddr(slot.callerPC));
    }

    std::vector<std::pair<uint16_t, uint32_t>> byFrequency(gTrapFrequency.begin(), gTrapFrequency.end());
    std::sort(byFrequency.begin(), byFrequency.end(),
              [](const auto &a, const auto &b) { return a.second > b.second; });
    mosLog("most frequently called traps overall:\n");
    int shown = 0;
    for (const auto &entry : byFrequency) {
        if (shown++ >= 5)
            break;
        mosLog("  0x%04X %s: called %u times\n", entry.first, trapName(entry.first), entry.second);
    }
    mosLog("------------------------\n");
}
