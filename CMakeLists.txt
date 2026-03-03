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

#===================================================================================================
# Third party dependencies to fetch or find and to build
#===================================================================================================

include(FetchContent)

set(FETCHCONTENT_QUIET OFF)

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

#
# TinyXML2 for XML parsing.
#
FetchContent_Declare(
    tinyxml2
    GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
    GIT_PROGRESS.  TRUE
)
FetchContent_MakeAvailable(tinyxml2)

#
# Set XZ options
#
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(XZ_BUILD_CLITOOLS OFF CACHE BOOL "" FORCE)
set(XZ_DOC OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)


#
# Fetch XZ, this is for LZMA compression.
#
FetchContent_Declare(
    xz
    GIT_REPOSITORY https://github.com/tukaani-project/xz.git
    GIT_TAG        v5.4.5
    GIT_SHALLOW    TRUE
    GIT_PROGRESS.  TRUE
)
FetchContent_MakeAvailable(xz)

if(TARGET liblzma)
    target_compile_options(liblzma PRIVATE -w)
endif()

#
# Find if we have ZLIB, quietly since we want to be able to fall back to fetching it if we don't.
#
find_package(ZLIB QUIET)

#
# Fetching from source if not found because platforms like Windows don't have it iirc. I know mine didn't
# till I added fetching it.
#
if(ZLIB_FOUND)
    message(STATUS "Found system ZLIB: ${ZLIB_VERSION_STRING}")
else()
    message(STATUS "System ZLIB not found. Fetching from source...")
    FetchContent_Declare(
        zlib
        GIT_REPOSITORY https://github.com/madler/zlib.git
        GIT_TAG        v1.3.1 # Current stable tag
        GIT_SHALLOW    TRUE
        GIT_PROGRESS.  TRUE
    )
    FetchContent_MakeAvailable(zlib)

    # The madler/zlib CMake file creates a 'zlibstatic' target.
    # We alias it to ZLIB::ZLIB so our target_link_libraries works seamlessly either way!
    add_library(ZLIB::ZLIB ALIAS zlibstatic)
endif()

#
# SDL3
#
FetchContent_Declare(
    sdl3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-3.4.2
    GIT_SHALLOW    TRUE
    GIT_PROGRESS.  TRUE
)
FetchContent_MakeAvailable(sdl3)

#===================================================================================================
# Configuration
#===================================================================================================

#
# C++ 20
#
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#
# Rendering API(s) to use.
#
if(USE_D3D9)
    message(STATUS "Enabling DirectX 9 rendering")
endif()
if(USE_D3D11)
    message(STATUS "Enabling DirectX 11 rendering")
endif()
if(USE_D3D12)
    message(STATUS "Enabling DirectX 12 rendering")
endif()
if(USE_VULKAN)
    message(STATUS "Enabling Vulkan rendering")
endif()
if(USE_OPENGL)
    message(STATUS "Enabling OpenGL rendering")
endif()

#
# Linux
#
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(REDLYNX_LINUX TRUE)
else()
    set(REDLYNX_LINUX FALSE)
endif()

#
# Windows
#
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(REDLYNX_WINDOWS TRUE)
    add_compile_options(-D_CRT_SECURE_NO_WARNINGS)
else()
    set(REDLYNX_WINDOWS FALSE)
endif()

#
# Forceinclude our build_defines.hpp with every file, sets up things such as build configuration
# and specific target defines.
#
if(NOT MSVC)
    add_compile_options(-include ${CMAKE_CURRENT_SOURCE_DIR}/code/shared/build_defines.hpp)
else()
    add_compile_options(/FI${CMAKE_CURRENT_SOURCE_DIR}/code/shared/build_defines.hpp)
endif()

#
# Our include directories
#
include_directories(code extern ${tinyxml2_SOURCE_DIR} ${xz_SOURCE_DIR}/src/liblzma/api ${ZLIB_INCLUDE_DIRS} ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})

#
# Core ImGui library
#
add_library(imgui STATIC
    extern/imgui/imconfig.h
    extern/imgui/imgui.cpp             extern/imgui/imgui.h
    extern/imgui/imgui_draw.cpp
    extern/imgui/imgui_widgets.cpp
    extern/imgui/imgui_tables.cpp
    extern/imgui/imgui_demo.cpp
)

