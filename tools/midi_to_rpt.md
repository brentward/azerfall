# MIDI to RPTracker

`midi_to_rpt.py` converts Standard MIDI format 0/1 files to an editable RPTracker
song, using only Python's standard library (Python 3.10 or newer). The default
is **RPT3**, which stores tempo for the current upstream source build.

## Current tracker build

Use `build/RPTracker-latest/RPTracker.rp6502`, built with native OPL2 from
upstream commit `6300f89d7eef22ef05a0401b00070d5172f94827` (latest checked on
2026-09-20). Source and license are in `build/RPTracker-reference`.
No local source patches are applied. This commit includes partial-transfer
loops, RPT3 tempo loading, and RPT4 custom instrument-bank saving.

The build succeeded; this version still needs a hardware playback check.
The patched v0.8 build below was confirmed working on hardware and remains
available as a fallback. To reproduce the current build, check out the pinned
commit in `build/RPTracker-reference` and run:

```sh
cmake -S build/RPTracker-reference -B build/RPTracker-latest -G "MinGW Makefiles" -DCMAKE_MAKE_PROGRAM=make -DLLVM_MOS_PLATFORM=rp6502 -DCMAKE_PREFIX_PATH=C:/opt/llvm-mos -DCMAKE_BUILD_TYPE=Release "-DCMAKE_C_FLAGS_RELEASE=-Oz -flto -DNDEBUG" -DUSE_NATIVE_OPL2=ON -DPython3_EXECUTABLE=C:/Users/brent/AppData/Local/Python/bin/python.exe
cmake --build build/RPTracker-latest -j 4
```

## Legacy v0.8 load/save fix

The hardware round-trip file `assets/music/CHECK.RPT` confirmed a truncated
transfer: it is 33,031 bytes, exactly the original 8-byte header followed by
32,767 pattern bytes and the next 256 pattern bytes incorrectly used as the
sequence. All saved bytes match that prediction. This explains repeated
opening patterns and invalid pattern selections. The correct RPT2 size is
46,344 bytes. A one-pattern piano test can play despite this bug.

`tools/rptracker-v08-io.patch` fixes the v0.8 release's `src/song.c` by transferring
patterns and order data in at most 4,096-byte chunks, handling partial transfers
and reporting failures. It preserves RPT2 and the release's playback code.
The local patched source and successfully compiled native-OPL2 ROM are at:

```text
build/RPTracker-v08-fixed/src/song.c
build/RPTracker-v08-fixed/out/RPTracker.rp6502
```

For this fallback ROM, regenerate a song using `--rpt-version 2` and set its
tempo manually. The default RPT3 output is incompatible with v0.8. Hardware
playback with the patched release and RPT2 was confirmed working by the user.
Never load the truncated CHECK file as a song.
To reproduce, extract RPTracker tag `v0.8`, apply the patch with
`git apply --ignore-space-change` (to accommodate Windows line endings),
and build with llvm-mos. This workspace used these Windows commands:

```sh
cmake -S build/RPTracker-v08-fixed -B build/RPTracker-v08-fixed/out -G "MinGW Makefiles" -DCMAKE_MAKE_PROGRAM=make -DCMAKE_PREFIX_PATH=C:/opt/llvm-mos -DCMAKE_BUILD_TYPE=Release "-DCMAKE_C_FLAGS_RELEASE=-Oz -flto -DNDEBUG" -DPython3_EXECUTABLE=C:/Users/brent/AppData/Local/Python/bin/python.exe
cmake --build build/RPTracker-v08-fixed/out -j 4
```

The extracted source includes upstream's GPL-3.0 license. The patched source
tree is kept with the ROM; this is a local diagnostic build, not a published release.

## Convert the song

From the repository root:

```sh
python tools/midi_to_rpt.py assets/music/z3lightw.mid assets/music/Z3LIGHTW.RPT --rows-per-beat 6 --report build/music/z3lightw.json
```

The six-row grid suits this song's snare triplets. It produces 204 **tracker**
BPM while preserving the source's 136 BPM playback speed: RPTracker has four
rows per tracker beat, so the converter scales its tempo to fit six rows per
original beat. `--bpm` overrides the grid's tempo, also preserving the MIDI's
elapsed timing. Neither option is a playback-speed control.

The generated song contains all 1,270 notes, 26 unique patterns, and 34 sequence
entries. The RPT3 file is 46,346 bytes; this is the editor format, not the eventual
game asset size. Source duration is 77.647 seconds; tracker duration is 79.971
seconds because the last pattern is padded. Maximum note-boundary quantization
error is about 55 ms, including minimum one-row durations for very short notes.
Peak demand is seven simultaneous voices, so the song fits channels 0-6 without
dropping notes. Channels 7 and 8 are left completely empty for sound effects.

## Load and export

1. Copy `Z3LIGHTW.RPT` to the USB storage used by RPTracker.
2. Press **Ctrl+O**, enter `Z3LIGHTW.RPT`, and load it.
3. Verify the loaded tempo is **204 BPM** (stored in RPT3).
   Use **F8** to select song mode and **Enter** to audition the sequence.
   Adjust instruments, timing, volume, and the ending as needed.
