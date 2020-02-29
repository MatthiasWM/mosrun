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

#ifndef mosrun_main_h
#define mosrun_main_h


#define MOS_BOUNDS_CHECK 0

#define MOS_O_RDONLY      0x00      /* Open for reading only.            */
#define MOS_O_WRONLY      0x01      /* Open for writing only.            */
#define MOS_O_RDWR        0x02      /* Open for reading & writing.       */
#define MOS_O_APPEND      0x08      /* Write to the end of the file.     */
#define MOS_O_RSRC        0x10      /* Open the resource fork.           */
#define MOS_O_ALIAS       0x20      /* Open alias file.                  */
#define MOS_O_CREAT      0x100      /* Open or create a file.            */
#define MOS_O_TRUNC      0x200      /* Open and truncate to zero length. */
#define MOS_O_EXCL       0x400      /* Create file only; fail if exists. */
#define MOS_O_BINARY     0x800      /* Open as a binary stream.          */
#define MOS_O_NRESOLVE  0x4000      /* Don't resolve any aliases.        */

#define MOS_SEEK_CUR        1
#define MOS_SEEK_END        2
#define MOS_SEEK_SET        0


const int MOS_STACK_SIZE = 0x8000;

const int mosNoErr =         0; // success, party!
const int mosBdNamErr =    -37; // bad file name or volume name (perhaps zero length)
const int mosEofErr =      -39; // error setting position beyon EOF
const int mosFnfErr =      -43; // file not found
const int mosDupFNErr =    -48; // duplicate filename (rename)
const int mosParamErr =    -50; // bad parameter passed

typedef unsigned int mosPtr;
typedef unsigned int mosHandle;
typedef unsigned int uint;
typedef unsigned char byte;
typedef unsigned int mosResType;

typedef void (*mosTrap)(unsigned short);

extern byte *theApp;
extern unsigned int theAppSize;
extern byte *theRsrc;
extern unsigned int theRsrcSize;
extern byte *theJumpTable;

extern byte gFilterStdoutDataFrom;
extern byte gFilterStdoutDataTo;

#endif
