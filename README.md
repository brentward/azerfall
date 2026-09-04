# RP6502 Project Template

Scaffolding for a new Picocomputer 6502 software project. It builds with
either 6502 compiler, cc65 or llvm-mos, and switching between them is one
setting. Three "Hello, world!" examples are included to start from:

 * `src/main.c` — C, and builds with either compiler.
 * `src/main-cc65.s` — assembly for cc65, which uses the ca65 syntax.
 * `src/main-llvm-mos.s` — the same program in llvm-mos assembly.

Make sure `CMakeLists.txt` points to the one you want, then delete the others.
The two assembly files pick up where the C runtime leaves off, so they read
alike; only the assembler directives differ.

### Requirements:
 * CMake 3.21 or newer
 * Python 3
 * git
 * A build tool CMake can drive — GNU Make or Ninja
 * At least one 6502 compiler:
   [CC65](https://cc65.github.io/getting-started.html) and/or
   [LLVM-MOS](https://llvm-mos.org/wiki/Welcome). Install both if you want to
   try both; nothing here makes you choose once.

CC65 needs a source build. LLVM-MOS has its own installer. Do not use the
CC65 or LLVM-MOS from a package manager, we need the latest version you
can tolerate. The other requirements aren't as version sensitive.

Linux:
```bash
$ sudo apt install cmake python3 git build-essential
```

Windows:
 * `winget install -e --id Git.Git`
 * `winget install -e --id Kitware.CMake`
 * `winget install -e --id GnuWin32.Make`
   Add `C:\Program Files (x86)\GnuWin32\bin` to your PATH.
 * The current snapshot of [CC65](https://cc65.github.io/getting-started.html) -
   Do not skip the step about adding the `bin` directory to your PATH.
   And/or an install of [LLVM-MOS](https://llvm-mos.org/wiki/Welcome).
 * Install Python by typing `python3` in a command prompt, which will launch
   the Microsoft Store where you can start the installation. If Python runs,
   this has already been done - exit Python with Ctrl-Z plus Enter.

### Getting started:
**The documentation is [RP6502-SDK](https://picocomputer.github.io/sdk.html).**
It covers VS Code, assets, linker configuration, and packaging in full.
The remainder of this README is a quick start guide for someone already
familiar with the tools.


### Use the template:
Go to the [GitHub template](https://github.com/picocomputer/rp6502-sdk) and
select "Use this template" then "Create a new repository". GitHub will create
a clean project for you to start with. Then you can clone the repository.

```bash
$ git clone [path_to_github]
$ cd [to_where_it_cloned]
```

### Building:
The compiler is a CMake preset. There is a Debug and a Release of each,
building into its own directory under `build/`, so you can switch back and
forth without a rebuild from scratch. Debugging needs a Debug build.

```bash
$ cmake --list-presets
$ cmake --preset cc65/Debug
$ cmake --build --preset cc65/Debug
```

That leaves a ROM at `build/cc65/debug/hello.rp6502`. The first configure
fetches `tools/` and the emulator; nothing is fetched after that.

### Running:
`tools/rp6502.py` sends a ROM to a Picocomputer and gives you its console.
It needs nothing but Python.

```bash
$ python3 tools/rp6502.py run build/cc65/debug/hello.rp6502
```

That uploads the ROM, starts it, and attaches a terminal. Ctrl-A then X exits,
Ctrl-A then B sends a break. Other commands:

The device defaults to the USB serial port where the Picocomputer usually
mounts: `/dev/ttyACM0` on Linux, `/dev/cu.usbmodem*` on macOS, `COM1` on
Windows. Override it with `-d`, or connect over telnet by giving a hostname
plus the passkey:

```bash
$ python3 tools/rp6502.py -d /dev/ttyUSB0 run build/cc65/debug/hello.rp6502
$ python3 tools/rp6502.py -d picocomputer.local -k mykey term
```

### The tools directory:
`tools/` holds the python and CMake scripts which drive the SDK.
A new project downloads them the first time you configure with CMake.
The emulator for your machine comes down at the same time and into the
same directory.

To pull down the current versions:

```bash
$ cmake -P tools/rp6502.cmake
```

### Updating an older project:
Projects made before this template merged cc65 and llvm-mos have their compiler
wired into the top of `CMakeLists.txt`, and a `tools/` that predates any of
this. Start by copying this template's `tools/rp6502.cmake` over yours. That
name used to be the cc65 toolchain file; it is now the small script that
fetches everything, and the toolchain it replaces comes back as
`tools/cc65-toolchain.cmake` on the first configure.

Then replace everything above `project()` with:

```cmake
cmake_minimum_required(VERSION 3.21)

include(${CMAKE_CURRENT_LIST_DIR}/tools/rp6502.cmake)
```

Delete `tools/CMakeLists.txt` and the `add_subdirectory(tools)` line that
pulled it in — the `include()` above replaces both. Copy `CMakePresets.json`
from this template as well; that is where the compiler is chosen now. Old
projects called `rp6502_executable()` with the address their compiler happened
to use, and `DATA default RESET default` works under both.

### Documentation:
 * [Picocomputer](https://picocomputer.github.io)
 * [CC65](https://cc65.github.io/)
 * [LLVM-MOS](https://llvm-mos.org/)
