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

#include "fileio.h"
#include "filename.h"
#include "memory.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/xattr.h>
#include <fcntl.h>
#include <sys/stat.h>

extern "C" {
#include "musashi331/m68k.h"
}


MosFile stdFiles[] = {
  { STDIN_FILENO,  "/dev/stdin", true, false },
  { STDOUT_FILENO, "/dev/stdout", true, false },
  { STDERR_FILENO, "/dev/stderr", true, false }
};


///* 'd' => "directory" ops */
//#define F_DELETE        (('d'<<8)|0x01)
//#define F_RENAME        (('d'<<8)|0x02)
//#define F_OPEN          (('d'<<8)|0x00)     /* reserved for operating system use  */
///* 'e' => "editor" ops */
//#define F_GTABINFO      (('e'<<8)|0x00)     /* get tab offset for file            */
//#define F_STABINFO      (('e'<<8)|0x01)     /* set  "   "      "   "              */
//#define F_GFONTINFO     (('e'<<8)|0x02)     /* get font number and size for file  */
//#define F_SFONTINFO     (('e'<<8)|0x03)     /* set  "     "     "   "    "   "    */
//#define F_GPRINTREC     (('e'<<8)|0x04)     /* get print record for file          */
//#define F_SPRINTREC     (('e'<<8)|0x05)     /* set   "     "     "   "            */
//#define F_GSELINFO      (('e'<<8)|0x06)     /* get selection information for file */
//#define F_SSELINFO      (('e'<<8)|0x07)     /* set     "          "       "   "   */
//#define F_GWININFO      (('e'<<8)|0x08)     /* get current window position        */
//#define F_SWININFO      (('e'<<8)|0x09)     /* set    "      "       "            */
//#define F_GSCROLLINFO   (('e'<<8)|0x0A)     /* get scroll information             */
//#define F_SSCROLLINFO   (('e'<<8)|0x0B)     /* set    "        "                  */
//#define F_GMARKER       (('e'<<8)|0x0D)     /* Get Marker                         */
//#define F_SMARKER       (('e'<<8)|0x0C)     /* Set   "                            */
//#define F_GSAVEONCLOSE  (('e'<<8)|0x0F)     /* Get Save on close                  */
//#define F_SSAVEONCLOSE  (('e'<<8)|0x0E)     /* Set  "   "    "                    */


/** Open a file.
 * Stack
 *   +00.l = return address
 *   +04.l = filename
 *   +08.l = command
 *   +0c.l = ptr to return value?
 */
void trapSyFAccess(unsigned short) {
  unsigned int sp = m68k_get_reg(0L, M68K_REG_SP);
  const char *filename = (char*)m68k_read_memory_32(sp+4);
  unsigned int cmd = m68k_read_memory_32(sp+8);
  unsigned int file = m68k_read_memory_32(sp+12);
  unsigned short flags = m68k_read_memory_16(file);
  mosTrace("Accessing file '%s', cmd=0x%08X, arg=0x%08X, flags=0x%04X\n", filename, cmd, file, flags);
  if (cmd!=0x00006400) { // '..d.'
    mosError("trapSyFAccess: Unknown file access command 0x%08X\n", cmd);
    m68k_set_reg(M68K_REG_D0, EINVAL); // no error
    return;
  }
  char *uxFilename = strdup(mosFilenameConvertTo(filename, MOS_TYPE_UNIX));
  // TODO: add our MosFile reference for internal data management
  // TODO: find the actual file and open it
  // open the file
  // TODO: what if the file is already open?
  // FIXME: do we need to convert the flags?
  int fd = -1;
  unsigned short mode = ((flags&3)-1);  // convert O_RDRW, O_RDONLY and O_WRONLY
  if ( flags & MOS_O_APPEND ) mode |= O_APPEND;
  if ( flags & MOS_O_APPEND ) mode |= O_APPEND;
  if ( flags & MOS_O_CREAT ) mode |= O_CREAT;
  if ( flags & MOS_O_TRUNC ) mode |= O_TRUNC;
  if ( flags & MOS_O_EXCL ) mode |= O_EXCL;
  if ( flags & MOS_O_NRESOLVE ) mode |= O_NOFOLLOW;
  // MOS_O_BINARY not tested
  if ( flags & MOS_O_RSRC ) {
    mosError("Open File %s: no resource fork support yet!\n", uxFilename);
    errno = 2;
  } else if ( flags & MOS_O_ALIAS ) {
    mosError("Open File %s: no alias support yet!\n", uxFilename);
    errno = 2;
  } else {
    fd = ::open(uxFilename, mode, 0644);
  }
  if (fd==-1) { // error
    mosDebug("Can't open file %s (mode 0x%04X): %s\n", uxFilename, mode, strerror(errno));
    m68k_set_reg(M68K_REG_D0, errno); // just return the error code
    free(uxFilename);
  } else {
    MosFile *mf = (MosFile*)calloc(1, sizeof(MosFile));
    m68k_write_memory_32(file+8, (unsigned int)mf);
    mf->fd = fd;
    mf->filename = uxFilename;
    mf->open = true;
    mf->allocated = true;
    m68k_set_reg(M68K_REG_D0, 0); // no error
  }
}


