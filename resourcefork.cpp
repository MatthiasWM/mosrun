/*
 mosrun - the MacOS MPW runtime emulator
 Copyright (C) 2013  Matthias Melcher

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
 The latest source code can be found at http://code.google.com/p/dynee5/
 */

#include "resourcefork.h"

#include "memory.h"
#include "breakpoints.h"
#include "log.h"
#include "systemram.h"

#include <string.h>

extern "C" {
    #include "musashi331/m68k.h"
    #include "musashi331/m68kcpu.h"
    #include "musashi331/m68kops.h"
}

std::vector<CodeSegmentInfo> gCodeSegments;
mosPtr gMosA5WorldStart = 0;
mosPtr gMosA5WorldEnd = 0;


/**
 * Record (or update) the byte range of a loaded 'CODE' resource, for
 * printAddr()'s debug symbolication.
 */
static void recordCodeSegment(int id, mosPtr start, mosPtr end)
{
    for (auto &seg : gCodeSegments) {
        if (seg.id==id) {
            seg.start = start;
            seg.end = end;
            return;
        }
    }
    printf("recordCodeSegment: adding new segment %d from 0x%08X to 0x%08X\n", id, start, end);
    gCodeSegments.emplace_back(id, start, end);
}


/**
 * Convert a host address into segment number plus segment offset.
 */
const char *printAddr(unsigned int addr)
{
    static char buf[8][32] = { { 0 } };
    static int currBuf = 0;

    // use the next buffer
    currBuf = (currBuf+1) & 7;
    char *dst = buf[currBuf];

    for (const auto &seg : gCodeSegments) {
        if (addr>=seg.start && addr<seg.end) {
            snprintf(dst, 31, "%02d.%05X", seg.id, addr-seg.start);
            return dst;
        }
    }
    if (gMosA5WorldStart!=gMosA5WorldEnd && addr>=gMosA5WorldStart && addr<gMosA5WorldEnd) {
        snprintf(dst, 31, "(A5).%05X", addr-gMosA5WorldStart);
        return dst;
    }
    snprintf(dst, 31, "(ERR).%08X", addr);
    return dst;
}

void printPCHistory()
{
    mosTrace("PC history:\n");
    mosDebug("   pc: 0x%08X (%s)\n", m68k_get_reg(0L, M68K_REG_PC), printAddr(m68k_get_reg(0L, M68K_REG_PC)));
    for (int i=0; i<M68K_PC_HISTORY_SIZE; i++) {
        unsigned int pc = m68k_get_pc_history(i);
        if (pc==0) break;
        mosDebug("  %3d: 0x%08X (%s)\n", i, pc, printAddr(pc));
    }
}

#define M68K_PC_HISTORY_SIZE 128
extern void m68ki_add_to_pc_history(unsigned int pc);
extern unsigned int m68k_get_pc_history(int index);


/**
 * Very crude attempt at dumping what we find in a resource map.
 *
 * \todo This function urgently needs refactoring.
 */
