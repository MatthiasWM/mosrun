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

#include "memory.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <assert.h>

// This is the emulated RAM.
byte *MosMem;

/**
 Convert a mosPtr into a host memory address.
 \param mp emulated pointer into mos memory
 \return pointer into host memory
 */
void *mosToHost(mosPtr mp)
{
    assert(mp>=0);
    assert(mp<kMosMemMax);
    return (void*)(MosMem+mp);
}

/**
 Convert a host memory pointer into a mosPtr.
 \param p pointer into host memory
 \return emulated pointer into mos memory
 */
mosPtr hostToMos(void *p)
{
    assert(p>=MosMem);
    assert(p<(MosMem+kMosMemMax));
    return (mosPtr)((byte*)(p)-MosMem);
}

const mosPtr mosMemBlockPrev = 0;
const mosPtr mosMemBlockNext = 4;
const mosPtr mosMemBlockSize = 8;
const mosPtr mosMemBlockFlags = 12;
const mosPtr mosSizeofMemBlock = 16;

const uint32_t mosMemFlagFree = 0;
const uint32_t mosMemFlagUsed = 1;
const uint32_t mosMemFlagHandles = 2;
const uint32_t mosMemFlagLast = 3;

void mosMemoryInit()
{
    MosMem = (byte*)calloc(kMosMemMax, 1);
    // create a first and a last memory managing block
    mosPtr firstBlock = 0x00001E00;
    mosPtr lastBlock = kMosMemMax - mosSizeofMemBlock;
    // the first block starts a list of all blocks of memory
    mosWrite32(firstBlock+mosMemBlockPrev, 0); // no previous block
    mosWrite32(firstBlock+mosMemBlockNext, lastBlock); // next block is the last block
    mosWrite32(firstBlock+mosMemBlockSize, lastBlock-firstBlock-mosSizeofMemBlock); // available bytes in this block
    mosWrite32(firstBlock+mosMemBlockFlags, mosMemFlagFree); // this space for rent
    // the last block marks the end of managed space
    mosWrite32(lastBlock+mosMemBlockPrev, firstBlock);
    mosWrite32(lastBlock+mosMemBlockNext, 0);
    mosWrite32(lastBlock+mosMemBlockSize, 0);
    mosWrite32(lastBlock+mosMemBlockFlags, mosMemFlagLast);
}

// FIXME: this is very buggy!
mosPtr mosMalloc(uint size)
{
    // calculate the minimal block size we need to find
    if (size==0) return 0;
    // align with 4 bytes
    uint32_t minimumBlockSize = ((size + 3) & ~0x00000003);
    // now run the list of blocks (could be otimized for speed)
    mosPtr b = 0x00001E00;
    for (;;) {
        mosPtr next = mosRead32(b+mosMemBlockNext);
        if (next==0) {
            fprintf(stderr, "MOSMalloc failed: out of memory\n");
            assert(1);
        }
        uint32_t flags = mosRead32(b+mosMemBlockFlags);
        if (flags==mosMemFlagFree) { // a free block
            uint32_t availableBlockSize = mosRead32(b+mosMemBlockSize);
            if (availableBlockSize==minimumBlockSize) {
                // the size matches; just mark it used and return its address
                mosWrite32(b+mosMemBlockFlags, mosMemFlagUsed);
                return b+mosSizeofMemBlock;
            } else if (availableBlockSize>minimumBlockSize+mosSizeofMemBlock) {
                // size is bigger than needed plus room for a new block: split this block
                mosPtr c = b + mosSizeofMemBlock + minimumBlockSize;
                // creaste the splitting new mem block
                mosWrite32(c+mosMemBlockPrev, b);
                mosWrite32(c+mosMemBlockNext, next);
                mosWrite32(c+mosMemBlockSize, next - c - mosSizeofMemBlock );
                mosWrite32(c+mosMemBlockFlags, mosMemFlagFree);
                // update the current mem block
                mosWrite32(b+mosMemBlockNext, c);
                mosWrite32(b+mosMemBlockSize, size);
                mosWrite32(b+mosMemBlockFlags, mosMemFlagUsed);
                // update the block after the splitting block
                mosWrite32(next+mosMemBlockPrev, c);
                return b+mosSizeofMemBlock;
            } else {
                // size is too small, test the next block
            }
        }
        b = next;
    }
}

void mosFree(mosPtr addr)
{
    // TODO: implement me!
}


/**
 * Allocate and copy a text string in the memory list.
 *  FIXME: make sure that 'text' is pointing to host memory, not mos memory!
 */
mosPtr mosNewPtr(const char *text)
{
    int size = strlen(text)+1;
    mosPtr mp = mosMalloc(size);
    memcpy(mosToHost(mp), text, size);
    return mp;
}


/**
 * Allocate and clear memory in the memory list.
 */
mosPtr mosNewPtr(unsigned int size)
{
    mosPtr mp = mosMalloc(size);
    memset(mosToHost(mp), 0, size);
    return mp;
}


/**
 * Free memory and unlink it from the memory list.
 */
