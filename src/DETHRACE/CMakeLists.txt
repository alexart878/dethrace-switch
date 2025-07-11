# Create object files so we can link them into the main binary and into tests without building twice.
add_library(dethrace_obj OBJECT)

option(DETHRACE_REPLAY_DEBUG "Debug replay" OFF)

target_include_directories(dethrace_obj
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        common
        pd
)

if (NINTENDO_SWITCH)

    target_include_directories(
        dethrace_obj
        PUBLIC
        ${DEVKITPRO}/libnx/include
        ${DEVKITPRO}/portlibs/switch/include
        ${DEVKITPRO}/portlibs/switch/include/SDL2
        ${DEVKITPRO}/devkitA64/aarch64-none-elf/include
    )

endif ()

if (DETHRACE_ASAN)
    target_compile_options(dethrace_obj PUBLIC -fsanitize=address)
    target_link_options(dethrace_obj PUBLIC -fsanitize=address)
endif()

if (NOT NINTENDO_SWITCH)
    target_link_libraries(dethrace_obj PUBLIC SDL2::SDL2 smackw32 harness BRender::Full BRender::DDI s3)
else()
    target_compile_options(dethrace_obj PRIVATE -march=armv8-a -mtune=cortex-a57 -O2 -ffunction-sections)
    target_link_libraries(dethrace_obj PUBLIC SDL2 smackw32 harness BRender::Full BRender::DDI s3)
endif()

if(MSVC)
    target_compile_definitions(dethrace_obj PRIVATE -D_CRT_SECURE_NO_WARNINGS)
    target_compile_options(dethrace_obj PRIVATE
        /wd4101
        /wd4996
    )
else()
    target_compile_options(dethrace_obj PRIVATE
        -Wall
    )
    add_compile_flags_if_supported(dethrace_obj
        -Wformat
        -Wno-unused-but-set-variable
        -Wno-unused-variable
        -Wno-unused-result
        -Wno-absolute-value
        -Wstrict-prototypes
    )
endif()
target_compile_definitions(dethrace_obj PRIVATE INSIDE_DETHRACE)
if(DETHRACE_FIX_BUGS)
    target_compile_definitions(dethrace_obj PRIVATE DETHRACE_FIX_BUGS)
endif()
if(DETHRACE_3DFX_PATCH)
    target_compile_definitions(dethrace_obj PRIVATE DETHRACE_3DFX_PATCH)
endif()

if(IS_BIGENDIAN)
    target_compile_definitions(dethrace_obj PRIVATE BR_ENDIAN_BIG=1)
else()
    target_compile_definitions(dethrace_obj PRIVATE BR_ENDIAN_LITTLE=1)
endif()

if(DETHRACE_REPLAY_DEBUG)
    target_compile_definitions(dethrace_obj PRIVATE DETHRACE_REPLAY_DEBUG)
endif()

target_sources(dethrace_obj PRIVATE
    common/brucetrk.c
    common/brucetrk.h
    common/car.c
    common/car.h
    common/controls.c
    common/controls.h
    common/crush.c
    common/crush.h
    common/cutscene.c
    common/cutscene.h
    common/demo.c
    common/demo.h
    common/depth.c
    common/depth.h
    common/displays.c
    common/displays.h
    common/drfile.c
    common/drfile.h
    common/drmem.c
    common/drmem.h
    common/errors.c
    common/errors.h
    common/finteray.c
    common/finteray.h
    common/flicplay.c
    common/flicplay.h
    common/globvars.c
    common/globvars.h
    common/globvrbm.c
    common/globvrbm.h
    common/globvrkm.c
    common/globvrkm.h
    common/globvrme.c
    common/globvrme.h
    common/globvrpb.c
    common/globvrpb.h
    common/grafdata.c
    common/grafdata.h
    common/graphics.c
    common/graphics.h
    common/init.c
    common/init.h
    common/input.c
    common/input.h
    common/intrface.c
    common/intrface.h
    common/loading.c
    common/loading.h
    common/loadsave.c
    common/loadsave.h
    common/main.c
    common/main.h
    common/mainloop.c
    common/mainloop.h
    common/mainmenu.c
    common/mainmenu.h
    common/netgame.c
    common/netgame.h
    common/network.c
    common/network.h
    common/newgame.c
    common/newgame.h
    common/oil.c
    common/oil.h
    common/oppocar.c
    common/oppocar.h
    common/opponent.c
    common/opponent.h
    common/oppoproc.c
    common/oppoproc.h
    common/options.c
    common/options.h
    common/pedestrn.c
    common/pedestrn.h
    common/piping.c
    common/piping.h
    common/powerup.c
    common/powerup.h
    common/pratcam.c
    common/pratcam.h
    common/racestrt.c
    common/racestrt.h
    common/racesumm.c
    common/racesumm.h
    common/raycast.c
    common/raycast.h
    common/replay.c
    common/replay.h
    common/skidmark.c
    common/skidmark.h
    common/sound.c
    common/sound.h
    common/spark.c
    common/spark.h
    common/structur.c
    common/structur.h
    common/trig.c
    common/trig.h
    common/utility.c
    common/utility.h
    common/world.c
    common/world.h
    constants.h
    dr_types.h
    pd/net.h
    pd/sys.h
    pc-win95/win95sys.c
    pc-win95/dinput.h
    pc-win95/ssdx.c
    pc-win95/ssdx.h
    pc-win95/win95net.c
    pc-dos/dosnet.c
    pc-dos/dossys.c
)

