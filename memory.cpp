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

//#define MOS_CHECK_MEMORY_COHERENCE mosCheckMemoryCoherence();
#define MOS_CHECK_MEMORY_COHERENCE
//#define MOS_TRACE_MEMORY(a) a
#define MOS_TRACE_MEMORY(a)

#include "memory.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <arpa/inet.h>
#endif
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

const mosPtr mosMemBlockPrev    = 0;
const mosPtr mosMemBlockNext    = 4;
const mosPtr mosMemBlockSize    = 8;
const mosPtr mosMemBlockFlags   = 12;
const mosPtr mosSizeofMemBlock  = 16;

const uint32_t mosMemFlagMagic      = 0xbeef0000;
const uint32_t mosMemFlagMagicMask  = 0xffff0000;
const uint32_t mosMemFlagFree       = mosMemFlagMagic | 0;
const uint32_t mosMemFlagUsed       = mosMemFlagMagic | 1;
const uint32_t mosMemFlagHandles    = mosMemFlagMagic | 2;
const uint32_t mosMemFlagLast       = mosMemFlagMagic | 3;

const mosPtr mosMemBlockStart = kSystemHeapStart;

void mosMemoryInit()
{
    MosMem = (byte*)calloc(kMosMemMax, 1);
    // create a first and a last memory managing block
    mosPtr firstBlock = mosMemBlockStart;
    mosPtr lastBlock = kMosMemMax - mosSizeofMemBlock;
    // the first block starts a list of all blocks of memory
    mosWriteUnsafe32(firstBlock+mosMemBlockPrev, 0); // no previous block
    mosWriteUnsafe32(firstBlock+mosMemBlockNext, lastBlock); // next block is the last block
    mosWriteUnsafe32(firstBlock+mosMemBlockSize, lastBlock-firstBlock-mosSizeofMemBlock); // available bytes in this block
    mosWriteUnsafe32(firstBlock+mosMemBlockFlags, mosMemFlagFree); // this space for rent
    // the last block marks the end of managed space
    mosWriteUnsafe32(lastBlock+mosMemBlockPrev, firstBlock);
    mosWriteUnsafe32(lastBlock+mosMemBlockNext, 0);
    mosWriteUnsafe32(lastBlock+mosMemBlockSize, 0);
    mosWriteUnsafe32(lastBlock+mosMemBlockFlags, mosMemFlagLast);

    mosCheckMemoryCoherence();
}

mosPtr mosMalloc(uint size)
{
    // calculate the minimal block size we need to find
    if (size==0) return 0;
    MOS_CHECK_MEMORY_COHERENCE
    // align with 4 bytes
    uint32_t minimumBlockSize = ((size + 3) & ~0x00000003);
    // now run the list of blocks (could be otimized for speed)
    mosPtr b = mosMemBlockStart;
    for (;;) {
        mosPtr next = mosReadUnsafe32(b+mosMemBlockNext);
        if (next==0) {
            fprintf(stderr, "MOSMalloc failed: out of memory\n");
            assert(1);
        }
        uint32_t flags = mosReadUnsafe32(b+mosMemBlockFlags);
        if (flags==mosMemFlagFree) { // a free block
            uint32_t availableBlockSize = mosReadUnsafe32(b+mosMemBlockSize);
            if (availableBlockSize==minimumBlockSize) {
                // the size matches; just mark it used and return its address
                mosWriteUnsafe32(b+mosMemBlockFlags, mosMemFlagUsed);
                MOS_TRACE_MEMORY( printf("-- malloc match at 0x%08X, n=%d\n", b+mosSizeofMemBlock, size); )
                MOS_CHECK_MEMORY_COHERENCE
                return b+mosSizeofMemBlock;
            } else if (availableBlockSize>minimumBlockSize+mosSizeofMemBlock) {
                // size is bigger than needed plus room for a new block: split this block
                mosPtr c = b + mosSizeofMemBlock + minimumBlockSize;
                // creaste the splitting new mem block
                mosWriteUnsafe32(c+mosMemBlockPrev, b);
                mosWriteUnsafe32(c+mosMemBlockNext, next);
                mosWriteUnsafe32(c+mosMemBlockSize, next - c - mosSizeofMemBlock );
                mosWriteUnsafe32(c+mosMemBlockFlags, mosMemFlagFree);
                // update the current mem block
                mosWriteUnsafe32(b+mosMemBlockNext, c);
                mosWriteUnsafe32(b+mosMemBlockSize, size);
                mosWriteUnsafe32(b+mosMemBlockFlags, mosMemFlagUsed);
                // update the block after the splitting block
                mosWriteUnsafe32(next+mosMemBlockPrev, c);
                MOS_TRACE_MEMORY( printf("-- malloc split at 0x%08X, n=%d\n", b+mosSizeofMemBlock, size); )
                MOS_CHECK_MEMORY_COHERENCE
                return b+mosSizeofMemBlock;
            } else {
                // size is too small, test the next block
            }
        }
        b = next;
    }
}

