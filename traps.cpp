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

// OS Trap Table: Contains 00E0 (224 decimal) entries starting at low-memory
// base $00000400. (Opcodes range from $A000 to $A0DF).
// Toolbox Trap Table: Contains 0200 (512 decimal) entries starting at
// low-memory base $00000E00. (Opcodes range from $A800 to $A9FF and beyond up to $AFFP).

// 1010.1zyx.xxxx.xxxx: Toolbox: x = trap #, if z is 1, pop the extra return address from the stack
// z=1 adds an extra return address between the return address and the parameters on the stack.
// Setting the auto-pop bit tells the dispatcher to strip that 4-byte return address before jumping into the real routine.
// x.xxxx.xxx = 512 (0x200) possible entries. y was later added, resulting in 1024 (0x400)possible entries.
// This is tested by checking if 0xAA6E and 0xA86E (InitGraf) point to the same address (0x200)
// or different addresses (0x400) (GetTrapAddress vs. NGetTrapAddress)
// Pascal calling convention: args pushed left-to-right, callee pops everything
// including its own return address before resuming)

// 1010.0yyz.xxxx.xxxx: OS trap: x = trap #, yy are extra flags that can be used by the traps
// If z is set, A0 returns a value, otherwise A0 will be preserved.
// OS traps arguments are stored in A0 and D0, or pointer to by A0 if more than two arguments are needed.
// Data is returned in D0 (result code), and optionally in A0 (result data) as well.
// xxxx.xxxx = 256 possible entries

// TODO: missing traps:
//    UInt32 CmpStringMarks(BytePtr textPtrA, BytePtr textPtrB, UInt32 lengthAB) ??
//    int CountResources(theType: ResType) Given the type, return the number of resources of that type accessable in ALL open maps.
//    int CurResFile() : Returns the reference number of the current resource file.
//    Get1IxResource not documented
//    void HNoPurge(handle h) : make a resource unpurgeable!
//    int HomeResFile(theResource: Handle) : Given a handle, returns the refnum of the resource file that the resource lives in.
//    void HPurge(handle h) : make a resource purgeable but don;t purge it!
//    handle NewHandle() : allocate memory and a master pointer to it
//    void ReleaseResource(handle) : Given handle, releases the resource and disposes of the handle.
//    SecondsToDate
//    TickCount

// Inlcude all the required system headers

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

// Include our own interfaces

#include "traps.h"
#include "OS/TrapManager.h"
#include "names.h"
#include "log.h"
#include "memory.h"
#include "resourcefork.h"
#include "breakpoints.h"
#include "systemram.h"
#include "debug.h"
#include "fileio.h"
#include "gestalt.h"
#include "quickdraw.h"
#include "windowmgr.h"
#include "eventmgr.h"
#include "menumgr.h"
#include "dialogmgr.h"

// Inlcude Musahi's m68k emulator

extern "C" {
#include "musashi331/m68k.h"
#include "musashi331/m68kcpu.h"
#include "musashi331/m68kops.h"
}

// Any machine command starting with 0xA... jumps to this address where it will
// find a new trap, 0xAFFD, which call back into mosrun to dispatch the trap.
unsigned int gTrapDispatchALineStub = 0;

// Any machine command starting with 0xF... jumps to this address where it will
// find a new trap, 0xAFFB, which handle Floating Point and MMU instructions..
unsigned int gTrapDispatchFLineStub = 0;

// When the MacOS application exits, it will jump to this address where it will
// find trap 0xAFFC, which calls back into mosrun to handle the application exit.
unsigned int gTrapExitAppStub = 0;

// The current trap number. This is used to dispatch the appropriate trap handler
// in gTrapDispatchALineStub and gTrapDispatchFLineStub.
uint16_t gCurrentTrap = 0;

mosPtr gGlue_Unimplemented = 0;

/**
 * Utility function.
 */
void hexDump(mosPtr a, unsigned int n)
{
    unsigned int i = 0;
    for (;;) {
        mosTrace("%08X: ", a);
        for (i=0; i<16; i++) {
            if (i<n) {
                byte b = mosReadUnsafe8(a+i);
                mosTrace("%02X ", b);
            } else {
                mosLog("   ");
            }
        }
        for (i=0; i<16; i++) {
            if (i<n) {
                byte b = mosReadUnsafe8(a+i);
                mosTrace("%c", (b>=32 && b<127)?b:'.');
            } else {
                mosTrace(" ");
            }
        }
        mosTrace("\n");
        fflush(stdout);
        if (n<=16) break;
        a += 16;
        n -= 16;
    }
}


//typedef void (*TrapNative)(unsigned short);
//typedef struct {
//    unsigned short cmd;
//    TrapNative trapNative;
//    unsigned short rts;
//} TrapNativeCall;

// every entry in this array points to an m68k code segment ("glue") that calls a native function in host memory
mosPtr gToolboxTrapTable = 0;
mosPtr gOSTrapTable = 0;

/**
 * Load a resource using a fourCC code.
 *
 * sp+10.l = return value (handle to resource)
 * sp+6.l  = rsrc
 * sp+4.w  = id
 * sp.l    = return address
 *
 * \todo set gMosResErr as needed
 */
void trapGetResource(unsigned short )
{
    unsigned int sp   = m68k_get_reg(0L, M68K_REG_SP);

    unsigned int ret  = mosRead32(sp); sp += 4;
    unsigned int id   = m68k_read_memory_16(sp); sp+=2;
    unsigned int rsrc = m68k_read_memory_32(sp); sp+=4;

    mosTrace("            GetResource('%c%c%c%c', %d)\n",
             rsrc>>24, rsrc>>16, rsrc>>8, rsrc, id);
    unsigned int hdl = (unsigned int)GetResource(rsrc, id);

    m68k_write_memory_32(sp, hdl);
    sp-=4; m68k_write_memory_32(sp, ret);

    m68k_set_reg(M68K_REG_SP, sp);
    m68k_set_reg(M68K_REG_D0, 0);
}


/**
 * [A9A2] Load a resource using a handle.
 *
 * sp+4.l  = handle
 * sp.l    = return address
 *
 * \todo set gMosResErr as needed
 */
void trapLoadResource(unsigned short )
{
    unsigned int sp   = m68k_get_reg(0L, M68K_REG_SP);

    unsigned int ret  = mosRead32(sp); sp += 4;
    /*unsigned int hdl  =*/ m68k_read_memory_32(sp); sp+=4;

    //hdl = 0; // we don't support unloaded resources, so the resource should still be in memory

    sp-=4; m68k_write_memory_32(sp, ret);

    m68k_set_reg(M68K_REG_SP, sp);
}


/**
 * Load a resource using a name.
 *
 * sp+12.l = return value (handle to resource)
 * sp+8.l  = rsrc
 * sp+4.l  = name
 * sp.l    = return address
 *
 * \todo set gMosResErr as needed
 */
void trapGetNamedResource(unsigned short )
{
    unsigned int sp   = m68k_get_reg(0L, M68K_REG_SP);

    unsigned int ret  = m68k_read_memory_32(sp); sp+=4;
    unsigned int name = m68k_read_memory_32(sp); sp+=4;
    unsigned int rsrc = m68k_read_memory_32(sp); sp+=4;

    mosTrace("            GetNamedResource('%c%c%c%c', %*s)\n",
             rsrc>>24, rsrc>>16, rsrc>>8, rsrc,
             m68k_read_memory_8(name), mosToHost(name+1));
    mosHandle hdl = GetNamedResource(rsrc, (byte*)mosToHost(name));

    m68k_write_memory_32(sp, hdl);
    sp-=4; m68k_write_memory_32(sp, ret);

    m68k_set_reg(M68K_REG_SP, sp);
    m68k_set_reg(M68K_REG_D0, 0);
}


/**
 * Size of the resource on disk.
 *
 * sp+8.l  = return value (handle to resource)
 * sp+4.l  = handle of resource (in most cases, this will point to NULL and we must search the resource on disk)
 * sp.l    = return address
 *
 * \todo this must return the size of the resource *in the file*, not in memory!
 */
void trapSizeResource(unsigned short )
{
    unsigned int sp   = m68k_get_reg(0L, M68K_REG_SP);

    unsigned int ret  = m68k_read_memory_32(sp); sp += 4;
    unsigned int hdl = m68k_read_memory_32(sp); sp+=4;

    unsigned int ptr = m68k_read_memory_32(hdl);
    unsigned int size = mosPtrSize(ptr);
    mosTrace("            SizeResource(0x%08X) = %d\n", hdl, size);

    m68k_write_memory_32(sp, size);
    sp-=4; m68k_write_memory_32(sp, ret);

    m68k_set_reg(M68K_REG_SP, sp);
    m68k_set_reg(M68K_REG_D0, 0);
}


