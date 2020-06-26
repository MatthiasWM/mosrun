cmake_minimum_required (VERSION 2.6)
project (mosrun)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-W -Wall -Wextra -pedantic)
endif()

source_group("Source Files\\musashi331" REGULAR_EXPRESSION "musashi331/*")

set(
    MOSRUN_SRCS
    names.cpp names.h
    main.cpp main.h
    log.cpp log.h
    memory.cpp memory.h
    breakpoints.cpp breakpoints.h
    fileio.cpp fileio.h
    resourcefork.cpp resourcefork.h
    cpu.cpp cpu.h
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

add_executable(mosrun           ${MOSRUN_SRCS} rsrc.cpp rsrc.h)
add_executable(AIFtoNTK         ${MOSRUN_SRCS} tools/rsrc_aiftontk.cpp)
add_executable(ARM6asm          ${MOSRUN_SRCS} tools/rsrc_arm6asm.cpp)
add_executable(ARM6c            ${MOSRUN_SRCS} tools/rsrc_arm6c.cpp)
add_executable(ARMCpp           ${MOSRUN_SRCS} tools/rsrc_armcpp.cpp)
add_executable(ARMLink          ${MOSRUN_SRCS} tools/rsrc_armlink.cpp)
add_executable(DumpAIF          ${MOSRUN_SRCS} tools/rsrc_dumpaif.cpp)
add_executable(DumpAOF          ${MOSRUN_SRCS} tools/rsrc_dumpaof.cpp)
add_executable(ProtocolGenTool  ${MOSRUN_SRCS} tools/rsrc_protocolgentool.cpp)
add_executable(Rex              ${MOSRUN_SRCS} tools/rsrc_rex.cpp)
add_executable(Packer           ${MOSRUN_SRCS} tools/rsrc_packer.cpp)

if(MSVC)
else()
    add_executable(DumpRex          tools/dumprex.cpp tools/relocatepkg.cpp)
    add_executable(BuildRex         tools/buildrex.cpp tools/relocatepkg.cpp)
endif()

install(TARGETS mosrun
    AIFtoNTK
    ARM6asm
    ARM6c
    ARMCpp
    ARMLink
    DumpAIF
    DumpAOF
    ProtocolGenTool
    Rex
    Packer
    RUNTIME DESTINATION bin)

add_custom_target(documentation SOURCES
    README.md
    BUILDING.md
    LICENSE
    scratchpad.txt
    )

