# Build and assemble the mod (Windows x64)

## Native DLL

Install Visual Studio 2022 (Desktop development with C++), CMake >= 3.24, Git,
and vcpkg. Use an x64 VS developer terminal. Set `VCPKG_ROOT` to your vcpkg
checkout, then from this repository:

```powershell
cmake -S . -B build/windows -G "Visual Studio 17 2022" -A x64 `
  "-DCMAKE_TOOLCHAIN_FILE=$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
  -DVCPKG_TARGET_TRIPLET=x64-windows-static-md
cmake --build build/windows --config Release --parallel 2
cmake --install build/windows --config Release --prefix build/package
```

CMake fetches CommonLibF4RD at pinned commit
`b092bcc70f690b2c51b980c0b251fa56bf2d3460`. Its vcpkg dependencies are declared
in `vcpkg.json`. The DLL is staged under `build/package/F4SE/Plugins`.
No game install is needed to compile the DLL. This build has not been run in the
Linux workspace: CommonLibF4RD requires Windows/MSVC.

### Run the GitHub build

Open the repository's **Actions** tab, select **Build framework**, and choose
**Run workflow** on `main`. It also runs automatically on pushes to `main` and
on pull requests. The Linux logic tests run first, then the Windows x64 DLL build.

When both jobs pass, open the run and download the artifact named
`native-dll-only-not-a-complete-mod`. Place its DLL under `Data/F4SE/Plugins`.
The artifact does not contain the PEX scripts, companion ESP, or Runtime Database;
complete the assembly steps below before testing in-game. A successful build
establishes compilation, not in-game correctness.

## Papyrus and companion plugin (required)

Use your Fallout 4 Creation Kit and its supplied Papyrus base sources/flags,
plus the scripts from your installed F4SE version. Compile both files under
`scripts/Source/User` using the Creation Kit Papyrus compiler (not Skyrim's
compiler). The compiler imports must include the CK base sources, the F4SE
source folder providing `UI.psc`, and this repository's `scripts/Source/User`.
Use the CK-supplied Fallout 4 flags file. Stage `TSSD_Native.pex` and
`TSSD_Controller.pex` in `build/package/Scripts`.

In Creation Kit, load `Fallout4.esm`, then:

1. Create a new quest with editor ID **TSSD_DreamController**. Set **Start Game
   Enabled**, leave **Run Once** unchecked, and do not add objectives, stages,
   aliases, or conditions. It is a background controller, not a visible quest.
2. Attach **TSSD_Controller** to that quest. It has no properties to fill.
   Do not attach the hidden global native declaration script to anything.
3. Save as **TheSoleSurvivorDreams.esp**. In FO4Edit, verify the VMAD attachment
   and start flag, compact the new plugin's FormIDs for ESL if necessary, and
   set its ESL flag. Do this before distributing it or making persistent saves.
4. Copy the resulting ESL-flagged ESP into `build/package`.

This small plugin is necessary to instantiate the Papyrus listener; the DLL
alone cannot receive its sleep callbacks. These are assembly instructions,
not a claim that a binary plugin has been created or validated here.

## End-user package

Stage these paths beneath the game's `Data` directory (or install as a mod
manager archive whose root corresponds to `Data`):

```text
TheSoleSurvivorDreams.esp
Scripts/TSSD_Controller.pex
Scripts/TSSD_Native.pex
F4SE/Plugins/TheSoleSurvivorDreams.dll
```

Separately install F4SE matching the actual executable and the
[CommonLibF4RD Runtime Database](https://github.com/Zzyxz/CommonLibF4RD#runtime-database)
at `Data/F4SE/Plugins/f4rd-runtime.bin`. Do not rename the database by runtime.
Launch through F4SE and enable the ESP. No DLC dependencies are intended.
No FO4Edit, game masters, compiler, or Runtime Database binaries are bundled.

Check the standard F4SE log folder under your Fallout 4 user documents for
`TheSoleSurvivorDreams.log`. Missing actor values cause the selected dream to be
skipped with an error rather than modifying an unrelated stat.