/**
 * Close a file using its file descriptor.
 */
void trapSyClose(unsigned short) {
  unsigned int sp = m68k_get_reg(0L, M68K_REG_SP);
  unsigned int file = m68k_read_memory_32(sp+4);
  MosFile *mosFile = (MosFile*)m68k_read_memory_32(file+8);
  int ret = close(mosFile->fd);
  if (ret==-1) {
    m68k_set_reg(M68K_REG_D0, errno);
  } else {
    m68k_set_reg(M68K_REG_D0, 0);
  }
  if (mosFile->allocated) {
    m68k_write_memory_32(file+8, 0);
    if (mosFile->filename) {
      free((char*)mosFile->filename);
    }
    free(mosFile);
  }
}


/**
 * Read data from a file or stream.
 */
void trapSyRead(unsigned short) {
  // file identifier is on the stack
  unsigned int sp = m68k_get_reg(0L, M68K_REG_SP);
  unsigned int file = m68k_read_memory_32(sp+4);
  MosFile *mosFile = (MosFile*)m68k_read_memory_32(file+8);
  void *buffer = (void*)m68k_read_memory_32(file+16);
  unsigned int size = m68k_read_memory_32(file+12);
  int ret = read(mosFile->fd, buffer, size);
  if (ret==-1) {
    m68k_set_reg(M68K_REG_D0, errno);
  } else {
    m68k_write_memory_32(file+12, size-ret);
    m68k_set_reg(M68K_REG_D0, 0); // no error
  }
}


/**
 * Write data to a file or stream.
 */
void trapSyWrite(unsigned short) {
  unsigned int sp = m68k_get_reg(0L, M68K_REG_SP);
  unsigned int file = m68k_read_memory_32(sp+4);
  MosFile *mosFile = (MosFile*)m68k_read_memory_32(file+8);
  void *buffer = (void*)m68k_read_memory_32(file+16);
  unsigned int size = m68k_read_memory_32(file+12);
  
  // convert buffer if it is not binary // FIXME: this needs a lot more work!
  if (mosFile->fd==1) { // stdout
    if (gFilterStdoutDataFrom==MOS_TYPE_MAC && gFilterStdoutDataTo==MOS_TYPE_UNIX)
      buffer = (void*)mosDataMacToUnix((char*)buffer, size);
  } else if (mosFile->fd==2) { // stderr
    buffer = (void*)mosDataMacToUnix((char*)buffer, size);
  }

  int ret = write(mosFile->fd, buffer, size);
  if (ret==-1) {
    m68k_set_reg(M68K_REG_D0, errno);
  } else {
    m68k_write_memory_32(file+12, size-ret);
    m68k_set_reg(M68K_REG_D0, 0); // no error
  }
}


// stack: // ioctl