if(REDLYNX_WINDOWS)
add_library(imgui_backend_win32 STATIC
    extern/imgui/backends/imgui_impl_win32.cpp extern/imgui/backends/imgui_impl_win32.h
)
message(STATUS "Enabling ImGui Win32 backend")
endif()

add_library(imgui_backend_sdl3 STATIC
    extern/imgui/backends/imgui_impl_sdl3.cpp extern/imgui/backends/imgui_impl_sdl3.h
)

if(USE_VULKAN)
add_library(imgui_backend_vulkan STATIC
    extern/imgui/backends/imgui_impl_vulkan.cpp extern/imgui/backends/imgui_impl_vulkan.h
)
message(STATUS "Enabling ImGui Vulkan backend")
endif()

if(USE_OPENGL)
add_library(imgui_backend_opengl3 STATIC
    extern/imgui/backends/imgui_impl_opengl3.cpp extern/imgui/backends/imgui_impl_opengl3.h
)
message(STATUS "Enabling ImGui OpenGL3 backend")
endif()

if(USE_D3D12)
add_library(imgui_backend_dx12 STATIC
    extern/imgui/backends/imgui_impl_dx12.cpp extern/imgui/backends/imgui_impl_dx12.h
)
message(STATUS "Enabling ImGui DirectX 12 backend")
endif()

if(USE_D3D11)
add_library(imgui_backend_dx11 STATIC
    extern/imgui/backends/imgui_impl_dx11.cpp extern/imgui/backends/imgui_impl_dx11.h
)
message(STATUS "Enabling ImGui DirectX 11 backend")
endif()

if(USE_D3D9)
add_library(imgui_backend_dx9 STATIC
    extern/imgui/backends/imgui_impl_dx9.cpp extern/imgui/backends/imgui_impl_dx9.h
)
message(STATUS "Enabling ImGui DirectX 9 backend")
endif()

target_include_directories(imgui PUBLIC extern/imgui)

if(REDLYNX_WINDOWS)
target_include_directories(imgui_backend_win32 PUBLIC extern/imgui)
endif()

target_include_directories(imgui_backend_sdl3 PUBLIC extern/imgui)
target_link_libraries(imgui_backend_sdl3 PRIVATE imgui SDL3::SDL3)

if(USE_VULKAN)
target_include_directories(imgui_backend_vulkan PUBLIC extern/imgui)
endif()

if(USE_OPENGL)
target_include_directories(imgui_backend_opengl3 PUBLIC extern/imgui)
endif()

if(USE_D3D12)
target_include_directories(imgui_backend_dx12 PUBLIC extern/imgui)
endif()

if(USE_D3D11)
target_include_directories(imgui_backend_dx11 PUBLIC extern/imgui)
endif()

if(USE_D3D9)
target_include_directories(imgui_backend_dx9 PUBLIC extern/imgui)
endif()

if(REDLYNX_WINDOWS)
target_link_libraries(imgui_backend_win32 PRIVATE imgui)
endif()
if(USE_VULKAN)
target_link_libraries(imgui_backend_vulkan PRIVATE imgui)
endif()
if(USE_OPENGL)
target_link_libraries(imgui_backend_opengl3 PRIVATE imgui)
endif()
if(USE_D3D12)
target_link_libraries(imgui_backend_dx12 PRIVATE imgui)
endif()
if(USE_D3D11)
target_link_libraries(imgui_backend_dx11 PRIVATE imgui)
endif()
if(USE_D3D9)
target_link_libraries(imgui_backend_dx9 PRIVATE imgui)
endif()

#===================================================================================================
# Core Libraries / Executables
#===================================================================================================

#
# shared is code shared between everything
#
add_library(shared STATIC
    code/shared/build_defines.hpp
    code/shared/lib.cpp
    code/shared/math.cpp
    code/shared/math.hpp
    code/shared/memory.cpp
    code/shared/memory.hpp
    code/shared/timer.cpp
    code/shared/timer.hpp
    code/shared/util.hpp
)

