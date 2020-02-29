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


/**
 * This is the header for every block of simulator memory.
 *
 * Memory is managed in two double-linked lists. One list manages all memory
 * allocations, one list manages all master pointers.
 */
typedef struct MosBlock {
  /** next block in the list */
  struct MosBlock *hNext;
  /** previous block in the list */
  struct MosBlock *hPrev;
  unsigned int hSize;
  unsigned int hFlags;
} MosBlock;


/**
 * Manages a double-linked list of memory blocks.
 */
class MosBlockList
{
public:
  MosBlockList();
  ~MosBlockList();
  void link(MosBlock*);
  void unlink(MosBlock*);
  MosBlock *first() { return hFirst; }
  MosBlock *next(MosBlock *me) { return me->hNext; }
  bool contains(MosBlock *me);
  bool contains(mosPtr, unsigned int size);
private:
  MosBlock *hFirst;
  MosBlock *hLast;
};


MosBlockList gMemList;
MosBlockList gHandleList;


/**
 * Create a new memory list.
 */
MosBlockList::MosBlockList()
{
  hFirst = 0;
  hLast = 0;
}


/**
 * Delete the list and all allocated blocks.
 */
MosBlockList::~MosBlockList()
{
  MosBlock *me = first();
  while (me) {
    MosBlock *nextMe = next(me);
    free(me);
    me = nextMe;
  }
  hFirst = 0;
  hLast = 0;
}


/**
 * Link a block into this list.
 *
 * This function does not allocate anything.
 * Do not link blocks that are already linked anywhere else, or desaster
 * will strike.
 */
void MosBlockList::link(MosBlock *me)
{
  MosBlock *prev = hLast;
  if (prev) {
    prev->hNext = me;
    hLast = me;
    me->hPrev = prev;
    me->hNext = 0;
  } else {
    hFirst = me;
    hLast = me;
    me->hPrev = 0;
    me->hNext = 0;
  }
}


/**
 * Unlink a block from this list.
 *
 * This function does not free any memeory.
 * Do not unlink blocks taht are not in this list, or desaster will strike.
 */
void MosBlockList::unlink(MosBlock *me)
{
  MosBlock *prev = me->hPrev;
  MosBlock *next = me->hNext;
  if (prev) {
    prev->hNext = next;
  } else {
    hFirst = next;
  }
  if (next) {
    next->hPrev = prev;
  } else {
    hLast = prev;
  }
  me->hPrev = 0;
  me->hNext = 0;
}


/**
 * Find this block in the list.
 *
 * \return true if the block is actually in the list.
 */
bool MosBlockList::contains(MosBlock *me)
{
  MosBlock *b = first();
  while (b) {
    if (b==me) return true;
    b = next(b);
  }
  return false;
}


/**
 * Find a range of memory in the list.
 *
 * \param start start address of the memory range
 * \param size size of the memory range
 * \return true if the range of memory is fully contained anywhere in the list.
 */
bool MosBlockList::contains(mosPtr start, unsigned int size)
{
  MosBlock *me = first();
  while (me) {
    unsigned int bStart = mosToPtr(me);
    unsigned int bSize = me->hSize;
    if ( (start>=bStart) && (start+size<=bStart+bSize))
      return true;
    me = next(me);
  }
  return false;
}


/**
 * Allocate and copy a text string in the memory list.
 */
mosPtr mosNewPtr(const char *text)
{
  int size = strlen(text)+1;
  mosPtr p = mosNewPtr(size);
  memcpy((void*)p, text, size);
  return p;
}


/**
 * Allocate and clear memory in the memory list.
 */
mosPtr mosNewPtr(unsigned int size)
{
  MosBlock *me = (MosBlock*)calloc(1, size+sizeof(MosBlock));
  me->hSize = size;
  //printf("    NewPtr 0x%08X = 0x%08X (%d)\n", me, mosToPtr(me), size);
  gMemList.link(me);
  return mosToPtr(me);
}


/**
 * Free memory and unlink it from the memory list.
 */
