include(FetchContent)
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

# Pins match the Vulkan 1.3.296 SDK; the runtime baseline remains Vulkan 1.1.
FetchContent_Declare(vulkan_headers
    URL https://codeload.github.com/KhronosGroup/Vulkan-Headers/tar.gz/1d9bcc9af77d93ba355b15994b9f82a130e9df3a
    URL_HASH SHA256=a99524ad2fe7defded00b27f3bd4b84eaeb11f78c611f016eb51daa526a1bad2)
FetchContent_MakeAvailable(vulkan_headers)
set(VULKAN_HEADERS_INSTALL_DIR "${vulkan_headers_SOURCE_DIR}")
set(VOLK_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(volk
    URL https://codeload.github.com/zeux/volk/tar.gz/59d26900f53c7621a8ba8ab0e3f18d3bd883fa9a
    URL_HASH SHA256=9f51d3cec07b899a9f87150c3276d5495a607fc38f16048e935cd9287eedbc76)
FetchContent_MakeAvailable(volk)

find_package(SDL2 2.0.6 CONFIG QUIET)
if(NOT TARGET SDL2::SDL2 AND NOT TARGET SDL2::SDL2-static)
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    set(SDL_TEST OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(sdl2
        URL https://codeload.github.com/libsdl-org/SDL/tar.gz/5d249570393f7a37e037abf22cd6012a4cc56a71
        URL_HASH SHA256=10f1194f8d2e4a73ca1c7c553b3189d0b68ab9ef3544e8d2a268e4429456b373)
    FetchContent_MakeAvailable(sdl2)
endif()
if(TARGET SDL2::SDL2-static)
    set(machines_sdl_target SDL2::SDL2-static)
else()
    set(machines_sdl_target SDL2::SDL2)
endif()

add_library(machines_vulkan STATIC context.cpp)
target_compile_features(machines_vulkan PUBLIC cxx_std_11)
target_compile_definitions(machines_vulkan PUBLIC SDL_MAIN_HANDLED)
target_include_directories(machines_vulkan PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(machines_vulkan PUBLIC volk::volk ${machines_sdl_target})
if(MSVC)
    target_compile_options(machines_vulkan PRIVATE /W4)
else()
    target_compile_options(machines_vulkan PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(machines-vulkan-smoke ../../../projects/vulkan-smoke/main.cpp)
target_link_libraries(machines-vulkan-smoke PRIVATE machines_vulkan)
if(MINGW)
    target_link_options(machines-vulkan-smoke PRIVATE -static-libgcc -static-libstdc++ -static)
endif()
if(WIN32 AND TARGET SDL2::SDL2 AND NOT TARGET SDL2::SDL2-static)
    add_custom_command(TARGET machines-vulkan-smoke POST_BUILD COMMAND ${CMAKE_COMMAND} -E
        copy_if_different $<TARGET_FILE:SDL2::SDL2> $<TARGET_FILE_DIR:machines-vulkan-smoke>)
endif()

option(MACHINES_VULKAN_BUILD_SHADERS "Compile the migrated game shaders to SPIR-V" ON)
if(MACHINES_VULKAN_BUILD_SHADERS)
    find_program(GLSLANG_VALIDATOR glslangValidator HINTS "$ENV{VULKAN_SDK}/Bin" "$ENV{VULKAN_SDK}/bin")
    if(NOT GLSLANG_VALIDATOR)
        message(FATAL_ERROR "Install glslang-tools or the Vulkan SDK, or explicitly disable MACHINES_VULKAN_BUILD_SHADERS for presentation-only testing.")
    endif()
    set(shader_outputs)
    foreach(shader gui.vert textured.frag world.vert world.frag billboard.vert)
        set(source "${PROJECT_SOURCE_DIR}/data/shaders/vulkan/${shader}")
        set(output "${CMAKE_CURRENT_BINARY_DIR}/shaders/${shader}.spv")
        add_custom_command(OUTPUT "${output}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/shaders"
            COMMAND "${GLSLANG_VALIDATOR}" -V --target-env vulkan1.1 "${source}" -o "${output}"
            DEPENDS "${source}" VERBATIM)
        list(APPEND shader_outputs "${output}")
    endforeach()
    add_custom_target(machines-vulkan-shaders ALL DEPENDS ${shader_outputs})
    find_program(SPIRV_VAL spirv-val HINTS "$ENV{VULKAN_SDK}/Bin" "$ENV{VULKAN_SDK}/bin")
    if(SPIRV_VAL)
        foreach(output IN LISTS shader_outputs)
            get_filename_component(shader_name "${output}" NAME)
            add_test(NAME "vulkan-spirv-${shader_name}"
                COMMAND "${SPIRV_VAL}" --target-env vulkan1.1 "${output}")
        endforeach()
    endif()
    foreach(vertex gui world billboard)
        if(vertex STREQUAL "world")
            set(fragment world)
        else()
            set(fragment textured)
        endif()
        add_test(NAME "vulkan-shader-interface-${vertex}"
            COMMAND "${GLSLANG_VALIDATOR}" -V --target-env vulkan1.1 -l
                "${PROJECT_SOURCE_DIR}/data/shaders/vulkan/${vertex}.vert"
                "${PROJECT_SOURCE_DIR}/data/shaders/vulkan/${fragment}.frag"
                -o "${CMAKE_CURRENT_BINARY_DIR}/shaders/${vertex}-linked.spv")
    endforeach()
endif()

add_test(NAME vulkan-cli-help COMMAND machines-vulkan-smoke --help)
add_test(NAME vulkan-cli-invalid COMMAND machines-vulkan-smoke --frames 0)
set_tests_properties(vulkan-cli-invalid PROPERTIES WILL_FAIL TRUE)
# Requires a desktop and Vulkan driver; opt in explicitly when testing presentation.
option(MACHINES_VULKAN_GPU_TESTS "Register the GPU presentation/resize test" OFF)
if(MACHINES_VULKAN_GPU_TESTS)
    add_test(NAME vulkan-presentation COMMAND machines-vulkan-smoke --frames 120 --exercise-window --validation)
    set_tests_properties(vulkan-presentation PROPERTIES TIMEOUT 60)
endif()