/**
 * Allocate a block of non-movable bytes.
 *
 * D0 = desired size in bytes.
 * Bit9: CLEAR
 * Bit10: SYS (1 = allocate system RAM, 0 = allocate app RAM)
 * \returns pointer to allocated area in A0
 * \returns possible error in D0
 *
 * \todo this must also set the global MemErr variable
 */
void trapNewPtr(unsigned short instr)
{
    unsigned int size = m68k_get_reg(0L, M68K_REG_D0);

    unsigned int ptr = 0;
    if (mosOSTrapBit9(instr)) {
        mosTrace("            NewPtrClear(%d)\n", size);
        ptr = mosNewPtr(size);
    } else {
        mosTrace("            NewPtr(%d)\n", size);
        ptr = mosNewPtr(size);
    }

    m68k_set_reg(M68K_REG_A0, ptr);
    m68k_set_reg(M68K_REG_D0, 0);
}


/**
 * Allocate a block of movable bytes.
 *
 * In the original code, we have flags for sys memory and clear memory.
 * a122 a322 a522 a722
 *
 * D0 = desired size in bytes.
 * \returns handle to allocated area in A0
 * \returns possible error in D0
 *
 * \todo this must also set the global MemErr variable
 */
void trapNewHandle(unsigned short)
{
    unsigned int size = m68k_get_reg(0L, M68K_REG_D0);
    unsigned int hdl = 0;

    mosTrace("            NewHandle(%d)\n", size);
    hdl = mosNewHandle(size);

    m68k_set_reg(M68K_REG_A0, hdl);
    m68k_set_reg(M68K_REG_D0, 0);
}


/**
 * Find the Master Pointer that points to some memoryand return a handle to it.
 *
 * Even though this function may have come for free on the original Macintosh,
 * it is redundant in itself and requires the simulator to add a whole new
 * layer of memory management.
 *
 * As opposed to almost all other traps, this one does not set D0!
 *
 * A0 = pointer to "lost" block of memory.
 * \returns handle to memory in A0
 *
 * \todo this must also set the global MemErr variable?
 */
void trapRecoverHandle(unsigned short)
{
    unsigned int ptr = m68k_get_reg(0L, M68K_REG_A0);

    unsigned int hdl = mosRecoverHandle(ptr);
    mosTrace("            RecoverHandle(0x%08X)=0x%08X\n", ptr, hdl);

    m68k_set_reg(M68K_REG_A0, hdl);
}


/**
 * [A025] Get the size of the memory allocation that we point to.
 *
 * A0 = handle
 * \return size in D0 or the result code (D0<0)
 */
void trapGetHandleSize(unsigned short)
{
    unsigned int hdl = m68k_get_reg(0L, M68K_REG_A0);
    unsigned int ret = 0;

    unsigned int ptr = m68k_read_memory_32(hdl);
    if (ptr) {
        ret = mosPtrSize(ptr);
    }

    m68k_set_reg(M68K_REG_D0, ret);
}


/**
 * [A024] Set the size of the memory allocation that we point to.
 *
 * A0 = handle
 * D0 = new size
 * \return result code in D0
 */
void trapSetHandleSize(unsigned short)
{
    unsigned int hdl  = m68k_get_reg(0L, M68K_REG_A0);
    unsigned int size = m68k_get_reg(0L, M68K_REG_D0);
    unsigned int ret = 0;

    ret = mosSetHandleSize(hdl, size);

    m68k_set_reg(M68K_REG_D0, ret);
}


/**
 * Free the memory that was allocated with NewPtr().
 *
 * A0 = pointer to allocated memory.
 * \returns error in D0.
 *
 * \todo this must also set the global MemErr variable
 */
void trapDisposePtr(unsigned short)
{
    unsigned int ptr = m68k_get_reg(0L, M68K_REG_A0);

    mosTrace("            DisposePtr(0x%08X)\n", ptr);
    mosDisposePtr(ptr);

    m68k_set_reg(M68K_REG_D0, 0);
}


/**
 * Free the master pointer and the memory that was allocated with NewHandle().
 *
 * A0 = handle = pointer to master pointer
 * \returns error in D0.
 *
 * \todo this must also set the global MemErr variable
 */
void trapDisposeHandle(unsigned short)
{
    unsigned int hdl = m68k_get_reg(0L, M68K_REG_A0);

    unsigned int ptr = hdl?m68k_read_memory_32(hdl):0;
    mosTrace("            DisposeHandle(0x%08X(->0x%08X))\n", hdl, ptr);
    mosDisposeHandle(hdl);

    m68k_set_reg(M68K_REG_D0, 0);
}


/**
 * Move a block of memory.
 *
 * Blocks may overlap.
 *
 * A0 = source
 * A1 = destination
 * D0 = number of bytes to move
 *
 * \todo This does not do any bound checking! Fix that (it's easy, the calls are there!)
 */
void trapBlockMove(unsigned short)
{
    unsigned int src  = m68k_get_reg(0L, M68K_REG_A0);
    unsigned int dst  = m68k_get_reg(0L, M68K_REG_A1);
    unsigned int size = m68k_get_reg(0L, M68K_REG_D0);

    mosTrace("            BlockMove(src:%s, dst:%s, %d)\n", printAddr(src), printAddr(dst), size);
    if (src<0x1000 || dst<0x1000) {
        mosError("The addresses seem highly unlikely. You may have an uninitialized pointer or an uncought error\n");
        // FIXME: however, it could be copying the app name from the global variable filed
    }
    memmove(mosToHost(dst), mosToHost(src), size);

    m68k_set_reg(M68K_REG_D0, 0);
}




/**
 * Used in the jump table to load a segment from the resource file.
 *
 * sp+4.w = resource id for 'CODE' resource
 * sp.l   = return address = address of the jump table entry plus 6
 *
 * An unloaded jump table entry is 8 bytes: offset(2), 0x3F3C "MOVE.W
 * #segNum,-(SP)"(2), segNum(2), 0xA9F0 "_LoadSeg" trap(2). Executing it
 * pushes segNum then traps, landing here with the entry's own offset at
 * (ret-8) and segNum already popped as `id`. Once patched, the entry
 * becomes: id(2), 0x4EF9 "JMP abs32"(2), absolute target(4).
 *
 * A segment can have several jump table entries (one per external entry
 * point). Previously only the one entry that happened to be called first
 * got patched; every other entry for the same segment stayed as an
 * unloaded stub until it was individually called, redundantly re-resolving
 * an already-loaded segment. Since we already know the segment's base
 * address here, also patch every other unloaded entry for this segment id
 * that we can find, each using its own offset.
 *
 * The entry that actually triggered this call is always patched directly
 * via `ret`, which is exact regardless of where the jump table lives.
 * Finding *other* entries for the same segment requires scanning a range,
 * and most of a segment's entries are typically populated at startup by
 * whichever segment performs the app's own global/jump-table
 * initialization (often loaded here as a very early LoadSeg call), not by
 * CODE 0's own tiny initial jump table -- so the scan covers the whole A5
 * world (gMosA5WorldStart/gMosA5WorldEnd, set up in createA5World) and
 * additionally requires the found offset to fall within this segment's own
 * code, to guard against misinterpreting unrelated global data that
 * happens to contain the same two marker words.
 *
 * \todo separate the interface from the code.
 */
void trapLoadSeg(unsigned short )
{
    unsigned int sp   = m68k_get_reg(0L, M68K_REG_SP);

    unsigned int ret  = m68k_read_memory_32(sp); sp += 4;
    unsigned int id   = m68k_read_memory_16(sp); sp += 2;

    mosTrace("            LoadSeg(%d)\n", id);
    // now load the resource
    mosHandle hCode = GetResource('CODE', id);
    if (!hCode) {
        mosDebug("Code Resource %d not found!\n", id);
    } else {
        unsigned int code = m68k_read_memory_32(hCode);
        unsigned int codeSize = mosPtrSize(code);
        hexDump(hCode, 64);

        // the entry that triggered this call: always correct, exact address
        unsigned int triggerEntry = ret-8;
        unsigned int triggerOffset = m68k_read_memory_16(triggerEntry);
        m68k_write_memory_16(triggerEntry+0, id);           // save the block id
        m68k_write_memory_16(triggerEntry+2, 0x4ef9);       // 'jmp nnnnnnnn' instruction
        m68k_write_memory_32(triggerEntry+4, code+triggerOffset+4);  // +4 -> skip the entry that gives the number of jump table entries?

        // best-effort: patch any other still-unloaded entries for this segment
        unsigned int entry = gMosA5WorldStart;
        unsigned int scanEnd = gMosA5WorldEnd;
        for (; entry+8<=scanEnd; entry+=8) {
            if (entry==triggerEntry) continue;
            if (m68k_read_memory_16(entry+2)==0x3f3c &&
                m68k_read_memory_16(entry+6)==0xa9f0 &&
                m68k_read_memory_16(entry+4)==id) {
                unsigned int offset = m68k_read_memory_16(entry);
                if (offset<codeSize) {
                    m68k_write_memory_16(entry+0, id);
                    m68k_write_memory_16(entry+2, 0x4ef9);
                    m68k_write_memory_32(entry+4, code+offset+4);
                }
            }
        }
    }

    sp -= 4; m68k_write_memory_32(sp, ret-6);

    m68k_set_reg(M68K_REG_SP, sp);
}

