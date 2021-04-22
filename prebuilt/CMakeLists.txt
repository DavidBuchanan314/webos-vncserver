add_library(vncserver STATIC IMPORTED GLOBAL)

set_target_properties(vncserver PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES      ${CMAKE_CURRENT_SOURCE_DIR}/include
    IMPORTED_LOCATION                  ${CMAKE_CURRENT_SOURCE_DIR}/lib/libvncserver.a
)