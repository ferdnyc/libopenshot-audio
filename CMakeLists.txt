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
set(PROJECT_VERSION_FULL "0.2.0-dev3")
set(PROJECT_SO_VERSION 7)

# Remove the dash and anything following, to get the #.#.# version for project()
STRING(REGEX REPLACE "\-.*$" "" VERSION_NUM "${PROJECT_VERSION_FULL}")

################### SETUP PROJECT ###################
# This will define the following variables
# PROJECT_NAME
# PROJECT_VERSION, libopenshot-audio_VERSION
# PROJECT_VERSION_MAJOR, libopenshot-audio_VERSION_MAJOR
# PROJECT_VERSION_MINOR, libopenshot-audio_VERSION_MINOR
# PROJECT_VERSION_PATCH, libopenshot-audio_VERSION_PATCH
PROJECT(libopenshot-audio LANGUAGES C CXX VERSION ${VERSION_NUM})

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
    ${PROJECT_SOURCE_DIR}/src/hex_version.cpp
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

message("
Generating build files for OpenShot with CMake ${CMAKE_VERSION}
  Building ${PROJECT_NAME} version ${PROJECT_VERSION_FULL} (${PROJECT_VERSION_HEX})
  SO/API/ABI Version: ${PROJECT_SO_VERSION}
")


# Define install paths according to system conventions
# XXX: This must be AFTER THE PROJECT() COMMAND w/ languages enabled,
#      in order to properly configure CMAKE_INSTALL_LIBDIR path
include(GNUInstallDirs)
include(FeatureSummary)

### Build configuration (options)
option(ENABLE_AUDIO_DOCS "Attempt to build API docs for audio library" ON)
option(AUTO_INSTALL_DOCS "Include documentation in the default install" ON)

# Specify alternative location for JUCE modules (debian has their own)
set(JUCE_MODULES_PATH "${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode/modules" CACHE PATH
    "Location of the JUCE source code 'modules' directory")

message(STATUS "Using JUCE module path: ${JUCE_MODULES_PATH}")

########## Configure headers ##############
configure_file(include/JuceHeader.h.in include/JuceHeader.h @ONLY)
configure_file(include/AppConfig.h.in include/AppConfig.h @ONLY)
# We'll want those installed later
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libopenshot-audio )

# Juce requires either DEBUG or NDEBUG to be defined on MacOS.
# -DNDEBUG is set by cmake for all release configs, so add
# -DDEBUG for debug builds. We'll do this for all OSes, even
# though only MacOS requires it.
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
# Make sure we've picked some build type, default to release
if(NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
	set(CMAKE_BUILD_TYPE "Release")
endif()

# Default extension for source files
if(UNIX AND APPLE)
  SET(SOURCE_EXTENSION "mm")
else ()
  SET(SOURCE_EXTENSION "cpp")
endif()

# List of modules to build
set(JUCE_MODULES
	juce_audio_basics
	juce_audio_devices
	juce_audio_formats
	juce_core
	juce_data_structures
	juce_events )
# Convert to list of source files (extension based on OS)
foreach(j_module IN LISTS JUCE_MODULES)
  list(APPEND JUCE_SOURCES
    JuceLibraryCode/include_${j_module}.${SOURCE_EXTENSION})
endforeach()

add_library(openshot-audio SHARED ${JUCE_SOURCES})

# Include header directories
target_include_directories(openshot-audio PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
	$<BUILD_INTERFACE:${JUCE_MODULES_PATH}>
	$<INSTALL_INTERFACE:include/libopenshot-audio> )

# Set SONAME and other library properties
set_target_properties(openshot-audio
		PROPERTIES
		VERSION ${PROJECT_VERSION}
		SOVERSION ${PROJECT_SO_VERSION}
		CXX_STANDARD 11
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS OFF
		MACOSX_RPATH OFF
		INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")

# Threading library -- uses IMPORTED target Threads::Threads (since CMake 3.1)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
target_link_libraries(openshot-audio PUBLIC Threads::Threads)

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

# Enable stack-unwinding support in c objects on gcc-based platforms.
# Failing to do so will cause your program to be terminated when a png
# or a jpeg exception is thrown on linux or macosx.
if(CMAKE_COMPILER_IS_GNUCC)
  target_compile_options(openshot-audio PUBLIC -fexceptions)
endif()

if(WIN32)
	# Find the base directory of the ASIO SDK (if any)
	find_path(ASIO_SDK_DIR iasiodrv.h PATHS $ENV{ASIO_SDK_DIR} )

	IF (ASIO_SDK_DIR)
		message(STATUS "FOUND ASIO_SDK_DIR: ${ASIO_SDK_DIR}")
		target_compile_definitions(openshot-audio PRIVATE JUCE_ASIO=1)
		target_include_directories(openshot-audio PRIVATE ${ASIO_SDK_DIR})
  endif()

	# Order here can be important!
	# For example, winmm.lib must come before kernel32.lib (if linked)
	# or older 32-bit windows versions will have linking issues for
	# certain entry points
	target_link_libraries(openshot-audio PRIVATE
    winmm.lib
    ws2_32.lib
    wininet.lib
    version.lib
    Shlwapi.dll
  )
endif()

if(APPLE)
	target_link_libraries(openshot-audio PRIVATE
    -framework Carbon
    -framework Cocoa
    -framework CoreFoundation
    -framework CoreAudio
    -framework CoreMidi
    -framework IOKit
    -framework AGL
    -framework AudioToolbox
    -framework QuartzCore
    -framework Accelerate
    objc
  )
  target_compile_options(openshot-audio PRIVATE -flax-vector-conversions)
endif()

# Link the platform dynamic loader
target_link_libraries(openshot-audio PRIVATE ${CMAKE_DL_LIBS})

# PROCESS SUB-DIRECTORIES
add_subdirectory(src)


#################### INSTALLATION #####################

# Install Header Files
foreach(j_module IN LISTS JUCE_MODULES)
  install(DIRECTORY ${JUCE_MODULES_PATH}/${j_module}/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libopenshot-audio/${j_module}
    FILES_MATCHING PATTERN "*.h" )
endforeach()

# Install library
INSTALL(TARGETS openshot-audio
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR} )

# Install manpage
install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/doc/openshot-audio-test-sound.1
  DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
)

