# OnDanse implementation plan

OnDanse is a desktop live-coding app for music. Users write `.alors` scripts that describe beats and melodies; the app compiles them and plays them in real time. Inspired by [Strudel](https://strudel.cc/) and [TidalCycles](https://tidalcycles.org/).

## Stack

**C++20 + [JUCE](https://juce.com/) 9**, built with CMake (JUCE and Catch2 fetched via `FetchContent`).

| Need | JUCE piece |
|---|---|
| Low-latency audio (WASAPI/ASIO, CoreAudio) | `AudioDeviceManager` |
| Sample decoding | `AudioFormatManager` |
| Code editor with syntax highlighting | `CodeEditorComponent` + custom `CodeTokeniser` |
| Resizable 3-panel layout | `StretchableLayoutManager` |

Alternative considered: Tauri/Electron UI (Monaco editor) + native audio engine. Richer editor for free, at the cost of two languages and an IPC bridge. Revisit if editor features become the bottleneck.

Licensing: JUCE 9 is AGPLv3 or commercial (free Starter tier under a revenue cap). Check before distributing.

## Key design decisions

### Compile to a pattern IR, not machine code

As in Strudel/Tidal, a pattern is a function you query: *"which events occur between cycle t₀ and t₁?"*. The compiler lowers `.alors` source into a tree of pattern operations (the IR); a scheduler queries it ahead of the playhead. This gives millisecond recompiles, hot-swap at the next cycle boundary without stopping audio, and no native code generation.

Time is exact: cycle positions are rationals (`alors::Fraction`), so subdivisions like triplets never drift.

### The IR lives in `Engine/Pattern`

The IR is the contract between compiler and engine. It is a pure C++ library (`alors_pattern`, no JUCE) so it can be unit-tested headless and linked by the compiler without pulling in audio code.

Current IR nodes: `Silence`, `Pure`, `Sequence`, `Alternate`, `Stack`, `Fast`, `Shift`, `Merge`. `euclid`, `slow` and `early` are built from these. Event values are a `ControlMap` (`s`, `n`, `gain`, `pan`, `speed`, ...) so the syntax can add controls without changing the engine.

### Lint/format reuse the compiler front end

`Interpreter/` (linter, formatter, highlight classes) consumes the compiler's lexer and parser, never its own. The lexer keeps comments and whitespace as trivia so the formatter can round-trip source.

### Threading

| Thread | Work | Rules |
|---|---|---|
| UI (message) | Editing, panels | — |
| Compile | Background compile on edit / Ctrl+Enter | Never blocks the UI |
| Scheduler | Queries the pattern ~150 ms ahead, converts cycles to sample positions, pushes events | Owns the pattern; may allocate |
| Audio | Pops events, triggers voices sample-accurately, mixes | No allocation, no locks; talks to others only via lock-free SPSC queues and atomics |

Hot-swap: `Scheduler::play()` while playing stores a pending pattern; the scheduler switches at the next cycle boundary inside its query window.

## Architecture

```
 .alors text
     │
     ▼
 Compiler: Lexer → Parser → AST → Sema → Lowering ──────────► Pattern IR (Engine/Pattern)
     │               │          (resolves sound names                │
     │               └──► Interpreter: Linter, Formatter,             ▼
     ▼                    highlight classes                   Scheduler (lookahead thread)
 Diagnostics (source spans) ──► editor squiggles                     │ SPSC queue
                                                                     ▼
                                                          AudioEngine (audio thread)
                                                          SamplerVoices → master → device
```

Dependencies point one way: `Frontend → Core → {Compiler, Interpreter, Engine}`, `Interpreter → Compiler → Engine/Pattern`. Nothing below `Frontend` uses JUCE GUI classes.

## Folder structure

```
OnDanse/
├── CMakeLists.txt            root build; alors_juce shared JUCE module library
├── CMakePresets.json         dev (Ninja, compile_commands.json) and vs presets
├── .vscode/                  IntelliSense wired to CMake Tools / compile_commands.json
├── Compiler/                 (M4) lexer, parser, AST, sema, lowering to IR, diagnostics
├── Interpreter/              (M7) linter + rules, formatter, highlight classes
├── Engine/
│   ├── Pattern/              alors_pattern: Fraction, TimeSpan, Event, Pattern IR + query
│   ├── Audio/                alors_engine: AudioEngine, Scheduler, SamplerVoices,
│   │                         SampleBank, BuiltInKit, SpscQueue
│   └── tests/                pattern tests, real-time scheduling tests
├── Application/
│   ├── Core/                 alors_app_core: PlaybackController, Workspace, DemoPatterns
│   └── Frontend/             JUCE app: MainComponent, EditorPanel, FileBrowserPanel,
│                             LibraryPanel, Theme, Shortcuts
└── docs/PLAN.md
```

## UI

- **Left: Explorer.** `.alors` files under the workspace folder (default `Documents/OnDanse/Sketches`, changeable with *Open...*).
- **Middle: Editor.** Dark VS Code-style `CodeEditorComponent`. Toolbar (top right): cycle readout, BPM, **Compile / Play / Stop**. Status strip at the bottom.
- **Right: Sounds.** Built-in kit plus any samples in `Documents/OnDanse/Library` (top-level files by name; subfolders become one sound with numbered variants, e.g. `Library/bd/*.wav` → `bd:0`, `bd:1`...). Click to preview.

Shortcuts: **Ctrl+Enter** play/evaluate, **Ctrl+.** stop, **Ctrl+S** save.

Tempo: BPM with 4 beats per cycle (120 BPM = 0.5 cycles/second, Strudel's default).

## Milestones

| # | Milestone | Status |
|---|---|---|
| 1 | Scaffold: CMake + JUCE, 3-panel resizable window, CI | ✅ Done |
| 2 | Audio spike: device output, click-to-preview sounds | ✅ Done |
| 3 | Engine core: pattern IR + scheduler playing a hard-coded pattern, hot-swap | ✅ Done |
| 4 | Language v0: lexer, parser, AST, diagnostics, lowering to IR, all unit-tested | ⏳ Needs syntax spec |
| 5 | End-to-end: background compile, Ctrl+Enter hot-swap, error squiggles, highlighting | |
| 6 | Workspace & Library: new/rename/delete files, file watching, drag sound into editor, library rescan | |
| 7 | Tooling: linter rules, formatter | |
| 8 | Stretch: synths & effects, pattern visualiser, MIDI out, render to WAV | |

### What M1–M3 delivered

- `alors_pattern` with exact rational time, Strudel-compatible `sequence`/`alternate` (fastcat/slowcat) semantics and Bjorklund euclidean rhythms. 15 test cases.
- `alors_engine`: lock-free event queue, 64-voice sampler with voice stealing and linear-interpolated resampling, equal-power pan, tanh soft-clip master, procedurally synthesised built-in kit (`bd sd hh oh cp lt ht rim`), user sample folder loading.
- Scheduler with 150 ms lookahead, sample-accurate placement, tempo changes without jumps, hot-swap on cycle boundaries. Real-time tests verify exact sample spacing, true silence on stop, and swap timing.
- Play currently runs `demoBeat()` (`Application/Core/DemoPatterns.cpp`) because there is no compiler yet; Compile is disabled.

### Known limitations to address later

- Sample bank is fixed at startup (rescan in M6 needs a safe bank swap to the audio thread).
- Switching files auto-saves the current one; no unsaved-changes prompt yet.
- Unknown sound names are silently skipped by the scheduler; M4/M5 should surface them as diagnostics at compile time.
- Device sample-rate changes mid-playback are not re-anchored.

## Open questions (block M4)

1. **Syntax.** Mini-notation strings (`"bd sd [hh hh]"`), statement/block based, or both? How are layers/tracks written? Cycles or bars as the time unit?
2. **Execution model.** Confirm Strudel-style live hot-swap (current engine) vs. compile-then-play-from-start.
3. **Platforms.** Windows only, or macOS too?
4. **Sound sources.** When do synths (oscillators, envelopes) need to arrive relative to samples?
