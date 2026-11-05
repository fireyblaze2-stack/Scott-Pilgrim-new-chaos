cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake"
        CACHE PATH "VitaSDK toolchain")
endif()

project(NewChaos LANGUAGES C CXX)

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

# Leave room after the executable code segment for Vita module metadata.
# Keep writable data and C/C++ constructor arrays in later memory segments.
target_link_options(${PROJECT_NAME} PRIVATE
    "-Wl,-Tdata=0x81200000"
    "-Wl,--section-start=.init_array=0x81430000"
    "-Wl,--section-start=.fini_array=0x81430020"
)

# Modern vitaGL is a C++ static library with runtime shader compiler helpers.
# Static-library order matters: dependencies follow the libraries that use them.
target_link_libraries(${PROJECT_NAME}
    SDL2
    vitaGL
    vitashark
    mathneon
    SceShaccCgExt
    SceShaccCg_stub
    SceKernelDmacMgr_stub
    taihen_stub
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
    pthread
    m
    stdc++
)

vita_create_self(eboot.bin ${PROJECT_NAME} UNSAFE)
set(VPK_LIVEAREA_FILES assets/scott_idle.bmp)
foreach(asset
    sce_sys/icon0.png
    sce_sys/livearea/contents/bg.png
    sce_sys/livearea/contents/template.xml
)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${asset}")
        list(APPEND VPK_LIVEAREA_FILES "${asset}")
    endif()
endforeach()

vita_create_vpk(NewChaos.vpk ${VITA_TITLEID} eboot.bin
    VERSION ${VITA_VERSION}
    NAME "New Chaos"
    FILES ${VPK_LIVEAREA_FILES}
)
