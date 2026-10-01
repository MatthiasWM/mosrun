// world.cpp – C++ with UTF-8: «Français» ‘single’ π ≠ ∞
#include "hello.h"

class World {
public:
    const char *name() const { return "Welt – Erde ∑"; }
};

extern "C" const char *worldName(void)
{
    World w;
    return w.name();
}