void mosDisposePtr(mosPtr mp)
{
    if (mp==0) return;
    mosFree(mp);
}


/**
 * Get the allocated size of a memory block.
 */
unsigned int mosPtrSize(mosPtr mp)
{
    return mosRead32(mp-mosSizeofMemBlock+mosMemBlockSize);
}


/**
 * Allocate memory and a master pointer and link them into the lists.
 * FIXME: we need to keep block of handles around to make this efficient
 */
mosHandle mosNewHandle(unsigned int size)
{
    if (size==0)
        return 0;
    mosPtr mp = mosNewPtr(size);
    mosPtr mh = mosMalloc(4);
    mosWriteUnsafe32(mh, mp);
    return (mosHandle)mh;
}


/**
 * Reallocate the memory block with a new size.
 */
int mosSetHandleSize(mosHandle hdl, unsigned int newSize)
{
    // get the old allocation data
    mosPtr oldPtr = mosRead32(hdl);
    unsigned int oldSize = mosPtrSize(oldPtr);

    if (newSize==oldSize)
        return 0;

    // allocate a new block
    mosPtr newPtr = mosMalloc(newSize);
    mosWriteUnsafe32(hdl, newPtr);

    // copy the old contents over
    unsigned int size = (newSize<oldSize)?newSize:oldSize;
    memcpy(mosToHost(newPtr), mosToHost(oldPtr), size);

    // free the old allocation
    mosFree(oldPtr);

    return 0;
}


/**
 * Free memeory and its master pointer.
 */
void mosDisposeHandle(mosHandle hdl)
{
    if (!hdl) return;

    mosPtr ptr = mosRead32(hdl);
    if (ptr) {
        mosFree(ptr);
    }

    mosFree(hdl); // FIXME: handles!
}


/**
 * Find the master pointer using a memory pointer.
 *
 * \todo this function has not been verified.
 */
mosHandle mosRecoverHandle(mosPtr ptr)
{
//    MosBlock *me = gHandleList.first();
//    while (me) {
//        mosHandle hdl = mosToPtr(me);
//        if (mosRead32(hdl)==ptr)
//            return hdl;
//        me = gHandleList.next(me);
//    }
    // FIXME: mark handles so we can walk the list of handles
    return 0;
}

void mosWrite64(mosPtr addr, uintptr_t value)
{
    byte *d = (byte*)mosToHost(addr);
    *d++ = value>>56;
    *d++ = value>>48;
    *d++ = value>>40;
    *d++ = value>>32;
    *d++ = value>>24;
    *d++ = value>>16;
    *d++ = value>>8;
    *d++ = value>>0;
    //*((unsigned int*)(addr)) = htonl(value);
}

/**
 * Write anywhere into allocated RAM.
 */
void mosWriteUnsafe32(mosPtr addr, unsigned int value)
{
    byte *d = (byte*)mosToHost(addr);
    *d++ = value>>24;
    *d++ = value>>16;
    *d++ = value>>8;
    *d++ = value>>0;
    //*((unsigned int*)(addr)) = htonl(value);
}

/**
 * Verify address allocation and write into RAM.
 */
void mosWrite32(mosPtr addr, unsigned int value)
{
#if MOS_BOUNDS_CHECK
    // FIXME: implement
    if (!gHandleList.contains(addr, 4) && !gMemList.contains(addr, 4)) {
        mosError("Attempt to write 4 bytes to unknown address 0x%08X (0x%08x)!\n", addr, value);
    }
#endif
    mosWriteUnsafe32(addr, value);
}


/**
 * Write anywhere into allocated RAM.
 */
void mosWriteUnsafe16(mosPtr addr, unsigned short value)
{
    byte *d = (byte*)mosToHost(addr);
    *d++ = value>>8;
    *d++ = value>>0;
//    *((unsigned short*)(addr)) = htons(value);
}


/**
 * Verify address allocation and write into RAM.
 */
void mosWrite16(mosPtr addr, unsigned short value)
{
#if MOS_BOUNDS_CHECK
    // FIXME:
    if (!gHandleList.contains(addr, 2) && !gMemList.contains(addr, 2)) {
        mosError("Attempt to write 2 bytes to unknown address 0x%08X (0x%04x)!\n", addr, value);
    }
#endif
    mosWriteUnsafe16(addr, value);
}


/**
 * Write anywhere into allocated RAM.
 */
void mosWriteUnsafe8(mosPtr addr, unsigned char value)
{
    byte *d = (byte*)mosToHost(addr);
    *d++ = value>>0;
//    *((unsigned char*)(addr)) = value;
}


/**
 * Verify address allocation and write into RAM.
 */
void mosWrite8(mosPtr addr, unsigned char value)
{
#if MOS_BOUNDS_CHECK
    // FIXME:
    if (!gHandleList.contains(addr, 1) && !gMemList.contains(addr, 1)) {
        mosError("Attempt to write 1 byte to unknown address 0x%08X (0x%02x)!\n", addr, value);
    }
#endif
    mosWriteUnsafe8(addr, value);
}

