# Machines migration workspace

This entire project is one Git repository, rooted in this directory. It contains
the untouched reference source, the conversion source, and project documentation.
Run repository-wide Git commands here; run builds against the appropriate source
folder.

```text
machines-game-experiment/
  .git/                     Git history for the whole project
  original/                 Untouched upstream source, pinned baseline
  conversion/               Working fork, Vulkan migration
  docs/                     Shared brief, roadmap, architecture, guides, history
  README.md                 Project introduction and documentation index
  workspace.json            Checkout locations and source baseline
```

| Source folder | Baseline | Purpose |
| --- | --- | --- |
| `original` | Unmodified snapshot of `2c857564ac9145fa8913ed57e533a5069faf7662` | Source and behavior reference |
| `conversion` | Same baseline, with migration changes | Vulkan conversion and later modernization |

The project branch is `vulkan-port`. The root repository's `origin` is
https://github.com/Juandi-M/machines and `upstream` is
https://github.com/NightDive-Studio/machines.

The released baseline is a 2020 OpenGL port of the 1999 game. Here, "original"
means this unmodified released source, not the untouched 1999 DirectX code.
Neither source folder contains a nested Git repository or a submodule. Git at the
project root tracks both source trees and the documentation. The previous
reference clone's Git metadata is preserved locally in
`.git/layout-backup/original.git`; it is not part of the source tree or a file
to publish. Root Git LFS storage and upstream history have been preserved.

## Documentation

- [Project brief](docs/PROJECT_BRIEF.md): introduction, problem statement, vision,
  agreed goals, scope, preservation policy, current state, and open decisions.
- [Roadmap](docs/ROADMAP.md): the three agreed stages, work areas, and completion
  criteria, with unresolved acceptance details marked explicitly.
- [Architecture](docs/ARCHITECTURE.md): observed code organization, current Vulkan
  prototype, constraints, and proposed migration boundaries.
- [Practical guides](docs/guides/README.md): build, run, test, and maintenance
  procedures, starting with the standalone Vulkan bootstrap.
- [Migration record](docs/migration/README.md): decisions, implementation history,
  original bugs, changes, validation, and lessons.

These are working drafts that distinguish agreed decisions, observed facts, and
open questions. The ongoing discussion will refine them.

## Source preservation

- Keep `original` unchanged at its recorded source baseline. Build it out of tree and run it from a
  separate runtime directory so generated files, settings, and saves do not
  alter the reference checkout.
- Make all code, dependency, build-system, and renderer changes in `conversion`.
  Preserve the upstream directory structure so corresponding files remain easy
  to compare. Use Git history to record migrations rather than copying changed
  source back into the reference folder.
- Keep the same gameplay, simulation timing, AI, assets, and rendering intent.
  Treat an intentional behavior change as a separate decision.
- Compare both executables with the same assets, scenario, settings, and
  hardware. Give each run separate writable settings/saves/logs.
- Validate one change at a time: toolchain, dependency, platform boundary, then
  rendering capability. Do not combine a renderer port with a game-logic rewrite.

The reference source is protected by this workflow, not by filesystem ACLs.
Review changes to `original/` separately; ordinary migration work belongs in
`conversion/`. Every reference file was verified byte-for-byte during the
repository layout correction. Once this layout is committed, use
`git diff HEAD -- original/` to check for reference changes.

## Architecture direction

The existing engine remains the application. Its game-facing rendering classes
are the starting point for the boundary that will isolate the graphics API:

```text
Game logic and simulation (machlog, sim)
              |
Scene, entities, and animation (world4d, machphys)
              |
Existing rendering contracts (RenDevice, RenSurface, RenTexture, RenDisplay)
              |
Graphics backend implementation
       +------+------+
       |             |
  OpenGL reference   Vulkan implementation
```

This is the target boundary, not a description of a completed abstraction.
The existing render headers still expose OpenGL types, and GL calls also appear
outside `device.cpp`. The new Vulkan context is a presentation foundation;
it is not yet plugged into the game's render contracts.

The two complete source trees provide the reference and conversion executables.
Inside `conversion`, retaining an OpenGL path temporarily can also help compare
the same refactored code through both backends. The migration's end state is
Vulkan; reference support is useful only while bringing that implementation up.

## Agreed stages

**Ground zero:** build and run the existing OpenGL game through the upstream
Linux/Docker cross-build procedure, then establish its native Windows 11 build
without that container. Both experiments precede further Vulkan integration.

1. **Complete playable Vulkan port on Windows 11.** Every existing game feature,
   including multiplayer, must be playable. Known bugs are acceptable; missing
   or unplayable features do not meet this milestone.
2. **Faithful stabilization.** Fix migration regressions and verify the original
   experience. Preserve inherited bugs and quirks by default, documenting them
   for separate decisions later.
3. **Modern internals, original experience.** Modernize C++, dependencies, and
   architecture as much as practical while retaining the same gameplay, graphics
   style, and assets.

Both practical build/maintenance guides and a detailed migration record are
project deliverables. The project brief, separate roadmap, and architecture drafts
are in the shared `docs/` directory and will develop through the planning discussion.
The **1999 retail game is the authority for fidelity**. The published 2020 source
is our implementation starting point; access to the 1999 source code is not
required. Reference-game execution and comparison procedures still need setup.

See the [Vulkan bootstrap guide](docs/guides/VULKAN_BOOTSTRAP.md) for the
implemented presentation prototype, its verification, and the remaining work.

## Current executable and build directories

The tested presentation executable moved intact to:

```powershell
.\conversion\build\vulkan-windows\machines-vulkan-smoke.exe --frames 120 --exercise-window
```

Existing build caches were retained during relocation and may contain absolute
paths from the former checkout location. Create a fresh build directory rather
than reusing those caches. New native builds should start from `conversion`:

```powershell
Set-Location conversion
cmake -S . -B build/vulkan-native -DMACHINES_VULKAN_BOOTSTRAP=ON
cmake --build build/vulkan-native --config Release
```

The compiler and shader-tool prerequisites are documented in the port guide.
For container builds, bind `conversion` itself to `/work`. A container created
before restructuring has the old parent mount; create a new container with the
correct source mount before building again.

For a reference build, use a directory outside `original`, for example
`cmake -S original -B build/original`, with the original toolchain/dependencies
configured appropriately. This example specifies the directory layout; it is
not a claim that the legacy game currently builds with the host toolchain.
