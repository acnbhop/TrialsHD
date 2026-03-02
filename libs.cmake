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

add_library(shared STATIC
    code/shared/build_defines.hpp
    code/shared/util.hpp
    code/shared/lib.cpp
)

add_library(game STATIC
    #
    # Asset system
    #
    code/game/asset/actor.cpp           code/game/asset/actor.hpp
    code/game/asset/anim.cpp            code/game/asset/anim.hpp
    code/game/asset/font.cpp            code/game/asset/font.hpp
    code/game/asset/gfx.cpp             code/game/asset/gfx.hpp
    code/game/asset/model.cpp           code/game/asset/model.hpp
    code/game/asset/package.cpp         code/game/asset/package.hpp
    code/game/asset/shader.cpp          code/game/asset/shader.hpp
    code/game/asset/texture.cpp         code/game/asset/texture.hpp
    code/game/asset/track.cpp           code/game/asset/track.hpp
    code/game/asset/video.cpp           code/game/asset/video.hpp
    code/game/asset/xgs.cpp             code/game/asset/xgs.hpp
    code/game/asset/xml.cpp             code/game/asset/xml.hpp
    code/game/asset/xsb.cpp             code/game/asset/xsb.hpp
    code/game/asset/xur.cpp             code/game/asset/xur.hpp
    code/game/asset/xus.cpp             code/game/asset/xus.hpp
    code/game/asset/xwb.cpp             code/game/asset/xwb.hpp
    #
    # Assets (XML)
    #
    code/game/xmldata/actorinfo.cpp     code/game/xmldata/actorinfo.hpp
    code/game/xmldata/bike_config.cpp   code/game/xmldata/bike_config.hpp
    code/game/xmldata/challenges.cpp    code/game/xmldata/challenges.hpp
    code/game/xmldata/game_config.cpp   code/game/xmldata/game_config.hpp
    code/game/xmldata/gear.cpp          code/game/xmldata/gear.hpp
    code/game/xmldata/gfx.cpp           code/game/xmldata/gfx.hpp
    code/game/xmldata/object.cpp        code/game/xmldata/object.hpp
    code/game/xmldata/objectlist.cpp    code/game/xmldata/objectlist.hpp
    code/game/xmldata/pack.cpp          code/game/xmldata/pack.hpp
    code/game/xmldata/trackset.cpp      code/game/xmldata/trackset.hpp
)
target_link_libraries(game PRIVATE shared imgui)
target_compile_definitions(game PUBLIC LZMA_API_STATIC)
