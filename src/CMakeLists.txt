cmake_minimum_required(VERSION 2.8.9)
project (vramvnc)


add_executable(vramvnc vramvnc.c uinput.c)

# TODO: what's the proper way of doing this?

SET_TARGET_PROPERTIES(vramvnc PROPERTIES LINK_FLAGS -Wl,-L../libvncserver/build/)
target_link_libraries(vramvnc im vncserver pthread z jpeg png gcrypt gnutls ssl crypto)
