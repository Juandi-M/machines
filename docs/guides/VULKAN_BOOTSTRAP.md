# Vulkan bootstrap: build, run, and verification

This guide covers the presentation prototype, not the complete game port. See
the [project brief](../PROJECT_BRIEF.md), [roadmap](../ROADMAP.md), and
[architecture](../ARCHITECTURE.md) for project-wide decisions.

The working source tree is `conversion/` inside the single project repository.
Its sibling `original/` is an untouched source snapshot of the baseline below;
both folders are tracked by the root repository. Run build commands in this guide from
`conversion/`, and mount that checkout itself at `/work` for container builds.
Build caches retained from before relocation contain old absolute paths; use a
fresh build directory. The existing Windows executable remains usable.

The project target is the whole game on Windows 11 through Vulkan in stage one,
followed by faithful stabilization and internal modernization. This prototype
currently requests Vulkan 1.1. Its minimum version is not a finalized project-wide
hardware/API requirement; the intended current-Vulkan target still needs exact
version and hardware decisions.

## Source baseline

- Upstream: https://github.com/NightDive-Studio/machines
- Revision: `2c857564ac9145fa8913ed57e533a5069faf7662` (2020-10-19).
- This release is already an OpenGL 2.1 port, not the untouched 1999 renderer.
- The existing game still uses OpenGL. The Vulkan milestone below is separate.
- The source repository does not include the full playable asset set.

## Implemented milestone

`RenVulkanContext` implements SDL window-surface integration, device and queue
selection, a FIFO swapchain, image views, framebuffers, a clear render pass,
command submission, acquire/present synchronization, swapchain recreation,
minimized-window handling, and resource destruction. It supports separate graphics
and presentation queue families, though that configuration still needs hardware
testing. A single rendering context is supported; volk uses global dispatch.

`machines-vulkan-smoke` opens a window and presents a dark background. It needs
no game assets and exits after a specified number of frames. It does **not**
render the game, load meshes, or execute the migrated shaders yet.

The five shaders under `data/shaders/vulkan` translate the original GUI,
billboard, and world shaders to Vulkan GLSL/SPIR-V. They retain the original
BGRA vertex-color interpretation, texture multiplication, and alpha-preserving
fog calculation. Clip-space Y and depth are converted in the vertex shaders so
the original OpenGL-style projection matrices can be retained. Future pipelines
must use positive viewport height and account for the winding reversal when
configuring front-face culling. UV orientation must be checked against actual
texture uploads and offscreen render targets.

All migrated shaders use descriptor set 0, binding 0 for their uniform block
and binding 1 for the combined texture sampler. World uniforms are three
column-major mat4 values followed by two vec4 values. GUI uniforms contain a
vec2 screen size (with std140 block padding); billboard uniforms contain a mat4.
These layouts are defined for future integration, not wired into `RenDevice`.

## Build on Windows

Requirements: a C++ compiler (MSVC or MinGW-w64), CMake 3.16+, a build tool,
network access for the initial pinned dependency downloads, and glslangValidator
(from the Vulkan SDK). A Vulkan-capable GPU driver is required to run it.
The SDK's validation layer is optional except when passing `--validation`.

```powershell
cmake -S . -B build/vulkan -DMACHINES_VULKAN_BOOTSTRAP=ON
cmake --build build/vulkan --config Release
ctest --test-dir build/vulkan -C Release --output-on-failure
```

The executable is under `build/vulkan/src/libdev/render/vulkan`, with an
additional `Release` directory for multi-configuration generators. Run:

```powershell
.\build\vulkan\src\libdev\render\vulkan\Release\machines-vulkan-smoke.exe --frames 120 --exercise-window --validation
```

Omit `--validation` if the Khronos validation layer is unavailable. A missing
requested layer is reported as an error rather than silently skipping validation.
For presentation-only builds without a shader compiler, explicitly pass
`-DMACHINES_VULKAN_BUILD_SHADERS=OFF`; that does not verify shader migration.

The bootstrap option bypasses the legacy game's OpenGL, GLEW, OpenAL, ALURE,
FreeType, and ENet dependencies. It does not select Vulkan for the game.
With that option off, CMake follows the existing game build path. The old path
has not yet been modernized or built in this experiment.

## Isolated Linux build and Windows cross-build

