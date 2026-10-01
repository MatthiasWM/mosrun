#!/bin/sh
#
# Integration test for text conversion with the C/C++ tool chain.
#
# Compiles the same sources once as MacRoman/CR (no conversion) and once as
# UTF-8/LF (with ---text-in=utf8), then links both. All objects and linked
# images must be byte-identical.
#
# Usage: run_chain.sh <directory with ARM6c, ARMCpp, and ARMLink>

set -u

BIN="$1"
SRC="$(cd "$(dirname "$0")" && pwd)"
WORK="$(mktemp -d "${TMPDIR:-/tmp}/mosrun-text.XXXXXX")"
trap 'rm -rf "$WORK"' EXIT
FAILED=0

fail() {
    echo "FAIL: $*"
    FAILED=1
}

pass() {
    echo "ok:   $*"
}

cp -R "$SRC/mac" "$SRC/utf8" "$WORK/"

# Baseline: Mac sources, no conversion
cd "$WORK/mac"
"$BIN/ARM6c" -c hello.c -o hello.o 2>/dev/null || fail "ARM6c on Mac source"
"$BIN/ARMCpp" -c world.cpp -o world.o 2>/dev/null || fail "ARMCpp on Mac source"
"$BIN/ARMLink" -AOF -o all.aof hello.o world.o || fail "ARMLink on Mac objects"

# UTF-8 sources with conversion must produce the same objects
cd "$WORK/utf8"
"$BIN/ARM6c" ---text-in=utf8 -c hello.c -o hello.o 2>/dev/null || fail "ARM6c on UTF-8 source"
"$BIN/ARMCpp" ---text-in=utf8 -c world.cpp -o world.o 2>/dev/null || fail "ARMCpp on UTF-8 source"
"$BIN/ARMLink" -AOF -o all.aof hello.o world.o || fail "ARMLink on UTF-8 objects"
for f in hello.o world.o all.aof; do
    if cmp -s "$f" "../mac/$f"; then pass "$f from UTF-8 source matches Mac source"
    else fail "$f from UTF-8 source differs from Mac source"; fi
done

# Text that is already in Mac format must not be converted twice
cd "$WORK/mac"
"$BIN/ARM6c" ---text=utf8 -c hello.c -o hello2.o 2>/dev/null || fail "ARM6c with ---text=utf8 on Mac source"
if cmp -s hello2.o hello.o; then pass "Mac source is not converted again"
else fail "Mac source was changed by ---text=utf8"; fi

# Without the flag, UTF-8 sources must still be passed through unchanged
# (plain.c is valid C even when its LF line endings are not recognized)
cd "$WORK/utf8"
"$BIN/ARM6c" -c plain.c -o raw.o 2>/dev/null || fail "ARM6c on plain.c without flag"
"$BIN/ARM6c" ---text-in=utf8 -c plain.c -o conv.o 2>/dev/null || fail "ARM6c on plain.c with flag"
if LC_ALL=C grep -q "caf$(printf '\303\251')" raw.o && LC_ALL=C grep -q "caf$(printf '\216')" conv.o; then
    pass "no conversion without a flag"
else
    fail "plain.c: expected UTF-8 without flag and MacRoman with flag"
fi

# Unsupported characters are reported three times, then summarized
MSG="$("$BIN/ARM6c" ---text-in=utf8 -c misfit.c -o misfit.o 2>&1)"
COUNT="$(echo "$MSG" | grep -c "is not in MacRoman")"
if [ "$COUNT" = "3" ] && echo "$MSG" | grep -q "misfit.c: and 3 more"; then
    pass "unsupported characters reported"
else
    fail "unexpected misfit report: $MSG"
fi

# Text output is converted to UTF-8/LF, binary output is not touched
"$BIN/ARMLink" ---text-out=utf8 -AOF -o out.aof -Symbols syms.txt hello.o world.o \
    || fail "ARMLink with ---text-out=utf8"
if [ -s syms.txt ] && ! tr -d '\n' < syms.txt | grep -q "$(printf '\r')" \
   && grep -q "Symbol Table" syms.txt; then
    pass "text output uses LF"
else
    fail "text output still contains CR"
fi
if cmp -s out.aof all.aof; then pass "binary output not converted"
else fail "binary output was changed by ---text-out"; fi

# A failed compile deletes its partial output and exits with the tool's own
# error code (it used to crash with SIGTRAP when deleting the file)
cd "$WORK/utf8"
"$BIN/ARM6c" ---text-in=utf8 -c broken.c -o broken.o >/dev/null 2>&1
RC=$?
if [ "$RC" = "1" ] && [ ! -e broken.o ]; then pass "failed compile removes its object file"
else fail "failed compile: exit code $RC, broken.o exists: $([ -e broken.o ] && echo yes || echo no)"; fi

# The same for a text output that is buffered in memory
"$BIN/ARM6c" ---text=utf8 -S broken.c -o broken.s >/dev/null 2>&1
RC=$?
if [ "$RC" = "1" ] && [ ! -e broken.s ]; then pass "failed compile removes its buffered text output"
else fail "failed compile with -S: exit code $RC, broken.s exists: $([ -e broken.s ] && echo yes || echo no)"; fi

exit $FAILED
