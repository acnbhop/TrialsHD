#
# track_tool is used for converting track files to and from XML and or
# dumping the contents of them.
#
add_executable(track_tool
    code/tools/track_tool.cpp
)
target_link_libraries(track_tool PRIVATE shared game tinyxml2 liblzma ZLIB::ZLIB)
