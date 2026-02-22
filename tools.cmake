#
# track_tool is used for converting track files to and from XML and or
# dumping the contents of them.
#
add_executable(track_tool
    code/tools/track_tool.cpp
)
target_link_libraries(track_tool PRIVATE shared game tinyxml2 liblzma ZLIB::ZLIB)

#
# xur_tool is used for converting Xbox User Interface Resource (.xur) files
# to and from XML for editing string tables, image paths, and UI metadata.
#
add_executable(xur_tool
    code/tools/xur_tool.cpp
)
target_link_libraries(xur_tool PRIVATE shared game tinyxml2)

#
# xus_tool is used for converting Xbox User Interface String (.xus) files
# to and from XML for editing localized string tables and property overrides.
#
add_executable(xus_tool
    code/tools/xus_tool.cpp
)
target_link_libraries(xus_tool PRIVATE shared game tinyxml2)
