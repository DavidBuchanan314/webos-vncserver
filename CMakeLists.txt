cmake_minimum_required(VERSION 2.8.9)
project (vramvnc C)

option(BUILD_SHARED_LIBS OFF)

add_subdirectory(prebuilt)
add_subdirectory(tv-native-apis)

add_executable(vramvnc src/vramvnc.c src/uinput.c)
add_executable(gmvnc src/gmvnc.c src/uinput.c)

target_link_libraries(vramvnc im vncserver pthread z png jpeg ssl crypto gm)
target_link_libraries(gmvnc im vncserver pthread z png jpeg ssl crypto gm)