//# define FIOLSEEK               (('f'<<8)|0x00)  /* Apple internal use only */
//# define FIODUPFD               (('f'<<8)|0x01)  /* Apple internal use only */
//
//# define FIOINTERACTIVE (('f'<<8)|0x02)  /* If device is interactive */
//# define FIOBUFSIZE             (('f'<<8)|0x03)  /* Return optimal buffer size */
//# define FIOFNAME               (('f'<<8)|0x04)  /* Return filename */
//# define FIOREFNUM              (('f'<<8)|0x05)  /* Return fs refnum */
//# define FIOSETEOF              (('f'<<8)|0x06)  /* Set file length */
//
///*
// *   IOCTLs which begin with "TIO" are for TTY (i.e., console or
// *               terminal-related) device control requests.
// */
//
//# define TIOFLUSH   (('t'<<8)|0x00)             /* discard unread input.  arg is ignored */
//# define TIOSPORT   (('t'<<8)|0x01)             /* Obsolete -- do not use */
//# define TIOGPORT   (('t'<<8)|0x02)             /* Obsolete -- do not use */


/**
 * Additional file management functions.
 */
void trapSyIoctl(unsigned short) {
  unsigned int sp = m68k_get_reg(0L, M68K_REG_SP);
  unsigned int file = m68k_read_memory_32(sp+4);
  unsigned int cmd = m68k_read_memory_32(sp+8);
  unsigned int param = m68k_read_memory_32(sp+12);
  MosFile *mosFile = (MosFile*)m68k_read_memory_32(file+8);
  mosTrace("IOCTL of file at 0x%08X, cmd=0x%04X = '%c'<<8+%d, param=%d (0x%08X)\n",
           file, cmd, (cmd>>8)&0xff, cmd&0xff, param, param);
  switch (cmd) {
    case 0x6600: { // FIOLSEEK
                   // Parameter points to two longs, the first is the offset type, the second is the offset itself
                   // SEEK_SET=0 SEEK_CUR=1 SEEK_END=2
                   // lseek returns -1 on fail and the previous position on success
                   // ioctl return erroro in D0, and result in A6-4 (where the offset was originally)
                   // TODO: more error checking
      unsigned int whence = m68k_read_memory_32(param);
      unsigned int offset = m68k_read_memory_32(param+4);
      switch (whence) {
        case MOS_SEEK_SET: whence = SEEK_SET; break;
        case MOS_SEEK_CUR: whence = SEEK_CUR; break;
        case MOS_SEEK_END: whence = SEEK_END; break;
      }
      int ret = (unsigned int)lseek(mosFile->fd, offset, whence);
      if (ret==-1) {
        m68k_set_reg(M68K_REG_D0, errno);
      } else {
        m68k_write_memory_32(param+4, ret);
        m68k_set_reg(M68K_REG_D0, 0); // no error
      }
      break; }
    case 0x6601: // FIODUPFD
                 // TODO: more error checking
      // param is 0 in my tests
      // file is the current fd
      // TODO: I do not know what we must return here! Do I need to write the code to allocate a new fileStruct? Probably not!
      m68k_set_reg(M68K_REG_D0, 0); // no error
      break;
    case 0x6602: // FIOINTERACTIVE, return if device is interactive
                 // TODO: more error checking
      m68k_set_reg(M68K_REG_D0, 0); // no error
      break;
    case 0x6603: // FIOBUFSIZE, Return optimal buffer size (MPW buffers 1024 bytes)
      m68k_write_memory_16(param+2, 4096); // random value // FIXME: this looks like the wrong address to me!
      m68k_set_reg(M68K_REG_D0, 0); // no error
      break;
    case 0x6604: // FIOFNAME, Return filename
    case 0x6605: // FIOREFNUM, Return fs refnum
    case 0x6606: // FIOSETEOF, Set file length
    default:
      mosError("trapSyIoctl: unsupported ioctl 0x%04X on file operation\n", cmd);
      m68k_set_reg(M68K_REG_D0, EINVAL);
      break;
  }
}