void trapUnloadSeg(unsigned short )
{
    mosDebugPrintPCHistory(16);
    unsigned int sp   = m68k_get_reg(0L, M68K_REG_SP);

    mosPtr ret  = m68k_read_memory_32(sp); sp += 4;
    /*mosPtr func =*/ m68k_read_memory_32(sp); sp += 4;

    // Unload the Code segment that contains the routineAddr 'func'

    sp -= 4; m68k_write_memory_32(sp, ret);
    m68k_set_reg(M68K_REG_SP, sp);
}


/**
 * Get the state of a master pointer.
 *
 * If an error occurs during an attempt to get the state flags of the specified
 * relocatable block, HGetState returns the low-order byte of the result code as
 * its function result. For example, if the handle h points to a master pointer
 * whose value is NULL, then the signed byte returned by HGetState will contain
 * the value –109.
 *
 * kHandleIsResourceBit          = 5,
 * kHandlePurgeableBit           = 6,
 * kHandleLockedBit              = 7
 *
 * A0 = Handle
 * \return state in D0 (signed char!)
 *
 * \todo return the correct value!
 * \todo set MemErr
 * \todo add support for these flags (it's all already in the memory manager)
 */
void trapHGetState(unsigned short ) {
    mosPtr ptr = m68k_get_reg(0L, M68K_REG_A0);
    m68k_set_reg(M68K_REG_D0, mosHGetState(ptr));
}

/**
 * Restores a block's state byte previously saved by calling HGetState.
 *
 * \code
 * procedure HSetState(h: Handle; flags: SignedByte);
 * \endcode
 */
void trapHSetState(unsigned short ) {
    mosPtr ptr = m68k_get_reg(0L, M68K_REG_A0);
    uint16_t state = m68k_get_reg(0L, M68K_REG_D0);
    mosHSetState(ptr, state);
}

/**
 * Move handles high in memory to make room for big allocations.
 * /code
 * void MoveHHi(Handle h);
 * \endcode
 * There is no benefit to moving handles high in memory in this simulation.
 */
void trapMoveHHi(unsigned short ) {
    // nothing to do here
}


/**
 * Lock mamory in position.
 *
 * We never move memory, so there is no reason to implement this.
 */
void trapHLock(unsigned short ) {
    mosPtr ptr = m68k_get_reg(0L, M68K_REG_A0);
    mosHLock(ptr);
}


/**
 * [A02A] Unlock mamory in position.
 *
 * We never move memory, so there is no reason to implement this.
 */
void trapHUnlock(unsigned short ) {
    mosPtr ptr = m68k_get_reg(0L, M68K_REG_A0);
    mosHUnlock(ptr);
}


/**
 * Convert a 32-bit number into a 24-bit address.
 *
 * This function has been obsolete since 1992. Despite the Pascal-looking
 * documented signature (FUNCTION StripAddress (theAddress: Ptr) : Ptr),
 * real compiled callers invoke it as a register-based identity passthrough
 * (D0 to D0))
 */
void trapStripAddress(unsigned short ) {
    // nothing to do here
}


/**
 * One trap that dispatches into multiple new traps.
 *
 * sp.?+6 = data specific to the function called via the selector...
 * sp.w+4 = selector
 * sp.l   = return address
 *
 * selectors:
 * 0x15: mfMaxMemSel
 *    Returns the size, in bytes, of the largest contiguous free block in the current heap zone.
 * 0x18: mfFreeMemSel
 *    Returns the total amount of free space in the current heap zone.
 * 0x1D: mfTempNewHandleSel
 *    Allocates a new relocatable block of temporary memory.
 * 0x1E: mfTempHLockSel
 *    Locks a relocatable block in the temporary heap.
 * 0x1F: mfTempHUnLockSel
 *    Unlocks a relocatable block in the temporary heap.
 * 0x20: mfTempDisposHandleSel
 *    Releases a relocatable block in the temporary heap.
 * 0x37: ??
 * 0x3A: ??
 */
void trapOSDispatch(unsigned short )
{
    unsigned int sp = m68k_get_reg(0L, M68K_REG_SP);

    unsigned int stack_ret = m68k_read_memory_32(sp); sp += 4;
    unsigned short selector = m68k_read_memory_16(sp); sp += 2;

    switch (selector)
    {
        case 0x1D: { // mfTempNewHandleSel
            // Allocates a new relocatable block of temporary memory.
            // Handle TempNewHandle (Size logicalSize, OSErr *resultCode);
            unsigned int resultCodePtr = m68k_read_memory_32(sp); sp += 4;
            unsigned int size = m68k_read_memory_32(sp); sp += 4;

            unsigned int handle = mosNewHandle(size);
            mosTrace("trapALineDispatch(0x1D): Allocated a master pointer at 0x%08X\n", handle);

            if (resultCodePtr) m68k_write_memory_16(resultCodePtr, 0);
            m68k_write_memory_32(sp, handle);
            break;
        }
        case 0x1E: // TempHLock - nothing to do
            break;
        case 0x1F: // TempHUnlock - nothing to do
            break;
        case 0x20: { // mfTempDisposHandleSel
            // Releases a relocatable block in the temporary heap.
            // void TempDisposeHandle (Handle h, OSErr *resultCode );
            unsigned int resultCodePtr = m68k_read_memory_32(sp); sp += 4;
            unsigned int handle = m68k_read_memory_32(sp); sp += 4;

            mosDisposeHandle(handle);
            mosTrace("TempDisposeHandle(0x%08X)\n", handle);

            if (resultCodePtr) m68k_write_memory_16(resultCodePtr, 0);
            break;
        }
        case 0x3A: { // OSErr GetProcessInformation(PSN: ProcessSerialNumber,
                     //                             VAR info: ProcessInfoRec);
            // process mode flags:
            // modeDeskAccessory             = $00020000;
            // modeMultiLaunch               = $00010000;
            // modeNeedSuspendResume         = $00004000;
            // modeCanBackground             = $00001000;
            // modeDoesActivateOnFGSwitch    = $00000800;
            // modeOnlyBackground            = $00000400;
            // modeGetFrontClicks            = $00000200;
            // modeGetAppDiedMsg             = $00000100;
            // mode32BitCompatible           = $00000080;
            // modeHighLevelEventAware       = $00000040;
            // modeLocalAndRemoteHLEvents    = $00000020;
            // modeStationeryAware           = $00000010;
            // modeUseTextEditServices       = $00000008;

            unsigned int total, contig;
            mosFreeMemInfo(&total, &contig);

            unsigned int infoPtr = m68k_read_memory_32(sp); sp += 4;
            /*unsigned int psnPtr =*/ m68k_read_memory_32(sp); sp += 4;
            // TODO: verify that psn is 2

            mosPtr d = infoPtr;

            // struct ProcessInfoRec {
            //     unsigned long        processInfoLength;   /* 0: 0000003c length of record*/
            m68k_write_memory_32(d, 0x0000003c); d+=4;
            //     StringPtr            processName;         /* 4: 00000000 name of process*/
            m68k_write_memory_32(d, 0x00000000); d+=4;
            //     ProcessSerialNumber  processNumber;       /* 8: 00000000 00002002 psn of the process*/
            m68k_write_memory_32(d, 0x00000000); d+=4;
            m68k_write_memory_32(d, 0x00002002); d+=4;
            //     unsigned long        processType;         /* 16: 41 50 50 4c file type of app file*/
            m68k_write_memory_32(d, 'APPL'); d+=4;
            //     OSType               processSignature;    /* 20: 3f 3f 3f 3f signature of app file*/
            m68k_write_memory_32(d, 0x3f3f3f3f); d+=4;      // '????'
            //     unsigned long        processMode;         /* 24: 00 00 58 80 'SIZE' resource flags*/
            m68k_write_memory_32(d, 0x000058e0); d+=4;      // TODO: just read the damn SIZE Resource!
            //     Ptr                  processLocation;     /* 28: 03 f4 3e 94 address of partition*/
            m68k_write_memory_32(d, kSystemHeapStart); d+=4;
            //     unsigned long        processSize;         /* 32: partition size*/
            // m68k_write_memory_32(d, kMosMemMax-kSystemHeapStart-1*1024*1024); d+=4;      // must not be smaller than processFreeMem
            m68k_write_memory_32(d, total-1*1024*1024); d+=4;      // must not be smaller than processFreeMem
            //     unsigned long        processFreeMem;      /* 36: 00 00 b0 dc free bytes in heap*/
            m68k_write_memory_32(d, total-2*1024*1024); d+=4;
            //     ProcessSerialNumber  processLauncher;     /* 40: 00000000 00002000 proc that launched this one*/
            m68k_write_memory_32(d, 0x00002000); d+=4;
            m68k_write_memory_32(d, 0x0000003c); d+=4;
            //     unsigned long        processLaunchDate;   /* 48: 0000111f time when launched*/
            m68k_write_memory_32(d, 0x0000111f); d+=4;
            //     unsigned long        processActiveTime;   /* 52: 00000000 accumulated CPU time*/
            m68k_write_memory_32(d, 0x00000002); d+=4;
            //     FSSpecPtr            processAppSpec;      /* 56: 00000000 location of the file*/
            m68k_write_memory_32(d, 0x00000000); d+=4;
            // };

            m68k_write_memory_16(sp, 0); // noErr
            break;
        }
        default:
            mosError("Unimplemented OSDispatch 0x%02X\n", selector);
            debug_break();
            return;
    }

    sp -= 4; m68k_write_memory_32(sp, stack_ret);
    m68k_set_reg(M68K_REG_SP, sp);
}