void dumpResourceMap()
{
    unsigned int i = 0, j = 0;
    unsigned int rsrcData = m68k_read_memory_32((unsigned int)(theApp));
    // ---- read the map
    unsigned int rsrcMapTypeList = m68k_read_memory_16((unsigned int)(theRsrc+24));
    unsigned int rsrcMapNameList = m68k_read_memory_16((unsigned int)(theRsrc+26));
    // ------ resource map type list
    unsigned int rsrcMapTypeListSize = m68k_read_memory_16((unsigned int)(theRsrc+rsrcMapTypeList)) + 1;
    mosTrace("  Rsrc Type list at 0x%08X with %d types.\n", rsrcMapTypeList, rsrcMapTypeListSize);
    for (i=0; i<rsrcMapTypeListSize; i++) {
        unsigned int nRes = m68k_read_memory_16((unsigned int)(theRsrc+rsrcMapTypeList+8*i+6)) + 1;
        unsigned int resTable = m68k_read_memory_16((unsigned int)(theRsrc+rsrcMapTypeList+8*i+8)) + rsrcMapTypeList;
        mosTrace("    %d: Rsrc type '%c%c%c%c' has %d resouces listed at 0x%08X\n",
                 i,
                 m68k_read_memory_8((unsigned int)(theRsrc+rsrcMapTypeList+8*i+2)),
                 m68k_read_memory_8((unsigned int)(theRsrc+rsrcMapTypeList+8*i+3)),
                 m68k_read_memory_8((unsigned int)(theRsrc+rsrcMapTypeList+8*i+4)),
                 m68k_read_memory_8((unsigned int)(theRsrc+rsrcMapTypeList+8*i+5)),
                 nRes, resTable
                 );
        for (j=0; j<nRes; j++) {
            unsigned int data = (m68k_read_memory_32((unsigned int)(theRsrc+resTable+12*j+4)) & 0xffffff);
            unsigned int name = m68k_read_memory_16((unsigned int)(theRsrc+resTable+12*j+2));
            mosTrace("      %3d: ID=%d, name@%d, data=0x%08x, loaded=0x%0X, flags=0x%02X, %d bytes\n", j,
                     m68k_read_memory_16((unsigned int)(theRsrc+resTable+12*j+0)),
                     name,
                     data,
                     m68k_read_memory_32((unsigned int)(theRsrc+resTable+12*j+8)),
                     m68k_read_memory_8((unsigned int)(theRsrc+resTable+12*j+4)),
                     m68k_read_memory_32((unsigned int)(theApp+rsrcData+data))
                     );
            if (name!=0xffff) {
                unsigned short rsrcNameOffset = m68k_read_memory_16((unsigned int)(theRsrc+resTable+12*j+2));
                if (rsrcNameOffset==0xffff) continue; // unnamed resource
                mosPtr rsrcName = theRsrc+rsrcMapNameList+rsrcNameOffset;
                byte rsrcNameSize = mosRead8(rsrcName);
                char buf[256];
                mosMemcpy(buf, rsrcName+1, rsrcNameSize);
                buf[rsrcNameSize] = 0;
                mosTrace("           name='%s'\n", buf);
            }
            // 0x02: write to resource file
            // 0x04: preload (but when and how?)
            // 0x08: protected
            // 0x10: locked
            // 0x20: purgeable
            // 0x40: system heap
            m68k_write_memory_32((unsigned int)(theRsrc+resTable+12*j+8), 0); // FIXME: ugly hack!
        }
    }
    //unsigned int rsrcMapNameList = m68k_read_memory_16(rsrcMap + 26);

}


/**
 * Finds and loads the given resource, and returns a handle to it
 * Resource Data in Memory:
 * Master pointer: 4 bytes
 * Size: 4 bytes
 * Content: n bytes
 *
 * \todo This function urgently needs refactoring.
 * \todo this ignores the "do not load" flag in system memory
 */
