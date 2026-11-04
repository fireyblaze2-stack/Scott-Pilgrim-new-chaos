cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake"
        CACHE PATH "VitaSDK toolchain")
endif()

project(NewChaos C)

if(NOT DEFINED ENV{VITASDK})
    message(FATAL_ERROR "VITASDK is not set. Install VitaSDK and export VITASDK.")
endif()

include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_TITLEID "NCHA00001")
set(VITA_VERSION "01.00")

add_executable(${PROJECT_NAME}
    src/main.c
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -O2
    -Wall
    -Wextra
)

# SDL2 is provided by: vdpm install sdl2
target_link_libraries(${PROJECT_NAME}
    SDL2
    SceCtrl_stub
    SceDisplay_stub
    SceSysmodule_stub
    m
)

vita_create_self(eboot.bin ${PROJECT_NAME} UNSAFE)
vita_create_vpk(NewChaos.vpk ${VITA_TITLEID} eboot.bin
    VERSION ${VITA_VERSION}
    NAME "New Chaos"
)
