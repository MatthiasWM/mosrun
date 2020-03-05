cmake_minimum_required (VERSION 2.6)
project (mosrun)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-W -Wall -Wextra -pedantic)
endif()

source_group("Source Files\\musashi331" REGULAR_EXPRESSION "musashi331/*")

add_executable(mosrun
    names.cpp names.h
    main.cpp main.h
    log.cpp log.h
    memory.cpp memory.h
    breakpoints.cpp breakpoints.h
    fileio.cpp fileio.h
    resourcefork.cpp resourcefork.h
    cpu.cpp cpu.h
    rsrc.cpp rsrc.h
    traps.cpp traps.h
    filename.cpp filename.h
    systemram.cpp systemram.h
    ./musashi331/m68kops.c
    ./musashi331/m68kopac.c
    ./musashi331/m68kopnz.c
    ./musashi331/m68kdasm.c
    ./musashi331/m68kcpu.c
    ./musashi331/m68kopdm.c
    debug_break.h
    )
install(TARGETS mosrun RUNTIME DESTINATION bin)


add_executable(mosmake
    ./mosmake/mosmake.cpp
    )

add_custom_target(documentation SOURCES
    README.md
    BUILDING.md
    LICENSE
    scratchpad.txt
    )

