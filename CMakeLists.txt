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

# SDL2's Vita backend calls OpenGL-style functions supplied by vitaGL.
# Keep vitaGL after SDL2 so the static linker can resolve those symbols.
target_link_libraries(${PROJECT_NAME}
    SDL2
    vitaGL
    SceGxm_stub
    SceDisplay_stub
    SceCtrl_stub
    SceAppMgr_stub
    SceAppUtil_stub
    SceAudio_stub
    SceAudioIn_stub
    SceSysmodule_stub
    SceIofilemgr_stub
    SceCommonDialog_stub
    SceIme_stub
    SceTouch_stub
    SceHid_stub
    SceMotion_stub
    ScePower_stub
    SceProcessmgr_stub
    m
    pthread
)

vita_create_self(eboot.bin ${PROJECT_NAME} UNSAFE)
vita_create_vpk(NewChaos.vpk ${VITA_TITLEID} eboot.bin
    VERSION ${VITA_VERSION}
    NAME "New Chaos"
)