###
### DOCUMENTATION
###

# We need Python to process the source for Doxygen...
set(AUDIO_DOCS_ENABLED FALSE)
if(ENABLE_AUDIO_DOCS AND CMAKE_VERSION VERSION_GREATER 3.11)
  # Find python (used to preprocess documentation)
  find_package(PythonInterp 3)
  # Find Doxygen (used for documentation)
  find_package(Doxygen)

  if(PYTHONINTERP_FOUND AND DOXYGEN_FOUND)
    message(STATUS "Doxygen found, enabling ${PROJECT_NAME} documentation target")
    # Pre-process the sources to fix documentation formatting
    add_custom_target(process-source-files
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/doc/process_source_files.py ${JUCE_MODULES_PATH} "docs"
        COMMENT "Formatting source code for documentation"
        VERBATIM )

    # Processed docs are removed on "make clean"
    set_property(DIRECTORY
      APPEND PROPERTY
      ADDITIONAL_MAKE_CLEAN_FILES "docs"
    )

    set(DOXYGEN_QUIET YES)
    set(DOXYGEN_PROJECT_NAME "OpenShot Audio Library | ${PROJECT_NAME}")
    set(DOXYGEN_PROJECT_NUMBER ${PROJECT_VERSION})
    set(DOXYGEN_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/docs")
    set(DOXYGEN_STRIP_FROM_PATH "${PROJECT_BINARY_DIR}/docs")
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${PROJECT_SOURCE_DIR}/INSTALL.md")

    set(DOXYGEN_FILE_PATTERNS
      "juce_*.h"
      "juce_*.dox"
      "*.md"
    )

    set(DOXYGEN_INLINE_SIMPLE_STRUCTS YES)
    set(DOXYGEN_EXTRACT_ALL NO)
    set(DOXYGEN_HIDE_UNDOC_CLASSES YES)
    set(DOXYGEN_HIDE_FRIEND_COMPOUNDS YES)
    set(DOXYGEN_HIDE_IN_BODY_DOCS YES)
    set(DOXYGEN_SORT_BRIEF_DOCS YES)
    set(DOXYGEN_INTERNAL_DOCS YES)
    set(DOXYGEN_GENERATE_TODOLIST NO)
    set(DOXYGEN_GENERATE_TESTLIST NO)
    set(DOXYGEN_GENERATE_BUGLIST NO)

    set(DOXYGEN_WARNINGS NO)
    set(DOXYGEN_WARN_IF_UNDOCUMENTED NO)
    set(DOXYGEN_RECURSIVE YES)
    set(DOXYGEN_SOURCE_BROWSER YES)
    set(DOXYGEN_REFERENCED_BY_RELATION YES)

    set(DOXYGEN_HTML_OUTPUT "html")
    set(DOXYGEN_HTML_DYNAMIC_SECTIONS NO)

    set(DOXYGEN_DOT_IMAGE_FORMAT svg)
    set(DOXYGEN_INTERACTIVE_SVG YES)
    set(DOXYGEN_DOT_TRANSPARENT YES)

    set(DOXYGEN_EXCLUDE
      "${PROJECT_BINARY_DIR}/docs/juce_core/zip/zlib"
      "${PROJECT_BINARY_DIR}/docs/juce_audio_formats/codecs/flac"
      "${PROJECT_BINARY_DIR}/docs/juce_audio_formats/codecs/oggvorbis"
      "${PROJECT_BINARY_DIR}/docs/juce_audio_devices/native"
      "${PROJECT_BINARY_DIR}/docs/juce_core/native"
      "${PROJECT_BINARY_DIR}/docs/juce_events/native"
    )
    set(DOXYGEN_EXCLUDE_PATTERNS
      [["*/.*"]]
      [["*/.*/*"]]
      juce_CompilerSupport.h
      juce_StdFunctionCompat.h
      juce_FlacHeader.h
    )

    set(DOXYGEN_ALIASES
      tags{1}=
      [[topictag{1}=\1]]
      [["box{1}=<dl class=\"section attention\"><dt>\1</dt><dd>"]]
      endbox=</dd></dl>
      c_void=@s_code{void}
      c_bool=@s_code{bool}
      c_char=@s_code{char}
      c_float=@s_code{float}
      c_double=@s_code{double}
      c_int=@s_code{int}
      c_nullptr=@s_code{nullptr}
      [[c_for=@s_code{for()}]]
      [[c_if=@s_code{if()}]]
      c_ifelse=@s_code{if..else}
      [[c_while=@s_code{while()}]]
      c_true=@s_code{true}
      c_false=@s_code{false}
      c_enum=@s_code{enum}
      c_switch=@s_code{switch..case}
      c_static=@s_code{static}
      c_new=@s_code{new}
      c_typedef=@s_code{typedef}
    )
    set(DOXYGEN_VERBATIM_VARS
      DOXYGEN_ALIASES
      DOXYGEN_EXCLUDE_PATTERNS
    )

    doxygen_add_docs(${PROJECT_NAME}-doc
      ${PROJECT_BINARY_DIR}/docs
      ${PROJECT_BINARY_DIR}/include
      ${PROJECT_SOURCE_DIR}/INSTALL.md
      ${PROJECT_SOURCE_DIR}/doc
      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/docs
      COMMENT "Generate ${PROJECT_NAME} documentation"
    )
    # Make sure process-source comes before Doxygen targets,
    # assuming UseDoxygen found the tools and created them
    if(TARGET ${PROJECT_NAME}-doc)
        add_dependencies(${PROJECT_NAME}-doc process-source-files)
        set(AUDIO_DOCS_ENABLED TRUE)
    endif()
  endif()
endif()

# Doxygen was found
if (TARGET ${PROJECT_NAME}-doc)
  # Install docs, iff the user builds them with `make doc`
  install(CODE "MESSAGE(\"Checking for documentation files to install...\")")
  install(CODE "MESSAGE(\"(Compile with 'make doc' command, requires Python3 and Doxygen)\")")

  if(NOT AUTO_INSTALL_DOCS)
    set(DOCS_AUTO_INSTALL_ARG EXCLUDE_FROM_ALL)
  endif()
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/html/
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/API
    MESSAGE_NEVER  # Don't spew about file copies
    COMPONENT "Documentation"
    ${DOCS_AUTO_INSTALL_ARG}
    OPTIONAL  # No error if the docs aren't found
  )
  if(NOT TARGET doc)
    add_custom_target(doc)
  endif()
  if(TARGET doc)
    add_dependencies(doc ${PROJECT_NAME}-doc)
  endif()
endif()
add_feature_info("${PROJECT_NAME} docs" AUDIO_DOCS_ENABLED "Build ${PROJECT_NAME} API documentation")

###
### PRINT FEATURE SUMMARY
###
feature_summary(WHAT ALL
  INCLUDE_QUIET_PACKAGES
  FATAL_ON_MISSING_REQUIRED_PACKAGES
  DESCRIPTION "${PROJECT_NAME} build configuration:")
