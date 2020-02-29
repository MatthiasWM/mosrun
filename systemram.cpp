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


// Include our own interfaces

#include "systemram.h"
#include "names.h"
#include "log.h"
#include "memory.h"
#include "breakpoints.h"
#include "traps.h"


unsigned int gMosCurrentA5 = 0;
unsigned int gMosCurrentStackBase = 0;
unsigned int gMosCurJTOffset = 0;
unsigned int gMosResLoad = 1;
unsigned int gMosResErr = 0;
unsigned int gMosMPWHandle = 0;



unsigned int gMosMemErr = 0;

/**
 * Set the error code of the last memory operation.
 *
 * MacOS uses the following codes:
 * menuPrgErr     84  A menu was purged.
 * negZcbFreeErr  33  A heap has been corrupted.
 * memROZErr     -99  Operation on a read-only zone. This result code is not relevant in Mac OS X.
 * memFullErr   -108  Not enough memory in heap.
 * nilHandleErr -109  Handle argument is NULL.
 * memAdrErr    -110  Address is odd or out of range.
 * memWZErr     -111  Attempt to operate on a free block.
 * memPurErr    -112  Attempt to purge a locked or unpurgeable block.
 * memAZErr     -113  Address in zone check failed.
 * memPCErr     -114  Pointer check failed.
 * memBCErr     -115  Block check failed.
 * memSCErr     -116  Size check failed.
 * memLockedErr -117  Block is locked.
 */
void mosSetMemError(unsigned int err)
{
  gMosMemErr = err;
}

/**
 * Get the error code of the last memory operation.
 */
unsigned int mosGetMemError()
{
  return gMosMemErr;
}



/**
 * Read from memeory, including the system page.
 */
unsigned int m68k_read_memory_8(unsigned int address)
{
  if (address>=0x1E00) {
    return mosRead8(address);
  }
  if (address<0x1E00) {
    const char *rem = "";
    const char *var = gvarName(address, &rem);
    mosTrace("Read.b 0x%04x: %s %s\n", address, var, rem);
  }
  switch (address) {
    case 0x012d: return 0; // LoadTrap [GLOBAL VAR]  trap before launch? [byte]
    default:
      mosDebug("Accessing unsupported RAM.b address 0x%08X\n", address);
      break;
  }
  return 0;
}


/**
 * Read from memory, including the system page.
 */
unsigned int m68k_read_memory_16(unsigned int address)
{
  if (gPendingBreakpoint && gPendingBreakpoint->address==address) {
    return gPendingBreakpoint->originalCmd;
  }
  if (address>=0x1E00) {
    return mosRead16(address);
  }
  if (address<0x1E00) {
    const char *rem = "";
    const char *var = gvarName(address, &rem);
    mosTrace("Read.w 0x%04x: %s %s\n", address, var, rem);
  }
  switch (address) {
    case 0x0070: return 0xa9f4; // ExitToShell() -> debug trap: quit the app
    case 0x028e: return 0; // >0 if 128k ROM mac or later
    case 0x0934: return gMosCurJTOffset; // offset from A5 to first entry in jump table
    case 0x0a60: return gMosResErr; // Resource Manager error code
    case 0x0220: return mosGetMemError();
    case 0x0930: return 0; // FIXME: SaveSegHandle [GLOBAL VAR]  seg 0 handle [handle]
    default:
      mosDebug("Accessing unsupported RAM.w address 0x%08X\n", address);
      break;
  }
  return 0;
}


/**
 * Read from memory, including the system page.
 */
unsigned int m68k_read_memory_32(unsigned int address)
{
  if (address>=0x1E00) {
    return mosRead32(address);
  }
  if (address<0x1E00) {
    const char *rem = "";
    const char *var = gvarName(address, &rem);
    mosTrace("Read.l 0x%04x: %s %s\n", address, var, rem);
  }
  switch (address) {
    case 0: return 0;
    case 4: return 0;
    case 0x0028: return trapDispatchTrap;
    case 0x020C: return mosTickCount(); /* Time */
    case 0x0316: return gMosMPWHandle;
    case 0x0910: // CurApName [GLOBAL VAR] Name of current application (length byte followed by up to 31 characters) name of application [STRING[31]]
    case 0x0914:
    case 0x0918:
    case 0x091c:
    case 0x0920:
    case 0x0924:
    case 0x0928:
    case 0x092c:
      return 0;
    default:
      mosDebug("Accessing unsupported RAM.l address 0x%08X\n", address);
      break;
  }
  return 0;
}


/**
 * Read from memory, including the system page.
 */
unsigned int m68k_read_disassembler_8(unsigned int address)
{
  return m68k_read_memory_8(address);
}


/**
 * Read from memory, including the system page.
 */
unsigned int m68k_read_disassembler_16(unsigned int address)
{
  return m68k_read_memory_16(address);
}


/**
 * Read from memory, including the system page.
 */
unsigned int m68k_read_disassembler_32(unsigned int address)
{
  return m68k_read_memory_32(address);
}


/**
 * Write to memory, including the system page.
 */
void m68k_write_memory_8(unsigned int address, unsigned int value)
{
  if (address>=0x1E00) {
    mosWriteUnsafe8(address, value);
    return;
  }
  if (address<0x1E00) {
    const char *rem = "";
    const char *var = gvarName(address, &rem);
    mosTrace("Write.b 0x%04x = 0x%02X: %s %s\n", address, value & 0xff, var, rem);
  }
  switch (address) {
    case 0x0a5e: gMosResLoad = value; break; // ResLoad       0A5E  word  Auto-load feature
    default:
      mosDebug("Writing unsupported RAM.b address 0x%08X\n", address);
      break;
  }
}


/**
 * Write to memory, including the system page.
 */
void m68k_write_memory_16(unsigned int address, unsigned int value)
{
  if (address>=0x1E00) {
    mosWriteUnsafe16(address, value);
    return;
  }
  if (address<0x1E00) {
    const char *rem = "";
    const char *var = gvarName(address, &rem);
    mosTrace("Write.w 0x%04x = 0x%04X: %s %s\n", address, value & 0xffff, var, rem);
  }
  switch (address) {
    default:
      mosDebug("Writing unsupported RAM.w address 0x%08X\n", address);
      break;
  }
}


/**
 * Write to memory, including the system page.
 */
void m68k_write_memory_32(unsigned int address, unsigned int value)
{
  if (address>=0x1E00) {
    mosWriteUnsafe32(address, value);
    return;
  }
  if (address<0x1E00) {
    const char *rem = "";
    const char *var = gvarName(address, &rem);
    mosTrace("Write.l 0x%04x = 0x%08X: %s %s\n", address, value, var, rem);
  }
  switch (address) {
    default:
      mosDebug("Writing unsupported RAM.l address 0x%08X\n", address);
      break;
  }
}



