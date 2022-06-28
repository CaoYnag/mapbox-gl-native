if(TARGET mbgl-vendor-sdf)
    return()
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(FT REQUIRED freetype2)
include_directories(${FT_INCLUDE_DIRS})
link_directories(${FT_LIBRARY_DIRS})

add_library(
    mbgl-vendor-sdf INTERFACE
)

target_include_directories(
    mbgl-vendor-sdf SYSTEM
    INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/sdf-glyph-foundry/include
)

target_link_libraries(
    mbgl-vendor-sdf
    INTERFACE ${FT_LIBRARIES}
)

set_target_properties(
    mbgl-vendor-sdf
    PROPERTIES
        INTERFACE_MAPBOX_NAME "sdf-glyph-foundry"
        INTERFACE_MAPBOX_URL "https://github.com/mapbox/sdf-glyph-foundry"
        INTERFACE_MAPBOX_AUTHOR "Mapbox"
        INTERFACE_MAPBOX_LICENSE ${CMAKE_CURRENT_LIST_DIR}/sdf-glyph-foundry/LICENSE.md
)
