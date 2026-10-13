# OnDanse
Custom built-in language for creating beats/melodies from the input logic. Inspired by Strudel REPL

See [docs/PLAN.md](docs/PLAN.md) for the architecture and roadmap.

## Building

Requirements: CMake 3.25+, Visual Studio 2026 (MSVC). JUCE and Catch2 are downloaded automatically.

From a "Developer PowerShell for VS" (or let VS Code's CMake Tools pick the `dev` preset):

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
```

The app is at `build/dev/Application/OnDanse_artefacts/Debug/OnDanse.exe`. The `dev` preset also writes `build/dev/compile_commands.json`, which VS Code uses for IntelliSense (see `.vscode/settings.json`).

To use the Visual Studio IDE instead, run `cmake --preset vs` and open the generated solution in `build/vs`.
