cmake_minimum_required (VERSION 2.6)
project (mosrun)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

source_group("Source Files\\musashi331" REGULAR_EXPRESSION "musashi331/*")

add_executable(mosrun
    ./names.cpp
    ./main.cpp
    ./log.cpp
    ./memory.cpp
    ./breakpoints.cpp
    ./fileio.cpp
    ./resourcefork.cpp
    ./cpu.cpp
    ./rsrc.cpp
    ./traps.cpp
    ./filename.cpp
    ./systemram.cpp
    ./MosResource.cpp
    ./musashi331/m68kops.c
    ./musashi331/m68kopac.c
    ./musashi331/m68kopnz.c
    ./musashi331/m68kdasm.c
    ./musashi331/m68kcpu.c
    ./musashi331/m68kopdm.c
    )
install(TARGETS mosrun RUNTIME DESTINATION bin)

add_executable(mosmake
    ./mosmake/mosmake.cpp
    )

add_custom_target(documentation SOURCES
    README.md
    LICENSE
    scratchpad.txt
    )

