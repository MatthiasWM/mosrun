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

#ifndef __mosrun__filename__
#define __mosrun__filename__


const int MOS_TYPE_UNKNOWN  = 0;
const int MOS_TYPE_RAW      = 1;
const int MOS_TYPE_MAC      = 2;
const int MOS_TYPE_UNIX     = 3;


int mosFilenameGuessType(const char *filename);

char *mosFilenameConvertTo(const char *filename, int type);

const char *mosFilenameName(const char *filename);
const char *mosFilenameNameUnix(const char *filename);

char *mosDataUnixToMac(const char *text, unsigned int &size);
char *mosDataMacToUnix(const char *text, unsigned int &size);


#endif /* defined(__mosrun__filename__) */