/**
 * [a9c6] Convert the number of seconds to a date.
 *
 * D0 = number of seconds since midnight Jan 1, 1904
 * A0 = pointer to DateTimeRec structure
 */
void trapSecondsToDate(unsigned short )
{
    unsigned int seconds = m68k_get_reg(0L, M68K_REG_D0);
    unsigned int datetime = m68k_get_reg(0L, M68K_REG_A0);

    time_t clock = seconds - 2082844800; // plus the time in seconds between 1904 and 1970.
    struct tm *tm = gmtime(&clock);

    // fill the struct DateTimeRec
    m68k_write_memory_16(datetime   , tm->tm_year+1900);  // ux: +1900  mac: actual year
    m68k_write_memory_16(datetime+2 , tm->tm_mon+1);      // ux: 0-11   mac: 1-12 ?
    m68k_write_memory_16(datetime+4 , tm->tm_mday+1);     // ux: 1-31   mac: 1-31 ?
    m68k_write_memory_16(datetime+6 , tm->tm_hour);       // ux: 0-23   mac: 0-23 ?
    m68k_write_memory_16(datetime+8 , tm->tm_min);        // ux: 0-59   mac: 0-59 ?
    m68k_write_memory_16(datetime+10, tm->tm_sec);        // ux: 0-59   mac: 0-59 ?
    m68k_write_memory_16(datetime+12, tm->tm_wday+1);     // ux: Sun=0  mac: 1= ?
}


/**
 * Return the number of ticks (60th of a second) since this computer was
 * switched on.
 *
 * Derived from Musashi's own emulated-cycle counter (m68k_cycles_run()),
 * not host wall-clock time. A real Mac's TickCount is tied to a hardware
 * clock whose rate is fixed relative to the CPU's own execution speed, so
 * app code is free to assume elapsed ticks correlate with how much 68k
 * code has actually run -- some startup code (e.g. a CPU-speed
 * calibration loop that reads TickCount before/after a fixed busy-loop
 * and divides iterations by elapsed ticks) depends on exactly that.
 * mosrun's own interpretation speed relative to the host has nothing to
 * do with the *emulated* CPU's speed, so backing this with real elapsed
 * host time made such code see wildly different (sometimes zero) deltas
 * depending purely on how fast mosrun happened to run that session --
 * notably different between traced and untraced runs. Deriving ticks from
 * emulated cycles instead makes this deterministic and independent of
 * host speed. ~16MHz is a plausible clock speed for a 68020-class Mac of
 * this era; exact fidelity doesn't matter, only that elapsed ticks scale
 * with emulated work the way real hardware guarantees.
 */
unsigned int mosTickCount()
{
    const unsigned int cyclesPerTick = 16000000 / 60;
    return (unsigned int)m68k_cycles_run() / cyclesPerTick;
}


/**
 * [a975] Return the number of ticks (60th of a second since this computer was switched on.
 */
void trapTickCount(unsigned short )
{
    unsigned int sp = m68k_get_reg(0L, M68K_REG_SP);
    m68k_write_memory_32(sp+4, mosTickCount());
}


/**
 * [A162] Report free space in the current heap zone.
 *
 * PROCEDURE PurgeSpace (VAR total: LongInt; VAR contig: LongInt);
 *
 * Documented as a stack-based Pascal call with two VAR parameters, but it
 * is actually register-based like the other lightweight Memory Manager
 * "quick" traps (HLock, DisposeHandle, etc.): the caller's own glue code
 * (compiled after the trap opcode) reads the results out of D0/A0 and
 * copies them into the real VAR parameter locations itself; the trap never
 * touches the stack. Confirmed against real disassembly of the call site:
 * the glue does `MOVE.L A0,(ptr1)` then `MOVE.L D0,(ptr2)`, where ptr1 is
 * `contig` (pushed second, shallower on the stack) and ptr2 is `total`
 * (pushed first, deeper) -- i.e. A0=contig, D0=total.
 *
 * Reports real numbers from mosrun's own heap block list (see mosFreeMemInfo),
 * so an app that checks available memory before proceeding sees a sane answer
 * instead of thinking it is critically low on memory.
 */
void trapPurgeSpace(unsigned short )
{
    unsigned int total = 0, contig = 0;
    mosFreeMemInfo(&total, &contig);

    m68k_set_reg(M68K_REG_D0, total);
    m68k_set_reg(M68K_REG_A0, contig);
}


/**
 * [A049] Mark a block as purgeable.
 *
 * We never purge blocks in the simulation.
 */
void trapHPurge(unsigned short )
{
    mosPtr ptr = m68k_get_reg(0L, M68K_REG_A0);
    mosHPurge(ptr);
}


/**
 * [A04A] Mark a block as unpurgeable.
 *
 * We never purge blocks in the simulation.
 */
void trapHNoPurge(unsigned short )
{
    mosPtr ptr = m68k_get_reg(0L, M68K_REG_A0);
    mosHNoPurge(ptr);
}


/**
 * [A9A3] PROCEDURE ReleaseResource(theResource: Handle);
 *
 * Release a resource from memory unless it changed.
 *
 * We never release resources.
 */
void trapReleaseResource(unsigned short )
{
    unsigned int sp   = m68k_get_reg(0L, M68K_REG_SP);

    unsigned int ret  = m68k_read_memory_32(sp); sp += 4;
    unsigned int res  = m68k_read_memory_16(sp); sp += 4;

    (void)res; // don't actually do anything

    sp -= 4; m68k_write_memory_32(sp, ret);

    m68k_set_reg(M68K_REG_SP, sp);
}


/**
 * [A994] Return the number of the current Resource file.
 *
 * We only support a single res file, so return 1.
 */
void trapCurResFile(unsigned short )
{
    unsigned int sp   = m68k_get_reg(0L, M68K_REG_SP);

    unsigned int ret  = m68k_read_memory_32(sp); sp += 4;

    m68k_write_memory_16(sp, 1);  // my resource ID
    sp-=4; m68k_write_memory_32(sp, ret);

    m68k_set_reg(M68K_REG_SP, sp);
    m68k_set_reg(M68K_REG_D0, 0);
}


/**
 * [A99B] Set the gMosResLoad flag.
 *
 * sp+4.w  = flag
 * sp.l    = return address
 *
 * Flag should be honored by GetResource and GetNamedResource.
 */
void trapSetResLoad(unsigned short )
{
    unsigned int sp   = m68k_get_reg(0L, M68K_REG_SP);

    unsigned int ret  = m68k_read_memory_32(sp); sp += 4;
    unsigned int flag = m68k_read_memory_16(sp); sp += 2;

    gMosResLoad = flag ? 1 : 0;
    sp-=4; m68k_write_memory_32(sp, ret); // FIXME: stack correct?

    m68k_set_reg(M68K_REG_SP, sp);
    m68k_set_reg(M68K_REG_D0, 0);
}


/**
 * [A9A4] Use this Resource file from now on.
 *
 * We support only a single Resource file, so, yeah, OK.
 */
