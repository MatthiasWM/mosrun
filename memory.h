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

#ifndef __mosrun__memory__
#define __mosrun__memory__


#include "main.h"

void mosWrite32(mosPtr addr, unsigned int value);
void mosWrite16(mosPtr addr, unsigned short value);
void mosWrite8(mosPtr addr, unsigned char value);

unsigned int mosRead32(mosPtr addr);
unsigned short mosRead16(mosPtr addr);
unsigned char mosRead8(mosPtr addr);

void mosWriteUnsafe32(mosPtr addr, unsigned int value);
void mosWriteUnsafe16(mosPtr addr, unsigned short value);
void mosWriteUnsafe8(mosPtr addr, unsigned char value);

unsigned int mosReadUnsafe32(mosPtr addr);
unsigned short mosReadUnsafe16(mosPtr addr);
unsigned char mosReadUnsafe8(mosPtr addr);

mosPtr mosNewPtr(unsigned int size);
mosPtr mosNewPtr(const char *text);
void mosDisposePtr(mosPtr);
unsigned int mosPtrSize(mosPtr);

mosHandle mosNewHandle(unsigned int size);
void mosDisposeHandle(mosHandle);
mosHandle mosRecoverHandle(mosPtr);
int mosSetHandleSize(mosHandle, unsigned int);

unsigned int mosCheckBounds(mosPtr, unsigned int size);

mosPtr mosToPtr(struct MosBlock *block);
struct MosBlock *mosToBlock(mosPtr ptr);

#ifdef MOS_UNITTESTS
void mosMemoryUnittests();
#endif

#endif /* defined(__mosrun__memory__) */