#
# engine is the core game engine.
#
add_library(engine STATIC
    code/engine/asset/actor.cpp
    code/engine/asset/actor.hpp
    code/engine/asset/actorinfo.cpp
    code/engine/asset/actorinfo.hpp
    code/engine/asset/anim.cpp
    code/engine/asset/anim.hpp
    code/engine/asset/font.cpp
    code/engine/asset/font.hpp
    code/engine/asset/gfx.cpp
    code/engine/asset/gfx.hpp
    code/engine/asset/model.cpp
    code/engine/asset/model.hpp
    code/engine/asset/package.cpp
    code/engine/asset/package.hpp
    code/engine/asset/packagexml.cpp
    code/engine/asset/packagexml.hpp
    code/engine/asset/shader.cpp
    code/engine/asset/shader.hpp
    code/engine/asset/texture.cpp
    code/engine/asset/texture.hpp
    code/engine/asset/video.cpp
    code/engine/asset/video.hpp
    code/engine/asset/xgs.cpp
    code/engine/asset/xgs.hpp
    code/engine/asset/xsb.cpp
    code/engine/asset/xsb.hpp
    code/engine/asset/xur.cpp
    code/engine/asset/xur.hpp
    code/engine/asset/xus.cpp
    code/engine/asset/xus.hpp
    code/engine/asset/xwb.cpp
    code/engine/asset/xwb.hpp
    code/engine/camera.cpp
    code/engine/camera.hpp
    code/engine/renderer.cpp
    code/engine/renderer.hpp
    code/engine/scene.cpp
    code/engine/scene.hpp
    code/engine/xml.cpp
    code/engine/xml.hpp
)
target_link_libraries(engine PRIVATE shared imgui imgui_backend_sdl3 SDL3::SDL3 tinyxml2::tinyxml2)

#
# game is the main game code
#
add_library(game STATIC
    code/game/app.cpp
    code/game/app.hpp
    code/game/track.cpp
    code/game/track.hpp

    code/game/data/bike_config.cpp
    code/game/data/bike_config.hpp
    code/game/data/challenges.cpp
    code/game/data/challenges.hpp
    code/game/data/game_config.cpp
    code/game/data/game_config.hpp
    code/game/data/gear.cpp
    code/game/data/gear.hpp
    code/game/data/object.cpp
    code/game/data/object.hpp
    code/game/data/objectlist.cpp
    code/game/data/objectlist.hpp
    code/game/data/trackset.cpp
    code/game/data/trackset.hpp
)
target_link_libraries(game PRIVATE engine)
target_compile_definitions(game PUBLIC LZMA_API_STATIC)

#
# gameapp is the primary executable.
#
add_executable(gameapp
    code/game/main.cpp
)
target_link_libraries(gameapp PRIVATE game)

#===================================================================================================
# Tools
#===================================================================================================

#
# track_tool is used for converting track files to and from XML and or
# dumping the contents of them.
#
add_executable(track_tool
    code/tools/track_tool.cpp
)
target_link_libraries(track_tool PRIVATE game liblzma ZLIB::ZLIB)

#
# xur_tool is used for converting Xbox User Interface Resource (.xur) files
# to and from XML for editing string tables, image paths, and UI metadata.
#
add_executable(xur_tool
    code/tools/xur_tool.cpp
)
target_link_libraries(xur_tool PRIVATE game)

#
# xus_tool is used for converting Xbox User Interface String (.xus) files
# to and from XML for editing localized string tables and property overrides.
#
add_executable(xus_tool
    code/tools/xus_tool.cpp
)
target_link_libraries(xus_tool PRIVATE game)

#
# gfx_tool is used for converting Graphics scene files (.gfx) to and from
# XML for inspecting scene graphs, materials, and animation controllers.
#
add_executable(gfx_tool
    code/tools/gfx_tool.cpp
)
target_link_libraries(gfx_tool PRIVATE game)

#
# texture_tool is used for converting texture files (.tex) to and from
# DDS files for editing and inspecting texture data.
#
add_executable(texture_tool
    code/tools/texture_tool.cpp
)
target_link_libraries(texture_tool PRIVATE game)