void trapHomeResFile(unsigned short )
{
    unsigned int sp   = m68k_get_reg(0L, M68K_REG_SP);

    unsigned int ret  = m68k_read_memory_32(sp); sp += 4;
    unsigned int hdl  = m68k_read_memory_32(sp); sp += 4;

    (void)hdl;

    m68k_write_memory_32(sp, 0);
    sp-=4; m68k_write_memory_32(sp, ret);

    m68k_set_reg(M68K_REG_SP, sp);
    m68k_set_reg(M68K_REG_D0, 0);
}

/**
 * [A9A6] Get the attributes for a resource.
 *
 * sp+4.l  = res handle
 * sp.l    = return address
 *
 * resSysHeap     =  64;   {set if read into system heap}
 * resPurgeable   =  32;   {set if purgeable}
 * resLocked      =  16;   {set if locked}
 * resProtected   =   8;   {set if protected}
 * resPreload     =   4;   {set if to be preloaded}
 * resChanged     =   2;   {set if to be written to resource fork}
 * result code: resNotFound = -192, or 0
 */
void trapGetResAttrs(unsigned short )
{
    unsigned int sp   = m68k_get_reg(0L, M68K_REG_SP);

    unsigned int ret  = m68k_read_memory_32(sp); sp += 4;
    unsigned int hdl  = m68k_read_memory_32(sp); sp += 4;
    (void)hdl;

    m68k_write_memory_32(sp, 0);
    sp-=4; m68k_write_memory_32(sp, ret);  // FIXME: stack correct?

    m68k_set_reg(M68K_REG_SP, sp);
    m68k_set_reg(M68K_REG_D0, 0); // return attributes here
}


/**
 * [A9A7] Set the attributes for a resource.
 *
 * PROCEDURE SetResAttrs (theResource: Handle; attrs: INTEGER);
 *
 * sp+6.l  = theResource handle
 * sp+4.w  = attrs
 * sp.l    = return address
 *
 * Resource attributes (purgeable/locked/protected/preload/changed/system heap)
 * are irrelevant in this simulation, same stance as ReleaseResource/GetResAttrs.
 */
void trapSetResAttrs(unsigned short )
{
    unsigned int sp    = m68k_get_reg(0L, M68K_REG_SP);

    unsigned int ret    = m68k_read_memory_32(sp); sp += 4;
    /*unsigned int attrs =*/ m68k_read_memory_16(sp); sp += 2;
    /*unsigned int hdl   =*/ m68k_read_memory_32(sp); sp += 4;

    sp-=4; m68k_write_memory_32(sp, ret);

    m68k_set_reg(M68K_REG_SP, sp);
    m68k_set_reg(M68K_REG_D0, 0);
}


// PROCEDURE UseResFile (refNum: INTEGER);
// ERROR: unimplemented trap 0x0000AE00: _OpenCPort (POP)


/**
 * [A051] Almost entirely undocumented.
 *
 * A0 points to the heap, so I guess the application expects some value to be
 * written there.
 * D0 seems to be some magic number, probably an index into the PRAM
 *
 * We return 0 in D0 and 0 in (A0) and cross our fingers.
 */
void trapReadXPRam(unsigned short )
{
    unsigned int resultPtr = m68k_get_reg(0L, M68K_REG_A0);
    if (resultPtr) {
        m68k_write_memory_8(resultPtr, 0);
    }
    m68k_set_reg(M68K_REG_D0, 0);
}


/**
 * [A00C] _GetFileInfo.
 * int mosPBGetFInfo(unsigned int paramBlock, bool async)
 */
void trapGetFileInfo(unsigned short )
{
    unsigned int paramBlock = m68k_get_reg(0L, M68K_REG_A0);
    unsigned int async = 0; // If bit 10 of the instruction is set, it is async.

    unsigned int ret = mosPBGetFInfo(paramBlock, async);

    m68k_set_reg(M68K_REG_D0, ret);
}


/**
 * [A00D] _SetFileInfo.
 * int mosPBSetFInfo(unsigned int paramBlock, bool async)
 */
void trapSetFileInfo(unsigned short )
{
    unsigned int paramBlock = m68k_get_reg(0L, M68K_REG_A0);
    unsigned int async = 0; // If bit 10 of the instruction is set, it is async.

    unsigned int ret = mosPBSetFInfo(paramBlock, async);

    m68k_set_reg(M68K_REG_D0, ret);
}


/**
 * [A008] _Create
 * FUNCTION PBCreate (paramBlock: ParmBlkPtr; async: BOOLEAN) : OSErr;
 */
void trapCreate(unsigned short )
{
    unsigned int paramBlock = m68k_get_reg(0L, M68K_REG_A0);
    unsigned int async = 0; // If bit 10 of the instruction is set, it is async.

    unsigned int ret = mosPBCreate(paramBlock, async);

    m68k_set_reg(M68K_REG_D0, ret);
}


/**
 * [A012] _SetEOF
 * FUNCTION PBSetEOF (paramBlock: ParmBlkPtr; async: BOOLEAN) : OSErr;
 */
void trapSetEOF(unsigned short )
{
    unsigned int paramBlock = m68k_get_reg(0L, M68K_REG_A0);
    unsigned int async = 0; // If bit 10 of the instruction is set, it is async.

    unsigned int ret = mosPBSetEOF(paramBlock, async);

    m68k_set_reg(M68K_REG_D0, ret);
}


/**
 * [A012] _SetEOF
 * FUNCTION PBSetEOF (paramBlock: ParmBlkPtr; async: BOOLEAN) : OSErr;
 */
void trapSetFPos(unsigned short )
{
    unsigned int paramBlock = m68k_get_reg(0L, M68K_REG_A0);
    unsigned int async = 0; // If bit 10 of the instruction is set, it is async.

    unsigned int ret = mosPBSetFPos(paramBlock, async);

    m68k_set_reg(M68K_REG_D0, ret);
}


/**
 * [A002] _Read
 * FUNCTION PBRead (paramBlock: ParmBlkPtr; async: BOOLEAN) : OSErr;
 */
void trapRead(unsigned short )
{
    unsigned int paramBlock = m68k_get_reg(0L, M68K_REG_A0);
    unsigned int async = 0; // If bit 10 of the instruction is set, it is async.

    unsigned int ret = mosPBRead(paramBlock, async);

    m68k_set_reg(M68K_REG_D0, ret);
}


/**
 * [A003] _Write
 * FUNCTION PBWrite (paramBlock: ParmBlkPtr; async: BOOLEAN) : OSErr;
 */
void trapWrite(unsigned short )
{
    unsigned int paramBlock = m68k_get_reg(0L, M68K_REG_A0);
    unsigned int async = 0; // If bit 10 of the instruction is set, it is async.

    unsigned int ret = mosPBWrite(paramBlock, async);

    m68k_set_reg(M68K_REG_D0, ret);
}


/**
 * [A001] _Close
 * FUNCTION PBClose (paramBlock: ParmBlkPtr; async: BOOLEAN) : OSErr;
 */
void trapClose(unsigned short )
{
    unsigned int paramBlock = m68k_get_reg(0L, M68K_REG_A0);
    unsigned int async = 0; // If bit 10 of the instruction is set, it is async.

    unsigned int ret = mosPBClose(paramBlock, async);

    m68k_set_reg(M68K_REG_D0, ret);
}


/**
 * [A009] _Delete
 * FUNCTION PBClose (paramBlock: ParmBlkPtr; async: BOOLEAN) : OSErr;
 */
void trapDelete(unsigned short )
{
    unsigned int paramBlock = m68k_get_reg(0L, M68K_REG_A0);
    unsigned int async = 0; // If bit 10 of the instruction is set, it is async.

    unsigned int ret = mosPBDelete(paramBlock, async);

    m68k_set_reg(M68K_REG_D0, ret);
}


/**
 * [A060] _FSDispatch
 */
void trapFSDispatch(unsigned short )
{
    unsigned int paramBlock = m68k_get_reg(0L, M68K_REG_A0);
    unsigned int function = m68k_get_reg(0L, M68K_REG_D0);

    unsigned int ret = mosFSDispatch(paramBlock, function & 0xffff);

    m68k_set_reg(M68K_REG_D0, ret);
}


/**
 * [A200] _HOpen
 */
void trapHOpen(unsigned short )
{
    unsigned int paramBlock = m68k_get_reg(0L, M68K_REG_A0);
    unsigned int async = 0; // If bit 10 of the instruction is set, it is async.

    unsigned int ret = mosPBHOpen(paramBlock, async);

    m68k_set_reg(M68K_REG_D0, ret);
}


/**
 * [A9F4] Terminate the application and return control to the shell.
 *
 * PROCEDURE ExitToShell;
 *
 * Same effect as falling off the end of main() (the 0xaffc pseudo-op in
 * cpu.cpp), just invoked explicitly by the app instead of via RTS. Mirrors
 * that same result-code lookup so an app-initiated exit is reported the
 * same way.
 */
