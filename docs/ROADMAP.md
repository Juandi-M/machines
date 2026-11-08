# Machines modernization roadmap

**Status:** Working draft. Stage boundaries are agreed; detailed implementation
sequencing and acceptance evidence are still being defined.

**Last updated:** 2026-11-08

**Purpose and constraints:** [Project brief](PROJECT_BRIEF.md)

**Technical context:** [Architecture](ARCHITECTURE.md)

## 1. Roadmap overview

| Stage | Goal | Completion milestone |
| --- | --- | --- |
| Ground zero: establish the existing game | First build/run the OpenGL source with the author's Linux-container workflow, then build/run it natively on Windows 11 without that container. | Reproduced baseline builds and runtime observations, with failures and necessary environment/build adjustments documented. |
| 1. Complete playable Vulkan port | Bring the whole game to Windows 11 through Vulkan using the changes needed to achieve functionality. | Every existing game feature, including multiplayer, is playable. Known bugs are acceptable. |
| 2. Faithful stabilization | Resolve migration regressions and verify the original experience. | The port behaves faithfully and reliably according to the agreed reference and acceptance criteria. |
| 3. Internal modernization | Modernize C++, libraries, and architecture as thoroughly as practical. | Modern, maintainable internals retain the experience established in stage two, including its assets and visual style. |

Vulkan belongs in stage one. Multiplayer belongs in stage one. Missing or
unplayable features cannot be deferred to stage two while declaring stage one
complete. Stage three permits substantial architectural work; it is not limited
to upgrading dependency versions.

No dates or effort estimates have been agreed. Dependencies and evidence of
readiness determine the sequence for now.

## 2. Starting point

Completed foundations:

- Obtain the published source and establish the project fork.
- Preserve the untouched source and working conversion in one root repository.
- Implement and test a standalone Vulkan presentation prototype.
- Translate the existing GUI, world, and billboard shaders to Vulkan GLSL and
  check their compilation and interfaces.
- Start the project brief, roadmap, architecture, and documentation structure.

Still required before we can demonstrate a playable port:

- Assemble the complete game assets and understand runtime/configuration needs.
- Identify the retail executable/assets and execution environment available for
  the fidelity comparison; the retail game's source code is not required.
- Build the full conversion, connect Vulkan to actual game rendering, and exercise
  the complete game, including multiplayer.

The existing clear-window prototype is supporting work within stage one. It is
not a substitute for ground zero or the stage-one milestone.

### Ground zero: baseline experiments before Vulkan conversion

1. Follow the 2020 source README and supplied Ubuntu 18.04 Dockerfile to
   cross-compile its OpenGL implementation into a Windows executable. Run the
   resulting executable natively on Windows 11 with the required assets and DLLs.
2. Establish a native Windows compiler/build setup and build/run the same OpenGL
   game without a Linux container. Separate native build-path changes from any
   game behavior changes.
3. Record exact tools, dependencies, runtime asset provenance, commands, failures,
   workarounds, and observed functionality. Preserve `original/`; any necessary
   source changes belong in `conversion/` or clearly identified experiment copies.

Docker/Ubuntu is an upstream build environment, not the target game platform.
The Windows executable must be tested on Windows 11. The absence of 1999 source
or a retail comparison setup does not block these 2020-source experiments. The
1999 experience remains the preservation goal for later fidelity work.

## 3. Stage one: the whole game playable on Windows 11 with Vulkan

### Intent

Perform the initial conversion and get the complete game functioning on the
target platform. Keep changes focused on making that possible; broad cleanup
and architecture modernization belong to stage three unless a change is necessary
for the port.

### Work areas

1. Inventory the actual game features and runtime requirements, including the
   campaign, skirmish, first-person control, multiplayer, menus, audio/video,
   input, persistence, and other existing functionality discovered in the source.
2. Obtain the assets and document the build and launch process. Establish a
   comparison against the 1999 retail game's observable behavior when reference
   files and an execution environment are available. Missing 1999 source code is
   not a blocker; the 2020 source is our agreed implementation starting point.
   Record unavailable reference evidence while progressing with the functional
   port, and do not claim verified retail fidelity without that comparison.
3. Make the full conversion compile with a recorded toolchain and dependency set.
   Apply compatibility changes needed for Windows 11 and Vulkan.
