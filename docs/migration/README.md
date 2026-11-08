# Migration record

This section records what changed, why it changed, how it was checked, and what
we learned. It complements the [practical guides](../guides/README.md).

## Agreed decisions

| ID | Decision | Reason or consequence |
| --- | --- | --- |
| D-001 | One root Git repository owns reference source, conversion source, and docs. | The repository represents the whole project; source folders are not independent repositories. |
| D-002 | Preserve `original/` and make implementation changes in `conversion/`. | Retain a traceable source reference and initially comparable directory structure. |
| D-003 | Stage one is the whole game on Windows 11 through Vulkan, including multiplayer. | A partial game or presentation prototype is not the completion milestone. |
| D-004 | Stage two establishes stability and fidelity. | Known migration bugs may exist in stage one; they are addressed in stage two. |
| D-005 | Stage three modernizes C++, dependencies, and architecture together. | Substantial internal restructuring is allowed while preserving the experience. |
| D-006 | Preserve original behavior and inherited bugs by default. | Fix migration regressions; document original defects for separate future decisions. |
| D-007 | Preserve gameplay, graphics style, and assets. | Modernization concerns the software behind the original experience. |
| D-008 | Serve us initially and maintain both practical docs and migration history. | Public release planning is not a prerequisite; understanding the engineering journey is a deliverable. |
| D-009 | The 1999 retail game defines fidelity; the published 2020 source is the implementation starting point. | The user does not have the 1999 source code, and obtaining it is not required. Compare observable retail behavior, and distinguish retail quirks from differences introduced by the 2020 port or our migration. |
| D-010 | Establish ground zero before further Vulkan conversion: reproduce the OpenGL game with the upstream Linux-container build, then build/run it natively on Windows 11 without the container. | Determine the actual starting behavior and separate build-environment problems from game-code problems. The current work proceeds from the 2020 source without waiting for a retail comparison setup. |

These decisions summarize the discussion on 2026-11-08. The [project brief](../PROJECT_BRIEF.md)
contains the scope and unresolved questions; the [roadmap](../ROADMAP.md) contains
the stage contracts.

## Initial implementation history

- Obtained Nightdive's published source at
  `2c857564ac9145fa8913ed57e533a5069faf7662` and created the project fork.
- Added a standalone Vulkan presentation context, smoke executable, translated
  shaders, and build/test tooling. Previously recorded checks are in the
  [bootstrap guide](../guides/VULKAN_BOOTSTRAP.md). The full game remains unported.
- Corrected an initial repository-layout mistake: the two source folders had
  separate Git directories, leaving the project root outside Git. The root now
  owns both source trees and documentation. Upstream history, Git LFS objects,
  conversion work, and reference source were retained; reference content and file
  modes were checked against the upstream Git tree. Old reference Git metadata
  is backed up locally inside the root `.git` directory.
- Started the shared documentation drafts. No game implementation changes are
  part of this documentation round.

## Record format for subsequent work

For each meaningful change, capture the affected behavior/component, source
revision or commit when available, previous implementation, change, reason,
alternatives/tradeoffs, verification and its limits, and lessons or follow-ups.
Practical instructions belong in the guides and should be linked from the record.

For an inherited bug or quirk, record its reference version, reproduction steps,
expected preserved behavior, migration observations, and the evidence supporting
its classification. Distinguish behavior of the 1999 retail game, differences
already introduced by the 2020 source port, and our own migration regressions.
Keep proposed intentional fixes to retail behavior separate from migration
regressions. An uncertain classification stays uncertain until investigated.

No inherited game bug has yet been investigated and classified by this project;
this document does not infer defects from the age of the code.
