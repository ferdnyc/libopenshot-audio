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

cmake_minimum_required(VERSION 3.2...3.14 FATAL_ERROR)

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
set(PROJECT_VERSION_FULL "0.1.9-dev1")
set(PROJECT_SO_VERSION 7)
set(HEX_VERSION_OVERRIDE "0x109") # For CMake < 3.13

# Remove the dash and anything following, to get the #.#.# version for project()
string(REGEX REPLACE "\-.*$" "" VERSION_NUM "${PROJECT_VERSION_FULL}")

################### SETUP PROJECT ###################
# This will define the following variables
# PROJECT_NAME
# PROJECT_VERSION, OpenShotAudio_VERSION
# PROJECT_VERSION_MAJOR, OpenShotAudio_VERSION_MAJOR
# PROJECT_VERSION_MINOR, OpenShotAudio_VERSION_MINOR
# PROJECT_VERSION_PATCH, OpenShotAudio_VERSION_PATCH
project(OpenShotAudio LANGUAGES C CXX VERSION ${VERSION_NUM})

# JuceHeader.h needs a hexadecimal version number for the project
if(CMAKE_VERSION VERSION_LESS 3.13)
	set(PROJECT_VERSION_HEX ${HEX_VERSION_OVERRIDE}) # Ugly hardcoding
else()
	math(EXPR PROJECT_VERSION_HEX 
		"(${PROJECT_VERSION_MAJOR} << 16) + \
		(${PROJECT_VERSION_MINOR} << 8) + \
		(${PROJECT_VERSION_PATCH})" OUTPUT_FORMAT HEXADECIMAL )
endif()

message("\
Generating build files for ${PROJECT_NAME} \
version ${PROJECT_VERSION_FULL} (${PROJECT_VERSION_HEX})" )
message("SO/API/ABI Version: ${PROJECT_SO_VERSION}" )


# Define install paths according to system conventions
# XXX: This must be AFTER THE PROJECT() COMMAND w/ languages enabled,
#      in order to properly configure CMAKE_INSTALL_LIBDIR path
include(GNUInstallDirs)

# Make cmake find the msvc redistributables on Win32
set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP FALSE)
set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ${CMAKE_INSTALL_BINDIR})
include(InstallRequiredSystemLibraries)

