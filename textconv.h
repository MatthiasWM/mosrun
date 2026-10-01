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

#ifndef __mosrun__textconv__
#define __mosrun__textconv__

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>


/*
 Text conversion between the host and the emulated Mac.

 MPW tools expect MacRoman encoding and CR line endings. Modern editors and
 tools write UTF-8 with LF (or CRLF) line endings. This module converts
 between the two and has no dependencies on the emulator, so that it can be
 tested on its own.
 */


// MacRoman character used for Unicode characters that MacRoman can't express.
//const uint8_t kMosMacReplacementChar = 0xD7; // '◊' LOZENGE
const uint8_t kMosMacReplacementChar = 0xE0; // '‡' SECTION SIGN

/**
 What we found out about a block of text.
 */
struct MosTextInfo {
    bool binary = false;    // contains NUL bytes, never convert
    bool hasLF = false;     // contains at least one '\n'
    bool hasHigh = false;   // contains bytes >= 0x80
    bool validUtf8 = true;  // all bytes >= 0x80 form valid UTF-8 sequences

    // True if this text must be converted before a Mac tool can read it.
    bool isHostText() const { return !binary && (hasLF || (hasHigh && validUtf8)); }
};


/**
 A Unicode character that could not be converted to MacRoman.
 */
struct MosTextMisfit {
    unsigned int line;      // line number in the original text, starting at 1
    uint32_t codepoint;     // the Unicode character that was replaced
};


MosTextInfo mosTextAnalyze(const uint8_t *data, size_t size);

std::string mosTextHostToMac(const uint8_t *data, size_t size,
                             std::vector<MosTextMisfit> *misfits = nullptr);
std::string mosTextHostToMac(const std::string &text,
                             std::vector<MosTextMisfit> *misfits = nullptr);

std::string mosTextMacToHost(const uint8_t *data, size_t size);
std::string mosTextMacToHost(const std::string &text);

std::string mosTextUtf8(uint32_t codepoint);


#endif /* defined(__mosrun__textconv__) */