void mosJoinBlocks(mosPtr b)
{
    if (!b) return;
    uint32_t bFlags = mosReadUnsafe32(b+mosMemBlockFlags);
    if ( (bFlags&mosMemFlagMagicMask) != mosMemFlagMagic) {
        mosError("mosJoinBlocks received an illegal block at 0x%08X\n", b+mosSizeofMemBlock);
        assert(0);
    }
    if (bFlags!=mosMemFlagFree) return;

    mosPtr c = mosReadUnsafe32(b+mosMemBlockNext);
    if (!c) return;
    uint32_t cFlags = mosReadUnsafe32(c+mosMemBlockFlags);
    if ( (cFlags&mosMemFlagMagicMask) != mosMemFlagMagic) {
        mosError("mosJoinBlocks found an illegal block at 0x%08X\n", c+mosSizeofMemBlock);
        assert(0);
    }
    if (cFlags!=mosMemFlagFree) return;

    mosPtr d = mosReadUnsafe32(c+mosMemBlockNext);
    if (!d) return;
    uint32_t dFlags = mosReadUnsafe32(d+mosMemBlockFlags);
    if ( (dFlags&mosMemFlagMagicMask) != mosMemFlagMagic) {
        mosError("mosJoinBlocks found an illegal block at 0x%08X\n", d+mosSizeofMemBlock);
        assert(0);
    }

    // ok, so both blocks exist, are connected, and are free, so joint them
    mosWriteUnsafe32(d+mosMemBlockPrev, b);
    mosWriteUnsafe32(b+mosMemBlockNext, d);
    mosWriteUnsafe32(b+mosMemBlockSize, d-b-mosSizeofMemBlock);
    // clear remainders of the memory block that we removed
    mosWriteUnsafe32(c+mosMemBlockPrev, 0);
    mosWriteUnsafe32(c+mosMemBlockNext, 0);
    mosWriteUnsafe32(c+mosMemBlockSize, 0);
    mosWriteUnsafe32(c+mosMemBlockFlags, 0);
    MOS_TRACE_MEMORY( printf("-- malloc join 0x%08X - 0x%08X - 0x%08X\n", b+mosSizeofMemBlock, c+mosSizeofMemBlock, d+mosSizeofMemBlock); )
}

void mosFree(mosPtr addr)
{
    MOS_TRACE_MEMORY( printf("-- malloc free at 0x%08X\n", addr); )
    MOS_CHECK_MEMORY_COHERENCE
    mosPtr b = addr - mosSizeofMemBlock;
    mosPtr next = mosReadUnsafe32(b+mosMemBlockNext);
    uint32_t flags = mosReadUnsafe32(b+mosMemBlockFlags);
    if ( (flags&mosMemFlagMagicMask) != mosMemFlagMagic) {
        mosError("mosFree is trying to free a block at an invalid address 0x%08X\n", addr);
        assert(0);
    }
    if (flags==mosMemFlagFree) {
        mosError("mosFree is trying to free a block a second time at address 0x%08X\n", addr);
        assert(0);
    }
    if (flags!=mosMemFlagUsed && flags!=mosMemFlagHandles) {
        mosError("mosFree is trying to free a block that was not allocated at 0x%08X\n", addr);
        assert(0);
    }
    mosWriteUnsafe32(b+mosMemBlockFlags, mosMemFlagFree);
    mosWriteUnsafe32(b+mosMemBlockSize, next-b-mosSizeofMemBlock);
    // optimize: if the next block is free, join both blocks
    mosJoinBlocks(b);
    // optimize: if the previous block is free, join both blocks
    mosPtr prev = mosReadUnsafe32(b+mosMemBlockPrev);
    mosJoinBlocks(prev);
    MOS_CHECK_MEMORY_COHERENCE
}

/**
 Check if it is leagal to access the given range of memory.

 Check if the mos address is within physical range. This will fail if the memory block
 is not marked as free or if the memory block touches memeory management data.

 \param address check access starting from this address
 \param size check this byte range
 \return true, if access is allowed for the entire range
 \return false, if access is invalid, and if the flags are set, output a diagnostic message
 */
