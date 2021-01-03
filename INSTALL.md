# The OpenShot Audio Library

## Detailed instructions for your Operating System
Build instructions are available for all three major Operating Systems:
* [Building libopenshot for Windows](doc/INSTALL-WINDOWS.md)
* [Building libopenshot for MacOS](doc/INSTALL-MAC.md)
* [Building libopenshot for Linux](doc/INSTALL-LINUX.md)

## Getting Started

The best way to get started with libopenshot-audio is to
learn about our build system, obtain all the source code,
install a development IDE and tools,
and better understand our dependencies.
So, please read through the following sections and follow the instructions.
And keep in mind, every computer system is different.
Keep an eye out for subtle file path differences in the commands you type.

## Build Tools

CMake is the backbone of our build system.
It is a cross-platform build system, which checks for dependencies,
locates header files and libraries, generates makefiles,
and supports the cross-platform compiling of libopenshot and libopenshot-audio.
CMake uses an out-of-source build concept where all temporary build files,
such as makefiles, object files, and even the final binaries,
are created outside of the source code folder.
(A subfolder named `build/` is a common convention, demonstrated here.)
This prevents the build process from cluttering up the source code.

These instructions have only been tested with
the GNU compiler (including MSYS2/MinGW for Windows)
and the Clang compiler (including AppleClang).

## Dependencies

The following libraries are required to build libopenshot-audio.
Instructions on how to install these dependencies vary by operating system.
Libraries and Executables have been labeled in the list below,
to help distinguish between them.

### CMake (`cmake`)
  * <http://www.cmake.org/> (**Executable**)
  * Used to automate the generation of Makefiles, check for dependencies,
    and is the backbone of libopenshot-audio’s cross-platform build process.

### Doxygen (`doxygen`)
  * <http://doxygen.nl/> (**Executable**)
  * Used to auto-generate the documentation used by libopenshot-audio.

## Obtaining Source Code

The first step in installing libopenshot-audio is to obtain the source code.
The source code is available on
[GitHub](https://github.com/OpenShot/libopenshot-audio).
Use the following command to obtain the latest source:

```sh
git clone https://github.com/OpenShot/libopenshot-audio.git
```

## Linux Build Instructions (libopenshot-audio)
To compile libopenshot-audio, we need to go through a few additional steps
to manually build and install it.
Launch a terminal and enter:

```sh
cd [libopenshot-audio repo folder]
cmake -B build -S .
cmake --build build
./build/src/openshot-audio-test-sound  # (This should play a test sound)
cmake --install build  # (To /usr/local unless CMAKE_INSTALL_PREFIX set)
```

