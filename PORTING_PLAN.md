# Azerfall RP6502 Port Plan

## Current baseline

The source game in `My2DGame` is a Java/Swing tile-based RPG. The RP6502 port
uses a 320x180 canvas with unscaled 16x16 tiles and sprites. The canvas spans
20 tile widths and 11.25 tile heights, so tiles at the viewport edges may be
partially visible. The project builds a C program with cc65 and packages it
as an RP6502 ROM.

The port will preserve the game loop and player-facing behavior, but it will
replace desktop services with small target-specific systems. Java classes are
reference material, not files to translate one-for-one.

## Milestones

1. **Boot and build baseline**
   - Keep the ROM target buildable with cc65 and the RP6502 SDK.
   - Print a named runtime probe over the console.
   - Acceptance: the ROM builds and the console shows `runtime online`.

2. **Video and input smoke test**
   - Confirm the RP6502 video mode, VRAM/tile access, and controller or
     keyboard input available on the target.
   - Draw a fixed tile-sized test scene and move a cursor with one input
     action.
   - Acceptance: a stable 320x180 scene responds to input on hardware or the
     emulator.

3. **Core runtime and map renderer**
   - Add a fixed-rate update loop and explicit game states.
   - Convert one small source map into a compact target-friendly format.
   - Render the map with a camera and solid-tile collision data.
   - Acceptance: the player can walk around the first map without leaving
     the playable area.

4. **Player and interaction**
   - Port player movement, facing, animation timing, collision, attacks, and
     one interactable object.
   - Start with one player sprite set and one weapon.
   - Acceptance: movement, attack, damage, and an interaction can be played
     through without debug shortcuts.

5. **World entities and progression**
   - Add the remaining NPC, monster, projectile, item, and interactive-tile
     types behind shared compact data structures.
   - Port the minimum combat, inventory, experience, and area-transition
     behavior needed for the first complete route.
   - Acceptance: the first route from the starting area to its required
     objective is playable.

6. **UI, save data, audio, and remaining content**
   - Port title, pause, dialogue, inventory, map, options, game-over, and
     transition states.
   - Add save/load only after the in-memory data model is stable.
   - Convert audio and remaining image assets according to the RP6502 storage
     and memory limits.
   - Acceptance: the port has a complete start-to-save play session.

7. **Parity and optimization**
   - Compare behavior against the desktop game using focused scenarios.
   - Measure ROM, RAM, frame time, and asset sizes; optimize only where the
     measurements require it.
   - Acceptance: documented differences are intentional and the target stays
     within its resource budgets.

## First step

The boot probe is the first checkpoint. From the repository root:

```sh
cmake --build --preset cc65/Debug
```

The resulting ROM is `build/cc65/debug/azerfall.rp6502`. Run it with the
existing RP6502 emulator or hardware and verify these lines:

```text
AZERFALL RP6502 PORT
runtime online
next milestone: video and input
```

Once that works, the next implementation task is the video and input smoke
test. We should confirm the exact SDK calls from the installed RP6502 headers
before choosing a tile or sprite representation.

## Porting rules

- Keep target-independent game rules separate from RP6502 rendering and I/O.
- Prefer fixed-width integers, static storage, and bounded arrays over heap
  allocation.
- Convert assets with a repeatable tool or build step; do not hand-edit a
  large generated asset file.
- Port one visible, playable behavior at a time and keep the ROM buildable at
  every checkpoint.
- Treat the original Java game as the behavior reference for its tile
  coordinate system and state transitions, adapting the viewport to the
  port's unscaled 320x180 canvas.
