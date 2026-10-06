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

3. **Core runtime and map renderer — complete**
   - Add a fixed-rate update loop and explicit game states.
   - Convert one small source map into a compact target-friendly format.
   - Render the map with a camera and solid-tile collision data.
   - Acceptance: the player can walk around the first map without leaving
     the playable area.
   - Verified on real hardware (2026-09-13): the player is blocked at every
     playable-area boundary, including diagonal approaches.

4. **Player and interaction**
   - Port player movement, facing, animation timing, collision, and object
     interactions (keys, doors, and chests).
   - Acceptance: movement and the key/door/chest route work without debug
     shortcuts. Weapons and damage follow the life system and enemy targets.

5. **OPL2 music — next**
   - Follow the original game's implementation order by adding music before
     weapons and combat.
   - Convert `assets/music/z3lightw.mid` into an editable RPTracker `.RPT`
     song with `tools/midi_to_rpt.py`; see `tools/midi_to_rpt.md`.
   - Audition and adjust the song in RPTracker, then export its OPL2 register
     stream as `.BIN` for use by the game.
   - Add a bounded, nonblocking music player using the RP6502 native OPL2
     system, checking its memory layout against the existing graphics data.
   - Acceptance: music plays and loops while walking around the map without
     disrupting movement or rendering.

6. **Life system and first combat**
   - Add player health, damage, and death behavior.
   - Add an enemy target, then one weapon and attack/hit detection.
   - Acceptance: attacks damage an enemy, and the player can take damage and
     reach the death state without debug shortcuts.

7. **World entities and progression**
   - Add the remaining NPC, monster, projectile, item, and interactive-tile
     types behind shared compact data structures.
   - Port the minimum combat, inventory, experience, and area-transition
     behavior needed for the first complete route.
   - Acceptance: the first route from the starting area to its required
     objective is playable.

8. **UI, save data, sound effects, and remaining content**
   - Port title, pause, dialogue, inventory, map, options, game-over, and
     transition states.
   - Add save/load only after the in-memory data model is stable.
   - Add sound effects and remaining music/image assets according to the
     RP6502 storage and memory limits.
   - Acceptance: the port has a complete start-to-save play session.

9. **Parity and optimization**
   - Compare behavior against the desktop game using focused scenarios.
   - Measure ROM, RAM, frame time, and asset sizes; optimize only where the
     measurements require it.
   - Acceptance: documented differences are intentional and the target stays
     within its resource budgets.

## Next step

Prepare the first music asset from the repository root:

```sh
python tools/midi_to_rpt.py assets/music/z3lightw.mid assets/music/Z3LIGHTW.RPT --rows-per-beat 6
```

Use the native OPL2 build from upstream commit `6300f89`, available locally at
`build/RPTracker-latest/RPTracker.rp6502`. Load the regenerated `Z3LIGHTW.RPT`;
RPT3 stores its **204 BPM** tempo automatically. Select SONG mode with F8,
audition the complete song, and export
`Z3LIGHTW.BIN`. The converter is implemented; tracker listening checks and
in-game playback are the next tasks. See `tools/midi_to_rpt.md` for the
conversion limits and tracker controls.

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