void mosDisposePtr(mosPtr ptr)
{
  if (!ptr) return;
  
  MosBlock *me = mosToBlock(ptr);
  //printf("DisposePtr 0x%08X = 0x%08X\n", me, ptr);
  if (!gMemList.contains(me)) {
    mosError("Disposing unknown Ptr at 0x%08X!\n", ptr);
    return;
  }
  
  gMemList.unlink(me);
  free(me);
}


/**
 * Get the allocated size of a memory block.
 */
unsigned int mosPtrSize(mosPtr ptr)
{
  MosBlock *me = mosToBlock(ptr);
  return me->hSize;
}


/**
 * Allocate memory and a master pointer and link them into the lists.
 */
mosHandle mosNewHandle(unsigned int size)
{
  mosPtr ptr = 0;
  if (size) ptr = mosNewPtr(size);
  
  MosBlock *me = (MosBlock*)calloc(1, sizeof(MosBlock)+4);
  me->hSize = 4;
  gHandleList.link(me);
  
  mosHandle hdl = mosToPtr(me);
  mosWrite32(hdl, ptr);
  
  return hdl;
}


/**
 * Reallocate the memory block with a new size.
 */
int mosSetHandleSize(mosHandle hdl, unsigned int newSize)
{
  // get the old allocation data
  mosPtr oldPtr = mosRead32(hdl);
  unsigned int oldSize = mosPtrSize(oldPtr);
  
  // allocate a new block
  mosPtr newPtr = mosNewPtr(newSize);
  mosWrite32(hdl, newPtr);

  // copy the old contents over
  unsigned int size = (newSize<oldSize)?newSize:oldSize;
  memcpy((void*)newPtr, (void*)oldPtr, size);
  
  // free the old allocation
  mosDisposePtr(oldPtr);
  
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
    mosDisposePtr(ptr);
  }
  
  MosBlock *me = mosToBlock(hdl);
  if (!gHandleList.contains(me)) {
    mosError("Disposing unknown Handle at 0x%08X!\n", hdl);
    return;
  }
  
  gHandleList.unlink(me);
  free(me);
}


/**
 * Find the master pointer using a memory pointer.
 *
 * \todo this function has not been verified.
 */
mosHandle mosRecoverHandle(mosPtr ptr)
{
  MosBlock *me = gHandleList.first();
  while (me) {
    mosHandle hdl = mosToPtr(me);
    if (mosRead32(hdl)==ptr)
      return hdl;
    me = gHandleList.next(me);
  }
  return 0;
}


/**
 * Write anywhere into allocated RAM.
 */
void mosWriteUnsafe32(mosPtr addr, unsigned int value)
{
  *((unsigned int*)(addr)) = htonl(value);
}


/**
 * Verify address allocation and write into RAM.
 */
void mosWrite32(mosPtr addr, unsigned int value)
{
#if MOS_BOUNDS_CHECK
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
  *((unsigned short*)(addr)) = htons(value);
}


/**
 * Verify address allocation and write into RAM.
 */
void mosWrite16(mosPtr addr, unsigned short value)
{
#if MOS_BOUNDS_CHECK
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
  *((unsigned char*)(addr)) = value;
}


/**
 * Verify address allocation and write into RAM.
 */
void mosWrite8(mosPtr addr, unsigned char value)
{
#if MOS_BOUNDS_CHECK
  if (!gHandleList.contains(addr, 1) && !gMemList.contains(addr, 1)) {
    mosError("Attempt to write 1 byte to unknown address 0x%08X (0x%02x)!\n", addr, value);
  }
#endif
  mosWriteUnsafe8(addr, value);
}


/**
 * Read anywhere from allocated RAM.
 */
unsigned int mosReadUnsafe32(mosPtr addr)
{
  return htonl(*((unsigned int*)(addr)));
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
  return htons(*((unsigned short*)(addr)));
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
  return *((unsigned char*)(addr));
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


/**
 * Convert a host memory location into a simulation memory pointer.
 */
mosPtr mosToPtr(struct MosBlock *block)
{
  return ((unsigned int)(block)) + sizeof(MosBlock);
}


/**
 * Convert a simulation memory pointer into a host memory location.
 */
struct MosBlock *mosToBlock(mosPtr ptr)
{
  return ((MosBlock*)(ptr - sizeof(MosBlock)));
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
