/* hello.c — “smart quotes”, café, naïve, …
 * Line endings are Unix, encoding is UTF-8.
 */
#include "hello.h"

static const char text[] = "café ™ “quoted” — fin…";

const char *greeting(void)
{
    return text;   /* ü ö ä ß */
}

const char *mark(void) { return TRADEMARK; }
