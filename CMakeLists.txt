cmake_minimum_required(VERSION 3.20)
if(NOT MSVC_IDE AND NOT XCODE)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()
project(TrialsHD LANGUAGES CXX)

include(FetchContent)

#set(BUILD_DX11 OFF CACHE BOOL "" FORCE)
#set(BUILD_DX12 OFF CACHE BOOL "" FORCE)
#set(BUILD_TOOLS OFF CACHE BOOL "" FORCE)
#set(BUILD_SAMPLE OFF CACHE BOOL "" FORCE)
#
#FetchContent_Declare(
#    directxtex
#    GIT_REPOSITORY https://github.com/microsoft/DirectXTex.git
#    GIT_TAG        main 
#)
#FetchContent_MakeAvailable(directxtex)

FetchContent_Declare(
    tinyxml2
    GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(tinyxml2)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(XZ_BUILD_CLITOOLS OFF CACHE BOOL "" FORCE)
set(XZ_DOC OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    xz
    GIT_REPOSITORY https://github.com/tukaani-project/xz.git
    GIT_TAG        v5.4.5
)
FetchContent_MakeAvailable(xz)

find_package(ZLIB QUIET)

if(ZLIB_FOUND)
    message(STATUS "Found system ZLIB: ${ZLIB_VERSION_STRING}")
else()
    message(STATUS "System ZLIB not found. Fetching from source...")
    FetchContent_Declare(
        zlib
        GIT_REPOSITORY https://github.com/madler/zlib.git
        GIT_TAG        v1.3.1 # Current stable tag
    )
    FetchContent_MakeAvailable(zlib)
    
    # The madler/zlib CMake file creates a 'zlibstatic' target.
    # We alias it to ZLIB::ZLIB so our target_link_libraries works seamlessly either way!
    add_library(ZLIB::ZLIB ALIAS zlibstatic)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(REDLYNX_LINUX TRUE)
else()
    set(REDLYNX_LINUX FALSE)
endif()

if(NOT MSVC)
    add_compile_options(-include ${CMAKE_CURRENT_SOURCE_DIR}/code/shared/build_defines.hpp)
else()
    add_compile_options(/FI${CMAKE_CURRENT_SOURCE_DIR}/code/shared/build_defines.hpp)
endif()

include_directories(code ${tinyxml2_SOURCE_DIR}/tinyxml2 ${xz_SOURCE_DIR}/src ${ZLIB_INCLUDE_DIRS})

add_library(shared STATIC
    code/shared/build_defines.hpp
    code/shared/util.hpp
    code/shared/lib.cpp
)

add_library(game STATIC
    code/game/texture.cpp   code/game/texture.hpp
    code/game/track.cpp     code/game/track.hpp
)

#
# Tools
#
add_executable(track_tool
    code/tools/track_tool.cpp
)
add_executable(tex_tool
    code/tools/tex_tool.cpp
)

target_link_libraries(track_tool PRIVATE shared game tinyxml2 liblzma ZLIB::ZLIB)
target_link_libraries(tex_tool PRIVATE shared game)
