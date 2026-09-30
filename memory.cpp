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

#ifdef NDEBUG
#define MOS_CHECK_MEMORY_COHERENCE
#define MOS_TRACE_MEMORY(a)
#else
#define MOS_CHECK_MEMORY_COHERENCE \
    if (mosCheckMemoryCoherence()==false) {\
        mosDebugPrintPCHistory(32);\
        debug_break();\
    }
#define MOS_TRACE_MEMORY(a) a
#endif

#include "memory.h"
#include "log.h"
#include "debug.h"

#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <arpa/inet.h>
#endif
#include <assert.h>


// Handle:
// 8 bytes
// 0: 4 bytes: ptr to user memory
// 4: 2 bytes: flags
// 6: 2 bytes: lock count


// This is the emulated RAM.
byte *MosMem;

/**
 Convert a mosPtr into a host memory address.
 \param mp emulated pointer into mos memory
 \return pointer into host memory
 */
void *mosToHost(mosPtr mp)
{
    static bool handling_error = false;
    if (handling_error) {
        return MosMem; // Ouch!
    }
    if (mp >= kMosMemMax) {
        handling_error = true;
        mosDebugPrintPCHistory();
        fprintf(stderr, "mosToHost: pointer out of bounds: 0x%08X\n", mp);
        debug_break();
    }
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

/**
 Walk the heap block list and report total free space and the size of the
 largest contiguous free block, for traps like PurgeSpace/FreeMem/MaxMem.
 */
void mosFreeMemInfo(unsigned int *outTotal, unsigned int *outContig)
{
    unsigned int total = 0, contig = 0;
    mosPtr b = mosMemBlockStart;
    for (;;) {
        uint32_t flags = mosReadUnsafe32(b+mosMemBlockFlags);
        if (flags==mosMemFlagFree) {
            uint32_t size = mosReadUnsafe32(b+mosMemBlockSize);
            total += size;
            if (size>contig) contig = size;
        }
        mosPtr next = mosReadUnsafe32(b+mosMemBlockNext);
        if (next==0) break; // reached the last block
        b = next;
    }
    if (outTotal) *outTotal = total;
    if (outContig) *outContig = contig;
}


mosPtr mosMalloc(uint size)
{
    // A request for 0 bytes still gets a real, unique, non-NIL pointer to a
    // zero-length block -- that's what NewPtr(0)/NewHandle(0) do on real
    // Mac OS, so callers can dereference it (nothing to read, but it's not
    // a crash) and later grow it with SetHandleSize/SetPtrSize.
    MOS_CHECK_MEMORY_COHERENCE
    // align with 4 bytes
    uint32_t minimumBlockSize = ((size + 3) & ~0x00000003);
    // now run the list of blocks (could be otimized for speed)
    mosPtr b = mosMemBlockStart;
    for (;;) {
        mosPtr next = mosReadUnsafe32(b+mosMemBlockNext);
        if (next==0) {
            mosCheckMemoryCoherence();
            fprintf(stderr, "MOSMalloc failed: out of memory\n");
            assert(0);
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

/**
 Allocate memory as high in the emulated heap as possible.

 Like mosMalloc, but instead of taking the *first* free block that fits, it
 scans the whole free list and uses the *last* (i.e. highest-addressed) free
 block that fits, then carves the new block off the high-address end of that
 free block rather than the low end -- so the returned pointer sits as close
 to the top of RAM as the current free space allows. Any leftover free space
 stays behind at the low-address side of that same free block, so the free
 list's node for it does not move.

 Intended for things like MoveHHi, where real Mac OS relocates a block to
 the top of the heap zone to keep it out of the way of the bulk of the heap
 that grows from the bottom.
 */
mosPtr mosMallocHigh(uint size)
{
    MOS_CHECK_MEMORY_COHERENCE
    // align with 4 bytes
    uint32_t minimumBlockSize = ((size + 3) & ~0x00000003);

    // find the highest-addressed free block that is big enough
    mosPtr best = 0;
    mosPtr b = mosMemBlockStart;
    for (;;) {
        mosPtr next = mosReadUnsafe32(b+mosMemBlockNext);
        if (next==0) break; // reached the last block
        uint32_t flags = mosReadUnsafe32(b+mosMemBlockFlags);
        if (flags==mosMemFlagFree) {
            uint32_t availableBlockSize = mosReadUnsafe32(b+mosMemBlockSize);
            if (availableBlockSize==minimumBlockSize
                || availableBlockSize>minimumBlockSize+mosSizeofMemBlock) {
                best = b; // keep looking -- a later (higher) match wins
            }
        }
        b = next;
    }

    if (!best) {
        fprintf(stderr, "mosMallocHigh failed: out of memory\n");
        assert(0);
    }

    uint32_t availableBlockSize = mosReadUnsafe32(best+mosMemBlockSize);
    mosPtr next = mosReadUnsafe32(best+mosMemBlockNext);

    if (availableBlockSize==minimumBlockSize) {
        // the size matches exactly; just mark it used and return its address
        mosWriteUnsafe32(best+mosMemBlockFlags, mosMemFlagUsed);
        MOS_TRACE_MEMORY( printf("-- mallocHigh match at 0x%08X, n=%d\n", best+mosSizeofMemBlock, size); )
        MOS_CHECK_MEMORY_COHERENCE
        return best+mosSizeofMemBlock;
    } else {
        // carve the new block off the high end of the free block: the new
        // block's header sits right before `next`, and `best` shrinks in
        // place to cover whatever remains at the low end
        mosPtr c = next - mosSizeofMemBlock - minimumBlockSize;
        mosWriteUnsafe32(c+mosMemBlockPrev, best);
        mosWriteUnsafe32(c+mosMemBlockNext, next);
        mosWriteUnsafe32(c+mosMemBlockSize, size);
        mosWriteUnsafe32(c+mosMemBlockFlags, mosMemFlagUsed);
        // shrink the original free block to end where the new block begins
        mosWriteUnsafe32(best+mosMemBlockNext, c);
        mosWriteUnsafe32(best+mosMemBlockSize, c-best-mosSizeofMemBlock);
        // update the block after the new block
        mosWriteUnsafe32(next+mosMemBlockPrev, c);
        MOS_TRACE_MEMORY( printf("-- mallocHigh split at 0x%08X, n=%d\n", c+mosSizeofMemBlock, size); )
        MOS_CHECK_MEMORY_COHERENCE
        return c+mosSizeofMemBlock;
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
bool mosCheckMemoryAccess(mosPtr address, uint32_t size, bool verbose)
{
    mosPtr first = address, last = address+size-1;
    mosPtr b = mosMemBlockStart;
    if (first<b || last>=kMosMemMax) {
        if (verbose) {
            mosWarning("mosCheckMemoryAccess: address range 0x%08X to 0x%08X not within managed RAM.\n"
                       "  Legal memory range is 0x%08X to 0x%08X\n",
                       first, last,
                       mosMemBlockStart, kMosMemMax-1);
        }
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
            if (verbose) {
                mosWarning("mosCheckMemoryAccess: memory allocation for address range 0x%08X to 0x%08X not found.\n", first, last);
            }
            return false;
        }
        if (next>first) {
            uint32_t bSize = mosReadUnsafe32(b+mosMemBlockSize);
            uint32_t nSize = mosReadUnsafe32(next+mosMemBlockSize);
            if (verbose) {
                mosWarning("mosCheckMemoryAccess: memory allocation for address range 0x%08X to 0x%08X not found.\n"
                           "  Closest allocations are 0x%08X to 0x%08X and 0x%08X to 0x%08X\n",
                            first, last,
                            b+mosSizeofMemBlock, b+mosSizeofMemBlock+bSize-1,
                            next+mosSizeofMemBlock, next+mosSizeofMemBlock+nSize-1);
            }
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
    bool ret = true;
    mosPtr b = mosMemBlockStart;
    if (mosReadUnsafe32(b+mosMemBlockPrev)!=0)
        mosError("mosCheckMemoryCoherency: firstBlock.prev is not NULL!\n"), ret = false;
    if (mosReadUnsafe32(b+mosMemBlockNext)==0)
        mosError("mosCheckMemoryCoherency: firstBlock.next must not be NULL!\n"), ret = false;
    for (;;) {
        mosPtr next = mosReadUnsafe32(b+mosMemBlockNext);
        if (next==0) break; // this must be the final mem block
        if (mosReadUnsafe32(next+mosMemBlockPrev)!=b)
            mosError("mosCheckMemoryCoherency: block.next.first must point back at block!\n"), ret = false;
        uint32_t bFlags = mosReadUnsafe32(b+mosMemBlockFlags);
        if ( (bFlags&mosMemFlagMagicMask) != mosMemFlagMagic)
            mosError("mosCheckMemoryCoherency: missing magic value at 0x%08X\n", b+mosSizeofMemBlock), ret = false;
        uint32_t nFlags = mosReadUnsafe32(next+mosMemBlockFlags);
        if (bFlags==mosMemFlagFree && nFlags==mosMemFlagFree)
            mosError("mosCheckMemoryCoherency: a free block must not be followed by another free block!\n"), ret = false;
        uint32_t bSize = mosReadUnsafe32(b+mosMemBlockSize);
        if (bFlags==mosMemFlagFree && b+mosSizeofMemBlock+bSize!=next)
            mosError("mosCheckMemoryCoherency: free block has illegal block size!\n"), ret = false;
        if (bFlags==mosMemFlagUsed && b+mosSizeofMemBlock+bSize>next)
            mosError("mosCheckMemoryCoherency: used block has illegal block size!\n"), ret = false;
        if (bFlags==mosMemFlagHandles && (b+mosSizeofMemBlock+bSize!=next || bSize!=8))
            mosError("mosCheckMemoryCoherency: handle block has illegal block size!\n"), ret = false;
        b = next;
    }
    if (mosReadUnsafe32(b+mosMemBlockFlags)!=mosMemFlagLast)
        mosError("mosCheckMemoryCoherency: lastBlock.flagsa must indicate last block\n"), ret = false;
    if (b!=kMosMemMax-mosSizeofMemBlock)
        mosError("mosCheckMemoryCoherency: lastBlock at unexpected address\n"), ret = false;
    return ret;
}

void mosMemcpy(mosPtr dst, mosPtr src, uint32_t n)
{
    if (n==0) return; // avoid a bogus range check below (address+0-1 underflows)
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
    if (n==0) return;
    if (gCheckMemory && !mosCheckMemoryAccess(src, n)) {
        mosWarning("Attempt to read %n bytes from illegal address 0x%08X!\n", n, src);
        assert(gCheckMemory<2);
    }
    void *hostSrc = mosToHost(src);

    memcpy(hostDst, hostSrc, n);
}

void mosMemcpy(mosPtr dst, const void *hostSrc, uint32_t n)
{
    if (n==0) return;
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
    if (mp==0) return 0;
    return mosReadUnsafe32(mp-mosSizeofMemBlock+mosMemBlockSize);
}


/**
 * Allocate memory and a master pointer and link them into the lists.
 * In the original code, we have flags for sys memory and clear memory.
 * \note We could manage handles in blocks to make this more efficient.
 */
mosHandle mosNewHandle(unsigned int size)
{
    MOS_TRACE_MEMORY( printf("NewHandle(%u)\n", size); )
    // NewHandle(0) is legal and common (e.g. as a starting point before a
    // series of SetHandleSize calls). Unlike NewPtr(0) -- which has no
    // indirection to fall back on and so must return a real (if useless)
    // non-NIL address -- a Handle already has a stable identity separate
    // from the data it points to, so we represent "empty" the same way
    // EmptyHandle() does: a valid, non-NIL handle whose master pointer is
    // NIL. GetHandleSize/DisposeHandle already guard on `if (ptr)` before
    // touching the target, so a NIL-pointer handle is a state this code
    // already treats as first-class. The handle's state byte (see
    // mosHGetState) is unrelated to size either way -- it only tracks
    // lock/purge/resource bits, all cleared here exactly like any other
    // freshly allocated handle.

    mosPtr mp = size ? mosNewPtr(size) : 0;
    // mosPtr mp = mosNewPtr(size);
    if (size && !mp) {
        return 0;
    }

    mosPtr mh = mosMalloc(8);
    if (!mh) {
        mosFree(mp);
        return 0;
    }

    // Declare the block as a Handle for the memory manager
    mosWriteUnsafe32(mh-mosSizeofMemBlock+mosMemBlockFlags, mosMemFlagHandles);

    // Initialize the Handle
    mosWrite32(mh, mp);     // Pointer to the memory block
    mosWrite16(mh+4, 0);    // Flags
    mosWrite16(mh+6, 0);    // Lock Count

    // Return the wonderful new Handle
    return (mosHandle)mh;
}


/**
 * Reallocate the memory block with a new size.
 */
int mosSetHandleSize(mosHandle hdl, unsigned int newSize)
{
    // get the old allocation data -- a NIL master pointer (an empty
    // handle, see mosNewHandle) has no block to read a size from
    mosPtr oldPtr = mosRead32(hdl);
    unsigned int oldSize = oldPtr ? mosPtrSize(oldPtr) : 0;

    if (newSize==oldSize)
        return 0;

    // allocate a new block, unless shrinking to empty -- same convention
    // as mosNewHandle(0): a 0-byte handle is a NIL master pointer, not a
    // real, useless zero-length block
    mosPtr newPtr = newSize ? mosMalloc(newSize) : 0;
    mosWrite32(hdl, newPtr);

    // copy the old contents over
    unsigned int size = (newSize<oldSize)?newSize:oldSize;
    mosMemcpy(newPtr, oldPtr, size);

    // free the old allocation, if there was one
    if (oldPtr) mosFree(oldPtr);

    return 0;
}

/**
 * Empty the memory allocation that a handle points to.
 * \code
 * PROCEDURE EmptyHandle (h: Handle);
 * \endcode
 * \param hdl Handle to empty.
 */
void mosEmptyHandle(mosHandle hdl)
{
    if (!hdl) return;

    mosPtr ptr = mosRead32(hdl);
    if (ptr) {
        mosFree(ptr);
        mosWrite32(hdl, 0);
    }
}

/**
 * Reallocate the memory block that a handle points to.
 */
void mosReallocHandle(mosHandle hdl, unsigned int newSize)
{
    mosSetHandleSize(hdl, newSize);
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
 * Get the pointer from the handle (locking is not implemented)
 */
mosPtr mosPtrFromHandle(mosHandle hdl) {
    return mosRead32(hdl);
}

/**
 * Mark a handle locked.
 * \code
 * void HLock(Handle h)
 * \endcode
 */
void mosHLock(mosHandle hdl) {
    uint16_t flags = mosRead16(hdl+4);
    flags |= 0x80;
    mosWrite16(hdl+4, flags);
}

/**
 * Mark a handle unlocked.
 * \code
 * void HUnlock(Handle h)
 * \endcode
 */
void mosHUnlock(mosHandle hdl) {
    uint16_t flags = mosRead16(hdl+4);
    flags &= ~0x80;
    mosWrite16(hdl+4, flags);
}

/**
 * Mark a handle purgeable.
 * \code
 * void HPurge(Handle h)
 * \endcode
 */
void mosHPurge(mosHandle hdl) {
    uint16_t flags = mosRead16(hdl+4);
    flags |= 0x40;
    mosWrite16(hdl+4, flags);
}

/**
 * Mark a handle not purgeable.
 * \code
 * void HNoPurge(Handle h)
 * \endcode
 */
void mosHNoPurge(mosHandle hdl) {
    uint16_t flags = mosRead16(hdl+4);
    flags &= ~0x40;
    mosWrite16(hdl+4, flags);
}

uint16_t mosHGetState(mosHandle hdl)
{
    return mosRead16(hdl+4);
}

void mosHSetState(mosHandle hdl, uint16_t state)
{
    mosWrite16(hdl+4, state);
}

/**
 Return the handle that points to a relocatable block of memory.

 \param addr address of some previously allocated block of memory
 \return the handle for tha block, or 0 if there is no handle
 */
mosHandle mosRecoverHandle(mosPtr addr)
{
    mosPtr b = mosMemBlockStart;
    for (;;) {
        mosPtr next = mosReadUnsafe32(b+mosMemBlockNext);
        uint32_t flags = mosReadUnsafe32(b+mosMemBlockFlags);
        if (flags==mosMemFlagHandles) {
            mosPtr ptr = mosRead32(b+mosSizeofMemBlock);
            if (ptr==addr)
                return b+mosSizeofMemBlock;
        }
        b = next;
        if (b==0)
            return 0;
    }
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
