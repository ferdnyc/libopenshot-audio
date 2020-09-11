#################### CMakeLists.txt (libopenshot-audio) ######################
# @brief CMake build file for libopenshot-audio (used to generate makefiles)
# @author Jonathan Thomas <jonathan@openshot.org>
#
# @section LICENSE
#
# Copyright (c) 2008-2019 OpenShot Studios, LLC
# <http://www.openshotstudios.com/>. This file is part of
# OpenShot Audio Library (libopenshot-audio), an open-source project dedicated
# to delivering high quality audio editing and playback solutions to the
# world. For more information visit <http://www.openshot.org/>.
#
# OpenShot Audio Library (libopenshot-audio) is free software: you can
# redistribute it and/or modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# OpenShot Audio Library (libopenshot-audio) is distributed in the hope that it
# will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
# Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with OpenShot Audio Library. If not, see <http://www.gnu.org/licenses/>.
################################################################################

cmake_minimum_required(VERSION 3.1...3.14 FATAL_ERROR)

message("\
-----------------------------------------------------------------
          Welcome to the OpenShot Build System!

CMake will now check libopenshot-audio's build dependencies and
inform you of any missing files or other issues.

For more information, please visit <http://www.openshot.org/>.
-----------------------------------------------------------------")

################ ADD CMAKE MODULES ##################
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

################ PROJECT VERSION ####################
set(PROJECT_VERSION_FULL "0.2.1-dev1")
set(PROJECT_SO_VERSION 8)

# Remove the dash and anything following, to get the #.#.# version for project()
STRING(REGEX REPLACE "\-.*$" "" VERSION_NUM "${PROJECT_VERSION_FULL}")

################### SETUP PROJECT ###################
# This will define the following variables
# PROJECT_NAME
# PROJECT_VERSION, libopenshot-audio_VERSION
# PROJECT_VERSION_MAJOR, libopenshot-audio_VERSION_MAJOR
# PROJECT_VERSION_MINOR, libopenshot-audio_VERSION_MINOR
# PROJECT_VERSION_PATCH, libopenshot-audio_VERSION_PATCH
PROJECT(OpenShotAudio LANGUAGES C CXX VERSION ${VERSION_NUM})

# JuceHeader.h needs a hexadecimal version number for the project
if(CMAKE_VERSION VERSION_GREATER 3.13)
  math(EXPR PROJECT_VERSION_HEX
    "(${PROJECT_VERSION_MAJOR} << 16) + \
     (${PROJECT_VERSION_MINOR} << 8) + \
     (${PROJECT_VERSION_PATCH})" OUTPUT_FORMAT HEXADECIMAL )
else()
  # Compile and run a C++ program to generate the hex version
  set(HEX_COMPILE_DEFINITIONS
    -DVERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    -DVERSION_MINOR=${PROJECT_VERSION_MINOR}
    -DVERSION_PATCH=${PROJECT_VERSION_PATCH}
  )
  try_run(HEX_VERSION_RUN HEX_VERSION_BUILD
    ${CMAKE_CURRENT_BINARY_DIR}/hex_version
    ${OpenShotAudio_SOURCE_DIR}/src/hex_version.cpp
    COMPILE_DEFINITIONS ${HEX_COMPILE_DEFINITIONS}
    RUN_OUTPUT_VARIABLE HEX_VERSION_OUTPUT
  )
  if (NOT HEX_VERSION_BUILD)
    message(ERROR "Failed to compile hex-version utility!")
  elseif(HEX_VERSION_RUN STREQUAL FAILED_TO_RUN)
    message(ERROR "Could not execute hex-version utility!")
  else()
    set(PROJECT_VERSION_HEX ${HEX_VERSION_OUTPUT})
  endif()
endif()

message("\
Generating build files for ${PROJECT_NAME} \
version ${PROJECT_VERSION_FULL} (${PROJECT_VERSION_HEX})" )
message("SO/API/ABI Version: ${PROJECT_SO_VERSION}" )

# Define install paths according to system conventions
# XXX: This must be AFTER THE PROJECT() COMMAND w/ languages enabled,
#      in order to properly configure CMAKE_INSTALL_LIBDIR path
include(GNUInstallDirs)

set_directory_properties(PROPERTIES
    JUCE_COMPANY_NAME       "OpenShot Studios, LLC"
    JUCE_COMPANY_WEBSITE    "openshot.org"
    JUCE_COMPANY_EMAIL      "info@openshot.org"
    JUCE_COMPANY_COPYRIGHT  "JUCE Library Copyright © 2020 - Raw Material Software Limited")


# Juce requires either DEBUG or NDEBUG to be defined on MacOS.
# -DNDEBUG is set by cmake for all release configs, so add
# -DDEBUG for debug builds. We'll do this for all OSes, even
# though only MacOS requires it.
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
# Make sure we've picked some build type, default to debug
if(NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
	set(CMAKE_BUILD_TYPE "Debug")
endif()

# Enable stack-unwinding support in c objects on gcc-based platforms.
# Failing to do so will cause your program to be terminated when a png
# or a jpeg exception is thrown on linux or macosx.
if(CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fexceptions")
endif()

if(WIN32)
  # Find the base directory of the ASIO SDK (if any)
  find_path(ASIO_SDK_DIR iasiodrv.h PATHS $ENV{ASIO_SDK_DIR} )

  if(ASIO_SDK_DIR)
    message(STATUS "FOUND ASIO_SDK_DIR: ${ASIO_SDK_DIR}")
    add_definitions(-DJUCE_ASIO=1)
    include_directories(${ASIO_SDK_DIR})
  else()
    message(STATUS "ASIO_SDK_DIR NOT FOUND")
    add_definitions(-DJUCE_ASIO=0)
  endif()

  # Order here can be important!
  # For example, winmm.lib must come before kernel32.lib (if linked)
  # or older 32-bit windows versions will have linking issues for
  # certain entry points
  set(JUCE_PLATFORM_SPECIFIC_LIBRARIES
    winmm.lib
    ws2_32.lib
    wininet.lib
    version.lib
    Shlwapi.dll
    )
endif()

if(UNIX AND APPLE)
  set(JUCE_PLATFORM_SPECIFIC_DIR build/macosx/platform_specific_code)
  set(JUCE_PLATFORM_SPECIFIC_LIBRARIES "-framework Carbon -framework Cocoa -framework CoreFoundation -framework CoreAudio -framework CoreMidi -framework IOKit -framework AGL -framework AudioToolbox -framework QuartzCore -lobjc -framework Accelerate")
  set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -flax-vector-conversions")
endif()

### Build JUCE modules
add_subdirectory(JUCE)
#add_subdirectory(JUCE/extras/Build)

# Configure build helpers
#juce_add_module(juce_build_extras ALIAS_NAMESPACE juce)

# # List of modules to build
# juce_add_modules(
#   INSTALL_PATH "${CMAKE_INSTALL_INCLUEDIR}/${NAME}"
#   ALIAS_NAMESPACE juce
#   juce_audio_basics
#   juce_audio_devices
#   juce_audio_formats
#   juce_core
#   juce_data_structures
#   juce_events
#   )

set(_juce_module_names
  juce_audio_basics
  juce_audio_devices
  juce_audio_formats
  juce_core
  juce_data_structures
  juce_events
)
foreach(_j ${_juce_module_names})
  set(_t "juce::${_j}")
  list(APPEND _juce_targets ${_t})
endforeach()
unset(_j)
unset(_t)

add_library(openshot-audio)

# Link with JUCE module targets
target_link_libraries(openshot-audio
  PRIVATE
    ${_juce_targets}
  PUBLIC
    # juce::juce_build_tools
    # juce::juce_recommended_config_flags
    # juce::juce_recommended_lto_flags
    # juce::juce_recommended_warning_flags
    )

# Silliness to increase visibility of JUCE target properties
# target_compile_definitions(openshot-audio
#   INTERFACE
#     $<TARGET_PROPERTY:openshot-audio,COMPILE_DEFINITIONS>
#     )
# target_include_directories(openshot-audio
#   INTERFACE
#     $<TARGET_PROPERTY:openshot-audio,INCLUDE_DIRECTORIES>
#     )

# Set SONAME and other library properties
set_target_properties(openshot-audio
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_SO_VERSION}
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS OFF
    MACOSX_RPATH OFF
    INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
    )


# ALSA (Linux only)
if(UNIX AND NOT APPLE)
  find_package(ALSA REQUIRED)
  if (ALSA_FOUND AND NOT TARGET ALSA::ALSA) # CMake < 3.12
    add_library(ALSA::ALSA INTERFACE IMPORTED)
    set_target_properties(ALSA::ALSA PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${ALSA_INCLUDE_DIR}
      INTERFACE_LINK_LIBRARIES ${ALSA_LIBRARIES})
  endif()
  target_compile_definitions(openshot-audio PUBLIC LINUX)
  target_link_libraries(openshot-audio PUBLIC ALSA::ALSA)
endif()

# ZLIB -- uses IMPORTED target ZLIB::ZLIB which has existed since CMake 3.1
find_package(ZLIB REQUIRED)
target_link_libraries(openshot-audio PUBLIC ZLIB::ZLIB)
# target_link_libraries(openshot-audio PUBLIC
#   ${CMAKE_DL_LIBS}
#   ${JUCE_PLATFORM_SPECIFIC_LIBRARIES}
# )
#

juce_add_console_app(test-openshotaudio)
target_sources(test-openshotaudio PRIVATE src/Main.cpp)
target_link_libraries(test-openshotaudio
  PRIVATE
    juce::juce_core
    juce::juce_audio_devices
  PUBLIC
    juce::juce_build_tools
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
  )

target_compile_definitions(test-openshotaudio
  PUBLIC
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_INCLUDE_ZLIB_CODE=1
    JUCE_USE_FLAC=0
    JUCE_USE_OGGVORBIS=0
    JUCE_USE_MP3AUDIOFORMAT=0
    JUCE_USE_LAME_AUDIO_FORMAT=0
    JUCE_USE_WINDOWS_MEDIA_FORMAT=0
    )

juce_generate_juce_header(test-openshotaudio)

# PROCESS SUB-DIRECTORIES
#add_subdirectory(src)

###
### INSTALLATION
###

# if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.14")
#     set(extra_version_arg ARCH_INDEPENDENT)
# endif()
#
# include(CMakePackageConfigHelpers)
# write_basic_package_version_file("${OpenShotAudio_BINARY_DIR}/${OpenShotAudio_NAME}ConfigVersion.cmake"
#     VERSION ${OpenShotAudio_VERSION}
#     COMPATIBILITY ExactVersion
#     ${extra_version_arg})
#
# set(PROJECT_INSTALL_DESTINATION "lib/cmake/${OpenShotAudio_NAME}-${OpenShotAudio_VERSION}" CACHE STRING
#     "The location, relative to the install prefix, where the config file will be installed")
#
# set(MODULE_PATH "${CMAKE_INSTALL_INCLUDEDIR}/${OpenShotAudio_NAME}-${OpenShotAudio_VERSION}/modules")
# set(UTILS_INSTALL_DIR "${PROJECT_INSTALL_DESTINATION}")
# configure_package_config_file("${OpenShotAudio_CMAKE_UTILS_DIR}/JUCEConfig.cmake.in"
#     "${OpenShotAudio_BINARY_DIR}/${OpenShotAudio_NAME}Config.cmake"
#     PATH_VARS UTILS_INSTALL_DIR MODULE_PATH
#     INSTALL_DESTINATION "${PROJECT_INSTALL_DESTINATION}")
#
# set(MODULE_PATH "${JUCE_MODULES_DIR}")
# set(UTILS_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/extras/Build/CMake")
# configure_package_config_file("${OpenShotAudio_CMAKE_UTILS_DIR}/JUCEConfig.cmake.in"
#     "${OpenShotAudio_BINARY_DIR}/${OpenShotAudio_NAME}ExportConfig.cmake"
#     PATH_VARS UTILS_INSTALL_DIR MODULE_PATH
#     INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
#     INSTALL_DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
#
# install(FILES "${OpenShotAudio_BINARY_DIR}/${OpenShotAudio_NAME}ConfigVersion.cmake"
#               "${OpenShotAudio_BINARY_DIR}/${OpenShotAudio_NAME}Config.cmake"
#               "${OpenShotAudio_CMAKE_UTILS_DIR}/copyDir.cmake"
#               "${OpenShotAudio_CMAKE_UTILS_DIR}/${OpenShotAudio_NAME}HelperTargets.cmake"
#               "${OpenShotAudio_CMAKE_UTILS_DIR}/${OpenShotAudio_NAME}Utils.cmake"
#         DESTINATION "${PROJECT_INSTALL_DESTINATION}")
#
# # Install manpage
# INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/doc/openshot-audio-test-sound.1
# 	DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 )

########### DOCUMENTATION ##########
# We need Python to process the source for Doxygen...
find_package(PythonInterp 3)

if(PYTHONINTERP_FOUND)
  # Pre-process the sources to fix documentation formatting
  string(JOIN "," _subdir_list ${_juce_module_names})
  add_custom_target(process-source-files
    COMMAND ${PYTHON_EXECUTABLE}
      ${CMAKE_CURRENT_SOURCE_DIR}/doc/process_source_files.py
      --subdirs ${_subdir_list} ${PROJECT_SOURCE_DIR}/JUCE/modules docs
    COMMENT "Formatting source code for documentation"
    VERBATIM
  )

  # Processed docs are removed on "make clean"
  set_property(DIRECTORY APPEND PROPERTY
    ADDITIONAL_MAKE_CLEAN_FILES
      docs process_source_files.py
    )

  # Find Doxygen (used for documentation)
  set(DOXYFILE_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/docs)
  include(cmake/Modules/UseDoxygen.cmake)

  # Make sure process-source comes before Doxygen targets,
  # assuming UseDoxygen found the tools and created them
  if(TARGET doc)
    add_dependencies(doc process-source-files)
  endif()
  if(TARGET doxygen)
    add_dependencies(doxygen process-source-files)
  endif()
endif()

# Doxygen was found
if (TARGET doc)
  message(STATUS "Doxygen found, documentation target enabled")
  message("\nTo compile documentation in doc/html, run: 'make doc'")

  # Install docs, if the user builds them with `make doc`
  install(CODE "MESSAGE(\"Checking for documentation files to install...\")")
  install(CODE "MESSAGE(\"(Compile with 'make doc' command, requires Python3 and Doxygen)\")")

  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/html/
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/API
    MESSAGE_NEVER # Don't spew about file copies
    OPTIONAL )    # No error if the docs aren't found
endif()
