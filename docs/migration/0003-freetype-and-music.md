# Ground zero: FreeType linkage, first launch, and music packaging

Date: 2026-11-08. Branch: `feature/opengl-first-run`.

## FreeType and first launch

The first linked executable imported `libfreetype-6.dll`, which was missing from
the assembled runtime. Selected the bundled static FreeType archive instead,
using out-of-tree CMake cache overrides:

```text
FREETYPE_LIBRARY_RELEASE=/source/windows/lib64/libfreetype.a
FREETYPE_LIBRARY_DEBUG=/source/windows/lib64/libfreetype.a
BZ2_LIBRARY=/tmp/bzip2-1.0.8-matching/libbz2.a
PNG16_LIBRARY=/tmp/libpng-1.6.37-win64/libpng16.a
```

The bundled bzip2 and PNG archives referenced `__imp___acrt_iob_func`, the
same CRT mismatch encountered with ALURE. Rebuilt matching versions bzip2 1.0.8
and libpng 1.6.37 using the game's MinGW GCC 7.3 POSIX/MSVCRT compiler. An earlier
exploratory bzip2 1.0.6 build was not used in the linked game. No engine or
reference-source changes were needed. PNG used copied Ubuntu zlib development
headers and the bundled Windows `libz.a`, not a Linux binary library.

Source archives came from Ubuntu's package archive, with SHA-256 checked against
the corresponding source package descriptions:

| Source | SHA-256 |
| --- | --- |
| [bzip2 1.0.8](https://archive.ubuntu.com/ubuntu/pool/main/b/bzip2/bzip2_1.0.8.orig.tar.gz) | `ab5a03176ee106d3f0fa90e381da478ddae405918153cca248e682cd0c4a2269` |
| [libpng 1.6.37](https://archive.ubuntu.com/ubuntu/pool/main/libp/libpng1.6/libpng1.6_1.6.37.orig.tar.gz) | `ca74a0dace179a8422187671aee97dd3892b53e168627145271cad5b5ac81307` |

Successful artifact: `build/opengl/bin/machines-opengl-static-freetype.exe`,
SHA-256 `17a28e9275d34448fa604119d3ad5e7f2bf6ca7a769077c215297f6541743445`.
Its import table no longer requires a FreeType DLL. Logs are saved under
`build/opengl/logs`: `bzip2-1.0.8-build.log`, `png-configure.log`, `png-build.log`,
`game-configure-static-freetype.log`, and `game-link-static-freetype.log`.

The source-built executable launched on Windows 11: the loading screen and
opening movie were observed. Main-menu operation, gameplay, and multiplayer
were not established. The user requested an immediate stop because interactive
execution interfered with active GitHub/Playwright runners. The game process
was stopped and its exit verified. Do not relaunch without the user's explicit
go-ahead. Command-line `-w 1024 768` sets the initial window size but does not
override the fullscreen configuration; future windowed testing must configure
`Screen Resolution\Windowed` appropriately before launch.

## Music failure and fix

The launch stderr reported:

```text
Could not load sounds/music/track4.ogg reason: Unsupported type
```

ALURE 1.2's Windows Vorbis decoder dynamically opens `vorbisfile.dll`. The runtime
pack supplies the decoder as `libvorbisfile-3.dll`, so ALURE could not find it.
Created a byte-identical copy named `vorbisfile.dll`, retaining the original name.
Its transitive imports `libvorbis-0.dll` and `libogg-0.dll` are already present;
the rest are Windows system DLLs. No new download or game rebuild was needed.

The reproducible setup is [Prepare-OpenGLMusic.ps1](../../tools/Prepare-OpenGLMusic.ps1).
It checks all three files, accepts an already identical alias, and refuses to
overwrite a different existing decoder. The decoder SHA-256 is
`cd922aab8040109e12418cda4e57500f152a40c4eb7c2beb3ce0bdecb22f67b2`.

## Verification and limits

A small headless Windows probe loaded the exact `vorbisfile.dll` alias, checked
all six symbols used by ALURE (`ov_clear`, `ov_info`, `ov_open_callbacks`,
`ov_pcm_seek`, `ov_pcm_total`, `ov_read`), and decoded all ten Ogg music tracks
to PCM through end of file. All passed; exit code 0. The previously failing
`track4.ogg` produced 25,803,792 PCM bytes. No audio device was opened, audio
played, or game launched during this verification.

Probe source/executable remain ignored local artifacts at
`build/opengl/check-vorbis.c` and
`build/opengl/runtime-baseline/machines/check-vorbis.exe`. Results are in
`build/opengl/logs/music-decode.stdout.log` and `music-decode.stderr.log`.
This verifies DLL loading, required exports, and track decoding. ALURE integration
and audible in-game music still require a later authorized game test. This was
a runtime packaging mismatch, not evidence of an inherited retail-game bug.
