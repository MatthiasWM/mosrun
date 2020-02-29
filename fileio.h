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


typedef struct
{
  int fd;
  const char *filename;
  bool open;
  bool allocated;
} MosFile;


extern MosFile stdFiles[];


void trapSyFAccess(unsigned short);
void trapSyClose(unsigned short);
void trapSyRead(unsigned short);
void trapSyWrite(unsigned short);
void trapSyIoctl(unsigned short);

int mosPBGetFInfo(unsigned int paramBlock, bool async);
int mosPBSetFInfo(unsigned int paramBlock, bool async);
int mosPBCreate(unsigned int paramBlock, bool async);
int mosPBSetEOF(unsigned int paramBlock, bool async);
int mosPBSetFPos(unsigned int paramBlock, bool async);
int mosPBRead(unsigned int paramBlock, bool async);
int mosPBWrite(unsigned int paramBlock, bool async);
int mosPBClose(unsigned int paramBlock, bool async);
int mosPBHOpen(unsigned int paramBlock, bool async);
int mosPBDelete(unsigned int paramBlock, bool async);
int mosFSDispatch(unsigned int paramBlock, unsigned int func);

#endif /* defined(__mosrun__fileio__) */
