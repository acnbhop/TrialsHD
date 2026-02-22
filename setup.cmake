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
