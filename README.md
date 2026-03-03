# Trials HD
This is a reimplementation and research repository dedicated to the game "Trials HD" which was released on August 12th, 2009 on the Xbox 360 on the Xbox Live Arcade. The player guides a trial motorcycle with physics through various obstacles to reach each stage's finish line. The game has a track creator and leaderboards that connect to Xbox Live.

The game has never been ported in it's original form to PC, it's only been on Xbox 360.

# Build Configurations
- Release
	- Release build, has development info.
- ReleaseNoDev
	- Release but no dev info at all.
- BankRelease
	- Release build w/ debugging features.
- Profile
	- Used for profiling.
- Debug
	- Primary debug build used during development.

# Platform Support
As of right now, only 64-bit versions of these operating systems are supported.
- Windows (MSVC)
- macOS (Clang)
- Linux (GCC/Clang)

# Original Development Info
RedLynx developed Trials HD, and used a modified version of the Bullet Physics Library which was optimized to utilize the Xbox 360's CPU and vector units. RedLynx also tweaked the physics in a way to "blend the reality in just the proper way" RedLynx saw this as a "crucial thing in making Trials such a fun and addicting game" and it shows. Levels in the game were designed using the same in-game level editor that allowed players to make levels as well.

# Layout
This is the layout of the repo.
```
art/
    Contains original art assets for the game.
code/
    engine/
        The engine code.
    game/
        The game code.
    shared/
        Shared code across everything.
    tools/
        Tools for the game and engine.
data/
    dlc/
        DLC content.
    main/
        The main content, this is from the July build.
    prototype/
        Prototype content, from early builds and early dlc versions.
    raw/
        Raw content in raw data formats.
docs/
    Contains technical write ups and misc documents relating to Trials HD.
dump/
    Contains dumps of game files.
extern/
    Contains external code, e.g., bullet and imgui.
tools/
    Tools that aren't made for this project, e.g., quickbms and xextool, for research.
```

# Technical Info
- Platform: Xbox 360
- Architecture: PowerPC
- XDK used: March 2009