mosHandle GetResource(unsigned int myResType, unsigned short myId)
{
    unsigned int i = 0, j = 0;
    // ---- read the map
    unsigned int rsrcMapTypeList = m68k_read_memory_16((unsigned int)(theRsrc+24));
    // ------ resource map type list
    unsigned int rsrcMapTypeListSize = m68k_read_memory_16((unsigned int)(theRsrc+rsrcMapTypeList)) + 1;
    for (i=0; i<rsrcMapTypeListSize; i++) {
        unsigned int nRes = m68k_read_memory_16((unsigned int)(theRsrc+rsrcMapTypeList+8*i+6)) + 1;
        unsigned int resTable = m68k_read_memory_16((unsigned int)(theRsrc+rsrcMapTypeList+8*i+8)) + rsrcMapTypeList;
        unsigned int resType = m68k_read_memory_32((unsigned int)(theRsrc+rsrcMapTypeList+8*i+2));
        if (resType==myResType) {
            for (j=0; j<nRes; j++) {
                unsigned int id = m68k_read_memory_16((unsigned int)(theRsrc+resTable+12*j+0));
                if (id==myId) {
                    unsigned int handle = m68k_read_memory_32((unsigned int)(theRsrc+resTable+12*j+8));
                    if (handle) {
                        // resource is already in RAM
                        mosTrace("Resource already loaded\n");
                        return handle;
                    } else {
                        // resource must be copied from the file into memory
                        if (gMosResLoad==0) {
                            mosDebug("WARNING: Automatic Resource loading is disabled!\n");
                        }
                        mosTrace("Resource found, loading...\n");
                        unsigned int rsrcOffset = (m68k_read_memory_32((unsigned int)(theRsrc+resTable+12*j+4)) & 0xffffff);
                        unsigned int rsrcData = m68k_read_memory_32((unsigned int)(theApp));
                        unsigned int rsrcSize = m68k_read_memory_32((unsigned int)(theApp+rsrcData+rsrcOffset));

                        mosHandle hdl = mosNewHandle(rsrcSize);
                        mosPtr ptr = mosRead32(hdl);
                        mosMemcpy(ptr, theApp+rsrcData+rsrcOffset+4, rsrcSize);
                        // make the resource map point to the resource handle
                        m68k_write_memory_32((unsigned int)(theRsrc+resTable+12*j+8), hdl);
                        // set breakpoints
                        if (myResType=='CODE') {
                            mosPtr segStart, segEnd = (unsigned int)(ptr+4) + rsrcSize;
                            if (m68k_read_memory_16((unsigned int)(theApp+rsrcData+rsrcOffset+4))==0xffff) {
                                segStart = (unsigned int)(ptr+4+0x24);
                            } else {
                                segStart = (unsigned int)(ptr+4);
                            }
                            installBreakpoints(myId, segStart);
                            recordCodeSegment(myId, segStart, segEnd);
                            mosTrace("Resource %d from 0x%08X to 0x%08X\n", myId, segStart, segEnd);
                        }
                        return hdl;
                    }
                }
            }
        }
    }
    mosDebug("ERROR: Resource '%c%c%c%c', ID %d not found!\n",
             myResType>>24, myResType>>16, myResType>>8, myResType, myId);
    return 0;
}


/**
 * Finds and loads the given resource, and returns a handle to it
 *
 * \todo This function urgently needs refactoring.
 * \todo this ignores the "do not load" flag in system memory
 */
mosHandle GetNamedResource(unsigned int myResType, const byte *pName)
{
    unsigned int i = 0, j = 0;
    // ---- read the map
    unsigned int rsrcMapTypeList = m68k_read_memory_16((unsigned int)(theRsrc+24));
    unsigned int rsrcMapNameList = m68k_read_memory_16((unsigned int)(theRsrc+26));
    // ------ resource map type list
    unsigned int rsrcMapTypeListSize = m68k_read_memory_16((unsigned int)(theRsrc+rsrcMapTypeList)) + 1;
    for (i=0; i<rsrcMapTypeListSize; i++) {
        unsigned int nRes = m68k_read_memory_16((unsigned int)(theRsrc+rsrcMapTypeList+8*i+6)) + 1;
        unsigned int resTable = m68k_read_memory_16((unsigned int)(theRsrc+rsrcMapTypeList+8*i+8)) + rsrcMapTypeList;
        unsigned int resType = m68k_read_memory_32((unsigned int)(theRsrc+rsrcMapTypeList+8*i+2));
        if (resType==myResType) {
            for (j=0; j<nRes; j++) {
                unsigned short rsrcNameOffset = m68k_read_memory_16((unsigned int)(theRsrc+resTable+12*j+2));
                if (rsrcNameOffset==0xffff) continue; // unnamed resource
                byte* rsrcName = (byte*)mosToHost(theRsrc+rsrcMapNameList+rsrcNameOffset);
                mosTrace("%*s\n", rsrcName[0], rsrcName+1);
                if (memcmp(pName, rsrcName, pName[0]+1)==0) {
                    unsigned int handle = m68k_read_memory_32((unsigned int)(theRsrc+resTable+12*j+8));
                    unsigned int id = m68k_read_memory_16((unsigned int)(theRsrc+resTable+12*j+0));
                    if (handle) {
                        // resource is already in RAM
                        mosTrace("Resource already loaded\n");
                        return handle;
                    } else {
                        // resource must be copied from the file into memory
                        if (gMosResLoad==0) {
                            mosDebug("WARNING: Automatic Resource loading is disabled!\n");
                        }
                        mosTrace("Resource found, loading...\n");
                        unsigned int rsrcOffset = (m68k_read_memory_32((unsigned int)(theRsrc+resTable+12*j+4)) & 0xffffff);
                        unsigned int rsrcData = m68k_read_memory_32((unsigned int)(theApp));
                        unsigned int rsrcSize = m68k_read_memory_32((unsigned int)(theApp+rsrcData+rsrcOffset));

                        mosHandle hdl = mosNewHandle(rsrcSize);
                        mosPtr ptr = mosRead32(hdl);
                        mosMemcpy(ptr, theApp+rsrcData+rsrcOffset+4, rsrcSize);
                        // make the resource map point to the resource handle
                        m68k_write_memory_32((unsigned int)(theRsrc+resTable+12*j+8), hdl);
                        // set breakpoints
                        if (myResType=='CODE') {
                            mosPtr segStart, segEnd = (unsigned int)(ptr+4) + rsrcSize;
                            if (m68k_read_memory_16((unsigned int)(theApp+rsrcData+rsrcOffset+4))==0xffff) {
                                segStart = (unsigned int)(ptr+4+0x24);
                            } else {
                                segStart = (unsigned int)(ptr+4);
                            }
                            installBreakpoints(id, segStart);
                            recordCodeSegment(id, segStart, segEnd);
                            mosTrace("Resource %d from 0x%08X to 0x%08X\n", id, segStart, segEnd);
                        }
                        return hdl;
                    }
                }
            }
        }
    }
    mosDebug("ERROR: Resource '%c%c%c%c', name '%s' not found!\n",
             myResType>>24, myResType>>16, myResType>>8, myResType, pName);
    return 0;
}


