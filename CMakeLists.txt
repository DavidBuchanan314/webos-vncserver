cmake_minimum_required(VERSION 2.8.9)
project (vramvnc C)

option(BUILD_SHARED_LIBS OFF)


### Load External Project
include(ExternalProject)
ExternalProject_Add(
  ### Target name
  LibVNCServer
  URL https://github.com/LibVNC/libvncserver/archive/refs/tags/LibVNCServer-0.9.13.tar.gz
  CMAKE_ARGS  "-DBUILD_SHARED_LIBS=OFF" "-DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}/prebuilt"
)

add_subdirectory(tv-native-apis)
add_subdirectory(prebuilt)

add_executable(vramvnc src/vramvnc.c src/uinput.c)
add_executable(gmvnc src/gmvnc.c src/uinput.c)
add_dependencies(gmvnc LibVNCServer)

target_link_libraries(vramvnc im vncserver pthread z png jpeg ssl crypto gm)
target_link_libraries(gmvnc im vncserver pthread z png jpeg ssl crypto gm)
