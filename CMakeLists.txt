cmake_minimum_required(VERSION 3.28)

project(KX-Trainer-Free VERSION 3.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RC_FLAGS "${CMAKE_RC_FLAGS} /I\"${CMAKE_SOURCE_DIR}/src\"")

if(NOT WIN32)
    message(FATAL_ERROR "KX-Trainer-Free builds only on Windows/MSVC.")
endif()
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "KX-Trainer-Free is x64 only.")
endif()

get_filename_component(_kx_cl_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)
get_filename_component(_kx_host "${_kx_cl_dir}" DIRECTORY)
get_filename_component(_kx_bin "${_kx_host}" DIRECTORY)
get_filename_component(_kx_msvc "${_kx_bin}" DIRECTORY)
set(_kx_msvc_lib "${_kx_msvc}/lib/x64")

get_filename_component(_kx_rc_dir "${CMAKE_RC_COMPILER}" DIRECTORY)
get_filename_component(_kx_kit_ver_dir "${_kx_rc_dir}" DIRECTORY)
get_filename_component(_kx_kit_ver "${_kx_kit_ver_dir}" NAME)
get_filename_component(_kx_kit_bin "${_kx_kit_ver_dir}" DIRECTORY)
get_filename_component(_kx_kit "${_kx_kit_bin}" DIRECTORY)
set(_kx_um "${_kx_kit}/Lib/${_kx_kit_ver}/um/x64")
set(_kx_ucrt "${_kx_kit}/Lib/${_kx_kit_ver}/ucrt/x64")

if(NOT EXISTS "${_kx_um}/d3d11.lib")
    message(FATAL_ERROR "Windows SDK um/x64 is missing d3d11.lib (${_kx_um})")
endif()

add_link_options(
    "/LIBPATH:${_kx_msvc_lib}"
    "/LIBPATH:${_kx_um}"
    "/LIBPATH:${_kx_ucrt}")

set(_kx_out "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>")

add_library(KX-Trainer-Free SHARED
    src/dllmain.cpp
    src/gui_style.cpp
    src/internal/app.cpp
    src/internal/imgui_game_wndproc.cpp
    src/internal/hooks.cpp
    src/internal/overlay.cpp
    src/log.cpp
    src/memory.cpp
    src/hack_gui.cpp
    src/imgui/imgui.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/imgui_impl_dx11.cpp
    src/imgui/imgui_impl_win32.cpp
    src/imgui/imgui_tables.cpp
    src/imgui/imgui_widgets.cpp
    src/config.cpp
    src/hack.cpp
    src/key_chord.cpp
    src/version_gate.cpp
    src/internal/minhook/src/buffer.c
    src/internal/minhook/src/hook.c
    src/internal/minhook/src/trampoline.c
    src/internal/minhook/src/hde/hde64.c
    src/KX-Trainer-Free.rc)

target_include_directories(KX-Trainer-Free PRIVATE
    src
    src/internal/minhook/include)

target_compile_definitions(KX-Trainer-Free PRIVATE
    UNICODE
    _UNICODE
    _WINDOWS
    _USRDLL
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    $<$<CONFIG:Release>:_CRT_SECURE_NO_WARNINGS>)

target_link_libraries(KX-Trainer-Free PRIVATE d3d11 dxgi d3dcompiler)

set_target_properties(KX-Trainer-Free PROPERTIES
    OUTPUT_NAME "KX-Trainer-Free"
    CXX_SCAN_FOR_MODULES OFF
    RUNTIME_OUTPUT_DIRECTORY "${_kx_out}"
    ARCHIVE_OUTPUT_DIRECTORY "${_kx_out}"
    LIBRARY_OUTPUT_DIRECTORY "${_kx_out}"
    PDB_OUTPUT_DIRECTORY "${_kx_out}")

add_executable(KXTrainerInject injector/main.cpp)
target_include_directories(KXTrainerInject PRIVATE src)
target_compile_definitions(KXTrainerInject PRIVATE
    UNICODE
    _UNICODE
    _CONSOLE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>)
set_target_properties(KXTrainerInject PROPERTIES
    OUTPUT_NAME "KXTrainerInject"
    CXX_SCAN_FOR_MODULES OFF
    RUNTIME_OUTPUT_DIRECTORY "${_kx_out}"
    ARCHIVE_OUTPUT_DIRECTORY "${_kx_out}"
    PDB_OUTPUT_DIRECTORY "${_kx_out}")
add_dependencies(KXTrainerInject KX-Trainer-Free)

if(MSVC)
    target_compile_options(KX-Trainer-Free PRIVATE /W3 /permissive- /utf-8 /sdl)
    target_compile_options(KXTrainerInject PRIVATE /W3 /permissive- /utf-8 /sdl)
    target_link_options(KX-Trainer-Free PRIVATE
        /PDBALTPATH:KX-Trainer-Free.pdb
        /SUBSYSTEM:WINDOWS)
    target_link_options(KXTrainerInject PRIVATE /SUBSYSTEM:CONSOLE)
endif()
