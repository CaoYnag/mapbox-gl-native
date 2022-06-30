add_executable(
    mbgl-cache
    ${PROJECT_SOURCE_DIR}/bin/cache.cpp
)

target_link_libraries(
    mbgl-cache
    PRIVATE
        Mapbox::Base
        Mapbox::Base::Extras::args
        mbgl-compiler-options
        mbgl-core
)

add_executable(
    mbgl-offline
    ${PROJECT_SOURCE_DIR}/bin/offline.cpp
)

target_link_libraries(
    mbgl-offline
    PRIVATE Mapbox::Base::Extras::args mbgl-compiler-options mbgl-core
)

add_executable(
    mbgl-render
    ${PROJECT_SOURCE_DIR}/bin/render.cpp
)

target_link_libraries(
    mbgl-render
    PRIVATE Mapbox::Base::Extras::args mbgl-compiler-options mbgl-core
)

add_executable(
    crender
    ${PROJECT_SOURCE_DIR}/bin/crender.cpp
)

target_link_libraries(
    crender
    PRIVATE Mapbox::Base::Extras::args mbgl-compiler-options mbgl-core
)

add_executable(
    unpack_glyph_pbf
    ${PROJECT_SOURCE_DIR}/bin/unpack_glyph_pbf.cpp
)

target_include_directories(
    unpack_glyph_pbf
    PRIVATE ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(
    unpack_glyph_pbf
    PRIVATE Mapbox::Base::Extras::args mbgl-compiler-options mbgl-core
)

install(TARGETS mbgl-offline mbgl-render RUNTIME DESTINATION bin)

# FIXME: CI must have a valid token
#
# add_test(NAME mbgl-offline-tool-test COMMAND mbgl-offline -s mapbox://styles/mapbox/satellite-v9 --maxZoom=0 WORKING_DIRECTORY
# ${PROJECT_SOURCE_DIR} )
#
# add_test(NAME mbgl-render-tool-test COMMAND mbgl-render WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} )
