# CLionSourceCodeAccess
A CLion Plugin for Unreal Engine

The plugin creates a fully flushed out CMakeList file for use with CLion, adding intellisense, compiler definitions, etc.

Please visit https://github.com/dotBunny/CLionSourceCodeAccess/wiki for information on how to install and use the plugin.

## Mentions
[CLion Blog](https://blog.jetbrains.com/clion/2016/10/clion-and-ue4/)  
[UE Marketplace](https://www.unrealengine.com/marketplace/clion-integration)

## IMPORTANT
If you are a Windows user, there are some specific steps needed to make the plugin/CLion work with Unreal.

- Install [MSYS2](http://www.msys2.org) with packages:
   + `mingw64/mingw-w64-x86_64-gcc`
   + `mingw64/mingw-w64-x86_64-clang`
   + `mingw64/mingw-w64-x86_64-make`
   + `mingw64/mingw-w64-x86_64-cmake`
   + `mingw64/mingw-w64-x86_64-gdb` (optional)
- Install CLion and setup toolchains:
   + MinGW home: `[MSYS2 install location]/mingw64`
   + CMake path: `[MSYS2 install location]/mingw64/bin/cmake.exe`
- Add `"Win64"` element to `WhitelistPlatforms` section of the .uplugin file
- Configure the plugin:
   + C Compiler: `[MSYS2 install location]/mingw64/bin/clang.exe`
   + C++ Compiler: `[MSYS2 install location]/mingw64/bin/clang++.exe`
- Setup CLion as source code editor (optional)
