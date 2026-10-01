/*
 mosrun - the MacOS MPW runtime emulator
 Copyright (C) 2013-2020  Matthias Melcher

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
#include "textconv.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


// Size of the buffer that holds a converted filename.
static const size_t kFilenameBufferSize = 2048;


/**
 * Copy a converted filename into the caller's buffer without overflowing it.
 */
static void copyFilename(char *buffer, const std::string &name)
{
    size_t n = name.size() < kFilenameBufferSize-1 ? name.size() : kFilenameBufferSize-1;
    memcpy(buffer, name.data(), n);
    buffer[n] = 0;
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
    copyFilename(buffer, mosTextMacToHost(std::string(buffer)));
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
    copyFilename(buffer, mosTextHostToMac(std::string(buffer)));
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
    static char buffer[kFilenameBufferSize];
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

