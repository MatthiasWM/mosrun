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


unsigned int gResourceStart[20] = { 0 };
unsigned int gResourceEnd[20] = { 0 };


/**
 * Convert a host address into segment number plus segment offset.
 */
const char *printAddr(unsigned int addr)
{
  static char buf[8][32] = { 0 };
  static int currBuf = 0;
  
  // use the next buffer
  currBuf = (currBuf+1) & 7;
  char *dst = buf[currBuf];
  
  int i = 0;
  for (i=0; i<20; i++) {
    if (addr>=gResourceStart[i] && addr<gResourceEnd[i]) {
      sprintf(dst, "%02d.%05X", i, addr-gResourceStart[i]);
      return dst;
    }
  }
  sprintf(dst, "%08X", addr);
  return dst;
}


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
        byte* rsrcName = theRsrc+rsrcMapNameList+rsrcNameOffset;
        char buf[256] = { 0 };
        memcpy(buf, rsrcName+1, rsrcName[0]);
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
            memcpy((void*)ptr, theApp+rsrcData+rsrcOffset+4, rsrcSize);
            // make the resource map point to the resource handle
            m68k_write_memory_32((unsigned int)(theRsrc+resTable+12*j+8), hdl);
            // set breakpoints
            if (myResType=='CODE') {
              if (m68k_read_memory_16((unsigned int)(theApp+rsrcData+rsrcOffset+4))==0xffff) {
                installBreakpoints(myId, (unsigned int)(ptr+4+0x24)); // 0x24
                gResourceStart[myId] = (unsigned int)(ptr+4+0x24);
                gResourceEnd[myId] = (unsigned int)(ptr+4) + rsrcSize;
              } else {
                installBreakpoints(myId, (unsigned int)(ptr+4)); // 0x24
                gResourceStart[myId] = (unsigned int)(ptr+4);
                gResourceEnd[myId] = (unsigned int)(ptr+4) + rsrcSize;
              }
              mosTrace("Resource %d from 0x%08X to 0x%08X\n", myId, gResourceStart[myId], gResourceEnd[myId]);
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
        byte* rsrcName = theRsrc+rsrcMapNameList+rsrcNameOffset;
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
            memcpy((void*)ptr, theApp+rsrcData+rsrcOffset+4, rsrcSize);
            // make the resource map point to the resource handle
            m68k_write_memory_32((unsigned int)(theRsrc+resTable+12*j+8), hdl);
            // set breakpoints
            if (myResType=='CODE') {
              if (m68k_read_memory_16((unsigned int)(theApp+rsrcData+rsrcOffset+4))==0xffff) {
                installBreakpoints(id, (unsigned int)(ptr+4+0x24)); // 0x24
                gResourceStart[id] = (unsigned int)(ptr+4+0x24);
                gResourceEnd[id] = (unsigned int)(ptr+4) + rsrcSize;
              } else {
                installBreakpoints(id, (unsigned int)(ptr+4)); // 0x24
                gResourceStart[id] = (unsigned int)(ptr+4);
                gResourceEnd[id] = (unsigned int)(ptr+4) + rsrcSize;
              }
              mosTrace("Resource %d from 0x%08X to 0x%08X\n", id, gResourceStart[id], gResourceEnd[id]);
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
  theJumpTable = (byte*)mosNewPtr(aboveA5+belowA5);
  gMosCurJTOffset = offset;
  memcpy(theJumpTable+belowA5+offset, (byte*)(code0+16), length);
  gResourceStart[19] = (unsigned int)(theJumpTable + belowA5);
  gResourceEnd[19] = (unsigned int)(theJumpTable + belowA5 + length);
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
  theRsrc = (byte*)mosNewPtr(rsrcMapSize);
  theRsrcSize = rsrcMapSize;
  memcpy(theRsrc, theApp+rsrcMap, rsrcMapSize);
  dumpResourceMap();
}


