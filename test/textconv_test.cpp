/*
 Unit tests for the host <-> Mac text conversion in textconv.cpp.

 Build and run with ctest, or run the textconv_test executable directly.
 */

#include "../textconv.h"

#include <stdio.h>
#include <string>
#include <vector>


static int gFailures = 0;

#define CHECK(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "%s:%d: CHECK failed: %s\n", __FILE__, __LINE__, #cond); \
        gFailures++; \
    } } while (0)

static std::string h2m(const std::string &s, std::vector<MosTextMisfit> *m = nullptr)
{
    return mosTextHostToMac(s, m);
}

static std::string m2h(const std::string &s)
{
    return mosTextMacToHost(s);
}

static MosTextInfo analyze(const std::string &s)
{
    return mosTextAnalyze((const uint8_t*)s.data(), s.size());
}


static void testRoundTripAllMacRoman()
{
    std::string mac;
    for (int c = 1; c < 256; c++)
        if (c != '\n') mac += (char)c; // LF has no MacRoman meaning to preserve
    std::string host = m2h(mac);
    CHECK(analyze(host).validUtf8);
    std::vector<MosTextMisfit> misfits;
    CHECK(h2m(host, &misfits) == mac);
    CHECK(misfits.empty());
}

static void testLineEndings()
{
    CHECK(h2m("a\nb\n") == "a\rb\r");
    CHECK(h2m("a\r\nb\r\n") == "a\rb\r");
    CHECK(h2m("a\rb\r") == "a\rb\r");
    CHECK(h2m("a\r\n\r\nb") == "a\r\rb");
    CHECK(m2h("a\rb\r") == "a\nb\n");
}

static void testKnownCharacters()
{
    // é ™ “ ” — … ◊ €
    CHECK(h2m("caf\xC3\xA9") == "caf\x8E");
    CHECK(h2m("\xE2\x84\xA2") == "\xAA");
    CHECK(h2m("\xE2\x80\x9C" "q" "\xE2\x80\x9D") == "\xD2q\xD3");
    CHECK(h2m("\xE2\x80\x94\xE2\x80\xA6") == "\xD1\xC9");
    CHECK(h2m("\xE2\x97\x8A") == "\xD7");
    CHECK(h2m("\xE2\x82\xAC") == "\xDB");
    // The old converter dropped everything at or above U+0800 when writing
    CHECK(m2h("\xAA\xD2\xD3\xC9") == "\xE2\x84\xA2\xE2\x80\x9C\xE2\x80\x9D\xE2\x80\xA6");
}

static void testMisfits()
{
    std::vector<MosTextMisfit> misfits;
    // line 1: emoji (4 byte UTF-8), line 3: CJK, line 3: Greek alpha
    std::string mac = h2m("a\xF0\x9F\x98\x80" "b\n\n\xE4\xB8\xAD \xCE\xB1\n", &misfits);
    CHECK(mac == "a\xD7" "b\r\r\xD7 \xD7\r");
    CHECK(misfits.size() == 3);
    if (misfits.size() == 3) {
        CHECK(misfits[0].line == 1 && misfits[0].codepoint == 0x1F600);
        CHECK(misfits[1].line == 3 && misfits[1].codepoint == 0x4E2D);
        CHECK(misfits[2].line == 3 && misfits[2].codepoint == 0x03B1);
    }
    // CRLF counts as one line
    misfits.clear();
    h2m("x\r\ny\r\n\xCE\xB1", &misfits);
    CHECK(misfits.size() == 1 && misfits[0].line == 3);
}

static void testDetection()
{
    // Plain ASCII with CR or without line breaks is already Mac text
    CHECK(!analyze("int x;\rint y;\r").isHostText());
    CHECK(!analyze("int x;").isHostText());
    CHECK(!analyze("").isHostText());
    // LF or CRLF means host text
    CHECK(analyze("int x;\n").isHostText());
    CHECK(analyze("int x;\r\n").isHostText());
    // UTF-8 means host text, even with CR line endings
    CHECK(analyze("caf\xC3\xA9\r").isHostText());
    // MacRoman high bytes with CR are not valid UTF-8, so they are Mac text
    CHECK(!analyze("caf\x8E \xAA\r").isHostText());
    // NUL means binary, never convert
    MosTextInfo bin = analyze(std::string("ab\0\ncd", 6));
    CHECK(bin.binary);
    CHECK(!bin.isHostText());
}

static void testMacRomanWithLF()
{
    // MacRoman bytes with Unix line endings: keep the bytes, fix the lines
    std::string in = "caf\x8E\n\xAA\n";
    CHECK(!analyze(in).validUtf8);
    CHECK(h2m(in) == "caf\x8E\r\xAA\r");
}

static void testInvalidUtf8()
{
    // A truncated sequence at the end, an overlong encoding, and a surrogate
    CHECK(!analyze("abc\xE2\x84").validUtf8);
    CHECK(!analyze("\xC0\xAF").validUtf8);
    CHECK(!analyze("\xED\xA0\x80").validUtf8);
    // Invalid UTF-8 is passed through byte by byte
    CHECK(h2m("abc\xE2\x84") == "abc\xE2\x84");
}

static void testUtf8Encoding()
{
    CHECK(mosTextUtf8('A') == "A");
    CHECK(mosTextUtf8(0xE9) == "\xC3\xA9");
    CHECK(mosTextUtf8(0x25CA) == "\xE2\x97\x8A");
    CHECK(mosTextUtf8(0x1F600) == "\xF0\x9F\x98\x80");
}

static void testBinaryUnchanged()
{
    std::string bin("\x00\n\r\xC3\xA9", 5);
    CHECK(h2m(bin) == bin);
}


int main()
{
    testRoundTripAllMacRoman();
    testLineEndings();
    testKnownCharacters();
    testMisfits();
    testDetection();
    testMacRomanWithLF();
    testInvalidUtf8();
    testUtf8Encoding();
    testBinaryUnchanged();
    if (gFailures) {
        fprintf(stderr, "textconv_test: %d check(s) failed\n", gFailures);
        return 1;
    }
    printf("textconv_test: all checks passed\n");
    return 0;
}
