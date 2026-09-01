cmake_minimum_required(VERSION 3.21)

project(vktest LANGUAGES C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# For clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# -----------------------
# Dependencies
# -----------------------

find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)


# -----------------------
# Dear ImGui
# -----------------------

set(IMGUI_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
)

set(IMGUI_DEST
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Engine/Renderer/gui
)

set(IMGUI_FILES
    imgui.cpp
    imgui.h
    imgui_draw.cpp
    imgui_internal.h
    imgui_tables.cpp
    imgui_widgets.cpp
    imconfig.h
    imstb_rectpack.h
    imstb_textedit.h
    imstb_truetype.h

    backends/imgui_impl_glfw.cpp
    backends/imgui_impl_glfw.h
    backends/imgui_impl_vulkan.cpp
    backends/imgui_impl_vulkan.h
)

foreach(IMGUI_FILE ${IMGUI_FILES})

    get_filename_component(IMGUI_FILE_NAME
        ${IMGUI_FILE}
        NAME
    )

    configure_file(
        ${IMGUI_DIR}/${IMGUI_FILE}
        ${IMGUI_DEST}/${IMGUI_FILE_NAME}
        COPYONLY
    )

endforeach()


# -----------------------
# Source files
# -----------------------

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/*.c
    src/*.cc
    src/*.cpp
    src/*.cxx
    src/*.c++
)

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
    src/*.h
    src/*.hpp
)

add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Vulkan::Vulkan
        glfw
)


# -----------------------
# Shaders
# -----------------------

find_program(SLANGC slangc REQUIRED)

file(GLOB_RECURSE SHADERS CONFIGURE_DEPENDS
    assets/shaders/*.slang
)

set(SHADER_OUTPUT_DIR
    ${CMAKE_BINARY_DIR}/assets/shaders
)

foreach(SHADER ${SHADERS})

    get_filename_component(SHADER_NAME ${SHADER} NAME_WE)

    add_custom_command(
        OUTPUT
            ${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv

        COMMAND
            ${CMAKE_COMMAND} -E make_directory
            ${SHADER_OUTPUT_DIR}

        COMMAND
            ${SLANGC}
            ${SHADER}
            -target spirv
            -o
            ${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv

        DEPENDS
            ${SHADER}

        COMMENT
            "Compiling Slang shader ${SHADER_NAME}"
    )

    list(APPEND SPV_SHADERS
        ${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv
    )

endforeach()

add_custom_target(shaders ALL
    DEPENDS ${SPV_SHADERS}
)

add_dependencies(${PROJECT_NAME} shaders)