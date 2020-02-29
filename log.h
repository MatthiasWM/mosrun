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

#ifndef __mosrun__log__
#define __mosrun__log__


#include <stdio.h>


const int MOS_VERBOSITY_ERR   = 0;
const int MOS_VERBOSITY_WARN  = 1;
const int MOS_VERBOSITY_LOG   = 2;
const int MOS_VERBOSITY_DEBUG = 3;
const int MOS_VERBOSITY_TRACE = 4;

FILE *mosLogFile();
void mosLogVerbosity(int v);
int mosLogVerbosity();
void mosLogTo(FILE *out);
void mosLogClose();

void mosTrace(const char *format, ...);
void mosDebug(const char *format, ...);
void mosLog(const char *format, ...);
void mosWarning(const char *format, ...);
void mosError(const char *format, ...);


#endif /* defined(__mosrun__log__) */
