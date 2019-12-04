# pistol-whip-pc-modloader

## Introduction

A modloader for PC DLLs for Pistol Whip.

There are three main parts of this project:

1. `Modloader`: This contains the code that generates the `MODLOADER.dll` file that is used to load external `.dll` files from the `Mods` directory (which is automatically created in the game directory). Mods are loaded _after_ `il2cpp_init`, which is different than the Quest modloader, which loads mods in three different stages: 2 before `il2cpp_init` and one during `il2cpp_init`.
2. `pistol-whip-doorstop`: This contains the `winhttp.dll` file that is patched to execute `MODLOADER.dll`.
3. `pistol-whip-hook/PolyHook_2_0`: This contains the hook library used for x64 and x86 hooks, which are used to create mods that are loaded by `MODLOADER.dll`

### How it works

This modloader works as a standalone DLL (`MODLOADER.dll`). It does nothing by itself, the `load` function is called on the `MODLOADER.dll` by the patched `winhttp.dll` from `pistol-whip-doorstop`. `MODLOADER.dll` loads mods from the `Mods` directory (which it will create if it does not exist) and applies them to Pistol Whip.

### How to use it

* Download the latest `Modloader.zip` from [the releases page](https://github.com/sc2ad/pistol-whip-pc-modloader/releases/latest)
* Extract all 4 DLLs into the same directory as your `PistolWhip.exe` and `GameAssembly.dll` files
* Running the game once should create a `Mods` folder for you, which you can then place your mods in.

## Building

```bash
git clone https://github.com/sc2ad/pistol-whip-pc-modloader.git
cd pistol-whip-pc-modloader
git submodule update --init --recursive
```

1. Build the `pistol-whip-doorstop` `Visual Studio` project (located in the `pistol-whip-doorstop` submodule)

   The resulting `.dll` should be named `winhttp.dll`

2. Build the `pistol-whip-hook/PolyHook_2_0` project (located in the `pistol-whip-hook` submodule)

   Ensure that the `CMakeLists.txt` option for `BUILD_DLL` and `BUILD_STATIC` are set to `ON`

3. Copy your resultant build of `PolyHook_2.lib`, `PolyHook_2.dll`, and `capstone.lib` from your build directories: `pistol-whip-hook/PolyHook_2_0/build64/Release` and `pistol-whip-hook/PolyHook_2_0/build64/capstone/Release` to: `pistol-whip-hook/PolyHook_2_0/`

4. Build the `Modloader` project (located in the `Modloader` directory in the root directory)

   Adjust your project settings with the following:

    * Confirm that the selected build platform is `Release` and `x64`
    * Add the `pistol-whip-hook/PolyHook_2_0/` folder to your `IncludePath`
    * Add the `pistol-whip-hook/PolyHook_2_0/` folder to your `AdditionalIncludeDirectories`
    * Set your `LanguageStandard` to `stdcpplatest`
    * Set `GenerateDebugInformation` to `false`
    * Add both `pistol-whip-hook/PolyHook_2_0/PolyHook_2.lib` and `pistol-whip-hook/PolyHook_2_0/capstone.lib` to `AdditionalDependencies`

5. Package `MODLOADER.dll` `winhttp.dll` and `PolyHook_2.dll` into `Modloader.zip` and publish a release!

## Developing

In order to develop mods that will be loaded by this modloader, set up a build environment in Visual Studio where you follow the above project settings. The project should be a C++ `.dll` with exports, and the code that will get loaded by your mod will be called via a method you should define: `extern "C" MYMOD_API int load();`