4. **Ctrl+S** saves edits (v0.8 saves RPT2; newer source builds save RPT4).
5. **Ctrl+E** exports `Z3LIGHTW.BIN`, an OPL2 register stream for a future game
   player. Native OPL2 should be the tracker build's selected sound target.

The converter stops at RPT; exporting BIN happens inside RPTracker. The current
upstream exporter has a 36,000-byte size guard and can truncate a large export;
check its console output and audition the exported result before integration.
The game plays `assets/music/Z3LIGHTW.BIN`, not the RPT file. After regenerating
the RPT, export it again in RPTracker, replace that BIN, and rebuild the game.
The existing BIN still contains music on channels 7 and 8.

Empty RPT channels prevent music notes from using the SFX channels, but
RPTracker's BIN exporter also emits chip initialization and all-channel
note-offs. The current game player applies these writes unchanged and clears
the entire OPL register area when music stops. Protecting SFX during music
restart, loop boundaries, and stop additionally requires the music player to
preserve the reserved channels.

## Conversion behavior and limits

- Merges tracks using absolute MIDI time, including tempo changes and running
  status. SMPTE timing and format 2 files are rejected.
- Uses GM program numbers directly. GM channel 10 percussion notes 35–81 map
  to RPTracker bank entries 169–215 at MIDI pitch 60. This is the GM drum map,
  not RPTracker's custom USB-controller pad map.
- Applies note velocity, channel volume (CC7), and expression (CC11) at note-on;
  honors sustain pedal (CC64), All Sound Off, and All Notes Off.
- Volume changes during sounding notes are reported and applied only to later
  notes. The supplied song has 75 such changes, so its volume envelopes are
  approximate. Pitch bend, aftertouch, SysEx, and other controllers are reported
  as ignored. OPL2 patches will sound different from the original MIDI synth.
- Quantizes starts and releases to the nearest row, with a minimum one-row
  note duration. Timing uses RPTracker's actual 60 Hz / 8.8 fixed-point formula.
  The default is four rows per source beat; use six for this song's triplets.
- Assigns up to seven simultaneous voices on OPL channels 0-6, preferring the
  same voice for a MIDI channel. Channels 7 and 8 are always reserved for SFX;
  the RPT binary layout still stores nine channels for tracker compatibility.
  Exceeding seven voices fails by default; `--overflow drop` explicitly
  permits dropping new notes and reports the count. Quantization can increase
  simultaneous voice demand. No notes are dropped for the supplied song.
- Rejects melodic notes outside 12–107 rather than silently changing their
  octave (RPTracker clamps higher notes into its highest OPL block).
- Deduplicates identical 32-row patterns. More than 32 unique patterns fails;
  try a coarser grid. At most 255 sequence entries are emitted because the
  current player uses an 8-bit sequence cursor. Unused patterns/entries are zero.
- Adds explicit note-offs before the sequence ends. The final pattern is padded
  to 32 rows, so the loop may have trailing silence that needs tracker editing.

## Format reference and checks

Checked against the [v0.8 loader](https://github.com/jasonfrowe/RPTracker/blob/v0.8/src/song.c)
and [newer RPTracker source](https://github.com/jasonfrowe/RPTracker/tree/6300f89d7eef22ef05a0401b00070d5172f94827):
`src/song.c` (`load_song`), `src/player.c` (sequencer and export),
`src/opl.c` (pitch), and `src/instruments.c` (GM bank).
The initially generated RPT3 file was incompatible with v0.8: that loader
does not check the version and reads RPT3's extra two tempo bytes as pattern
data, shifting both patterns and sequence entries. Use the regenerated RPT2
file when using v0.8, not an RPT3 file. The current workflow instead pairs the
new source build with RPT3.

`--rpt-version 3` is available for newer source builds that explicitly support
RPT3. It stores BPM automatically. Both formats use the tracker's built-in bank;
no upstream patch data or player code is copied into this project.

| Data | RPT2 offset | RPT3 offset |
| --- | --- | --- |
| ASCII `RPT2` or `RPT3` | 0 | 0 |
| Editor octave, one byte (3) | 4 | 4 |
| Editor volume, one byte (63) | 5 | 5 |
| Sequence length, little-endian uint16 | 6 | 6 |
| Tracker BPM, little-endian uint16 (60–240) | Not stored | 8 |
| 32 patterns × 32 rows × 9 channels × 5 bytes | 8 | 10 |
| 256 one-byte pattern IDs | 46088 | 46090 |

Each cell is `note, instrument, volume, effect_low, effect_high`. Note 0 means
empty, 255 means release, and other values are MIDI pitches. Volume is 0–63.
The converter writes zero effects. Neither format includes an RPT4 custom bank;
playback uses the built-in GM patches.

Run the parser, timing, voice, capacity, and binary-layout regression checks:

```sh
python -m unittest discover -s tools -p test_midi_to_rpt.py -v
```

These include decoding the generated project song and verifying all 1,270 note
starts, final voice releases, and empty reserved channels in every stored
pattern for both RPT2 and RPT3. Actual tracker loading, listening, and BIN
export still need to be checked on the tracker/emulator.
