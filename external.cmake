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

#
# TinyXML2 for XML parsing.
#
FetchContent_Declare(
    tinyxml2
    GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(tinyxml2)

#
# Set XZ options
#
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(XZ_BUILD_CLITOOLS OFF CACHE BOOL "" FORCE)
set(XZ_DOC OFF CACHE BOOL "" FORCE)


#
# Fetch XZ, this is for LZMA compression.
#
FetchContent_Declare(
    xz
    GIT_REPOSITORY https://github.com/tukaani-project/xz.git
    GIT_TAG        v5.4.5
)
FetchContent_MakeAvailable(xz)

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
    )
    FetchContent_MakeAvailable(zlib)
    
    # The madler/zlib CMake file creates a 'zlibstatic' target.
    # We alias it to ZLIB::ZLIB so our target_link_libraries works seamlessly either way!
    add_library(ZLIB::ZLIB ALIAS zlibstatic)
endif()
