# Machines: faithful modernization

**Status:** Working draft; agreed decisions are recorded below and unresolved
questions remain open.

**Last updated:** 2026-11-08

**Related documents:** [Roadmap](ROADMAP.md) · [Architecture](ARCHITECTURE.md)

## 1. Introduction

Machines is a 1999 real-time strategy game whose combination of army command and
direct first-person control is worth preserving. This project is a personal
software engineering and preservation effort: bring the complete game onto
Windows 11 with Vulkan, establish a faithful implementation of its original
experience, and then modernize the software behind it as thoroughly as practical.

The intended result is **modern internals with the original experience**. The
game's identity, gameplay, graphics style, and assets are the constraints within
which the technology evolves. The engineering journey also matters: understanding
legacy code, migrating runtimes and dependencies, explaining architectural
decisions, and recording what the work teaches us.

## 2. Problem statement

The published source provides a starting point, but a source release alone does
not give this project a verified, complete, reproducibly buildable game on its
target platform. The code includes an older OpenGL rendering implementation,
legacy C++ conventions, aging build assumptions, and multiple platform and
library dependencies. Modernization can introduce subtle changes to graphics,
simulation, controls, persistence, and multiplayer behavior.

We need a disciplined way to make the complete game playable on Windows 11
through Vulkan, distinguish migration regressions from inherited behavior, and
replace outdated internals without losing the experience we are preserving.
We also need practical instructions and an engineering history so the result can
be understood, reproduced, maintained, and evaluated later.

This is the project's verified gap, not a claim that every existing Machines
release fails on Windows 11. The OpenGL source has now compiled and linked after
build-environment and ALURE dependency adjustments; the full game has not yet
been launched or validated in this workspace.

## 3. Vision and intended audience

The project initially serves us: a personal preservation project and an opportunity
to do substantial legacy software modernization. Public distribution, community
support, and contributions may be considered later; they are not prerequisites
for the first successful version.

Success includes both a usable game and understandable software. Someone
returning to the project should be able to determine how to build it, what changed,
why each change was necessary, how it was checked, and what remains uncertain.

## 4. Objectives

Before the three migration stages, establish **ground zero**: build and run the
existing OpenGL game using the upstream Linux/Docker procedure, then build and
run it natively on Windows 11 without the Linux container. Record what actually
works and what needs adjustment before continuing the Vulkan migration.

1. Make the **whole game playable on Windows 11 with Vulkan in stage one**,
   including multiplayer and every existing game feature. Known bugs may remain.
2. Stabilize the port and establish fidelity in stage two, correcting problems
   introduced by the migration.
3. Modernize C++, libraries, and architecture in stage three, including substantial
   internal restructuring where appropriate, while retaining the same experience.
4. Preserve the original graphics style and assets throughout the project.
5. Produce practical build and maintenance documentation and a detailed record
   of changes, reasons, validation, and lessons throughout all three stages.

## 5. Preservation and bug policy

**Preserve original behavior by default, including original bugs and quirks.**

- Fix regressions introduced by the migration.
- Document inherited bugs so they can be considered separately later.
- Do not silently fold fixes for original bugs into compatibility work or cleanup.
- When a discrepancy's origin is unknown, record the uncertainty and investigate
  against the reference rather than assuming either version is correct.
- Intentional changes to inherited behavior require a separate decision and a
  record of their effect on the preservation goal.

Here, "original behavior" means the **1999 retail game**. Behavior introduced by
the later OpenGL source port is not automatically part of the preservation
contract. Track differences already present in the 2020 starting point separately
from retail quirks and regressions introduced by our own migration.

Stage one permits known migration bugs while the whole game is playable. Stage
two is where those regressions are resolved and fidelity is established. Stage
three must preserve the fidelity established in stage two.

## 6. Scope and boundaries

| Area | Agreed direction |
| --- | --- |
| Target platform | Windows 11; Windows 7 was a dictation error. |
| Graphics direction | Vulkan in stage one, not deferred to a later modernization stage. |
| Stage-one coverage | The whole game, explicitly including multiplayer. A window, menu, or single mission is insufficient. |
| Internal modernization | A combination of newer C++, updated libraries, safer/clearer code, and architectural restructuring. |
| External experience | Preserve gameplay, behavior, graphics style, and assets. |
| Original defects | Preserve by default; document and decide separately later. |
| Initial audience | Us initially. |
| Documentation | Practical guides plus a detailed migration history. |
| Repository layout | One root repository containing `original/`, `conversion/`, and shared `docs/`. |