/**
 * Create a segment of memory that hold global variables and jump tables.
 *
 * \todo Please document this better.
 */
unsigned int createA5World(mosHandle hCode0)
{
    // dereference the handle
    unsigned int code0 = m68k_read_memory_32(hCode0);
    // create jump table and space for the app global variables
    unsigned int aboveA5 = m68k_read_memory_32(code0 +  0);
    unsigned int belowA5 = m68k_read_memory_32(code0 +  4);
    unsigned int length  = m68k_read_memory_32(code0 +  8);
    unsigned int offset  = m68k_read_memory_32(code0 + 12);
    // create jump table
    theJumpTable = mosNewPtr(aboveA5+belowA5);
    gMosCurJTOffset = offset;
    mosMemcpy(theJumpTable+belowA5+offset, code0+16, length);
    // Covers the whole allocated A5 world (below-A5 globals, the jump table,
    // and above-A5 globals), not just the length of CODE 0's own initial
    // jump table data -- CODE 0 typically only pre-populates a handful of
    // entries (often just the segment that performs the app's real global/
    // jump-table initialization at startup); the rest of the jump table
    // ends up populated later, elsewhere in this same block. Used both for
    // printAddr's debug symbolication and, more importantly, as the search
    // range trapLoadSeg (traps.cpp) scans to find sibling jump table entries
    // for a segment that's just been loaded. A dedicated pair of globals,
    // not a CodeSegmentInfo entry -- this isn't a CODE resource and must
    // not collide with one's ID.
    gMosA5WorldStart = (unsigned int)(theJumpTable);
    gMosA5WorldEnd = (unsigned int)(theJumpTable + aboveA5 + belowA5);
    return (unsigned int)(theJumpTable + belowA5);
}


/**
 * Copy the resource map into a different place in RAM.
 *
 * \todo This function urgently needs refactoring.
 * \todo Resource maps are set up to be manipulated "in situ". Kick this out.
 */
void readResourceMap()
{
    unsigned int rsrcMap = m68k_read_memory_32((unsigned int)(theApp+4));
    unsigned int rsrcMapSize = m68k_read_memory_32((unsigned int)(theApp+12));
    mosTrace("Rsrc Map %d bytes at 0x%08X\n", rsrcMapSize, rsrcMap);
    theRsrc = mosNewPtr(rsrcMapSize);
    theRsrcSize = rsrcMapSize;
    mosMemcpy(theRsrc, theApp+rsrcMap, rsrcMapSize);
    dumpResourceMap();
}


