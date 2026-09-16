# Astrophilux Nebula — real VST3 project

This is the native JUCE/C++ version of Nebula. It embeds the supplied C3 WAV and Infinity artwork directly in the plug-in, so Decent Sampler is not required.

## Features

- 16-voice sample instrument mapped from C1 to C6
- Full ADSR envelope
- Tone filter, Drift chorus, Echo delay, and Space reverb
- Custom Astrophilux Infinity interface
- Saved parameters and DAW session recall
- Native VST3 builds for Windows and macOS

## Get the real VST3 files

1. Create a GitHub repository and upload the contents of this folder—not the outer ZIP.
2. Open the repository's **Actions** tab.
3. Select **Build Astrophilux Nebula VST3** and click **Run workflow**.
4. After the run finishes, download the Windows and macOS artifacts. Windows includes an `.exe` installer; macOS includes a `.pkg` installer and zipped `.vst3`.

The workflow compiles on actual Windows and macOS machines. These are unsigned private test builds. Windows installs the `.vst3` bundle to `C:\Program Files\Common Files\VST3`. macOS installs it to `/Library/Audio/Plug-Ins/VST3` or `~/Library/Audio/Plug-Ins/VST3`.

## Local build

Install CMake, Git, and a C++ compiler, then run:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target AstrophiluxNebula_VST3
```

JUCE is fetched automatically during configuration.
