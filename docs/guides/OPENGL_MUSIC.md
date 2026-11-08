# Preparing OpenGL music playback

Applies to the current ground-zero ALURE 1.2 build and SDL2 community runtime
under `build/opengl/runtime-baseline/machines`. This prepares files only; it
does not launch the game or play audio.

From the repository root in PowerShell:

```powershell
./tools/Prepare-OpenGLMusic.ps1
```

For a different runtime folder:

```powershell
./tools/Prepare-OpenGLMusic.ps1 -RuntimeDirectory 'C:\path\to\machines'
```

The folder must already contain compatible 64-bit `libvorbisfile-3.dll`,
`libvorbis-0.dll`, and `libogg-0.dll`. The script copies the first file to
`vorbisfile.dll`, the exact name requested by ALURE's dynamic loader. Keep
both names and the dependent libraries together beside the game executable.
The script is safe to rerun with the same files and refuses to replace a
different existing `vorbisfile.dll`.

The `Unsupported type` error can indicate that the decoder did not load, even
when the Ogg file is valid. All ten current music tracks decoded successfully
in a headless Windows check after this filename fix. In-game playback has not
yet been retested. See the [migration record](../migration/0003-freetype-and-music.md)
for evidence and limits.

Interactive game testing is paused at the user's request because this machine
also runs GitHub/Playwright jobs. Obtain explicit go-ahead before launching.
