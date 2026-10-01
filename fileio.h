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


#ifndef __mosrun__fileio__
#define __mosrun__fileio__

#include "main.h"

#include <string>


// File extensions that are treated as text unless the user adds more.
#define MOS_DEFAULT_TEXT_EXTENSIONS \
    ".c .h .cc .cp .cpp .cxx .c++ .hh .hpp .hxx .i .ii .s .a .asm .inc .r .exp .def .make .mk .txt"


// Command line settings for text conversion (see ---help).
extern bool gMosTextIn;         // convert text files read by the tool from UTF-8/LF
extern bool gMosTextOut;        // convert text files written by the tool to UTF-8/LF
extern bool gMosStdoutToHost;   // convert stdout to UTF-8/LF
extern bool gMosStderrToHost;   // convert stderr to UTF-8/LF

void mosTextAddExtensions(const char *list);
bool mosFileIsText(const char *filename);


/**
 A file or stream that the emulated tool has opened.

 Most files pass reads and writes straight through to the host file. Text
 files that need conversion are kept in memory in MacRoman with CR line
 endings, so that their size and random access match what the tool expects.
 They are converted back and written to disk when they are closed.
 */
class MosFile
{
public:
    MosFile(int fd, const char *filename, bool allocated);

    void setupTextConversion(unsigned short mpwFlags);
    int read(void *dst, unsigned int size);
    int write(const void *src, unsigned int size);
    long seek(long offset, int whence);
    int flush();
    int close();
    void discard();

    int fd;
    std::string filename;   // host (Unix) path
    bool allocated;         // false for the permanent stdin, stdout, and stderr entries

private:
    bool inMemory = false;  // all reads and writes go to data instead of fd
    bool append = false;    // every write goes to the end of the file
    bool dirty = false;     // data must be written back to the host file
    bool writeAsHost = false; // convert data to UTF-8/LF when writing it back
    bool deleted = false;   // the file was deleted while open, never write it back
    std::string data;       // file content in MacRoman with CR line endings
    size_t pos = 0;
};


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
