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

/** \file filename.cpp
 This module converts filenames and their path from one OS format into another.
 
 We need to conver file separators and font encodings.
 
 Mac Path Names:
 - separator is a ':'
 - path names starting with a ':' are relative to the current directory
 - path names without a ':' are absolute - the first word is the disk name
 - starting with a '::' sets the parent directory, more ":" go further up the tree
 - if there are no ":" at all in the name, it is a leaf name and it is relative
 - test.c => test.c
 - :test.c => ./test.c
 - ::test.c => ../test.c
 - :::test.c => ../../test.c
 - :Emaples: => ./Examples/
 - Examples: => /Examples/
 - Examples => would not refer to a directory, but a file "./Examples"

 */

#include "filename.h"

#include "main.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


// TODO: not yet used. 
static const unsigned short ucLUT[] = {
  0x00C4, 0x00C5, 0x00C7, 0x00C9, 0x00D1, 0x00D6, 0x00DC, 0x00E1,
  0x00E0, 0x00E2, 0x00E4, 0x00E3, 0x00E5, 0x00E7, 0x00E9, 0x00E8,
  0x00EA, 0x00EB, 0x00ED, 0x00EC, 0x00EE, 0x00EF, 0x00F1, 0x00F3,
  0x00F2, 0x00F4, 0x00F6, 0x00F5, 0x00FA, 0x00F9, 0x00FB, 0x00FC,
  0x2020, 0x00B0, 0x00A2, 0x00A3, 0x00A7, 0x2022, 0x00B6, 0x00DF,
  0x00AE, 0x00A9, 0x2122, 0x00B4, 0x00A8, 0x2260, 0x00C6, 0x00D8,
  0x221E, 0x00B1, 0x2264, 0x2265, 0x00A5, 0x00B5, 0x2202, 0x2211,
  0x220F, 0x03C0, 0x222B, 0x00AA, 0x00BA, 0x03A9, 0x00E6, 0x00F8,
  0x00BF, 0x00A1, 0x00AC, 0x221A, 0x0192, 0x2248, 0x2206, 0x00AB,
  0x00BB, 0x2026, 0x00A0, 0x00C0, 0x00C3, 0x00D5, 0x0152, 0x0153,
  0x2013, 0x2014, 0x201C, 0x201D, 0x2018, 0x2019, 0x00F7, 0x25CA,
  0x00FF, 0x0178, 0x2044, 0x20AC, 0x2039, 0x203A, 0xFB01, 0xFB02,
  0x2021, 0x00B7, 0x201A, 0x201E, 0x2030, 0x00C2, 0x00CA, 0x00C1,
  0x00CB, 0x00C8, 0x00CD, 0x00CE, 0x00CF, 0x00CC, 0x00D3, 0x00D4,
  0xF8FF, 0x00D2, 0x00DA, 0x00DB, 0x00D9, 0x0131, 0x02C6, 0x02DC,
  0x00AF, 0x02D8, 0x02D9, 0x02DA, 0x00B8, 0x02DD, 0x02DB, 0x02C7,
};


static char *buffer = 0;
static int NBuffer = 0;


/**
 * Make sure that at least size bytes fit into the buffer.
 */
static void allocateBuffer(int size)
{
  if (size>NBuffer) {
    NBuffer = (size+1024)&0xfffffc00;
    if (buffer)
      free(buffer);
    buffer = (char*)malloc(NBuffer);
  }
}


/**
 * Convert a text block in Unix/UTF-8 encoding to MacRoman.
 *
 * This will fail if there is a partial UTF-8 character at the end of the 
 * text block!
 *
 * \return pointer to a static buffer
 */
char *mosDataUnixToMac(const char *text, unsigned int &size)
{
  // The Mac string can nevr be longer than the Unix string
  const char *s = text;
  int i, count = size;
  // now we have the size. Make sure we have space
  allocateBuffer(count+1);
  // copy and convert characters
  byte *d = (byte*)buffer;
  for (i=0;i<size;i++) {
    byte c = (byte)*s++;
    if (c=='\n') {
      *d++ = '\r';
    } else if (c<128) {
      *d++ = c;
    } else {
      // UTF8 character
      unsigned short uc = 0;
      if ( (c&0xe0)==0xc0) {
        uc =  ((((unsigned short)c)&0x1f)<<6);
        uc |=  (((unsigned short)(byte)s[0])&0x3f);
        s++; size--;
      } else if ( (c&0xf0)==0xe0) {
        uc =  ((((unsigned short)c)&0x1f)<<12);
        uc |= ((((unsigned short)(byte)s[0])&0x3f)<<6);
        uc |=  (((unsigned short)(byte)s[1])&0x3f);
        s+=2; size-=2;
      }
      if (uc) {
        int j;
        for (j=0; j<128; j++) {
          if (ucLUT[j]==uc) {
            *d++ = j+128;
            break;
          }
        }
        if (j==128)
          *d++ = '$';
      } else {
        *d++ = '$';
      }
    }
  }
  *d = 0;
  size = ((char*)d)-buffer;
  return buffer;
}



