cmake_minimum_required(VERSION 3.16)
project(ShooterSpace CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include include/generators include/entities include/audio)

file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/generators/*.cpp"
    "src/entities/*.cpp"
    "src/audio/*.cpp"
)

if(DEFINED VITA OR DEFINED ENV{VITASDK})
    if(NOT DEFINED VITASDK)
        set(VITASDK $ENV{VITASDK})
    endif()
    message(STATUS "Configurando compilación para PS Vita (VitaSDK en ${VITASDK})")
    include("${VITASDK}/share/vita.cmake" REQUIRED)

    set(VITA_APP_NAME "Shooter Space")
    set(VITA_TITLEID  "SHTR00001")
    set(VITA_VERSION  "01.10")

    add_executable(${PROJECT_NAME} ${SOURCES})

    set(VITAGL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/vitagl)
    set(VITAGL_LIB ${VITAGL_DIR}/libvitaGL.a)

    set(VITASHARK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/vitashark)
    set(VITASHARK_LIB ${VITASHARK_DIR}/libvitashark.a)

    target_include_directories(${PROJECT_NAME} PRIVATE
        ${VITAGL_DIR}/source
        ${VITASHARK_DIR}/include
        ${VITASDK}/arm-vita-eabi/include/SDL2
    )

    # Cadena completa de dependencias de enlace para VitaSDK con vitaGL vendorizado
    target_link_libraries(${PROJECT_NAME}
        SDL2
        ${VITAGL_LIB}
        ${VITASHARK_LIB}
        ${VITAGL_LIB}
        SceShaccCgExt
        taihen_stub
        mathneon
        SceShaccCg_stub
        SceKernelDmacMgr_stub
        SceIme_stub
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
        SceTouch_stub
        SceHid_stub
        SceMotion_stub
        ScePower_stub
        SceProcessmgr_stub
        m
    )

    target_link_options(${PROJECT_NAME} PRIVATE -Wl,--allow-multiple-definition)

    vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME} UNSAFE)
    vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
        VERSION ${VITA_VERSION}
        NAME ${VITA_APP_NAME}
        FILE assets/PressStart2P-Regular.ttf PressStart2P-Regular.ttf
        FILE sce_sys/icon0.png sce_sys/icon0.png
        FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
        FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
        FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
    )

else()
    message(STATUS "Configurando compilación para Host (Desktop / SDL2)")

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)

    add_executable(${PROJECT_NAME} ${SOURCES})

    target_include_directories(${PROJECT_NAME} PRIVATE
        ${SDL2_INCLUDE_DIRS}
    )

    target_link_directories(${PROJECT_NAME} PRIVATE
        ${SDL2_LIBRARY_DIRS}
    )

    target_link_libraries(${PROJECT_NAME}
        ${SDL2_LIBRARIES}
    )

    # Rules self-check: build/selfcheck
    add_executable(selfcheck tests/selfcheck.cpp src/LevelManager.cpp src/entities/Ships.cpp)
endif()
