cmake_minimum_required(VERSION 3.20)
if(NOT MSVC_IDE AND NOT XCODE)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()
# Multi-configuration generator
#
# e.g., Visual Studio, Xcode, Ninja Multi-Config
if(CMAKE_GENERATOR MATCHES "Visual Studio" OR CMAKE_GENERATOR MATCHES "Xcode" OR CMAKE_GENERATOR MATCHES "Ninja Multi-Config")
    set(CMAKE_CONFIGURATION_TYPES "Release;ReleaseNoDev;BankRelease;Profile;Debug" CACHE STRING "" FORCE)
else()
    # Single-configuration generator
    #
    # e.g., Ninja, Makefiles
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()
project(TrialsHD LANGUAGES CXX)

include(external.cmake)
include(setup.cmake)
include(global.cmake)
include(libs.cmake)
include(tools.cmake)