int mosPBGetFInfo(unsigned int paramBlock, bool async)
{
  mosDebug("mosPBGetFInfo called\n");

  //mosPtr ioCompletion = m68k_read_memory_32(paramBlock+12);
  mosPtr ioNamePtr = m68k_read_memory_32(paramBlock+18);
  //unsigned int ioVRefNum = m68k_read_memory_16(paramBlock+22);
  //unsigned int ioFVersNum = m68k_read_memory_8(paramBlock+26);
  //int ioDirIndex = m68k_read_memory_16(paramBlock+28);
  
  // do we have a file name
  if (!ioNamePtr) {
    mosDebug("mosPBGetFInfo: no file name\n");
    m68k_write_memory_16(paramBlock+16, mosBdNamErr);
    return mosBdNamErr;
  }
  
  unsigned int fnLen = m68k_read_memory_8(ioNamePtr);
  if (fnLen==0) {
    mosDebug("mosPBGetFInfo: zero length file name\n");
    m68k_write_memory_16(paramBlock+16, mosBdNamErr);
    return mosBdNamErr;
  }
  
  char cFilename[2048];
  memcpy(cFilename, (unsigned char*)ioNamePtr+1, fnLen);
  cFilename[fnLen] = 0;
  mosDebug("mosPBGetFInfo: get info for '%s'\n", cFilename);
  
  struct stat st;
  int ret = stat(cFilename, &st);
  if (ret==-1) {
    mosDebug("mosPBGetFInfo: can't get status, return 'file not found'\n");
    m68k_write_memory_16(paramBlock+16, mosFnfErr);
    return mosFnfErr; // TODO: we could differentiate here a lot more!
  }

  // FIXME: ioDirIndex must be 0 or less!
  
  m68k_write_memory_16(paramBlock+16, mosNoErr);
  m68k_write_memory_16(paramBlock+24, -1);  // FIXME: ioRefNum  path reference number
  m68k_write_memory_8(paramBlock+30, 0);  // FIXME: ioFlAttrib  file attributes bit0: locked
  m68k_write_memory_8(paramBlock+31, 0);  // FIXME: ioFlVersNum  version number
  // 32: ioFlFndrInfo, 16 bytes finder info
  // 48.l: ioFlNum  file number
  // 52.w: ioFlStBlk  first allocation block of data fork. 0 if no data
  m68k_write_memory_16(paramBlock+52, 1234);
  // 54.l: ioFlLgLen  logical end of file for data fork
  m68k_write_memory_32(paramBlock+54, (unsigned int)st.st_size);
  // 58.l: ioFlPyLen  physical end of file
  m68k_write_memory_32(paramBlock+58, (unsigned int)st.st_size);
  // 62.w: ioFlRStBlk  first allocation block of resource fork. 0 if no fork
  m68k_write_memory_16(paramBlock+62, 0);
  // 64.l: ioFlRLgLen  size
  m68k_write_memory_32(paramBlock+64, 0);
  // 68.l: ioFlRPyLen  phys size
  m68k_write_memory_32(paramBlock+64, 0);
  // 72.l: ioFlCrDat  date and time of creation, seconds sinc 1.1.1904
#ifndef _POSIX_SOURCE
  m68k_write_memory_32(paramBlock+72, st.st_ctimespec.tv_sec + 2082844800);
#else
  m68k_write_memory_32(paramBlock+72, st.st_ctime + 2082844800);
#endif
  // 76.l: ioFlMdDat  date and time of last modification
#ifndef _POSIX_SOURCE
  m68k_write_memory_32(paramBlock+76, st.st_mtimespec.tv_sec + 2082844800);
#else
  m68k_write_memory_32(paramBlock+76, st.st_mtime + 2082844800);
#endif
  
  m68k_write_memory_16(paramBlock+16, mosNoErr);
  return mosNoErr; // .. and check which fields are read
}


int mosPBSetFInfo(unsigned int paramBlock, bool async)
{
  mosDebug("mosPBSetFInfo called\n");
  // FIXME: what can the user set here?
  // "the application should call PBSetFInfo (after PBCreate) to fill in the information needed by the Finder"
  m68k_write_memory_16(paramBlock+16, mosNoErr);
  return mosNoErr; // .. and check which fields are read
}


