# Dethrace NX 

This Nintendo Switch port based on this repo: [link](https://github.com/dethrace-labs/dethrace), thanks to its creators!

IMPORTANT -> put `config.txt` (from `packaging/switch`) next to the `.nro` executable.

The build targets firmware 21+/Atmosphere 1.10+ (libnx 4.10 or newer, see [the TLS note](#firmware-21-and-libnx)).

## Controls

### Menu

- Up / Down - DPAD Up / Down
- Left / Right - Left Thumbstick Left / Right
- OK - Plus (A)
- Back - Minus

### Game sequence

- Pause - Minus
- Gas - ZR (DPAD Up)
- Brake - ZL (DPAD Down)
- Left / Right - Left Thumbstick Left / Right
- Camera control - Right Thumbstick
- Screen size adjustment - DPAD Left / Right

## Game content

Dethrace does not ship with any content. You'll need access to the data from the original game. If you don't have an original CD then you can [buy Carmageddon from GoG.com](https://www.gog.com/game/carmageddon_max_pack).

`dethrace` also supports the various freeware demos:
- [Original Carmageddon demo](https://rr2000.cwaboard.co.uk/R4/PC/carmdemo.zip)
- [Splat Pack demo](https://rr2000.cwaboard.co.uk/R4/PC/splatdem.zip)
- [Splat Pack Xmas demo](https://rr2000.cwaboard.co.uk/R4/PC/Splatpack_christmas_demo.zip)


## Building

### Dependencies

Those, who want improve something, or just compile by their own, should follow next steps:

- Setup switch homebrew enviroment, more info: [link](https://devkitpro.org/wiki/Getting_Started)
- Install `switch-sdl2` (`switch-sdl2_mixer` is no longer needed: audio goes through miniaudio and an SDL audio callback)
- `libnx` 4.10 or newer, CMake and Ninja

### Clone

Dethrace uses [git submodules](https://git-scm.com/book/en/v2/Git-Tools-Submodules), so we must pull them after the inital clone:
```sh
git clone https://github.com/alexart878/dethrace-switch.git
cd dethrace-switch
git submodule update --init --recursive
```

Dethrace uses [cmake](https://cmake.org/) for generating build files.

To generate the build files (generally only required once):
```sh
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DNINTENDO_SWITCH=ON -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Switch.cmake
```

Use `-DCMAKE_BUILD_TYPE=Release`: devkitPro's toolchain file leaves the build type empty, CMake then falls back to Debug and BRender's asserts/trace logging make the `.nro` hang at startup.

To build `.nro` executable:
```sh
cmake --build build/
```

## Running the game

Firstly, you need a copy of the [Carmageddon game content](https://github.com/dethrace-labs/dethrace?tab=readme-ov-file#game-content). Extract the zip file if necessary.

Dethrace expects to be placed into the top level Carmageddon folder. You know you have the right folder when you see the original `CARMA.EXE` there. If you are on Windows, you must also place `SDL2.dll` in the same folder.

For NX version, create folder `sdmc:/switch/dethrace`, put `.nro` and data files (`CARMA` folder contents) to it.

<img width="638" alt="Screenshot 2024-09-20 at 12 25 05 PM" src="https://github.com/user-attachments/assets/fda77818-9007-44fa-9d8d-c311396fd435">


### Graphics

Set `opengl=true` in `config.txt` to run in 3dfx mode (the default). The red/blue track bug of this mode is fixed by the BRender submodule update (it pulls dethrace-labs/BRender-v1.3.2#34, #37 and #38; see dethrace issues #1125 and #1158). `novoodoo=true` selects the plain software renderer.

### CD audio

Dethrace supports the GOG cd audio convention. If there is a `MUSIC` folder in the Carmageddon folder containing files `Track02.ogg`, `Track03.ogg` etc, then Dethrace will use those files in place of the original CD audio functions.

<img width="571" alt="Screenshot 2024-09-30 at 8 31 59 AM" src="https://github.com/user-attachments/assets/cec72203-9156-4c2a-a15a-328609e65c68">





## Firmware 21 and libnx

Firmware 21.0.0 (and Atmosphere 1.10+ on every firmware) uses the thread local region slot that libnx < 4.10 handed out as user TLS storage, so older homebrew builds get their GL state overwritten and render wrong colours or crash. Binaries built with libnx 4.10+ use offset `0x180`; this can be checked with a disassembler for `ldr x, [x, #0x180]` in `threadTlsGet`.

## Credits
- CrayzKirk (manually matching up functions and data structures in the executable to the debugging symbols)
- The developer at Stainless Software who left an old debugging .SYM file on the Splat Pack CD ;)
- [dethrace-labs](https://github.com/dethrace-labs) for his original repo

## Legal
Dethrace is released under the GPLv3 license. The documentation and function provided by Dethrace may only be utilized with assets provided by ownership of Carmageddon.

If you use the source code you may not charge others for access to it or any derivative work thereof.

Dethrace and any of its' maintainers are in no way associated with or endorsed by SCi, Stainless Software or THQ Nordic.