bool mosCheckMemoryAccess(mosPtr address, uint32_t size)
{
    mosPtr first = address, last = address+size-1;
    mosPtr b = mosMemBlockStart;
    if (first<b || last>=kMosMemMax) {
        mosWarning("mosCheckMemoryAccess: address range 0x%08X to 0x%08X not within managed RAM.\n"
                   "  Legal memory range is 0x%08X to 0x%08X\n", mosMemBlockStart, kMosMemMax-1);
        return false;
    }
    for (;;) {
        mosPtr next = mosReadUnsafe32(b+mosMemBlockNext);
        if (next>=last) {
            uint32_t flags = mosReadUnsafe32(b+mosMemBlockFlags);
            if (flags==mosMemFlagHandles || flags==mosMemFlagUsed) {
                uint32_t bSize = mosReadUnsafe32(b+mosMemBlockSize);
                mosPtr bFirst = b+mosSizeofMemBlock, bLast = bFirst+bSize-1;
                if (bFirst<=first && bLast>=last)
                    return true;
            }
        }
        if (next==0) {
            mosWarning("mosCheckMemoryAccess: memory allocation for address range 0x%08X to 0x%08X not found.\n", first, last);
            return false;
        }
        if (next>first) {
            uint32_t bSize = mosReadUnsafe32(b+mosMemBlockSize);
            uint32_t nSize = mosReadUnsafe32(next+mosMemBlockSize);
            mosWarning("mosCheckMemoryAccess: memory allocation for address range 0x%08X to 0x%08X not found.\n"
                       "  Closest allocations are 0x%08X to 0x%08X and 0x%08X to 0x%08X\n",
                       first, last,
                       b+mosSizeofMemBlock, b+mosSizeofMemBlock+bSize-1,
                       next+mosSizeofMemBlock, next+mosSizeofMemBlock+nSize-1);
            if (next>first)
            return false;
        }
        b = next;
    }
}

/**
 Check if all the memory management structures are linked correctly.

 \return true, if everything is ok
 */
bool mosCheckMemoryCoherence()
{
    mosPtr b = mosMemBlockStart;
    if (mosReadUnsafe32(b+mosMemBlockPrev)!=0)
        mosError("mosCheckMemoryCoherency: firstBlock.prev is not NULL!\n");
    if (mosReadUnsafe32(b+mosMemBlockNext)==0)
        mosError("mosCheckMemoryCoherency: firstBlock.next must not be NULL!\n");
    for (;;) {
        mosPtr next = mosReadUnsafe32(b+mosMemBlockNext);
        if (next==0) break; // this must be the final mem block
        if (mosReadUnsafe32(next+mosMemBlockPrev)!=b)
            mosError("mosCheckMemoryCoherency: block.next.first must point back at block!\n");
        uint32_t bFlags = mosReadUnsafe32(b+mosMemBlockFlags);
        if ( (bFlags&mosMemFlagMagicMask) != mosMemFlagMagic)
            mosError("mosCheckMemoryCoherency: missing magic value at 0x%08X\n", b+mosSizeofMemBlock);
        uint32_t nFlags = mosReadUnsafe32(next+mosMemBlockFlags);
        if (bFlags==mosMemFlagFree && nFlags==mosMemFlagFree)
            mosError("mosCheckMemoryCoherency: a free block must not be followed by another free block!\n");
        uint32_t bSize = mosReadUnsafe32(b+mosMemBlockSize);
        if (bFlags==mosMemFlagFree && b+mosSizeofMemBlock+bSize!=next)
            mosError("mosCheckMemoryCoherency: free block has illegal block size!\n");
        if (bFlags==mosMemFlagUsed && b+mosSizeofMemBlock+bSize>next)
            mosError("mosCheckMemoryCoherency: used block has illegal block size!\n");
        if (bFlags==mosMemFlagHandles && (b+mosSizeofMemBlock+bSize!=next || bSize!=4))
            mosError("mosCheckMemoryCoherency: handle block has illegal block size!\n");
        b = next;
    }
    if (mosReadUnsafe32(b+mosMemBlockFlags)!=mosMemFlagLast)
        mosError("mosCheckMemoryCoherency: lastBlock.flagsa must indicate last block\n");
    if (b!=kMosMemMax-mosSizeofMemBlock)
        mosError("mosCheckMemoryCoherency: lastBlock at unexpected address\n");
    return true;
}

