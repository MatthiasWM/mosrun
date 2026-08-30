/*
 mosrun - the MacOS MPW runtime emulator
 Copyright (C) 2013-2020  Matthias Melcher

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


#ifndef __mosrun__resourcefork__
#define __mosrun__resourcefork__


#include "main.h"

#include <map>


/**
 * Byte range in mos memory occupied by one loaded 'CODE' resource, recorded
 * purely for printAddr()'s debug symbolication -- not used by the emulator
 * for anything functional.
 */
class CodeSegmentInfo {
public:
    CodeSegmentInfo(int id, mosPtr start, mosPtr end) : id(id), start(start), end(end) { }
    int id;
    mosPtr start;
    mosPtr end;
};

extern std::map<int, CodeSegmentInfo> gCodeSegments;

// The A5 world (below-A5 globals, jump table, above-A5 globals), as one
// contiguous span, set up by createA5World(). Used by trapLoadSeg (traps.cpp)
// to search for sibling jump table entries, and by printAddr() as a fallback
// label. Deliberately separate from gCodeSegments/CodeSegmentInfo -- this is
// not a CODE resource and must not share a resource ID's slot.
extern mosPtr gMosA5WorldStart;
extern mosPtr gMosA5WorldEnd;


void dumpResourceMap();
int CountResources(unsigned int myResType);
mosHandle GetResource(unsigned int myResType, unsigned short myId);
mosHandle GetNamedResource(unsigned int myResType, const byte *pName);
unsigned int createA5World(mosHandle hCode0);
void readResourceMap();
const char *printAddr(unsigned int addr);
void printPCHistory();
uint32_t codeOffsetToAddr(uint16_t code, uint32_t offset);


#endif /* defined(__mosrun__resourcefork__) */