# Create our main game binary.
add_executable(dethrace
    WIN32
    ${CMAKE_SOURCE_DIR}/packaging/macos/dethrace.icns
    ${CMAKE_SOURCE_DIR}/packaging/windows/dethrace.rc
)

target_link_libraries(dethrace PRIVATE dethrace_obj compile_with_werror)
target_sources(dethrace PRIVATE main.c)

if(MSVC)
    target_link_libraries(dethrace PRIVATE dbghelp)
    target_link_options(dethrace PRIVATE /subsystem:windows /ENTRY:mainCRTStartup)
    target_compile_definitions(dethrace PRIVATE -D_CRT_SECURE_NO_WARNINGS -DSDL_MAIN_HANDLED -DWIN32_LEAN_AND_MEAN)
endif()

if(DETHRACE_IDE_ROOT_DIR)
    set_target_properties(dethrace PROPERTIES
        VS_DEBUGGER_ENVIRONMENT "DETHRACE_ROOT_DIR=${DETHRACE_IDE_ROOT_DIR}"
    )
endif()
if(DETHRACE_IDE_ARGUMENTS)
    message(STATUS "before: ${DETHRACE_IDE_ARGUMENTS}")
    separate_arguments(DETHRACE_IDE_ARGUMENTS NATIVE_COMMAND "${DETHRACE_IDE_ARGUMENTS}")
    message(STATUS "after: ${DETHRACE_IDE_ARGUMENTS}")
    set_target_properties(dethrace PROPERTIES
        VS_DEBUGGER_COMMAND_ARGUMENTS "${DETHRACE_IDE_ARGUMENTS}"
    )
endif()

if(DETHRACE_INSTALL)
    install(TARGETS dethrace
        BUNDLE DESTINATION . COMPONENT Runtime
        RUNTIME DESTINATION . COMPONENT Runtime
    )

    if(APPLE)
        set_target_properties(dethrace PROPERTIES
            BUNDLE True
            MACOSX_BUNDLE True
            MACOSX_BUNDLE_DISPLAY_NAME "Deth Race"
            OUTPUT_NAME "Deth Race"
            MACOSX_BUNDLE_BUNDLE_VERSION ${DETHRACE_VERSION}
            MACOSX_BUNDLE_SHORT_VERSION_STRING ${DETHRACE_VERSION}
            MACOSX_BUNDLE_ICON_FILE dethrace.icns
        )
        set_source_files_properties(${CMAKE_SOURCE_DIR}/packaging/macos/dethrace.icns PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources"
        )
        target_sources(dethrace_obj PRIVATE
            ${CMAKE_SOURCE_DIR}/packaging/macos/dethrace.icns
        )

        install(CODE "
            include(BundleUtilities)
            fixup_bundle(\"${CMAKE_BINARY_DIR}/Deth Race.app\" \"\" \"/Library/Frameworks\")
            execute_process(COMMAND
                codesign -s - -f \"${CMAKE_BINARY_DIR}/Deth Race.app/Contents/Frameworks/SDL2.framework\"
            )
            "
            COMPONENT RUNTIME
        )

    endif()

    if(MSVC)
        install(FILES $<TARGET_PDB_FILE:dethrace>
            DESTINATION "."
            OPTIONAL
        )
    endif()
    if(WIN32)
        install(FILES  $<TARGET_RUNTIME_DLLS:dethrace>
            DESTINATION "."
            OPTIONAL
        )
    endif()
endif()

if (NINTENDO_SWITCH)

    # create .nro target
    set(NRO_OUT "${CMAKE_BINARY_DIR}/dethrace.nro")
    set(NRO_ICON "${CMAKE_SOURCE_DIR}/packaging/carma_256.jpg")
    set(NACP_OUT "${CMAKE_BINARY_DIR}/dethrace.nacp")
    set(NACP_NAME "DethRaceNX")
    set(NACP_AUTHOR "alexart878")
    set(NACP_VERSION "1.1")

    if(NOT COMMAND nx_generate_nacp)
        message(FATAL_ERROR "The `nx_generate_nacp` cmake command is not available. Please use an appropriate Nintendo Switch toolchain.")
    endif()

    if(NOT COMMAND nx_create_nro)
        message(FATAL_ERROR "The `nx_create_nro` cmake command is not available. Please use an appropriate Nintendo Switch toolchain.")
    endif()

    nx_generate_nacp(
        ${NACP_OUT}
        NAME ${NACP_NAME}
        AUTHOR ${NACP_AUTHOR}
        VERSION ${NACP_VERSION}
    )

    nx_create_nro(
        dethrace
        OUTPUT ${NRO_OUT}
        ICON ${NRO_ICON}
        NACP ${NACP_OUT}
    )

endif ()
