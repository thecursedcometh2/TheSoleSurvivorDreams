# The Sole Survivor Dreams

Fallout 4 dream framework using F4SE, CommonLibF4RD and its Runtime Database.
A small Papyrus quest listens for completed player sleep; the native DLL rolls
15% once per wake and selects one of 28 equally likely dream placeholders.
The popup is requested one real second after `SleepWaitMenu` has closed
(Papyrus and main-thread scheduling can make it later).
Interrupted sleep, waiting, and simply cancelling the menu are excluded.

## Write the dreams

Edit **[src/Dreams.h](src/Dreams.h)**. Replace the text inside each
`R"dream(...)dream"` string with your dream. Multiline text is supported; do not
include the delimiter `)dream"` inside it. Rebuild the DLL after editing.
Every entry has a comment identifying its effect:

- +1 / -1 Strength, Perception, Endurance, Charisma, Intelligence, Agility, Luck.
- +10 / -10 maximum health, maximum AP, damage resistance, energy resistance,
  environmental radiation resistance, XP multiplier, carry weight.

Each effect lasts **12 game hours**, including time spent sleeping, waiting, or
fast travelling. Expiry is checked every real second during play and after load.
A new dream replaces the old modifier; a failed 15% roll leaves an unexpired
modifier alone. Effects do not stack. SPECIAL changes may also affect derived
stats through Fallout 4's normal rules. Radiation resistance affects exposure,
not radiation ingested from food/drink. XP modifies `ExperienceMult` by 10
percentage points (vanilla multiplier 100 -> 110 or 90), rather than awarding
or removing XP immediately. Other mods' multipliers combine additively.

## Build and install

See **[docs/BUILD.md](docs/BUILD.md)** for the Windows DLL build, Papyrus compilation,
and required controller quest setup. F4SE is required by this implementation;
the general concept could also be implemented with a plugin and vanilla Papyrus.
The Runtime Database is a separate end-user download, not the CommonLib source.

**This repository is a source framework, not an installable release.** It does not
yet contain a built DLL, compiled PEX files, or the companion ESL-flagged plugin.
Windows compilation, quest assembly, and in-game acceptance testing remain required.
CommonLibF4RD advertises OG/NG/AE runtime support; this mod has not yet been tested
on any runtime. No unsupported hard-coded game offsets are used here.

## Persistence and removal

Native modifiers are included in the game's save; the matching `.f4se` co-save
stores the exact delta, actor value, and expiry so loading does not apply them
twice. Keep the save and its co-save together. Do not delete the co-save or
remove this mod while a modifier is active. A missing/corrupt co-save cannot
safely reconstruct which modifier belonged to this mod.

Before removing, run `cqf TSSD_DreamController DisableDreams`, wait a few seconds
with gameplay running, verify the modifier is gone, and save before uninstalling.
This disables the controller permanently in that save. Use a disposable save for
initial testing; see [docs/TESTING.md](docs/TESTING.md).

## Portable tests

On Linux or Windows with a C++20 compiler:

```sh
g++ -std=c++20 -Wall -Wextra -Werror -Isrc tests/DreamsTests.cpp -o /tmp/tssd-tests
/tmp/tssd-tests
```

Or use CMake with `-DDREAMS_TESTS_ONLY=ON` and run CTest. These tests cover catalog,
chance boundaries, and time calculations. They do not validate game integration.