New gameplay, changed balance, replacement artwork, a new visual identity, and
extra engine capabilities are not part of the agreed three-stage goal. Shipping
other platforms and organizing a public release have not been requested. Linux
tooling used for builds or testing does not establish Linux as a product target.

The desired Vulkan direction is current technology. Exact SDK/API versions,
minimum hardware, and version-pinning policy still need agreement; the existing
prototype's Vulkan 1.1 baseline is an implementation fact, not a finalized product
requirement.

## 7. Repository and source provenance

The entire `machines-game-experiment` directory is one Git repository. Both source
folders are ordinary tracked directories, not nested repositories or submodules.

- **Source provider:** [NightDive-Studio/machines](https://github.com/NightDive-Studio/machines).
- **Source revision:** `2c857564ac9145fa8913ed57e533a5069faf7662` (2020-10-19).
- **Project fork:** [Juandi-M/machines](https://github.com/Juandi-M/machines).
- **Reference source:** `original/`, preserved unchanged at that revision.
- **Working source:** `conversion/`, initially the same structure and source,
  with migration changes applied there.
- **Shared project documentation:** `docs/`, indexed from the root README.

The published source is already an OpenGL port derived from the 1999 game. Calling
the folder `original` identifies our untouched source snapshot; it does not mean
we possess untouched 1999 engine source.

**Fidelity reference: the 1999 retail game. Implementation starting point: the
published 2020 source.** The user does not have the 1999 source code, and obtaining
it is not a requirement for beginning or carrying out this migration. Fidelity can
be evaluated against the retail game's observable behavior. Establishing access
to a retail executable, its assets, and a suitable reference environment remains
practical work for the comparison process; availability has not yet been confirmed.
If reference evidence is unavailable for a behavior, record that validation gap
rather than treating the 2020 implementation as proof of retail fidelity.

Keep the source license and attribution intact. The available source does not
include the complete playable asset set; asset sourcing and any future packaging
need their own documented treatment.

## 8. Current state

| Item | Current evidence |
| --- | --- |
| Source/reference layout | Present in the single root repository; original source verified against the upstream Git tree. |
| Vulkan foundation | Standalone SDL/Vulkan presentation context and a smoke executable exist. |
| Shader work | Five translated shaders compile to SPIR-V; shader interfaces have been checked. |
| Prototype execution | Previously presented 120 frames on Windows 11 / RTX 4080 SUPER and passed isolated software-driver validation checks. |
| Full game | The reference OpenGL source compiled, linked with repaired dependencies, and displayed its loading screen and opening movie on Windows 11. It was stopped at the user's request due to active runners; further launches require explicit go-ahead. Gameplay and multiplayer remain unverified. The full conversion/Vulkan game is not built. |
| Playable assets | Community assets and source metadata supported the initial launch. A music decoder DLL filename mismatch was repaired; all ten Ogg music tracks passed headless decoding. Audible in-game playback remains unverified. |
| Stage one | In progress at the prototype level; its completion milestone has not been reached. |

The prototype's successful checks demonstrate its own behavior. They do not
demonstrate game rendering, complete game functionality, multiplayer, or fidelity.
Detailed historical results are in the [Vulkan bootstrap guide](guides/VULKAN_BOOTSTRAP.md).

## 9. Success criteria and open decisions

The [roadmap](ROADMAP.md) defines the three agreed completion milestones. Across
them, success requires the original source to remain preserved, changes to be
traceable, and documentation to reflect what has actually been demonstrated.

Q-01 is resolved: the 1999 retail game defines fidelity; the 2020 source is the
accepted implementation starting point. The 1999 source code is not required.

The following decisions remain open:

| ID | Question |
| --- | --- |
| Q-02 | What are the minimum GPU/driver requirements and the exact Vulkan/toolchain version policy? |
| Q-03 | What complete feature inventory and playthrough/multiplayer evidence will demonstrate stage-one completion? |
| Q-04 | What tolerances and scenarios define graphics, timing, audio, input, save/load, and multiplayer fidelity? |
| Q-05 | What concrete C++/dependency targets and architectural outcomes make stage three complete? |
| Q-06 | What practical limits, schedule expectations, and possible future distribution goals should shape the roadmap? |

These are questions for the continuing discussion, not implicit defaults or
implementation decisions already made.