void mosMemcpy(mosPtr dst, mosPtr src, uint32_t n)
{
    if (gCheckMemory && !mosCheckMemoryAccess(src, n)) {
        mosWarning("Attempt to read %n bytes from illegal address 0x%08X!\n", n, src);
        assert(gCheckMemory<2);
    }
    void *hostSrc = mosToHost(src);

    if (gCheckMemory && !mosCheckMemoryAccess(dst, n)) {
        mosWarning("Attempt to write %n bytes to illegal address 0x%08X!\n", n, dst);
        assert(gCheckMemory<2);
    }
    void *hostDst = mosToHost(dst);

    memcpy(hostDst, hostSrc, n);
}

void mosMemcpy(void *hostDst, mosPtr src, uint32_t n)
{
    if (gCheckMemory && !mosCheckMemoryAccess(src, n)) {
        mosWarning("Attempt to read %n bytes from illegal address 0x%08X!\n", n, src);
        assert(gCheckMemory<2);
    }
    void *hostSrc = mosToHost(src);

    memcpy(hostDst, hostSrc, n);
}

void mosMemcpy(mosPtr dst, const void *hostSrc, uint32_t n)
{
    if (gCheckMemory && !mosCheckMemoryAccess(dst, n)) {
        mosWarning("Attempt to write %n bytes to illegal address 0x%08X!\n", n, dst);
        assert(gCheckMemory<2);
    }
    void *hostDst = mosToHost(dst);

    memcpy(hostDst, hostSrc, n);
}

/**
 * Allocate and copy a text string from host memory to mos.
 */
mosPtr mosNewPtr(const char *text)
{
    uint32_t size = strlen(text)+1;
    mosPtr mp = mosMalloc(size);
    mosMemcpy(mp, text, size);
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
    return mosReadUnsafe32(mp-mosSizeofMemBlock+mosMemBlockSize);
}


/**
 * Allocate memory and a master pointer and link them into the lists.
 * \note We could manage handles in blocks to make this more efficient.
 */
mosHandle mosNewHandle(unsigned int size)
{
    if (size==0)
        return 0;
    mosPtr mp = mosNewPtr(size);
    mosPtr mh = mosMalloc(4);
    mosWriteUnsafe32(mh-mosSizeofMemBlock+mosMemBlockFlags, mosMemFlagHandles);
    mosWrite32(mh, mp);
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
    mosWrite32(hdl, newPtr);

    // copy the old contents over
    unsigned int size = (newSize<oldSize)?newSize:oldSize;
    mosMemcpy(newPtr, oldPtr, size);

    // free the old allocation
    mosFree(oldPtr);

    return 0;
}


/**
 * Free memory and its master pointer.
 */
void mosDisposeHandle(mosHandle hdl)
{
    if (!hdl) return;

    mosPtr ptr = mosRead32(hdl);
    if (ptr) {
        mosFree(ptr);
    }

    mosFree(hdl);
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

void mosWriteUnsafe64(mosPtr addr, uintptr_t value)
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
    if (gCheckMemory && !mosCheckMemoryAccess(addr, 4)) {
        mosWarning("Attempt to write 4 bytes to illegal address 0x%08X!\n", addr);
        assert(gCheckMemory<2);
    }
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
    if (gCheckMemory && !mosCheckMemoryAccess(addr, 2)) {
        mosWarning("Attempt to write 2 bytes to illegal address 0x%08X!\n", addr);
        assert(gCheckMemory<2);
    }
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
    if (gCheckMemory && !mosCheckMemoryAccess(addr, 1)) {
        mosWarning("Attempt to write 1 byte to illegal address 0x%08X!\n", addr);
        assert(gCheckMemory<2);
    }
    mosWriteUnsafe8(addr, value);
}

uintptr_t mosReadUnsafe64(mosPtr addr)
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
    if (gCheckMemory && !mosCheckMemoryAccess(addr, 4)) {
        mosWarning("Attempt to read 4 bytes from illegal address 0x%08X!\n", addr);
        assert(gCheckMemory<2);
    }
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
    if (gCheckMemory && !mosCheckMemoryAccess(addr, 2)) {
        mosWarning("Attempt to read 2 bytes from illegal address 0x%08X!\n", addr);
        assert(gCheckMemory<2);
    }
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
    if (gCheckMemory && !mosCheckMemoryAccess(addr, 1)) {
        mosWarning("Attempt to read 1 byte from illegal address 0x%08X!\n", addr);
        assert(gCheckMemory<2);
    }
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
