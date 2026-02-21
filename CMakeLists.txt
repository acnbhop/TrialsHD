cmake_minimum_required(VERSION 3.20)
if(NOT MSVC_IDE AND NOT XCODE)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()
project(TrialsHD LANGUAGES CXX)

find_package(LibXml2 REQUIRED)

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

include_directories(code
    $<$<BOOL:${REDLYNX_LINUX}>:/usr/include/libxml2>
)

add_library(shared STATIC
    code/shared/build_defines.hpp
    code/shared/util.hpp

    code/shared/lib.cpp
)

add_library(game STATIC
    code/game/track.cpp     code/game/track.hpp
)

#
# Tools
#
add_executable(track_tool
    code/tools/track_tool.cpp
)

target_link_libraries(track_tool PRIVATE shared game LibXml2::LibXml2
    $<$<BOOL:${REDLYNX_LINUX}>:lzma>
)