int mosPBSetEOF(unsigned int paramBlock, bool async)
{
  mosDebug("mosPBSetEOF called\n");
  int ioRefNum = m68k_read_memory_16(paramBlock+24);
  unsigned int ioMisc = m68k_read_memory_32(paramBlock+28);
  
  int ret = ftruncate(ioRefNum, ioMisc);
  if (ret==-1) {
    mosDebug("mosPBSetEOF %d %d failed: %s\n", ioRefNum, ioMisc, strerror(errno));
    m68k_write_memory_16(paramBlock+16, mosFnfErr);
    return mosFnfErr; // TODO: get more detailed here?
  }
  
  m68k_write_memory_16(paramBlock+16, mosNoErr);
  return mosNoErr; // .. and check which fields are read
}


int mosPBSetFPos(unsigned int paramBlock, bool async)
{
  int ioRefNum = m68k_read_memory_16(paramBlock+24);
  unsigned short ioPosMode = m68k_read_memory_16(paramBlock+44);
  unsigned int ioPosOffset = m68k_read_memory_32(paramBlock+46);
  mosDebug("mosPBSetFPos called: RefNum=%d, PosMode=%d, PosOffset=%d\n",
           ioRefNum, ioPosMode, ioPosOffset);
  
  int ret = 0;
  switch (ioPosMode) {
    case 0: break; // fsAtMark
    case 1: ret = (int)lseek(ioRefNum, ioPosOffset, SEEK_SET); break; // fsFromStart
    case 2: ret = (int)lseek(ioRefNum, ioPosOffset, SEEK_END); break; // fsFromLEOF
    case 3: ret = (int)lseek(ioRefNum, ioPosOffset, SEEK_CUR); break; // fsFromMark
  }
  if (ret==-1) {
    mosDebug("mosPBSetFPos failed: %s\n", strerror(errno));
    m68k_write_memory_16(paramBlock+16, mosEofErr);
    return mosEofErr; // TODO: get more detailed here?
  }
  
  m68k_write_memory_16(paramBlock+16, mosNoErr);
  return mosNoErr; // .. and check which fields are read
}


int mosPBRead(unsigned int paramBlock, bool async)
{
  int ioRefNum = m68k_read_memory_16(paramBlock+24);
  unsigned int ioBuffer = m68k_read_memory_32(paramBlock+32);
  unsigned int ioReqCount = m68k_read_memory_32(paramBlock+36);
  unsigned short ioPosMode = m68k_read_memory_16(paramBlock+44);
  unsigned int ioPosOffset = m68k_read_memory_32(paramBlock+46);
  mosDebug("mosPBRead called: RefNum=%d, Buffer=0x%08X, ReqCount=%d, POsMode=%d, PosOffset=%d\n",
           ioRefNum, ioBuffer, ioReqCount, ioPosMode, ioPosOffset);
  
  switch (ioPosMode) {
    case 0: break;
    case 1: lseek(ioRefNum, ioPosOffset, SEEK_SET); break;
    case 2: lseek(ioRefNum, ioPosOffset, SEEK_END); break;
    case 3: lseek(ioRefNum, ioPosOffset, SEEK_CUR); break;
  }
  int ret = read(ioRefNum, (void*)ioBuffer, ioReqCount);
  if (ret==-1) {
    mosDebug("mosPBRead failed: %s\n", strerror(errno));
    m68k_write_memory_16(paramBlock+16, mosFnfErr);
    return mosFnfErr; // TODO: get more detailed here?
  }
  
  m68k_write_memory_32(paramBlock+40, ret);
  m68k_write_memory_32(paramBlock+46, (unsigned int)lseek(ioRefNum, 0, SEEK_CUR));
  m68k_write_memory_16(paramBlock+16, mosNoErr);
  return mosNoErr; // .. and check which fields are read
}


