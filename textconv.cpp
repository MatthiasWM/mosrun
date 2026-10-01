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

#include "textconv.h"


// Unicode code points for the MacRoman characters 0x80 to 0xFF.
static const uint16_t kMacRomanToUnicode[128] = {
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


/**
 Decode one UTF-8 sequence.

 \return the number of bytes used, or 0 if this is not a valid sequence
 */
static size_t decodeUtf8(const uint8_t *s, const uint8_t *end, uint32_t &cp)
{
    uint8_t c = s[0];
    size_t n;
    uint32_t min;
    if (c < 0x80) { cp = c; return 1; }
    else if ((c & 0xE0) == 0xC0) { n = 2; cp = c & 0x1F; min = 0x80; }
    else if ((c & 0xF0) == 0xE0) { n = 3; cp = c & 0x0F; min = 0x800; }
    else if ((c & 0xF8) == 0xF0) { n = 4; cp = c & 0x07; min = 0x10000; }
    else return 0;
    if ((size_t)(end - s) < n) return 0;
    for (size_t i = 1; i < n; i++) {
        if ((s[i] & 0xC0) != 0x80) return 0;
        cp = (cp << 6) | (s[i] & 0x3F);
    }
    if (cp < min || cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF))
        return 0; // overlong, out of range, or a surrogate
    return n;
}


static int unicodeToMacRoman(uint32_t cp)
{
    if (cp < 0x80) return (int)cp;
    for (int i = 0; i < 128; i++) {
        if (kMacRomanToUnicode[i] == cp)
            return i + 0x80;
    }
    return -1;
}


static void appendUtf8(std::string &out, uint32_t cp)
{
    if (cp < 0x80) {
        out += (char)cp;
    } else if (cp < 0x800) {
        out += (char)(0xC0 | (cp >> 6));
        out += (char)(0x80 | (cp & 0x3F));
    } else if (cp < 0x10000) {
        out += (char)(0xE0 | (cp >> 12));
        out += (char)(0x80 | ((cp >> 6) & 0x3F));
        out += (char)(0x80 | (cp & 0x3F));
    } else {
        out += (char)(0xF0 | (cp >> 18));
        out += (char)(0x80 | ((cp >> 12) & 0x3F));
        out += (char)(0x80 | ((cp >> 6) & 0x3F));
        out += (char)(0x80 | (cp & 0x3F));
    }
}


/**
 Encode a single Unicode character as UTF-8, for messages.
 */
std::string mosTextUtf8(uint32_t codepoint)
{
    std::string out;
    appendUtf8(out, codepoint);
    return out;
}


/**
 Find out if a block of text needs conversion before a Mac tool can read it.
 */
MosTextInfo mosTextAnalyze(const uint8_t *data, size_t size)
{
    MosTextInfo info;
    const uint8_t *s = data, *end = data + size;
    while (s < end) {
        uint8_t c = *s;
        if (c == 0) {
            info.binary = true;
            return info;
        }
        if (c == '\n') info.hasLF = true;
        if (c < 0x80) {
            s++;
            continue;
        }
        info.hasHigh = true;
        uint32_t cp;
        size_t n = info.validUtf8 ? decodeUtf8(s, end, cp) : 0;
        if (n == 0) {
            info.validUtf8 = false;
            n = 1;
        }
        s += n;
    }
    return info;
}


/**
 Convert host text (UTF-8, LF or CRLF line endings) to MacRoman with CR.

 If the text is not valid UTF-8, it is assumed to be MacRoman already, and
 only the line endings are converted. Binary data is returned unchanged. Characters that MacRoman can't express
 are replaced with kMosMacReplacementChar and listed in misfits.
 */
std::string mosTextHostToMac(const uint8_t *data, size_t size,
                             std::vector<MosTextMisfit> *misfits)
{
    MosTextInfo info = mosTextAnalyze(data, size);
    if (info.binary)
        return std::string((const char*)data, size);
    bool decode = info.hasHigh && info.validUtf8;
    std::string out;
    out.reserve(size);
    unsigned int line = 1;
    const uint8_t *s = data, *end = data + size;
    while (s < end) {
        uint8_t c = *s;
        if (c == '\r') {
            out += '\r';
            s++;
            if (s < end && *s == '\n') s++; // CRLF
            line++;
        } else if (c == '\n') {
            out += '\r';
            s++;
            line++;
        } else if (c < 0x80 || !decode) {
            out += (char)c;
            s++;
        } else {
            uint32_t cp = 0;
            size_t n = decodeUtf8(s, end, cp);
            int mac = unicodeToMacRoman(cp);
            if (mac < 0) {
                mac = kMosMacReplacementChar;
                if (misfits) misfits->push_back({ line, cp });
            }
            out += (char)mac;
            s += n;
        }
    }
    return out;
}


std::string mosTextHostToMac(const std::string &text, std::vector<MosTextMisfit> *misfits)
{
    return mosTextHostToMac((const uint8_t*)text.data(), text.size(), misfits);
}


/**
 Convert MacRoman text with CR line endings to UTF-8 with LF line endings.

 Every MacRoman character has a Unicode equivalent, so this never fails.
 */
std::string mosTextMacToHost(const uint8_t *data, size_t size)
{
    std::string out;
    out.reserve(size + size / 8);
    for (size_t i = 0; i < size; i++) {
        uint8_t c = data[i];
        if (c == '\r') {
            out += '\n';
        } else if (c < 0x80) {
            out += (char)c;
        } else {
            appendUtf8(out, kMacRomanToUnicode[c - 0x80]);
        }
    }
    return out;
}


std::string mosTextMacToHost(const std::string &text)
{
    return mosTextMacToHost((const uint8_t*)text.data(), text.size());
}