uintptr_t mosRead64(mosPtr addr)
{
    uintptr_t v = 0;
    byte *s = (byte*)mosToHost(addr);
    v |= ((uintptr_t)(*s++))<<56;
    v |= ((uintptr_t)(*s++))<<48;
    v |= ((uintptr_t)(*s++))<<40;
    v |= ((uintptr_t)(*s++))<<32;
    v |= ((uintptr_t)(*s++))<<24;
    v |= ((uintptr_t)(*s++))<<16;
    v |= ((uintptr_t)(*s++))<<8;
    v |= ((uintptr_t)(*s++))<<0;
    return v;
}

/**
 * Read anywhere from allocated RAM.
 */
unsigned int mosReadUnsafe32(mosPtr addr)
{
    uint v = 0;
    byte *s = (byte*)mosToHost(addr);
    v |= ((uint)(*s++))<<24;
    v |= ((uint)(*s++))<<16;
    v |= ((uint)(*s++))<<8;
    v |= ((uint)(*s++))<<0;
//    return htonl(*((unsigned int*)(addr)));
    return v;
}


/**
 * Verify address allocation and read from RAM.
 */
unsigned int mosRead32(mosPtr addr)
{
#if MOS_BOUNDS_CHECK
    if (!gHandleList.contains(addr, 4) && !gMemList.contains(addr, 4)) {
        mosError("Attempt to read 4 bytes from unknown address 0x%08X!\n", addr);
    }
#endif
    return mosReadUnsafe32(addr);
}


/**
 * Read anywhere from allocated RAM.
 */
unsigned short mosReadUnsafe16(mosPtr addr)
{
    uint v = 0;
    byte *s = (byte*)mosToHost(addr);
    v |= ((uint)(*s++))<<8;
    v |= ((uint)(*s++))<<0;
//    return htons(*((unsigned short*)(addr)));
    return v;
}


/**
 * Verify address allocation and read from RAM.
 */
unsigned short mosRead16(mosPtr addr)
{
#if MOS_BOUNDS_CHECK
    if (!gHandleList.contains(addr, 2) && !gMemList.contains(addr, 2)) {
        mosError("Attempt to read 2 bytes from unknown address 0x%08X!\n", addr);
    }
#endif
    return mosReadUnsafe16(addr);
}


/**
 * Read anywhere from allocated RAM.
 */
unsigned char mosReadUnsafe8(mosPtr addr)
{
    uint v = 0;
    byte *s = (byte*)mosToHost(addr);
    v |= ((uint)(*s++))<<0;
//    return *((unsigned char*)(addr));
    return v;
}


/**
 * Verify address allocation and read from RAM.
 */
unsigned char mosRead8(mosPtr addr)
{
#if MOS_BOUNDS_CHECK
    if (!gHandleList.contains(addr, 1) && !gMemList.contains(addr, 1)) {
        mosError("Attempt to read 2 bytes from unknown address 0x%08X!\n", addr);
    }
#endif
    return mosReadUnsafe8(addr);
}


#ifdef MOS_UNITTESTS
/**
 * Run a few tests on memory lists and allocations.
 */
void mosMemoryUnittests()
{
    mosPtr p1 = mosNewPtr(100);
    mosPtr p2 = mosNewPtr(200);
    MosBlock *me = gMemList.first();
    if (mosToPtr(me)!=p1) printf("ERROR: expected p1 to be the first block\n");
    if (me->hPrev!=0L) printf("ERROR: p1->prev should be NULL\n");
    if (mosToPtr(me->hNext)!=p2) printf("ERROR: p1->next should be p2\n");
    me = gMemList.next(me);
    if (mosToPtr(me)!=p2) printf("ERROR: expected p2 to be the second block\n");
    if (mosToPtr(me->hPrev)!=p1) printf("ERROR: p2->prev should be p1\n");
    if (me->hNext!=0L) printf("ERROR: p2->next should be NULL\n");
    if (!gMemList.contains(mosToBlock(p1))) printf("ERROR: List does not contain p1\n");
    if (!gMemList.contains(mosToBlock(p2))) printf("ERROR: List does not contain p2\n");
    if (gMemList.contains(mosToBlock(42))) printf("ERROR: List should not contain 42\n");
    mosPtr p3 = mosNewPtr(200);
    mosDisposePtr(p2);
    me = gMemList.first();
    if (mosToPtr(me)!=p1) printf("ERROR: 2 - expected p1 to be the first block\n");
    if (me->hPrev!=0L) printf("ERROR: 2 - p1->prev should be NULL\n");
    if (mosToPtr(me->hNext)!=p3) printf("ERROR: 2 - p1->next should be p3\n");
    me = gMemList.next(me);
    if (mosToPtr(me)!=p3) printf("ERROR: 2 - expected p3 to be the second block\n");
    if (mosToPtr(me->hPrev)!=p1) printf("ERROR: 2 - p3->prev should be p1\n");
    if (me->hNext!=0L) printf("ERROR: p3->next should be NULL\n");
}
#endif