void trapExitToShell(unsigned short )
{
    unsigned int mpwHandle = m68k_read_memory_32(0x0316);
    unsigned int mpwMem = m68k_read_memory_32(mpwHandle+4);
    unsigned int resultCode = m68k_read_memory_32(mpwMem+0x000E);
    mosDebugPrintPCHistory();
    mosDebug("ExitToShell (returns %d)\n", resultCode);
    exit(resultCode);
}


/**
 * Go here for unimplemented traps.
 *
 * This function serves two purposes. It points out unimplemented traps that
 * are called during simulation. It also serves as a dummy address for all
 * unimplemented traps, which is often used at run-time to find out if certain
 * traps are actually implemented. This is when there is a complete lookup
 * table for each possible trap number.
 *
 * No inputs or outputs.
 */
void trapUninmplemented(unsigned short trap)
{
    mosDebugPrintPCHistory();

    // Give some feedback on a Toolbox trap
    if (mosIsToolboxTrap(trap)) {
        mosError("Unimplemented Toolbox trap %04X, index=%d: %s\n",
            trap, mosToolboxTrapIndex(trap), trapName(trap));
    }

    // Give some feedback on an OS trap
    if (mosIsOSTrap(trap)) {
        mosError("Unimplemented OS trap %04X, index=%d: %s\n",
            trap, mosOSTrapIndex(trap), trapName(trap)
        );
    }

    // Jump into the debugger (or exit)
    debug_break();
}

void trapPopUpMenuSelect(unsigned short ) {
    mosError("Unimplemented trap PopUpMenuSelect: %04X: %s\n", gCurrentTrap, trapName(gCurrentTrap));
    mosError("This is a hack to get NTK to launch");
    debug_break();
}

void trapWaitNextEvent(unsigned short ) {
    mosError("Unimplemented trap WaitNextEvent: %04X: %s\n", gCurrentTrap, trapName(gCurrentTrap));
    mosError("This is a hack to get NTK to launch");
    debug_break();
}

// params
// selector.w
// return address.l
// AECreateList:  INLINE $303C, $0706, $A816;
// AECountItems:  INLINE $303C, $0407, $A816;
// AEPutPtr:      INLINE $303C, $0A08, $A816;
// AEPutDesc:     INLINE $303C, $0609, $A816;
// AEPutKeyPtr:   INLINE $303C, $0A0F, $A816;
void trapPack8(unsigned short /* A816 */)
{
    uint16_t selector = (uint16_t)m68k_get_reg(0L, M68K_REG_D0);
    uint16_t words_on_stack = selector >> 8;

    mosPtr sp = m68k_get_reg(0L, M68K_REG_SP);
    mosPtr ret = m68k_read_memory_32(sp); sp += 4;

    uint32_t osErr = 0;
    bool found = false;
    bool handled = false;

    switch (selector) {
        // OSErr AEGetCoercionHandler(DescType from_type, DescType to_type, AECoerceDescUPP *hdlr_out, int32_t *refcon_out, Boolean *from_type_is_desc_p_out, Boolean system_handler_p)
        case 0x0B24:
            break;
        // OSErr AECreateDesc(DescType type, const void *data, Size data_size, AEDesc *desc_out)
        case 0x0825:
            break;
        // OSErr AEDisposeDesc(AEDesc *desc)
        case 0x0204:
            break;
        // OSErr AECoerceDesc(AEDesc *desc, DescType result_type, AEDesc *desc_out)
        case 0x0603:
            break;
        // OSErr AEGetParamPtr(AERecord *record, AEKeyword keyword, DescType desired_type, DescType *type_out, Ptr data, Size max_size, Size *size_out)
        case 0x0E11:
            break;
        // OSErr AEGetParamDesc(AERecord *record, AEKeyword keyword, DescType desired_type, AEDesc *desc_out)
        case 0x0812:
            break;
        // OSErr AEPutParamPtr(AERecord *record, AEKeyword keyword, DescType type, const void *data, Size data_size)
        case 0x0A0F:
            break;
        // OSErr AEPutParamDesc(AERecord *record, AEKeyword keyword, AEDesc *desc)
        case 0x0610:
            break;
        // OSErr AESizeOfParam(AERecord *record, AEKeyword keyword, DescType *type_out, Size *size_out)
        case 0x0829:
            break;
        // OSErr AESetInteractionAllowed(AEInteractionAllowed level)
        case 0x011E:
            break;
        // OSErr AEResetTimer(AppleEvent *evt)
        case 0x0219:
            break;
        // OSErr AEGetTheCurrentEvent(AppleEvent *return_evt)
        case 0x021A:
            break;
        // OSErr AESetTheCurrentEvent(AppleEvent *evt)
        case 0x022C:
            break;
        // OSErr AESuspendTheCurrentEvent(AppleEvent *evt)
        case 0x022B:
            break;
        // OSErr AEResumeTheCurrentEvent(AppleEvent *evt, AppleEvent *reply, AEEventHandlerUPP dispatcher, int32_t refcon)
        case 0x0818:
            break;
        // OSErr AEGetInteractionAllowed(AEInteractionAllowed *return_level)
        case 0x021D:
            break;
        // OSErr AEDuplicateDesc(AEDesc *src, AEDesc *dst)
        case 0x0405:
            break;
        // OSErr AECountItems(AEDescList *list, int32_t *count_out)
        case 0x0407:
            break;
        // OSErr AEDeleteItem(AEDescList *list, int32_t index)
        case 0x040E:
            break;
        // OSErr AEDeleteParam(AERecord *record, AEKeyword keyword)
        case 0x0413:
            break;
        // OSErr AEInstallSpecialHandler(AEKeyword function_class, AEEventHandlerUPP hdlr, Boolean system_handler_p)
        case 0x0500:
            break;
        // OSErr AERemoveSpecialHandler(AEKeyword function_class, AEEventHandlerUPP hdlr, Boolean system_handler_p)
        case 0x0501:
            break;
        // OSErr AEGetSpecialHandler(AEKeyword function_class, AEEventHandlerUPP *hdlr_out, Boolean system_handler_p)
        case 0x052D:
            break;
        // OSErr AESend(AppleEvent *evt, AppleEvent *reply, AESendMode send_mode, AESendPriority send_priority, int32_t timeout, IdleUPP idle_proc, EventFilterUPP filter_proc)
        case 0x0D17:
            break;
        // OSErr AECoercePtr(DescType data_type, Ptr data, Size data_size, DescType result_type, AEDesc *desc_out)
        case 0x0A02:
            break;
        // OSErr AEGetEventHandler(AEEventClass event_class, AEEventID event_id, AEEventHandlerUPP *hdlr, int32_t *refcon, Boolean system_handler_p)
        case 0x0921:
            break;
        // OSErr AERemoveEventHandler(AEEventClass event_class, AEEventID event_id, AEEventHandlerUPP hdlr, Boolean system_handler_p)
        case 0x0720:
            break;
        // OSErr AEProcessAppleEvent(EventRecord *evt)
        case 0x021B:
            break;
        // OSErr AEPutDesc(AEDescList *list, int32_t index, AEDesc *desc)
        case 0x0609:
            break;
        // OSErr AEPutAttributePtr(AppleEvent *evt, AEKeyword keyword, DescType type, const void *data, Size size)
        case 0x0A16:
            break;
        // OSErr AEPutAttributeDesc(AppleEvent *evt, AEKeyword keyword, AEDesc *desc)
        case 0x0627:
            break;
        // OSErr AEGetNthPtr(AEDescList *list, int32_t index, DescType desired_type, AEKeyword *keyword_out, DescType *type_out, void *data, int32_t max_size, int32_t *size_out)
        case 0x100A:
            break;
        // OSErr AEGetAttributePtr(AppleEvent *evt, AEKeyword keyword, DescType desired_type, DescType *type_out, void *data, Size max_size, Size *size_out)
        case 0x0E15:
            break;
        // OSErr AEGetArray(AEDescList *list, AEArrayType array_type, AEArrayDataPointer array_ptr, Size max_size, DescType *return_item_type, Size *return_item_size, int32_t *return_item_count)
        case 0x0D0C:
            break;
        // OSErr AECreateAppleEvent(AEEventClass event_class, AEEventID event_id, AEAddressDesc *target, int16_t return_id, int32_t transaction_id, AppleEvent *evt)
        case 0x0B14:
            break;
        // OSErr AEInstallCoercionHandler(DescType from_type, DescType to_type, AECoerceDescUPP hdlr, int32_t refcon, Boolean from_type_is_desc_p, Boolean system_handler_p)
        case 0x0A22:
            break;
        // OSErr AEInstallEventHandler(AEEventClass event_class, AEEventID event_id, AEEventHandlerUPP hdlr, int32_t refcon, Boolean system_handler_p)
        case 0x091F:
            break;
        // OSErr AERemoveCoercionHandler(DescType from_type, DescType to_type, AECoerceDescUPP hdlr, Boolean system_handler_p)
        case 0x0723:
            break;
        // OSErr AEPutArray(AEDescList *list, AEArrayType type, AEArrayDataPointer array_data, DescType item_type, Size item_size, int32_t item_count)
        case 0x0B0D:
            break;
        // OSErr AECreateList(Ptr list_elt_prefix, Size list_elt_prefix_size, Boolean is_record_p, AEDescList *list_out)
        case 0x0706:
            break;
        // OSErr AEGetAttributeDesc(AppleEvent *evt, AEKeyword keyword, DescType desired_type, AEDesc *desc_out)
        case 0x0826:
            break;
        // OSErr AESizeOfAttribute(AppleEvent *evt, AEKeyword keyword, DescType *type_out, Size *size_out)
        case 0x0828:
            break;
        // OSErr AEGetNthDesc(AEDescList *list, int32_t index, DescType desired_type, AEKeyword *keyword_out, AEDesc *desc_out)
        case 0x0A0B:
            break;
        // OSErr AESizeOfNthItem(AEDescList *list, int32_t index, DescType *type_out, Size *size_out)
        case 0x082A:
            break;
        // OSErr AEPutPtr(AEDescList *list, int32_t index, DescType type, const void *data, Size data_size)
        case 0x0A08:
            break;
        // OSErr AEInteractWithUser(int32_t timeout, NMRecPtr nm_req, IdleUPP idle_proc)
        case 0x061C:
            found = true; // Preparing to show a dialog. Nothing else to do here at the moment.
            break;
        // OSErr AEManagerInfo(LONGINT *resultp)
        case 0x0441:
            break;
        // OSErr AEDisposeToken(AEDesc *theToken)
        case 0x023A:
            break;
        // OSErr AEResolve(AEDesc *objectSpecifier, INTEGER callbackFlags, AEDesc *theToken)
        case 0x0536:
            break;
        // OSErr AERemoveObjectAccessor(DescType desiredClass, DescType containerType, ProcPtr theAccessor, Boolean isSysHandler)
        case 0x0738:
            break;
        // OSErr AEInstallObjectAccessor(DescType desiredClass, DescType containerType, ProcPtr theAccessor, LONGINT refcon, Boolean isSysHandler)
        case 0x0937:
            break;
        // OSErr AEGetObjectAccessor(DescType desiredClass, DescType containerType, ProcPtr *theAccessor, LONGINT *accessorRefcon, Boolean isSysHandler)
        case 0x0939:
            break;
        // OSErr AECallObjectAccessor(DescType desiredClass, AEDesc *containerToken, DescType containerClass, DescType keyForm, AEDesc *keyData, AEDesc *theToken)
        case 0x0C3B:
            break;
        // OSErr AESetObjectCallbacks(ProcPtr myCompareProc, ProcPtr myCountProc, ProcPtr myDisposeTokenProc, ProcPtr myGetMarkTokenProc, ProcPtr myMarkProc, ProcPtr myAdjustMarksProc, ProcPtr myGetErrDescProc)
        case 0x0E35:
            break;
        default:
            break;
    }

    // Fix up the stack. Skip all the parameter that the caller pushed here.
    if (!handled) {
        sp += words_on_stack * 2; // each word is 2 bytes
    }

    m68k_write_memory_16(sp, osErr);
    sp -= 4; m68k_write_memory_32(sp, ret);
    m68k_set_reg(M68K_REG_SP, sp);
    //m68k_set_reg(M68K_REG_D0, osErr);

    if (found) return;

    mosDebugPrintPCHistory();
    mosError("Unimplemented trap Pack8: %04X: %s\n", gCurrentTrap, trapName(gCurrentTrap));
    mosError("This is a hack to get NTK to launch");

    debug_break();
}

