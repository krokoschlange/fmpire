# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project overview

FMpire is a wavetable/FM synthesizer written in C++23, built on [DPF](https://github.com/DISTRHO/DPF) (DISTRHO Plugin Framework, vendored as a git submodule at `DPF/`). It builds as a standalone JACK app and as CLAP, LV2, and VST3 plugins from a single codebase. It is a rewrite of https://github.com/krokoschlange/cr42ynth.

## Build

DPF is a required git submodule; ensure it's checked out (`git submodule update --init --recursive`) before configuring. Local modifications to DPF live as patches in `patches/dpf-*.patch` (paths relative to the DPF root, e.g. `git -C DPF diff > patches/dpf-<name>.patch`); CMake applies them to the submodule at configure time and skips any that are already applied.

```sh
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

The user always builds the project manually. Never run build commands (e.g. `cmake --build`) yourself; leave building and running the result to the user.

This produces, under `build/bin/`:
- `fmpire` — standalone JACK app
- `fmpire.lv2` — LV2 plugin bundle
- `fmpire.vst3` — VST3 plugin bundle
- (CLAP is also built as a `TARGETS` entry in `CMakeLists.txt`)

There are no automated tests in this repository.

## Architecture

The plugin is split into three independently-compiled DPF targets (`FILES_DSP`, `FILES_UI`, shared code) defined in `CMakeLists.txt`, all rooted under `src/`:

- `src/dsp/` — the audio engine (`Plugin` subclass), compiled into the DSP-side binary/plugin.
- `src/ui/` — the editor GUI (`UI` subclass), compiled separately and rendered with DPF's cairo backend. The DSP and UI sides run in different processes/threads in most plugin formats and never share memory directly — they only communicate through DPF's parameter/state API.
- `src/common/` — code shared by both sides (wavetable/waveform data structures, math utils, base32 encoding). It is compiled into *both* the DSP and UI targets (see `FILES_DSP`/`FILES_UI` in `CMakeLists.txt`), so changes here affect both binaries and must not depend on DSP- or UI-only headers.
- `src/DistrhoPluginInfo.h` — DPF plugin metadata (name, IDs, port counts, format-specific categories). This is the canonical place for plugin identity/capability flags.

### DSP signal path (`src/dsp/`)

- `FMpire` (`fmpire.h/.cpp`) is the DPF `Plugin` entry point. It owns a fixed-size pool of `FMPIRE_VOICE_COUNT` `Voice` objects (voice-stealing via `free_voice_queue`) and `FMPIRE_OSC_COUNT` `Oscillator` slots shared by all voices, plus a dynamic list of `Modulator`s. It handles raw MIDI events (note on/off, aftertouch, pitch wheel, CCs) and drives audio rendering in `run()`.
- `Voice` (`voice.h/.cpp`) represents one polyphonic voice. It holds one `OscillatorVoice` per oscillator slot (per-voice phase/unison state referencing the shared `Oscillator` config) and a `ModulatorVoice` per active modulator (per-voice envelope/LFO phase). `Voice::VoiceEndedCallback` (implemented by `FMpire`) is used to return a finished voice to the free pool.
- `Oscillator` — shared, per-slot oscillator configuration (wavetable reference, volume, detune, unison settings, etc.); the corresponding `OscillatorVoice` is the per-voice runtime state that reads it.
- Cross modulation (AM/FM/PM/RM between oscillators, as in cr42ynth): each carrier oscillator has a depth per type and modulator oscillator (`OscillatorParams::depth[type][modulator]`, 0..1, state keys `osc/<carrier>/depth/<type>/<mod>` and a `MATRIX` section in the full state). Each sample `Voice::run` runs the oscillator voices in index order; `OscillatorVoice::calculate_cross_modulation` combines the other oscillators' `get_osc_value()` (unison reference voice, before volume, including the modulation it received itself; lower indices from this sample, higher ones from the previous sample, so feedback works) into a gain (AM, RM), a frequency factor (FM) and a phase offset (PM). Only matrix cells with a base depth or a route are evaluated (`begin_block`). Every depth is also a modulation target (`TargetType::OSC_AM/FM/PM/RM`, `target_object = matrix_target_object(carrier, modulator)`), so LFOs/envelopes/macros can sweep them; use the `is_oscillator_target` / `is_matrix_target` / `is_modulator_target` helpers instead of comparing target values.
- Modulation (`modulator.h/.cpp`, `mod_source.h`, `Curve` in `src/common/curve.h`): `Modulator` is a user-drawn LFO or envelope (a `Curve` baked into a fixed table); `ModulatorVoice` (in `voice.h`) is its per-voice playback state. Everything that can drive a parameter is a `SourceId` (`MODULATOR`, host `MACRO`, `MIDI_CC`, pitch bend, pressure, velocity, key) so LFO/ENV and external automation share one mechanism. A `ModRoute` connects a source to a `TargetType` parameter with an adjustable bipolar `amount`; the target's own value stays the base and routes are added on top (`Voice::apply_routes`), so several routes can stack on one parameter. Modulators and routes live in growable vectors inside the `Patch` (see below); there is no fixed limit besides the 16-bit id space (`FMPIRE_ID_SPACE`). Modulator-to-modulator routes (`MOD_AMOUNT`, `MOD_FREQ`) use the previous sample's offsets, which makes cycles harmless. Live modulation display: besides the macros (parameters `0..FMPIRE_MACRO_COUNT-1`) the plugin has `FMPIRE_METER_COUNT` hidden *output* parameters, one per route slot; at the end of `run()` `FMpire::publish_meters` writes into each the offset (`Voice::get_modulation_offset`, from the newest active voice, 0 without voice) that the route's target received in the last sample. `FMpireUI::parameterChanged` forwards them to `ModulationModel::set_route_meter`, and each `Knob` draws its target's meter as a thick arc from the knob value on the modulation ring. Routes in slots >= `FMPIRE_METER_COUNT` work but have no live display. The same mechanism carries a playhead per modulator id (`FMPIRE_PLAYHEAD_COUNT` parameters after the route meters, -1 = not playing; `Voice::get_playhead`, `ModulationModel::get_playhead`), drawn as a vertical line by the `CurveEditor`. The `WavetableView` highlights the *playing* wavetable position (knob value + `ModulationModel::get_target_meter(OSC_WT_POS, osc)`), not just the knob value.
- Threading of plugin state: DPF calls `setState` from non-realtime threads (UI, worker, host) with no synchronization against `run()`, so the audio thread only ever *reads* a `Patch` (`patch.h`: the oscillators with their wavetables, the modulators and the routes; the note/modulation playback state stays in the voices). `FMpire::setState` (under `state_mutex`) parses the change into the state-side copy (`OscillatorState`, `modulator_shadows`, `route_shadows`, which is also what `getState` serializes), then rebuilds a complete `Patch` from it and publishes it through a lock-free `TripleBuffer` (`triple_buffer.h`). `run()` picks up the newest patch at the start of a block (`Voice::set_patch`); if `run()` isn't being called, publishing just replaces the unread patch, so nothing accumulates. Wavetables are shared between patches as `shared_ptr<const Wavetable>` and only released by the state side. Voices follow the patch on their own: knob values are re-read every block and a modulator whose `generation` changed (created/replaced) restarts its playback state. Never touch audio-side objects (`Patch`, voices) from `setState`/`getState`; the one allocation the audio thread may do is growing a voice's per-modulator state in `Voice::set_patch` when a patch has more modulators than ever before.
- `fx_chain.h/.cpp` — post-voice effects processing.

### Wavetable data model (`src/common/`)

- `Waveform` (single cycle) → `WaveformPart` (segments/breakpoints making up a waveform, likely spline/harmonic-based — check `waveform_part.h` before modifying) → `Wavetable` (a 2D table of `width` × `height` samples, sampled by `(position, phase)` with optional interpolation across both axes) → `WavetableCreator` builds a `Wavetable` from a sequence of `Waveform`s. This pipeline is shared code, used by the UI to author wavetables and by the DSP to play them back.

A `Waveform` can also be *interpolated* (`Waveform::is_interpolated`, `InterpolationType`, created by the WT editor's Morph bulk op): it is derived data, serialized as just `INTERP<type>`, and `WavetableCreator::update_interpolated()` (run at the end of `update()`, on the DSP and UI side alike) regenerates it from the nearest normal waveform before and after its run of consecutive interpolated waveforms, so editing an anchor updates the intermediates. The generated content is stored in a normal single part, so "making it editable" is just `set_interpolation(NONE)`. A run without a normal waveform on one side would be undefined, so when removing/moving waveforms the WT editor first calls `WavetableCreator::bake_orphaned_interpolated()` (turns the waveform at the table edge into a normal one, keeping its content, so it becomes the new anchor) and then sends the whole wavetable (`KEY_WT_ALL`) to the DSP instead of a single remove/insert. The UI shows these read-only (hatched rows in `WaveformSelector`, banner + "Make editable" button in `WaveformEditor`).

### UI (`src/ui/`)

- `FMpireUI` (`fmpire_ui.h/.cpp`) is the DPF `UI` entry point and top-level widget tree owner (`FMpireWindow`). It receives `parameterChanged`/`stateChanged` callbacks from DPF and owns a `StateManager`.
- `StateManager` (`state_manager.h/.cpp`) is the hub that keeps UI widgets (per-oscillator `OscillatorSettings`, the `WavetableEditor`) in sync with DSP-side state changes and pushes edits back out via DPF's `setState`.
- `src/ui/widgets/` — a small custom immediate-ish retained widget toolkit built on DPF's `SubWidget`/cairo primitives: `FMpireWidget` is the common base (gives access to `Theme` and a shared tooltip mechanism via `FMpireWindow`), with layout containers (`GridContainer`, `RelativeContainer`, `AspectRatioContainer`), controls (`Knob`, `Button`, `ImageButton`, `Selector`, `IntEditor`, `Arc`), and synth-specific views (`OscillatorBar`, `OscillatorSettings`, `WaveformEditor`, `WavetableEditor`, `WavetableView`, `WaveformSelector`). New widgets should derive from `FMpireWidget` to get theme/tooltip access for free.
- Modulation UI: `ModulationModel` (`src/ui/modulation_model.*`, owned by `StateManager`) mirrors the DSP's modulators/routes and is the only thing widgets edit; it sends the `mod/<id>/...` and `route/<slot>/...` state keys itself. The OSC tab is an `OscillatorPage` (oscillator panels + `ModulatorEditor` = `ModSourceList` | `CurveEditor` | `ModulatorSettingsPanel`); the MOD/FX tabs each get a `SourceListPage` with the same shared source list; the MOD tab's body is the `ModulationMatrix` (AM/FM/PM/RM selector + an 8x8 knob grid, one knob per modulator/carrier pair, values kept in `ModulationModel::get_matrix_depth`). Arming a source in any list (the "M" toggle) puts every knob registered with `Knob::set_mod_target(...)` into programming mode: dragging edits that source's route amount instead of the knob value, right-click flips bipolar, double-click removes the route.
- `theme.h/.cpp` centralizes colors/styling; `draw_operations.h/.cpp` holds shared cairo drawing helpers.

### DSP↔UI state sync

DPF plugin state is exchanged as string key/value pairs (`Plugin::setState`/`UI::stateChanged`). This project funnels effectively all plugin state through a single DPF state key (`KEY_EVERYTHING`, see `FMpire::initState`) whose value is a serialized blob covering all oscillators/modulators, then splits it internally using the `KEY_*` prefix constants in `src/common/defines.h` (e.g. `KEY_OSC_PREFIX "osc/"`, `KEY_MOD_PREFIX "mod/"`, plus per-parameter keys like `KEY_OSC_VOLUME`, `KEY_OSC_WT_POS`). When adding a new persisted parameter, add a `KEY_*` constant in `defines.h` and wire its (de)serialization on both the `Voice`/`Oscillator`/`Modulator` `set_state()` side and the UI `StateManager`/widget side — the two must agree on the key scheme since they only communicate through these strings, not shared memory.

Tunable engine limits (`FMPIRE_OSC_COUNT`, `FMPIRE_VOICE_COUNT`, `FMPIRE_MAX_UNISON_AMOUNT`) also live in `src/common/defines.h`.

## Code style

Formatting is enforced via `.clang-format` (based on Microsoft style, tabs, 4-space access-modifier offset). Run `clang-format` on touched files before committing. Namespacing: all project code lives in `namespace fmpire`; DPF's namespace macros (`USE_NAMESPACE_DISTRHO`, `START_NAMESPACE_DISTRHO`) are used where DPF base classes/types are referenced directly.
