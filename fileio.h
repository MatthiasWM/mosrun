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

#ifndef __mosrun__fileio__
#define __mosrun__fileio__

#include "main.h"


// FIXME: MosFile must be a mos object
typedef struct
{
    int fd;
    const char *filename;
    bool open;
    bool allocated;
} MosFile;


extern MosFile stdFiles[];


void trapSyFAccess(uint16_t);
void trapSyClose(uint16_t);
void trapSyRead(uint16_t);
void trapSyWrite(uint16_t);
void trapSyIoctl(uint16_t);

int mosPBGetFInfo(mosPtr paramBlock, bool async);
int mosPBSetFInfo(mosPtr paramBlock, bool async);
int mosPBCreate(mosPtr paramBlock, bool async);
int mosPBSetEOF(mosPtr paramBlock, bool async);
int mosPBSetFPos(mosPtr paramBlock, bool async);
int mosPBRead(mosPtr paramBlock, bool async);
int mosPBWrite(mosPtr paramBlock, bool async);
int mosPBClose(mosPtr paramBlock, bool async);
int mosPBHOpen(mosPtr paramBlock, bool async);
int mosPBDelete(mosPtr paramBlock, bool async);
int mosFSDispatch(mosPtr paramBlock, uint32_t func);

#endif /* defined(__mosrun__fileio__) */
