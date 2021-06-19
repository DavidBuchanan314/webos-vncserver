include(ExternalProject)

ExternalProject_Add(
  LibVNCServer
  URL https://github.com/LibVNC/libvncserver/archive/refs/tags/LibVNCServer-0.9.13.tar.gz
  CMAKE_ARGS  "-DBUILD_SHARED_LIBS=OFF" "-DWITH_SYSTEMD=OFF" "-DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}"
)

add_library(vncserver STATIC IMPORTED GLOBAL)

set_target_properties(vncserver PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES      ${CMAKE_CURRENT_SOURCE_DIR}/include
    IMPORTED_LOCATION                  ${CMAKE_CURRENT_SOURCE_DIR}/lib/libvncserver.a
)

add_dependencies(vncserver LibVNCServer)