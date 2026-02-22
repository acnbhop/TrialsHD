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
include_directories(code ${tinyxml2_SOURCE_DIR} ${xz_SOURCE_DIR}/src/liblzma/api ${ZLIB_INCLUDE_DIRS} ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
