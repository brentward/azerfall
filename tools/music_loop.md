# Z3LIGHTW loop audition

## Playback clock and instrument setup

`sound_init()` enables OPL before writing the shared waveform-enable setting.
Enabling the device resets its registers, so `music_init()` must not enable
it again. See the upstream `opl_xreg()` implementation in
[RP6502 OPL source](https://github.com/picocomputer/rp6502/blob/main/src/core/aud/opl.c).
Previously, music loading erased this setting and then filtered the BIN's
attempt to restore it, making alternate-waveform instruments sound wrong.

The main loop waits for a fresh VSYNC after game logic, then publishes camera
and sprite positions before processing audio. A separate audio timestamp
persists across game/draw work and waits.
Audio advances by the elapsed 60 Hz frames rather than once per rendered
frame. This prevents missed frames from permanently stretching the music;
a long stall can still produce late/bunched notes while catching up. The
8-bit clock must be serviced within 256 frames (about 4.27 seconds).

The first pass should take about 77.63 seconds and later repeats about
70.57 seconds, excluding pauses. Tests simulate the OPL reset, music reload,
loop restoration, reserved-channel protection, elapsed frames, and wraparound:

```powershell
python -m unittest tools.test_opl_voices tools.test_music_runtime
```

## Loop points

The game plays the intro once, then repeats from sequence **03**, row **00**
through the existing end marker. The current RPT3/BIN pair matches on all
1,270 note-on events (channel, pitch, and frame).

- Loop start: BIN byte **4116**, frame **424**, **7.067 seconds**.
- End marker: byte **26888**, frame **4658**, **77.633 seconds**.
- Audition starts at frame **4358**, five seconds before the end marker.
- The final key-off packet at byte **26884** originally waited 140 frames
  (2.333 seconds). Its delay is now zero, so the loop happens immediately
  after the final note release. The exporter's subsequent whole-chip reset
  is replaced with end markers to avoid abruptly zeroing release envelopes.
  All note timing before that release is unchanged.

The BIN contains register changes, not complete instrument definitions at
each row. The player reconstructs the register state before the loop point
and restores it on each repeat. Key states replaced by the first batch of
loop packets are not restored first: doing so could briefly retrigger an
old note before the new phrase. Sound effect channels and shared chip
settings remain protected. Instrument changes can still alter release tails;
the audible transition needs a listening check.

## Measure alignment

The source MIDI declares constant 4/4 at approximately 136 BPM. This export
uses six rows per source quarter note (24 rows per measure), with tracker
tempo scaled to 204 BPM. A 32-row tracker pattern is therefore not a measure.

Sequence 03 row 00 is absolute row 96: the downbeat of measure 5, after a
four-measure intro. The MIDI ends after 176 quarter notes, at absolute row
1056 (sequence 21 hex / 33 decimal, row 00), just before measure 45.
The trimmed BIN ends on that same row's frame, 4658. The repeat contains
960 rows = 160 beats = 40 complete measures. Frame rounding is limited by
the 60 Hz playback clock.

The six-row grid represents triplets but cannot represent every sixteenth
note exactly: quarter-beat positions fall between rows. Some internal note
timing is therefore quantized; correct loop boundaries do not eliminate
that conversion limitation. The checker reports the maximum onset error
and checks the endpoints against the MIDI measure grid.

## Listen

Load `build/cc65/music-audition/azerfall.rp6502` on the RP6502 or drag it into
the emulator. After about five seconds, the ending jumps to sequence 03.
Restart the ROM to audition that transition again. Subsequent loops play
the entire repeating section.

The audition fast-forwards register state, so notes already sounding at its
starting point restart their envelopes. Judge the transition five seconds
later, not the first instant of the preview. Listening on hardware remains
the final check; compiling and matching packets do not verify audio quality.

For the full intro and loop, load `build/cc65/debug/azerfall.rp6502`.

## Rebuild

The audition uses a separate CMake build with `MUSIC_AUDITION=ON`; the option
defaults to OFF for normal builds. With the existing configured directories:

```powershell
cmake --build build/cc65/music-audition
cmake --build build/cc65/debug
```

After editing/re-exporting the song, run:

```powershell
python tools/check_music_loop.py
```

This checks the current effect-free RPT3/native-OPL2 format and reports the
new offsets. Update the constants beside `music_init` in `src/game/game.c`
if necessary. The checker rejects mismatched note streams and unsupported
formats/effects instead of guessing a loop location.
It also rejects trailing delay after the final music key-off. The RPT remains
unchanged, so a fresh tracker export may restore the 140-frame ending delay;
trim that delay, remove the ending register reset, and rerun the checker
before rebuilding.
