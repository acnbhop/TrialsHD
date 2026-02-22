#
# Core ImGui library
#
add_library(imgui STATIC
    code/extern/imgui/imconfig.h
    code/extern/imgui/imgui.cpp             code/extern/imgui/imgui.h
    code/extern/imgui/imgui_draw.cpp
    code/extern/imgui/imgui_widgets.cpp
    code/extern/imgui/imgui_tables.cpp
    code/extern/imgui/imgui_demo.cpp
)

if(REDLYNX_WINDOWS)
add_library(imgui_backend_win32 STATIC
    code/extern/imgui/backends/imgui_impl_win32.cpp code/extern/imgui/backends/imgui_impl_win32.h
)
message(STATUS "Enabling ImGui Win32 backend")
endif()

add_library(imgui_backend_sdl3 STATIC
    code/extern/imgui/backends/imgui_impl_sdl3.cpp code/extern/imgui/backends/imgui_impl_sdl3.h
)

if(USE_VULKAN)
add_library(imgui_backend_vulkan STATIC
    code/extern/imgui/backends/imgui_impl_vulkan.cpp code/extern/imgui/backends/imgui_impl_vulkan.h
)
message(STATUS "Enabling ImGui Vulkan backend")
endif()

if(USE_OPENGL)
add_library(imgui_backend_opengl3 STATIC
    code/extern/imgui/backends/imgui_impl_opengl3.cpp code/extern/imgui/backends/imgui_impl_opengl3.h
)
message(STATUS "Enabling ImGui OpenGL3 backend")
endif()

if(USE_D3D12)
add_library(imgui_backend_dx12 STATIC
    code/extern/imgui/backends/imgui_impl_dx12.cpp code/extern/imgui/backends/imgui_impl_dx12.h
)
message(STATUS "Enabling ImGui DirectX 12 backend")
endif()

if(USE_D3D11)
add_library(imgui_backend_dx11 STATIC
    code/extern/imgui/backends/imgui_impl_dx11.cpp code/extern/imgui/backends/imgui_impl_dx11.h
)
message(STATUS "Enabling ImGui DirectX 11 backend")
endif()

if(USE_D3D9)
add_library(imgui_backend_dx9 STATIC
    code/extern/imgui/backends/imgui_impl_dx9.cpp code/extern/imgui/backends/imgui_impl_dx9.h
)
message(STATUS "Enabling ImGui DirectX 9 backend")
endif()

target_include_directories(imgui PUBLIC code/extern/imgui)

if(REDLYNX_WINDOWS)
target_include_directories(imgui_backend_win32 PUBLIC code/extern/imgui)
endif()

target_include_directories(imgui_backend_sdl3 PUBLIC code/extern/imgui)

if(USE_VULKAN)
target_include_directories(imgui_backend_vulkan PUBLIC code/extern/imgui)
endif()

if(USE_OPENGL)
target_include_directories(imgui_backend_opengl3 PUBLIC code/extern/imgui)
endif()

if(USE_D3D12)
target_include_directories(imgui_backend_dx12 PUBLIC code/extern/imgui)
endif()

if(USE_D3D11)
target_include_directories(imgui_backend_dx11 PUBLIC code/extern/imgui)
endif()

if(USE_D3D9)
target_include_directories(imgui_backend_dx9 PUBLIC code/extern/imgui)
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

add_library(shared STATIC
    code/shared/build_defines.hpp
    code/shared/util.hpp
    code/shared/lib.cpp
)

add_library(game STATIC
    code/game/track.cpp     code/game/track.hpp
    code/game/xur.cpp       code/game/xur.hpp
    code/game/xus.cpp       code/game/xus.hpp
)
target_link_libraries(game PRIVATE shared imgui)