/**
 * Set the current resource file.
 * \code
 * PROCEDURE UseResFile (refNum: Integer);
 * \endcode
 * Toolbox
 */
void trapUseResFile(unsigned short /* instr */) // A998
{
    mosPtr sp = m68k_get_reg(0L, M68K_REG_SP);
    mosPtr ret = m68k_read_memory_32(sp); sp+=4; // pop the return address
    uint16_t refNum = m68k_read_memory_16(sp); sp+=2; // pop the refNum

    mosDebug("UseResFile: refNum=%u\n", refNum);
    if (refNum != 1) {
        printf("UseResFile: refNum=%u is not 1, not supported\n", refNum);
    }

    sp -= 4; m68k_write_memory_32(sp, ret);
    m68k_set_reg(M68K_REG_SP, sp);

}
/**
 * Handle the _HWPriv trap.
 */
void trapHWPriv(unsigned short /* instr */) // A198
{
    //uint16_t selector = m68k_get_reg(0L, M68K_REG_D0);
    //mosDebug("_HWPriv trap invoked (selector = %d)\n", selector);
    m68k_set_reg(M68K_REG_D0, 0); // Pretend we did it!
}

/**
 * Go from m68k emulation into host native code.
 *
 * This is not an original MacOS trap.
 *
 * The current PC is used as a pointer to the TrapNativeCall structure, which
 * then contains the pointer to the native function and the RTS instruction.
 */
void trapGoNative(unsigned short instr)
{
    unsigned int pc = m68k_get_reg(0L, M68K_REG_PC);

    void (*nativeCodeEntry)(uint16_t) = (void (*)(uint16_t))mosReadUnsafe64(pc+4);
    nativeCodeEntry(instr);

    pc = pc + 12; // skip alignment space and native target address
    m68k_set_reg(M68K_REG_PC, pc);
}


/**
 * This trap is used to set a breakpoint anywhere in m68k code.
 *
 * This is not an original MacOS trap.
 *
 * It is up to the native code to set a true breakpoint within this function.
 * This function can alos be used to output diagnostig messages.
 *
 * After hitting a breakpoint, the original code is put back into place for
 * the next emulation step. After that, the breakpoint is reinstated.
 */
void trapBreakpoint(unsigned short )
{
    unsigned int pc = m68k_get_reg(0L, M68K_REG_PC);

    Breakpoint *bp = findBreakpoint(pc);
    if (bp) {
        mosDebug("BREAKPOINT: %s\n", bp->text);
        gPendingBreakpoint = bp;
    } else {
        mosDebug("BREAKPOINT UNLISTED!\n");
    }
    bp = 0;
}


/**
 * This is the actual Axxx trap that dispatches according to the xxx part of the command.
 *
 * The trap vector 0x00000028 points to a 0xaffd instruction, leading here.
 * The stack contains the processor specific exception information and the
 * CPU is in supervisor mode. However, we want to run the trap code in user
 * mode, having only the return address on the stack.
 */
void trapALineDispatch(unsigned short trap)
{
    unsigned int sp = m68k_get_reg(0L, M68K_REG_SP);
    unsigned int new_sr = m68k_read_memory_16(sp); sp+=2;   // pop the status register
    unsigned int ret_addr = m68k_read_memory_32(sp); sp+=4; // pop the return address
    /*unsigned int vec =*/ m68k_read_memory_16(sp); sp+=2;  // pop the vector number

    sp-=4; m68k_write_memory_32(sp, ret_addr);              // push the return address again
    m68ki_set_sr_noint(new_sr);                             // leave interrupt mode
    // just leave the return address on the stack
    m68k_set_reg(M68K_REG_SP, sp);
    // Handle Toolbox and OS traps according to the current trap number.
    // 1010.1z0x.xxxx.xxxx: Toolbox: x = trap #, if z is 1, pop the extra return address from the stack
    // 1010.0yyz.xxxx.xxxx: OS tarp: x = trap #, yy are extra flags that can be used by the traps

    mosPtr trapAddress = 0;
    if (mosIsOSTrap(trap)) {
        trapAddress = GetOSTrapAddress(trap);
    } else {
        if (mosToolboxTrapAutoPop(trap)) {
            sp+=4; // Pop the latest return address. It's just a trampoline somewhere.
            m68k_set_reg(M68K_REG_SP, sp);
            gCurrentTrap = trap = trap & 0xFBFF; // Clear the AutoPop flag.
        }
        trapAddress = GetToolboxTrapAddress(trap);
    }
    m68k_set_reg(M68K_REG_PC, trapAddress);
}


