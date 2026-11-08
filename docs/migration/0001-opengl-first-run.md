# Ground zero: first OpenGL build experiment

Follow-up: [rebuilding ALURE resolved the link failure](0002-alure-rebuild.md).
The results below describe the initial attempts.

Date: 2026-11-08. Branch: `feature/opengl-first-run`.

The project checkpoint is commit `e0af93391d55f9d50abe95992c70fb8aba06e94f`,
pushed to `vulkan-port` before creating and publishing the experiment branch.

## Procedure and observations

Used the unmodified `original/docker/Dockerfile`: Ubuntu 18.04, MinGW-w64 GCC
7.3.0, and the listed development dependencies. Mounted the reference source
read-only at `/source` and built out of tree at `/tmp/build-win64`. The output
target is a native Windows executable, not a Linux game.

1. The README CMake command failed to locate the bundled OpenAL headers.
2. Explicit header/library paths resolved dependency discovery:
   `OPENAL_INCLUDE_DIR=/source/windows/include/AL`,
   `CMAKE_INCLUDE_PATH=/source/windows/include`, and
   `CMAKE_LIBRARY_PATH=/source/windows/lib64`.
3. The supplied environment's CMake 3.10 rejected linking the `libaudio` object
   library. Using portable CMake 3.28.6 allowed configuration without source edits.
4. Release compilation with `DEV_BUILD=OFF` completed. Linking failed because
   bundled `libALURE32-static.a` requires `__imp___acrt_iob_func`, unavailable in
   the default runtime link configuration.
5. Explicitly adding bundled `libucrt.a` resolved that import but introduced
   duplicate definitions, including `__imp___getmainargs`, `__imp___initenv`, and
   `mbrtowc`. Mixing the supplied CRT archives is not a working solution.

## Runtime preparation

Downloaded the asset archive linked by WiredForWar's README and assembled an
ignored runtime directory at `build/opengl/runtime-baseline/machines`. Overlaid
the original source's data/model/sound metadata and copied runtime DLLs from
community release v1.7.1. The community executable was not substituted for the
source-built game. These runtime files have not yet been validated by launching
the source-built game.

Portable native Windows CMake 3.28.6 and WinLibs GCC 9.3.0 have been prepared under
ignored `.deps/opengl-native` for the subsequent native-build experiment.

## Result and next step

The README workflow does not produce a runnable executable as supplied in this
environment. Compilation succeeds after build-tool/path adjustments; linkage
remains blocked by the bundled audio library's CRT compatibility. No engine or
reference source was modified, and no game launch or gameplay success is claimed.

Next investigate rebuilding the compatible ALURE version with the same compiler
and runtime as the game, rather than combining incompatible CRT archives. Then
link, inspect DLL dependencies, and attempt the Windows launch. Native Windows
compilation remains a separate ground-zero milestone.

Initial logs and downloads are local artifacts under `build/opengl`, excluded
from Git. Subsequent logs are in the experiment container under `/tmp` and should
be exported before removing that container.