# Tools for EXPORT processing
include(CMakePackageConfigHelpers)


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
# Make sure we've picked some build type, default to debug
if(NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
	set(CMAKE_BUILD_TYPE "Debug")
endif()


########## Library source files ############

# Default extension for source files
if(UNIX AND APPLE)
  set(SOURCE_EXTENSION "mm")
else ()
  set(SOURCE_EXTENSION "cpp")
endif()

# List of modules to build
set(JUCE_MODULES
	audio_basics
	audio_devices
	audio_formats
	core
	data_structures
	events )
# Convert to list of source files (extension based on OS)
foreach(j_module IN LISTS JUCE_MODULES)
	list(APPEND JUCE_SOURCES
		JuceLibraryCode/include_juce_${j_module}.${SOURCE_EXTENSION} )
endforeach()


############ OpenShotAudio library ###############

add_library(openshot-audio SHARED ${JUCE_SOURCES} )

# Include header directories
target_include_directories(openshot-audio PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode/modules>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/libopenshot-audio> )

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

# Enable stack-unwinding support in c objects on gcc-based platforms.
# Failing to do so will cause your program to be terminated when a png
# or a jpeg exception is thrown on linux or macosx.
target_compile_options(openshot-audio PUBLIC "$<$<CXX_COMPILER_ID:GNU>:-fexceptions>")


##################### DEPENDENCIES #########################

# Threading library -- uses IMPORTED target Threads::Threads (since CMake 3.1)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
target_link_libraries(openshot-audio PUBLIC Threads::Threads)

# ZLIB -- uses IMPORTED target ZLIB::ZLIB which has existed since CMake 3.1
find_package(ZLIB REQUIRED)
target_link_libraries(openshot-audio PUBLIC ZLIB::ZLIB)

# Dynamic loader
target_link_libraries(openshot-audio PUBLIC ${CMAKE_DL_LIBS})

# ALSA (Linux only)
if(UNIX AND NOT APPLE)
  find_package(ALSA REQUIRED)
  if (ALSA_FOUND AND NOT TARGET ALSA::ALSA) # CMake < 3.12
    add_library(ALSA::ALSA UNKNOWN IMPORTED)
    set_target_properties(ALSA::ALSA PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${ALSA_INCLUDE_DIR}"
      IMPORTED_LOCATION "${ALSA_LIBRARIES}")
  endif()
  target_compile_definitions(openshot-audio PUBLIC LINUX)
  target_link_libraries(openshot-audio PUBLIC ALSA::ALSA)
endif()

# Windows-specific libraries and other dependencies
if (WIN32)
	# Find the base directory of the ASIO SDK (if any)
	find_path(ASIO_SDK_DIR iasiodrv.h PATHS "$ENV{ASIO_SDK_DIR}" )

	if (ASIO_SDK_DIR)
		message(STATUS "Found ASIO SDK in: ${ASIO_SDK_DIR}")
    add_target(ASIO::SDK UNKNOWN IMPORTED)
    set_target_properties(ASIO::SDK PROPERTIES
      INTERFACE_COMPILE_DEFINITIONS "JUCE_ASIO=1"
    INTERFACE_INCLUDE_DIRECTORIES "${ASIO_SDK_DIR}")
    target_link_libraries(openshot-audio PRIVATE ASIO::SDK)
	else()
		message(STATUS "ASIO SDK not found")
		target_compile_definitions(openshot-audio PRIVATE "JUCE_ASIO=0")
  endif()

	# Tell Visual Studio not to automatically link system DLLs
	target_compile_definitions(openshot-audio PRIVATE JUCE_DONT_AUTOLINK_TO_WIN32_LIBRARIES)

	# Order here can be important!
	# For example, winmm.lib must come before kernel32.lib (if linked)
	# or older 32-bit windows versions will have linking issues for
	# certain entry points
	target_link_libraries(openshot-audio PUBLIC
                winmm.lib
                ws2_32.lib
                wininet.lib
                version.lib
                Shlwapi.dll
		)
endif()

# MacOS-specific libraries and compile options
if(UNIX AND APPLE)
  target_link_libraries(openshot-audio PUBLIC
    "-framework Carbon"
    "-framework Cocoa"
    "-framework CoreFoundation"
    "-framework CoreAudio"
    "-framework CoreMidi"
    "-framework IOKit"
    "-framework AGL"
    "-framework AudioToolbox"
    "-framework QuartzCore"
    objc
    "-framework Accelerate")
	target_compile_options(openshot-audio PUBLIC -flax-vector-conversions)
endif()


# PROCESS SUB-DIRECTORIES
add_subdirectory(src)


#################### INSTALLATION #####################

# Install Header Files
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode/modules/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libopenshot-audio
	FILES_MATCHING PATTERN "*.h" )

# Install library
install(TARGETS openshot-audio EXPORT OpenShotAudioTargets
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} )
#INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libopenshot-audio )

# Install manpage
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/doc/openshot-audio-test-sound.1
	DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 )


################ EXPORT TARGETS ########################

# Create export file in build dir
export(EXPORT OpenShotAudioTargets NAMESPACE OpenShot::)

# Install exported targets to install prefix
install(EXPORT OpenShotAudioTargets NAMESPACE OpenShot::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/OpenShotAudio
  EXPORT_LINK_INTERFACE_LIBRARIES)

configure_package_config_file(
  OpenShotAudioConfig.cmake.in
  OpenShotAudioConfig.cmake
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/OpenShotAudio")

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/OpenShotAudioConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion)

# Install export configuration to install prefix
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/OpenShotAudioConfigVersion.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/OpenShotAudioConfig.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/OpenShotAudio)


########### DOCUMENTATION ##########
# We need Python to process the source for Doxygen...
find_package(PythonInterp 3)

if(PYTHONINTERP_FOUND)
  # Pre-process the sources to fix documentation formatting
  add_custom_target(process-source-files
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/doc/process_source_files.py ${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode/modules "docs"
    COMMENT "Formatting source code for documentation"
    VERBATIM )

  # Processed docs are removed on "make clean"
  set_property(DIRECTORY
    APPEND PROPERTY
    ADDITIONAL_MAKE_CLEAN_FILES "docs" )

  # Find Doxygen (used for documentation)
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
  install(CODE "message(\"Checking for documentation files to install...\")")
  install(CODE "message(\"(Compile with 'make doc' command, requires Python3 and Doxygen)\")")

  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/html/
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/API
    MESSAGE_NEVER # Don't spew about file copies
    OPTIONAL )    # No error if the docs aren't found
endif()