/**
 * Handle F-line (coprocessor / "Line 1111 Emulator") exceptions.
 *
 * mosrun does not emulate an FPU or MMU coprocessor. Any F-line instruction
 * (opcode 0xFxxx) that Musashi does not itself implement raises this real
 * 68k exception via vector 0x2C, the same mechanism used for A-line traps
 * via vector 0x28 -- except there is no gCurrentTrap to look up here, since
 * m68k_instruction_hook only pre-intercepts A-line opcodes before Musashi
 * executes them. Left unhandled, vector 0x2C used to read back as an
 * unmapped 0, so the CPU would jump to address 0 and start "executing"
 * whatever was in low memory, corrupting PC until it happened to wander
 * into something recognizable. Report the fault clearly and stop instead,
 * the same way trapUninmplemented does for missing traps.
 *
 * The trap vector 0x0000002C points to a 0xaffb instruction, leading here.
 * The stack contains the processor's exception stack frame; the CPU is in
 * supervisor mode.
 */
void trapFLineDispatch(unsigned short)
{
    unsigned int sp = m68k_get_reg(0L, M68K_REG_SP);
    unsigned int new_sr  = m68k_read_memory_16(sp); sp+=2; // pop the status register
    unsigned int ret_addr = m68k_read_memory_32(sp); sp+=4; // address of the faulting F-line instruction
    /*unsigned int vec =*/ m68k_read_memory_16(sp); sp+=2;

    unsigned int instr = m68k_read_memory_16(ret_addr);
    mosError("Unimplemented coprocessor (F-line) instruction 0x%04X at %s\n", instr, printAddr(ret_addr));
    debug_break();

    // If resumed under a debugger, skip past the offending opcode word so we
    // do not immediately re-trigger the same fault.
    sp -= 4; m68k_write_memory_32(sp, ret_addr+2);
    m68ki_set_sr_noint(new_sr);
    m68k_set_reg(M68K_REG_SP, sp);
    m68k_set_reg(M68K_REG_PC, ret_addr+2);
}


/**
 * Create jump table entry in simulator space.
 */
mosPtr createGlue(unsigned short trapNum, void (*nativeCodeEntry)(uint16_t))
{
    // Create the glue that connects the trap table entry to the native code function
    mosPtr p = mosNewPtr(16);           // 16 bytes
    mosWrite16(p+ 0, 0xAFFF);           // trap to native
    mosWriteUnsafe64(p+ 4, (uintptr_t)nativeCodeEntry);  // pointer into host memory
    mosWrite16(p+12, 0x4E75);           // rts

    if (trapNum == 0) return p;

    if (mosIsOSTrap(trapNum)) {
        // printf("Creating glue for OS trap 0x%04X (%d) at %p\n", trapNum, mosOSTrapIndex(trapNum), (void*)nativeCodeEntry);
        if (GetOSTrapAddress(trapNum) != gGlue_Unimplemented) {
            mosWarning("Overwriting existing OS trap glue for trap 0x%04X\n", trapNum);
        }
        SetOSTrapAddress(p, trapNum);
    } else {
        // printf("Creating glue for Toolbox trap 0x%04X (%d) at %p\n", trapNum, mosToolboxTrapIndex(trapNum), (void*)nativeCodeEntry);
        if (GetToolboxTrapAddress(trapNum) != gGlue_Unimplemented) {
            mosWarning("Overwriting existing Toolbox trap glue for trap 0x%04X\n", trapNum);
        }
        SetToolboxTrapAddress(p, trapNum);
    }

    return p;
}


/**
 * Create a jump table for all possible trap commands.
 */
void mosSetupTrapTable()
{
    int i;

    // Create the glue for the _Unimplemented call
    gGlue_Unimplemented = createGlue(0, trapUninmplemented);

    // Set up the Toolbox trap table to all _Unimplemented
    gToolboxTrapTable = mosNewPtr(kToolboxTrapTableSize*4);
    for (i=0; i<kToolboxTrapTableSize; i++) {
        mosWrite32(gToolboxTrapTable+i*4, gGlue_Unimplemented);
    }

    // Set up the OS trap table to all _Unimplemented
    gOSTrapTable = mosNewPtr(kOSTrapTableSize*4);
    for (i=0; i<kOSTrapTableSize; i++) {
        mosWrite32(gOSTrapTable+i*4, gGlue_Unimplemented);
    }

    // -- Initialization and Allocation

    // InitApplZone
    // SetApplBase
    // InitZone
    // GetApplLimit
    // SetAppleLimit
    // MaxApplZone
    // MoreMasters

    // -- Heap Zone Access

    // GetZone
    // SetZone
    // SystemZone
    // ApplicZone

    // -- Allocating and Releasing Relocatable Blocks

    createGlue(0xA122, trapNewHandle);
    createGlue(0xA023, trapDisposeHandle);
    createGlue(0xA025, trapGetHandleSize);
    createGlue(0xA024, trapSetHandleSize);
    // HandleZone
    createGlue(0xA128, trapRecoverHandle);
    // ReallocHandle

    // -- Allocating and Releasing Nonrelocatable Blocks

    createGlue(0xA11E, trapNewPtr);
    createGlue(0xA01F, trapDisposePtr);
    // GetPtrSize
    // SetPtrSize
    // PtrZone

    // -- Freeing Space in the Heap

    // FreeMem
    // MaxMem
    // CompactMem
    // ReservMem
    // PurgeMem
    // EmptyHandle

    // -- Properties of Relocatable Blocks

    createGlue(0xA029, trapHLock);
    createGlue(0xA02A, trapHUnlock);
    createGlue(0xA049, trapHPurge);
    createGlue(0xA04A, trapHNoPurge);
    createGlue(0xA162, trapPurgeSpace);
    //tncTable[0x0562] = tncTable[0x0162]; // _PurgeSpaceSys, same signature
    createGlue(0xA9F4, trapExitToShell);

    // -- Grow Zone Operations

    // SetGrowZone
    // GZSaveHnd

    // -- Misc

    createGlue(0xA02E, trapBlockMove);
    // TopMem
    createGlue(0xA064, trapMoveHHi);
    // MemError

    // -- Low Level File Functions

    createGlue(0xA000, trapHOpen);
    //tncTable[0x0200] = tncTable[0x0000];
    createGlue(0xA001, trapClose);
    createGlue(0xA002, trapRead);
    createGlue(0xA003, trapWrite);
    createGlue(0xA008, trapCreate);
    createGlue(0xA009, trapDelete);
    //tncTable[0x0209] = tncTable[0x0009];
    createGlue(0xA00C, trapGetFileInfo);
    createGlue(0xA00D, trapSetFileInfo);
    createGlue(0xA012, trapSetEOF);
    createGlue(0xA044, trapSetFPos);
    createGlue(0xA060, trapFSDispatch);

    // -- unsorted

    // createGlue(0xA146, trapGetTrapAddress);
    //tncTable[0x0746] = tncTable[0x0146];
    //tncTable[0x0346] = tncTable[0x0146];
    //createGlue(0xA647, trapSetTrapAddress);
    createGlue(0xA9F0, trapLoadSeg);
    createGlue(0xA9F1, trapUnloadSeg);
    createGlue(0xA069, trapHGetState);
    createGlue(0xA06A, trapHSetState);
    createGlue(0xA055, trapStripAddress);
    createGlue(0xA9A0, trapGetResource);
    createGlue(0xA81F, trapGetResource); // _Get1Resource
    createGlue(0xA9A2, trapLoadResource);
    createGlue(0xA9A5, trapSizeResource);
    createGlue(0xA9A1, trapGetNamedResource);
    createGlue(0xA820, trapGetNamedResource); // _Get1NamedResource
    //tncTable[0x0820] = tncTable[0x09A1];
    createGlue(0xA88F, trapOSDispatch);
    createGlue(0xA9C6, trapSecondsToDate);
    createGlue(0xA975, trapTickCount);
    createGlue(0xA9A3, trapReleaseResource);
    createGlue(0xA051, trapReadXPRam);
    createGlue(0xA994, trapCurResFile);
    createGlue(0xA99B, trapSetResLoad);
    createGlue(0xA9A4, trapHomeResFile);
    createGlue(0xA9A6, trapGetResAttrs);
    createGlue(0xA9A7, trapSetResAttrs);

    // For NTK: startup code chack if this is implemented
    createGlue(0xA80B, trapPopUpMenuSelect);
    createGlue(0xA860, trapWaitNextEvent);
    createGlue(0xA816, trapPack8);
    createGlue(0xA998, trapUseResFile);
    createGlue(0xA198, trapHWPriv); // _HWPriv

    mosSetupTrapManager();
    mosSetupGestaltTraps();
    mosSetupQuickDrawTraps();
    mosSetupWindowMgrTraps();
    mosSetupEventMgrTraps();
    mosSetupMenuMgrTraps();
    mosSetupDialogMgrTraps();
}