The development image supplies the compiler, software Vulkan driver, validation
layers, SDL2, and shader tools without changing the host's compiler installation.
Commands below are for a Linux/WSL shell with Docker. Build directories inside
the container avoid Windows/OneDrive rename and file-locking issues.

```sh
docker build -f docker/vulkan-dev.Dockerfile -t machines-vulkan-dev .
docker run --rm -it -v "$PWD:/work" machines-vulkan-dev bash

# Inside the container: Linux build and validation test.
cmake -S /work -B /tmp/machines-vulkan-linux -G Ninja \
  -DMACHINES_VULKAN_BOOTSTRAP=ON -DMACHINES_VULKAN_GPU_TESTS=ON
cmake --build /tmp/machines-vulkan-linux --parallel 4
VK_LAYER_ENABLES=VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT \
  xvfb-run -a ctest --test-dir /tmp/machines-vulkan-linux --output-on-failure

# Inside the container: Windows x64 build, statically linked SDL2/C++ runtime.
cmake -S /work -B /tmp/machines-vulkan-windows -G Ninja \
  -DMACHINES_VULKAN_BOOTSTRAP=ON -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=/work/cmake/Toolchain-windows-x64.cmake
cmake --build /tmp/machines-vulkan-windows --parallel 4
mkdir -p /work/build/vulkan-windows
cp /tmp/machines-vulkan-windows/src/libdev/render/vulkan/machines-vulkan-smoke.exe \
  /work/build/vulkan-windows/
```

The normal build pins Vulkan-Headers, volk, and fallback SDL2 by source revision
and SHA-256. Installed SDL2 packages can be used instead of the fallback. Volk
loads the GPU driver's Vulkan runtime dynamically; a Vulkan import library is
not required. Generated SPIR-V and downloaded third-party sources remain under
the ignored build directory. Preserve upstream dependency licenses if packaging
the executable for distribution.

## Remaining migration

1. Build and run the existing game with its full asset set as a visual baseline.
2. Connect a backend-neutral interface to `RenDevice`/`RenDisplay`. OpenGL calls
   also live in surface operations, meshes, alpha/coplanar groups, stars, and the
   minimap; switching only `device.cpp` will not be sufficient.
3. Add GPU vertex/index buffers, texture uploads, descriptor/uniform allocation,
   depth attachments, samplers, and graphics pipelines for the translated shaders.
4. Preserve material blending, depth masks/functions, culling, coplanar bias,
   texture color keys, mipmaps, font atlases, GUI blits, framebuffer readback,
   screenshots, and minimap render targets.
5. Compare the same scenes through both backends: menu, unit selection, large
   battle, first-person control, smoke/transparency, fog, and save/load. Check
   frame-time distributions as well as average FPS, with the same simulation.
6. Use Vulkan for the full playable game before declaring stage one complete.
   Stage two then establishes full fidelity and stability. Removal of unused
   dependencies and temporary comparison code must follow the project roadmap.

No game simulation, AI, unit limits, assets, or existing rendering behavior is
modified by the presentation milestone.

## Verification on 2026-11-08

- Built the presentation executable for Linux with GCC 13 and for Windows x64
  with MinGW-w64 GCC 13; Windows SDL2 and C++ runtimes are statically linked.
- Ran the Windows executable on the host RTX 4080 SUPER (driver exposed Vulkan
  1.4.351). Presented 120 frames, resized from 800x600 to 1024x720, exercised
  minimize/restore, and created five swapchain generations. Exit status: 0.
  Khronos validation was not enabled in this native Windows run.
- Ran 120 frames using Mesa llvmpipe in Xvfb with Khronos validation and
  synchronization validation enabled. Exit status: 0; no validation errors.
  Xvfb without a window manager cannot reliably exercise actual minimization;
  the native Windows test covers that path.
- All 11 CTest checks passed: five SPIR-V validations, three linked shader
  interface checks, two CLI checks, and the presentation/resize test.
- Inspected the Windows import table: only Windows system DLL dependencies.
- `git diff --check` passed. The existing game has not been built or run, and
  shader visual parity has not yet been checked in rendered game scenes.

The tested Windows executable is at
`build/vulkan-windows/machines-vulkan-smoke.exe` (ignored build output). It can
be run directly with `--frames 120 --exercise-window` on this host.
