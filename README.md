# Azerfall

Azerfall is a 2D tile-based adventure game being ported to the
[RP6502](https://picocomputer.github.io/) platform. The project is currently
in its early porting stage: the RP6502 build and ROM packaging are in place,
and the next milestone is a video and input smoke test.

The original game is [My2DGame](https://github.com/brentward/My2DGame), a Java
desktop game with tile-based exploration, combat, characters, items, maps,
and progression. Azerfall is a new C adaptation for the resource and runtime
constraints of the RP6502 rather than a direct line-by-line translation.

See [PORTING_PLAN.md](PORTING_PLAN.md) for the current milestones and the
planned order for bringing the game systems across.

## Requirements

- CMake 3.21 or newer
- Python 3
- Git
- GNU Make, Ninja, or another build tool supported by CMake
- [CC65](https://cc65.github.io/getting-started.html) and/or
  [LLVM-MOS](https://llvm-mos.org/wiki/Welcome)

The project uses CMake presets to select the 6502 compiler. The RP6502 tools
and emulator are downloaded when the project is configured.

## Build

List the available presets, configure one, and build the ROM:

```bash
cmake --list-presets
cmake --preset cc65/Debug
cmake --build --preset cc65/Debug
```

The resulting ROM is:

```text
build/cc65/debug/azerfall.rp6502
```

Release builds use the corresponding `cc65/Release` preset. LLVM-MOS presets
are available when LLVM-MOS is installed.

## Run

Use the included tool to upload the ROM and attach to its console:

```bash
python3 tools/rp6502.py run build/cc65/debug/azerfall.rp6502
```

The default device is `/dev/cu.usbmodem*` on macOS, `/dev/ttyACM0` on Linux,
and `COM1` on Windows. Pass `-d` to select another serial device, or provide
a hostname and `-k` passkey to connect over telnet.

For the current boot milestone, the console should show:

```text
AZERFALL RP6502 PORT
runtime online
next milestone: video and input
```

## Documentation

- [RP6502 SDK documentation](https://picocomputer.github.io/sdk.html)
- [Picocomputer](https://picocomputer.github.io/)
- [CC65](https://cc65.github.io/)
- [LLVM-MOS](https://llvm-mos.org/)

## Credits

Azerfall is based on my Java game project, [My2DGame](https://github.com/brentward/My2DGame), which was originally created by following RyiSnow's *How to Make a 2D Game in Java* / *Blue Boy Adventure* tutorial series.

My2DGame deviated from and expanded upon the tutorial implementation in
various areas, and Azerfall is a new adaptation of that project for the
RP6502 platform.

Special thanks to RyiSnow for creating the original tutorial series that
inspired this project.

**Original tutorial:** [RyiSnow's Blue Boy Adventure / How to Make a 2D Game in Java](https://www.youtube.com/playlist?list=PL_QPQmz5C6WUF-pOQDsbsKbaBZqXj4qSq)
