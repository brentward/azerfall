# sfxr.me to RP6502 PSG

Convert a serialized sfxr.me Base58 export using Python 3.10 or newer:

```powershell
python tools/sfxr_to_psg.py --input assets/sounds/example.sfxr.txt --output generated/sfx_example.h --name sfx_example --report build/sfx_example.json
```

Or pass the pasted string directly with `--code "7BM..."` instead of `--input`.
Use `--volume 0.7` for peak amplitude (0..1), and `--pan 0` (-63 left, 63 right).
The serialized export does not contain the website's volume setting.

The provided example is noise with rising pitch and vibrato: 15 frames, 120
bytes, approximately 0.25 seconds. The JSON report includes decoded parameters
and each output register frame.

Include the generated header in one C file, then play it:

```c
#include "../../generated/sfx_example.h"
#include "sound.h"

sound_play_effect(sfx_example, SFX_EXAMPLE_FRAMES);
```

The existing `sound_play_door_open()` wrapper plays this example. It has not
been attached to a game event; call it where you want to audition the sound.
The game already calls `sound_init(XRAM_SOUND_CONFIGS)` and `sound_update()`.
Call the latter once per 60 Hz tick. Calls return the allocated channel's
XRAM address, or 0xFFFF when unavailable. Up to eight effects can overlap.

Each frame contains frequency low/high, duty, attack volume/rate, decay
volume/rate, waveform/release rate, pan/gate, and reserved byte. Frequency is
Hz times three. The converter approximates the envelope using 16 attenuation
levels and the fastest hardware envelope rates; the runtime holds the gate
during playback and releases it for one tick afterward. Generated arrays live
in CPU-addressable program data, not in the song XRAM region. Only the current
eight bytes per active channel occupy PSG XRAM.

Pitch slide, delta slide, vibrato, pitch change, retrigger, square duty sweep
and envelope/punch are approximated at 60 Hz. Noise differs from sfxr's noise.
Filters and flanging are omitted and reported; the PSG has no equivalents.
Sub-frame changes are quantized, very short sounds occupy one frame, and
frequency above 21845 Hz is clipped. This is a sound-design starting point,
not waveform-identical synthesis. Audition and adjust volume on the target.

The original single-note `sound_play()` remains available. Its frequency
argument is Hz times three; duration and release are frame counts. Pass pan as
a signed -63..63 value (not pre-packed with PSG_PAN). Reserve enough release
ticks for the release-rate nibble you choose.

References:
- [sfxr serialization and synthesis](https://github.com/chr15m/jsfxr/blob/master/sfxr.js)
- [RP6502 PSG registers](https://picocomputer.github.io/ria.html#programmable-sound-generator)

Conversion tests: `python -m unittest discover -s tools -p test_sfxr_to_psg.py`.
Channel lifecycle tests use `tools/test_sound.c` and the minimal RIA stub in
`tools/test_support` with cc65's sim6502 target and sim65. They verify timing,
exhaustion, release and reuse, not the analog/audio result.
