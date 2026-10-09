// VIMEK — modifier-only shortcut state. GPL-3.0.
#pragma once
struct VimekModifierShortcut {
    unsigned previous = 0;
    bool armed = false, cancelled = false;
    // Normalized bits: Control=1, Alt/Option=2, Command/Win=4, Shift=8.
    // Fire once on release after an exact, unused chord. Keeping one modifier
    // held allows the other to be pressed and released for the next toggle.
    bool update(unsigned current, unsigned required, bool otherKeyDown = false, bool blocked = false) {
        if (!required) { reset(); return false; }
        if (otherKeyDown || blocked || (current & ~required)) cancelled = true;
        bool fire = armed && !cancelled && previous == required && current != required;
        if (current == required && !cancelled) armed = true;
        previous = current;
        if (fire) armed = false;
        if (!current) reset();
        return fire;
    }
    void reset() { previous = 0; armed = cancelled = false; }
};