/**
 * Convert a text block in MacRoman encoding to Unix/UTF-8.
 *
 * \return pointer to a static buffer
 */
char *mosDataMacToUnix(const char *text, unsigned int &size)
{
  // the Unix string is potentialy longer than the Mac string, so start counting
  const char *s = text;
  int i, count = 0;
  for (i=0;i<size;i++) {
    byte c = (byte)*s++;
    if (c<128) {
      count++;
    } else {
      unsigned short uc = ucLUT[c-128];
      if (uc<128) {
        count++;
      } else if (uc<0x07ff) {
        count+=2;
      } else if (uc<0x07ff) {
        count+=3;
      }
    }
  }
  // now we have the size. Make sure we have space
  allocateBuffer(count+1);
  // copy and convert characters
  byte *d = (byte*)buffer;
  s = text;
  for (i=0;i<size;i++) {
    byte c = (byte)*s++;
    if (c=='\r') {
      *d++ = '\n';
    } else if (c<128) {
      *d++ = c;
    } else {
      unsigned short uc = ucLUT[c-128];
      if (uc<128) {
        *d++ = uc;
      } else if (uc<0x07ff) {
        *d++ = ((uc>>6) & 0x1f) | 0xc0;
        *d++ = (uc & 0x3f) | 0x80;
      } else if (uc<0x07ff) {
        *d++ = ((uc>>12) & 0x0f) | 0xe0;
        *d++ = ((uc>>6) & 0x3f) | 0x80;
        *d++ = (uc & 0x3f) | 0x80;
      }
    }
  }
  *d = 0;
  size = count;
  return buffer;
}



/**
 Guess the type of a file name with path.
 
 There is no sure way to determine the format of a filepath. This function
 guesses the type by adding indicators. A file path separator can be one
 indicator, a correct utf-8 character can be another one.
 
 However, Unix does allow ':' in file names, which could be interpreted as a
 Mac file separator. MacOS however allows '/' in file names (and the default
 installation of tcl/tk on Mac actually uses it.
 
 A correct UTF-8 sequence can also be a legal two-character MacOS sequnce.
 
 If there are no indicators at all, we return MOS_TYPE_UNKNOWN, which also
 indicates, that probably no conversion is needed.
 
 FIXME: a single word could be a drive name in MacOS or a relative file Unix. Sigh.
*/
int mosFilenameGuessType(const char *filename)
{
  int unixType = 0, macType = 0;
  if (!filename || !*filename)
    return MOS_TYPE_UNKNOWN;
  int i, n = strlen(filename)-1;
  for (i=0; i<n; i++) {
    char c = filename[i];
    if (c=='/') {
      unixType++;  // Unix filename
    } else if (c==':') {
      macType++;  // Mac filename
    } else if ((c&0xE0)==0xC0 && (filename[i+1]&0xC0)==0x80) {
      unixType++; // UTF-8
    } else if ((c&0xF0)==0xE0
               && (filename[i+1]&0xC0)==0x80
               && (filename[i+2]&0xC0)==0x80) {
      unixType++; // UTF-8
    } else if ((c&0xF8)==0xF0
               && (filename[i+1]&0xC0)==0x80
               && (filename[i+2]&0xC0)==0x80
               && (filename[i+3]&0xC0)==0x80) {
      unixType++; // UTF-8
    } else if (c&0x80) {
      macType++; // not UTF-8, so probably MacOS
    }
  }
  
  // is there a clear winner?
  if (unixType>macType) return MOS_TYPE_UNIX;
  if (macType>unixType) return MOS_TYPE_MAC;

  // ok, so it's undecided:
  return MOS_TYPE_UNKNOWN;
}


static void convertFromMac(const char *filename, char *buffer)
{
  const char *src = filename;
  char *dst = buffer;
  
  // just copy all leading quotes, hoping that the trailing quotes match
  for (;;) {
    char c = *src;
    if (c=='"' || c=='\'' || c=='`') {
      *dst++ = c;
      src++;
    } else {
      break;
    }
  }
  
  // is the filename relative or absolute?
  char isRelative = false;
  if (*src==':') {
    // if the filename starts with a ':', it must be relative
    isRelative = true;
  } else if (strchr(filename, ':')) {
    // if the filename contains a ':', but does not start with one, it must be absolute
    isRelative = false;
  } else {
    // it's only a filename or a directory name. In either case, it's relative (an absolte directory name would have a trailing ':'
    isRelative = true;
  }
  
  // now begin the path
  if (isRelative) {
    src++;
    while (*src==':') { // multiple ':' at the start go up a directory
      *dst++ = '.'; *dst++ = '.'; *dst++ = '/';
      src++;
    }
  } else {
    *dst++ = '/';
  }
  
  // now copy the remaining path members over
  for (;;) {
    unsigned char c = (unsigned char)*src++;
    if (c==':') {
      *dst++ = '/';
      while (*src==':') { // multiple ':' go up a directory
        *dst++ = '.'; *dst++ = '.'; *dst++ = '/';
        src++;
      }
    } else if (c>=128) {
      *dst++ = c; // FIXME: convert Mac Roman to UTF-8
    } else {
      *dst++ = c;
    }
    if (c==0)
      break;
  }
//  fprintf(stderr, "FromMac: '%s' = '%s'\n", filename, buffer);
  unsigned int size = strlen(buffer);
  char *b2 = mosDataMacToUnix(buffer, size);
  // FIXME: this can create an ugly overflow!œ
  strcpy(buffer, b2);
}


