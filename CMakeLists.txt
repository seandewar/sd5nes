cmake_minimum_required(VERSION 3.1)
project(sd5nes)

set(CMAKE_CXX_STANDARD 11)

message(STATUS "Starting sd5nes CMAKE")

# Enable Debug symbols if not already enabled...
if(CMAKE_BUILD_TYPE STREQUAL "")
	set(CMAKE_BUILD_TYPE Debug)
endif()

# Define local include dir
include_directories(sd5nes/)

# Define the sources for the exe
set(sd5nes_SOURCE_FILES
	sd5nes/NESController.h
	sd5nes/NESCPU.h
	sd5nes/NESCPUEmuComm.h
	sd5nes/NESCPUOpConstants.h
	sd5nes/NESEmulationConstants.h
	sd5nes/NESEmulator.h
	sd5nes/NESException.h
	sd5nes/NESGamePak.h
	sd5nes/NESGamePakPowerState.h
	sd5nes/NESHelper.h
	sd5nes/NESMemory.h
	sd5nes/NESMemoryConstants.h
	sd5nes/NESMMC.h
	sd5nes/NESPPU.h
	sd5nes/NESPPUEmuComm.h
	sd5nes/NESReadBuffer.h
	sd5nes/NESTypes.h

	sd5nes/main.cpp
	sd5nes/NESController.cpp
	sd5nes/NESCPU.cpp
	sd5nes/NESCPUEmuComm.cpp
	sd5nes/NESCPUOpcodes.cpp
	sd5nes/NESEmulator.cpp
	sd5nes/NESGamePak.cpp
	sd5nes/NESGamePakPowerState.cpp
	sd5nes/NESHelper.cpp
	sd5nes/NESMMC.cpp
	sd5nes/NESPPU.cpp
	sd5nes/NESPPUEmuComm.cpp
	sd5nes/NESReadBuffer.cpp
)
add_executable(sd5nes ${sd5nes_SOURCE_FILES})

# Find SFML (Requires FindSFML.cmake in ./cmake/)
set(CMAKE_MODULE_PATH
	${CMAKE_SOURCE_DIR}/cmake
	${CMAKE_MODULE_PATH}
)
find_package(SFML 2 REQUIRED system window graphics audio)
if (SFML_FOUND)
	message(STATUS "SFML found - configuring include & target dirs (include ${SFML_INCLUDE_DIR})")
	include_directories(${SFML_INCLUDE_DIR})
	target_link_libraries(sd5nes ${SFML_LIBRARIES})
endif()

# Install target
install(TARGETS sd5nes DESTINATION bin)

# Include extra pre-reqs
include(InstallRequiredSystemLibraries)

message(STATUS "sd5nes CMAKE complete")
