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