static void convertToMac(const char *filename, char *buffer)
{
  const char *src = filename;
  char *dst = buffer;
  
  // just copy all leading quotes, hoping that the trailing quotes match
  for (;;) {
    char c = *src;
    if (c=='"' || c=='\'' || c=='`') {
      *dst++ = c;
      src++;
    } else {
      break;
    }
  }
  
  // is the filename relative or absolute?
  char isRelative = false;
  if (*src=='/') {
    // if the filename starts with a '/', it must be absolute
    isRelative = false;
  } else {
    // if no '/' at the start, it's relative
    isRelative = true;
  }

  // now begin the path
  if (isRelative) {
    if (strncmp(src, "../", 3)==0) { // "../" = parent directory
      *dst++ = ':';
      src+=2; // point to the trainling slash which will generate the second ':'
    } else if (strncmp(src, "./", 2)==0) { // "./" = current directory
      src++; // point to the trainling slash which will generate the ':'
    } else if (strcmp(src, "..")==0) { // filename ends with ".."
      *dst++ = ':';
      *dst++ = ':';
      src+=2;
    } else if (strcmp(src, ".")==0) { // filename ends with "."
      *dst++ = ':';
      src+=1;
    } else {
      *dst++ = ':';   // start with a ':'
    }
  } else {
    src++;          // skip the first '/'
  }
  
  // now copy the remaining path members over
  for (;;) {
    unsigned char c = (unsigned char)*src++;
    if (c=='/') {
      while (*src=='/') { // multiple '/' don't do anything
        src++;
      }
      if (strncmp(src, "../", 3)==0) { // "../" = parent directory
        *dst++ = ':';
        src+=2; // point to the trainling slash
      } else if (strncmp(src, "./", 2)==0) { // "./" = current directory
        src++; // point to the trainling slash
      } else if (strcmp(src, "..")==0) { // filename ends with ".."
        *dst++ = ':';
        *dst++ = ':';
        src+=2;
      } else if (strcmp(src, ".")==0) { // filename ends with "."
        *dst++ = ':';
        src+=1;
      } else {
        *dst++ = ':';
      }
    } else if (c>=128) {
      *dst++ = c;
    } else {
      *dst++ = c;
    }
    if (c==0)
      break;
  }
  // FIXME: when do we ned a trailing ':'
//  fprintf(stderr, "ToMac: '%s' = '%s'\n", filename, buffer);
  unsigned int size = strlen(buffer);
  char *b2 = mosDataUnixToMac(buffer, size);
  // FIXME: this can create an ugly overflow!œ
  strcpy(buffer, b2);
}


/**
 User API to converting full pathnames between formats.
 
 We determine the current filename for mat by guessing. Then we convert the
 path to Unix, simply because the Unix format incorporates all other features.
 And then at long last, we convert into the final format.
 
 \return pointer to a filename in static memory. Don't free, don't use twice in one call.
 */
char *mosFilenameConvertTo(const char *filename, int dstType)
{
  static char buffer[2048];
  char *tmpname;
  int srcType = mosFilenameGuessType(filename);
  if (srcType==dstType || srcType==MOS_TYPE_UNKNOWN) {
    strcpy(buffer, filename);
    return buffer;
  }
  switch (srcType) {
    case MOS_TYPE_MAC: convertFromMac(filename, buffer); break;
    default: strcpy(buffer, filename); break;
  }
  switch (dstType) {
    case MOS_TYPE_MAC: tmpname = strdup(buffer); convertToMac(tmpname, buffer); free(tmpname); break;
    default: break; // nothing to do, Unix type filename is in the buffer
    // TODO: actually, OS X expects another UTF-8 encoding than standard Unix, so, yes, there might be work to do
  }
  return buffer;
}


/**
 * Return a pointer to the name part of a filename with path information.
 *
 * We don't bother guessing the file path format, but instead use any of
 * the path separator character as an indicator.
 */
const char *mosFilenameName(const char *filename)
{
  if (!filename || !*filename)
    return filename;
  int n = strlen(filename)-1;
  while (n>-1) {
    char c = filename[n];
    if (c=='/') break;  // Unix filename
    if (c==':') break;  // Mac filename
    n--;
  }
  return filename + n + 1;
}


/**
 * Return a pointer to the name part of a filename with path information.
 *
 * Only check for '/' as a separator
 */
const char *mosFilenameNameUnix(const char *filename)
{
  if (!filename || !*filename)
    return filename;
  int n = strlen(filename)-1;
  while (n>-1) {
    char c = filename[n];
    if (c=='/') break;  // Unix filename
    n--;
  }
  return filename + n + 1;
}

