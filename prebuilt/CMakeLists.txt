include(ExternalProject)

find_package(PkgConfig REQUIRED)
pkg_check_modules(JPEG REQUIRED IMPORTED_TARGET libjpeg)
pkg_check_modules(ZLIB REQUIRED IMPORTED_TARGET zlib)

# FIXME this seems fairly wrong - probably due to old cmake or some other
# Yocto/NDK-related bugs these need to be manually passed in :/
ExternalProject_Add(
  LibVNCServer
  URL https://github.com/LibVNC/libvncserver/archive/0b8990784a81823eecc9089263ac1966d17bf399.tar.gz
  CMAKE_ARGS  "-DWITH_TESTS=OFF" "-DWITH_EXAMPLES=OFF" "-DBUILD_SHARED_LIBS=OFF" "-DWITH_SYSTEMD=OFF"
  "-DJPEG_LIBRARY=${JPEG_LIBRARIES}" "-DJPEG_INCLUDE_DIR=$ENV{PKG_CONFIG_SYSROOT_DIR}/${JPEG_INCLUDEDIR}"
  "-DZLIB_LIBRARY=${ZLIB_LIBRARIES}" "-DZLIB_INCLUDE_DIR=$ENV{PKG_CONFIG_SYSROOT_DIR}/${ZLIB_INCLUDEDIR}"
)

add_library(vncserver STATIC IMPORTED GLOBAL)

set_target_properties(vncserver PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES      ${CMAKE_CURRENT_SOURCE_DIR}/include
    IMPORTED_LOCATION                  ${CMAKE_CURRENT_SOURCE_DIR}/lib/libvncserver.a
)

add_dependencies(vncserver LibVNCServer)
