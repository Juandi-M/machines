# Ground zero: rebuilding ALURE and linking the game

Date: 2026-11-08. Branch: `feature/opengl-first-run`.

## Result

Rebuilt ALURE 1.2 using the game's MinGW-w64 GCC 7.3.0 POSIX compiler and
default MSVCRT runtime configuration. Replacing the bundled ALURE archive and
removing the failed UCRT workaround produced a Windows `machines.exe` successfully.
No game engine or reference source was edited. This resolves the link failure
recorded in [the first experiment](0001-opengl-first-run.md).

Successful linkage does not establish that the game launches, audio works, or
gameplay is correct. The prepared runtime directory currently lacks the imported
`libfreetype-6.dll`; the game has not been launched in this experiment.

## Why rebuilding worked

The bundled archive referenced `__imp___acrt_iob_func`, a UCRT import that the
game's default link configuration did not provide. Adding the full bundled UCRT
archive introduced duplicate CRT definitions. The newly compiled ALURE archive
instead references `__imp___iob_func`, consistent with the existing compiler/runtime
combination. The game linked without adding UCRT or changing engine code.

## Source and configuration

- ALURE's bundled game header identifies version 1.2.
- Obtained the original 1.2 source from Ubuntu's package archive:
  https://archive.ubuntu.com/ubuntu/pool/universe/a/alure/alure_1.2.orig.tar.bz2
- Verified SHA-256 against `alure_1.2-6build1.dsc` from that archive:
  `465e6adae68927be3a023903764662d64404e40c4c152d160e3a8838b1d70f71`.
- Built in the existing Ubuntu 18.04 experiment container, with portable CMake
  3.28.6 and the same `/source/cmake/Toolchain-mingw64.cmake` compiler selection.
- Source/build paths: `/tmp/alure-source` and `/tmp/alure-win64`.
- Options: `BUILD_SHARED=OFF`, `BUILD_STATIC=ON`, `BUILD_EXAMPLES=OFF`,
  `DYNLOAD=ON`, `DUMB=OFF`, `HAS_FLAC=ON`, and Release configuration.
- Used the bundled Windows OpenAL headers and import library. Explicitly quoted
  the complete CMake file-path arguments when passing them from PowerShell.

## Decoder support and its limits

The rebuilt archive contains the same 13 module names as the bundled archive:
core, buffer, stream handling, WAV, AIFF, SndFile, VorbisFile, FLAC, MPG123, and
FluidSynth implementations. DUMB and ModPlug were not present in the bundled
archive and remain disabled. The downloaded asset inventory contains 541 WAV
files and 10 Ogg files.

Installed development packages for libsndfile, libvorbis, FLAC, mpg123, and
FluidSynth inside the container. Copied their public headers, Ogg headers, and
mpg123's `fmt123.h` into `/tmp/alure-deps/include`. Provided isolated header-only
pkg-config entries under `/tmp/alure-deps/pkgconfig` for dynamically loaded codecs;
both `PKG_CONFIG_PATH` and `PKG_CONFIG_LIBDIR` pointed there. No Linux codec
binaries were linked into the Windows archive. FLAC compilation was enabled
explicitly because the old discovery module otherwise required a link library
even with runtime loading enabled.

The copied headers, generated configuration, and matching module list do not prove
Windows codec ABI compatibility or successful decoder loading. Actual audio
tests, expected DLL names, and the runtime library set still require verification.
The original ALURE source expects names such as `vorbisfile.dll`, which may differ
from the filenames in the downloaded runtime pack. This is a remaining runtime
packaging concern, not a completed audio-parity check.

## Re-link configuration

Changed only the out-of-tree game's CMake cache:

```text
ALURE_LIBRARY=/tmp/alure-win64/libALURE32-static.a
CMAKE_CXX_STANDARD_LIBRARIES=-lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32
```

Then ran `/tmp/cmake-portable/bin/cmake --build /tmp/build-win64 --target machines
--parallel 10`. Existing game object files were reused; the link reached
`[100%] Built target machines`. The build directory contains 1,115 object files.

The executable imports MSVCRT, OpenAL, SDL2, SDL2_image, ENet, FreeType, GLEW,
OpenGL, and Windows system DLLs. No new ALURE DLL is needed: its static archive
is incorporated into the executable. Optional codec DLLs are loaded at runtime
and therefore are not fully represented by this import-table check.

## Saved artifacts

Artifacts remain ignored under `build/opengl`:

- `bin/machines-opengl.exe`, SHA-256
  `b451cce324c419192fac61d7109312c72b037e4d9f578f798d9f36d264c00061`.
- `rebuilt-alure/libALURE32-static.a`, SHA-256
  `ec464a73de6dc2063c81574eb2574284cb6af7be4b1c26411410b961d162eb1e`.
- ALURE's license, CMake cache, and generated `config.h` under `rebuilt-alure/`.
- Configure/build/link logs under `logs/`.

Next verify and supply runtime dependencies, then attempt the Windows launch.
The independent native-Windows compilation milestone is still pending.