4. Connect Vulkan presentation to the game's real rendering path: textures,
   surfaces, buffers, shaders, state, world geometry, GUI, fonts, transparency,
   offscreen rendering, and readback as required by the existing game.
5. Bring up all game systems together and verify end-to-end play, including
   multiplayer matches. Record failures and migration discrepancies as they arise.

### Completion criteria

- The full game builds and runs on Windows 11 using the Vulkan rendering path.
- Every existing game feature is playable, explicitly including multiplayer.
- Known bugs are documented with enough information to reproduce and classify
  them; they do not make a feature unavailable or prevent playing through it.
- Reproduction instructions identify the assets, configuration, compiler,
  dependencies, and build/run procedure used.
- The feature inventory and evidence distinguish completed coverage from anything
  not yet exercised. The exact evidence matrix is an open planning decision.

A playable mission alone, an OpenGL-only full game, or a Vulkan menu with missing
gameplay does not satisfy this milestone. Thorough parity and regression cleanup
continue in stage two.

## 4. Stage two: faithful stabilization

### Intent

Turn the playable conversion into a reliable preservation of the original
experience. Fix migration-introduced problems while retaining inherited behavior,
including original bugs, by default.

### Work areas

- Classify discrepancies as bugs/quirks of the 1999 retail game, differences
  inherited from the 2020 source port, regressions introduced by our migration,
  or unresolved observations. The retail game's behavior is the authority.
- Resolve migration bugs in rendering, control, timing, simulation, audio,
  persistence, networking, resource lifecycle, and platform integration.
- Compare representative RTS and first-person scenes, units, effects, fog,
  transparency, GUI, fonts, and assets under matched conditions.
- Verify complete play flows, save/load, multiplayer behavior, and relevant
  window/display transitions. Agree on repeatability and endurance scenarios.
- Record the faithful version, test conditions, and remaining inherited issues as
  the behavioral reference for stage three.

### Completion criteria

- Migration regressions in the agreed acceptance set are resolved and verified.
- Original gameplay, graphics style, assets, and behavior are preserved against
  the chosen fidelity reference.
- Inherited bugs are documented rather than silently corrected.
- The agreed gameplay, graphics, persistence, and multiplayer checks pass with
  the defined fidelity tolerances and reliability expectations.
- Practical troubleshooting and the migration history describe the stabilized
  version and any explicitly unresolved observations.

The tolerance definitions, coverage matrix, and reliability thresholds are still
open. This draft does not invent a zero-crash duration, pixel-error threshold,
or a performance target that the user has not chosen.

## 5. Stage three: modern internals, original experience

### Intent

Modernize C++, libraries, and architecture together. Substantial restructuring is
allowed, provided the experience established in stage two remains intact.

### Work areas

- Select and record the C++ language standard, compilers, SDKs, library versions,
  and dependency lifecycle policy.
- Replace obsolete practices and improve ownership, resource management, error
  handling, interfaces, and understandability where justified by inspection.
- Restructure engine modules and platform/dependency boundaries to make the
  code easier to build, reason about, test, and maintain.
- Remove dependencies and compatibility code that are demonstrably obsolete.
- Check changes incrementally against stage two's behavioral reference; document
  architectural tradeoffs and the lessons from each migration.

### Completion criteria

- Agreed language, compiler, dependency, and architecture targets are met.
- The stage-two acceptance set still passes; the same gameplay, graphics style,
  and assets are retained, including intentionally preserved original quirks.
- Build, architecture, dependency, troubleshooting, and maintenance guides match
  the finished implementation.
- The migration history explains the changes, reasons, alternatives, validation,
  and lessons, with links to the relevant source revisions when available.

"As modern as possible" needs concrete targets before implementation can treat
this stage as decision complete. Those targets are part of the continuing Q&A.

## 6. Documentation and decisions throughout the stages

Practical guides and the engineering record are ongoing deliverables, not a task
reserved for the end. See [guides](guides/README.md) and the
[migration record](migration/README.md).

Record what changed, why, how it was checked, what was learned, and what remains
uncertain. Record inherited bugs separately from migration regressions. Changes
to the agreed stage boundaries or preservation policy must be explicit decisions.

For unresolved planning questions, use the [project brief's question list](PROJECT_BRIEF.md#9-success-criteria-and-open-decisions).
