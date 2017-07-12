# CLionSourceCodeAccess
A CLion Plugin for Unreal Engine

The plugin creates a fully flushed out CMakeList file for use with CLion, adding intellisense, compiler definitions, etc.

Please visit https://github.com/dotBunny/CLionSourceCodeAccess/wiki for information on how to install and use the plugin.

## Mentions
[CLion Blog](https://blog.jetbrains.com/clion/2016/10/clion-and-ue4/)

## IMPORTANT
The plugin has been modified to not list Win64 as a whitelisted platform due to the current problems building from Windows. If you want to experiment check this [commit](https://github.com/dotBunny/CLionSourceCodeAccess/commit/9bf1de60e1b5657bc55f980e62658044ca63dc8a) out to see how to turn it back on. This is only temporary, until a fix has been found for Windows.

**But there is a probably working way (as for me, it works):**
- Install [MSYS2](http://www.msys2.org) with packages:
   + `mingw64/mingw-w64-x86_64-gcc`
   + `mingw64/mingw-w64-x86_64-clang`
   + `mingw64/mingw-w64-x86_64-make`
   + `mingw64/mingw-w64-x86_64-cmake`
   + `mingw64/mingw-w64-x86_64-gdb` (optional)
- Install CLion and setup MinGW home: `[MSYS2 install location]/mingw64`
- Add `"Win64"` element to `WhitelistPlatforms` section of the .uplugin file
- Configure the plugin:
   + C Compiler: `[MSYS2 install location]/mingw64/bin/clang.exe`
   + C++ Compiler: `[MSYS2 install location]/mingw64/bin/clang++.exe`
- Setup CLion as source code editor (optional)