int mosPBWrite(unsigned int paramBlock, bool async)
{
  int ioRefNum = m68k_read_memory_16(paramBlock+24);
  unsigned int ioBuffer = m68k_read_memory_32(paramBlock+32);
  unsigned int ioReqCount = m68k_read_memory_32(paramBlock+36);
  unsigned short ioPosMode = m68k_read_memory_16(paramBlock+44);
  unsigned int ioPosOffset = m68k_read_memory_32(paramBlock+46);
  mosDebug("mosPBWrite called: RefNum=%d, Buffer=0x%08X, ReqCount=%d, POsMode=%d, PosOffset=%d\n",
           ioRefNum, ioBuffer, ioReqCount, ioPosMode, ioPosOffset);
  
  switch (ioPosMode) {
    case 0: break;
    case 1: lseek(ioRefNum, ioPosOffset, SEEK_SET); break;
    case 2: lseek(ioRefNum, ioPosOffset, SEEK_END); break;
    case 3: lseek(ioRefNum, ioPosOffset, SEEK_CUR); break;
  }
  int ret = write(ioRefNum, (void*)ioBuffer, ioReqCount);
  if (ret==-1) {
    mosDebug("mosPBWrite failed: %s\n", strerror(errno));
    m68k_write_memory_16(paramBlock+16, mosFnfErr);
    return mosFnfErr; // TODO: get more detailed here?
  }
  
  m68k_write_memory_32(paramBlock+40, ret);
  m68k_write_memory_32(paramBlock+46, (unsigned int)lseek(ioRefNum, 0, SEEK_CUR));
  m68k_write_memory_16(paramBlock+16, mosNoErr);
  return mosNoErr; // .. and check which fields are read
}


int mosPBClose(unsigned int paramBlock, bool async)
{
  mosDebug("mosPBClose called\n");
  int ioRefNum = m68k_read_memory_16(paramBlock+24);
  
  int ret = close(ioRefNum);
  if (ret==-1) {
    mosDebug("mosPBClose failed: %s\n", strerror(errno));
    m68k_write_memory_16(paramBlock+16, mosFnfErr);
    return mosFnfErr; // TODO: get more detailed here?
  }
  
  m68k_write_memory_16(paramBlock+16, mosNoErr);
  return mosNoErr; // .. and check which fields are read
}


int mosPBCreate(unsigned int paramBlock, bool async)
{
  mosDebug("mosPBCreate called\n");
  
  //mosPtr ioCompletion = m68k_read_memory_32(paramBlock+12);
  mosPtr ioNamePtr = m68k_read_memory_32(paramBlock+18);
  //unsigned int ioVRefNum = m68k_read_memory_16(paramBlock+22);
  //unsigned int ioFVersNum = m68k_read_memory_8(paramBlock+26);
  
  // do we have a file name
  if (!ioNamePtr) {
    mosDebug("mosPBCreate: no file name\n");
    m68k_write_memory_16(paramBlock+16, mosBdNamErr);
    return mosBdNamErr;
  }
  
  unsigned int fnLen = m68k_read_memory_8(ioNamePtr);
  if (fnLen==0) {
    mosDebug("mosPBCreate: zero length file name\n");
    m68k_write_memory_16(paramBlock+16, mosBdNamErr);
    return mosBdNamErr;
  }
  
  char cFilename[2048];
  memcpy(cFilename, (unsigned char*)ioNamePtr+1, fnLen);
  cFilename[fnLen] = 0;
  mosDebug("mosPBCreate: create file '%s'\n", cFilename);
  
  int ret = open(cFilename, O_CREAT|O_WRONLY, 0644);
  if (ret==-1) {
    mosDebug("mosPBCreate: can't create file\n");
    m68k_write_memory_16(paramBlock+16, mosDupFNErr);
    return mosDupFNErr; // TODO: we could differentiate here a lot more!
  }
  close(ret);
  
  m68k_write_memory_16(paramBlock+16, mosNoErr);
  return mosNoErr;
}


int mosPBHOpen(unsigned int paramBlock, bool async)
{
  mosDebug("mosPBHOpen called\n");
  
  //mosPtr ioCompletion = m68k_read_memory_32(paramBlock+12);
  mosPtr ioNamePtr = m68k_read_memory_32(paramBlock+18);
  //unsigned int ioVRefNum = m68k_read_memory_16(paramBlock+22);
  //unsigned int ioFVersNum = m68k_read_memory_8(paramBlock+26);
  
  // do we have a file name
  if (!ioNamePtr) {
    mosDebug("mosPBHOpen: no file name\n");
    m68k_write_memory_16(paramBlock+16, mosBdNamErr);
    return mosBdNamErr;
  }
  
  unsigned int fnLen = m68k_read_memory_8(ioNamePtr);
  if (fnLen==0) {
    mosDebug("mosPBHOpen: zero length file name\n");
    m68k_write_memory_16(paramBlock+16, mosBdNamErr);
    return mosBdNamErr;
  }
  
  char cFilename[2048];
  memcpy(cFilename, (unsigned char*)ioNamePtr+1, fnLen);
  cFilename[fnLen] = 0;
  mosDebug("mosPBHOpen: open file '%s'\n", cFilename);
  
  int file = -1;
  byte mode = m68k_read_memory_8(paramBlock+27); // ioPermssn
  switch (mode) {
    case 1: file = open(cFilename, O_RDONLY); break;  // fsRdPerm = 1
    case 2: file = open(cFilename, O_WRONLY); break;  // fsWrPern = 2
    case 0:                                           // fsCurPerm = 0
    case 3: file = open(cFilename, O_RDWR); break;    // fsRdWrPerm = 3
  }
  mosDebug("mosPBHOpen: open file '%s' mode=%d => %d\n", cFilename, mode, file);
  if (file==-1) {
    mosError("mosPBHOpen: can't open file '%s', %s\n", cFilename, strerror(errno));
    m68k_write_memory_16(paramBlock+16, mosDupFNErr);
    return mosDupFNErr; // TODO: we could differentiate here a lot more!
  }
  m68k_write_memory_16(paramBlock+24, file); // ioRefNum  
  m68k_write_memory_16(paramBlock+16, mosNoErr);
  return mosNoErr;
}


int mosPBDelete(unsigned int paramBlock, bool async)
{
  mosDebug("mosPBDelete called\n");
  
  //mosPtr ioCompletion = m68k_read_memory_32(paramBlock+12);
  mosPtr ioNamePtr = m68k_read_memory_32(paramBlock+18);
  //unsigned int ioVRefNum = m68k_read_memory_16(paramBlock+22);
  //unsigned int ioFVersNum = m68k_read_memory_8(paramBlock+26);
  
  // do we have a file name
  if (!ioNamePtr) {
    mosDebug("mosPBDelete: no file name\n");
    m68k_write_memory_16(paramBlock+16, mosBdNamErr);
    return mosBdNamErr;
  }
  
  unsigned int fnLen = m68k_read_memory_8(ioNamePtr);
  if (fnLen==0) {
    mosDebug("mosPBDelete: zero length file name\n");
    m68k_write_memory_16(paramBlock+16, mosBdNamErr);
    return mosBdNamErr;
  }
  
  char cFilename[2048];
  memcpy(cFilename, (unsigned char*)ioNamePtr+1, fnLen);
  cFilename[fnLen] = 0;
  
  mosDebug("mosPBDelete: deleteing file '%s'\n", cFilename);
  int ret = ::remove(cFilename);
  if (ret==-1) {
    mosError("mosPBDelete: can't remove file '%s', %s\n", cFilename, strerror(errno));
    m68k_write_memory_16(paramBlock+16, mosDupFNErr);
    return mosDupFNErr; // TODO: we could differentiate here a lot more!
  }
  
  m68k_write_memory_16(paramBlock+16, mosNoErr);
  return mosNoErr;
}


/**
 * Dispatch a single trap into multiple file system operations
 */
int mosFSDispatch(unsigned int paramBlock, unsigned int func)
{
  switch (func) {
    case 0x001A: // PBOpenDFSync
      return mosPBHOpen(paramBlock, false);
    case 0x002E:
      mosError("mosFSDispatch: PBDTOpenInform not implemented\n");
      break;
    case 0x002F: // PBDTDeleteSync
      return mosPBDelete(paramBlock, false);
    case 0x0060:
      mosError("mosFSDispatch: PBGetAltAccessSync not implemented\n");
      break;
    case 0x0061:
      mosError("mosFSDispatch: PBSetAltAccessSync not implemented\n");
      break;
    default:
      mosError("mosFSDispatch: unknown function 0x%04X\n", func);
      break;
  }

  m68k_write_memory_16(paramBlock+16, mosParamErr);
  return mosParamErr;
}


