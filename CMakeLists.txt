cmake_minimum_required(VERSION 3.21)

project(vkr LANGUAGES C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# For clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# ============================================================
# Engine Configuration
# ============================================================

set(ENGINE_WINDOWING "glfw" CACHE STRING
    "Windowing backend (glfw, sdl)"
)

set(ENGINE_RENDERING "vulkan" CACHE STRING
    "Rendering backend (vulkan, opengl)"
)

string(TOLOWER "${ENGINE_WINDOWING}" ENGINE_WINDOWING)
string(TOLOWER "${ENGINE_RENDERING}" ENGINE_RENDERING)


# ============================================================
# Dependencies
# ============================================================

# ------------------------------------------------------------
# Windowing
# ------------------------------------------------------------

find_package(glfw3 CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)


# ------------------------------------------------------------
# Rendering
# ------------------------------------------------------------

find_package(Vulkan REQUIRED)

if(APPLE)

    find_library(
        OPENGL_FRAMEWORK
        OpenGL
        REQUIRED
    )

elseif(WIN32)

    # OpenGL is provided by opengl32.dll.

elseif(UNIX)

    find_package(OpenGL REQUIRED)

endif()


# ============================================================
# Source Files
# ============================================================

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


# ============================================================
# Executable
# ============================================================

add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)


# ============================================================
# Include Directories
# ============================================================

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)


# ============================================================
# Windowing Backend
# ============================================================

if(ENGINE_WINDOWING STREQUAL "glfw")

    target_compile_definitions(${PROJECT_NAME}
        PRIVATE
            ENGINE_WINDOW_USE_GLFW
    )

elseif(ENGINE_WINDOWING STREQUAL "sdl")

    target_compile_definitions(${PROJECT_NAME}
        PRIVATE
            ENGINE_WINDOW_USE_SDL
    )

else()

    message(FATAL_ERROR
        "Unknown windowing backend: ${ENGINE_WINDOWING}"
    )

endif()


# ============================================================
# Rendering Backend
# ============================================================

if(ENGINE_RENDERING STREQUAL "vulkan")

    target_compile_definitions(${PROJECT_NAME}
        PRIVATE
            ENGINE_RENDER_USE_VULKAN
    )

elseif(ENGINE_RENDERING STREQUAL "opengl")

    target_compile_definitions(${PROJECT_NAME}
        PRIVATE
            ENGINE_RENDER_USE_OPENGL
    )

else()

    message(FATAL_ERROR
        "Unknown rendering backend: ${ENGINE_RENDERING}"
    )

endif()


# ============================================================
# GLAD
# ============================================================

set(GLAD_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad
)

set(GLAD_DEST
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Engine/Render/glad
)

file(COPY
    ${GLAD_DIR}/
    DESTINATION ${GLAD_DEST}
)

file(GLOB_RECURSE GLAD_SOURCES CONFIGURE_DEPENDS
    ${GLAD_DEST}/*.c
    ${GLAD_DEST}/*.cc
    ${GLAD_DEST}/*.cpp
    ${GLAD_DEST}/*.cxx
)

target_sources(${PROJECT_NAME}
    PRIVATE
        ${GLAD_SOURCES}
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${GLAD_DEST}/include
)


# ============================================================
# Dear ImGui
# ============================================================

set(IMGUI_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui
)

set(IMGUI_DEST
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Engine/Render/gui
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

    backends/imgui_impl_sdl3.cpp
    backends/imgui_impl_sdl3.h

    backends/imgui_impl_opengl3.cpp
    backends/imgui_impl_opengl3.h
    backends/imgui_impl_opengl3_loader.h
    
    backends/imgui_impl_vulkan.cpp
    backends/imgui_impl_vulkan.h
)

foreach(IMGUI_FILE ${IMGUI_FILES})

    get_filename_component(
        IMGUI_FILE_NAME
        ${IMGUI_FILE}
        NAME
    )

    configure_file(
        ${IMGUI_DIR}/${IMGUI_FILE}
        ${IMGUI_DEST}/${IMGUI_FILE_NAME}
        COPYONLY
    )

endforeach()


# ============================================================
# ImGui Configuration
# ============================================================

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        IMGUI_IMPL_OPENGL_LOADER_GLAD
)


# ============================================================
# Link Libraries
# ============================================================

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        glfw
        SDL3::SDL3
        Vulkan::Vulkan
)


# ============================================================
# OpenGL
# ============================================================

if(APPLE)

    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            ${OPENGL_FRAMEWORK}
    )

elseif(WIN32)

    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            opengl32

    )

elseif(UNIX)

    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            OpenGL::GL
    )

endif()

# ============================================================
# Compile Shaders
# ============================================================

find_program(SLANGC slangc REQUIRED)
find_program(SPIRV_CROSS spirv-cross REQUIRED)

set(SHADER_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders
)

set(SHADER_OUTPUT_DIR
    ${CMAKE_CURRENT_BINARY_DIR}/assets/shaders
)

file(GLOB_RECURSE SHADERS CONFIGURE_DEPENDS
    "${SHADER_DIR}/*.slang"
)

set(COMPILED_SHADERS)

foreach(SHADER ${SHADERS})

    get_filename_component(
        SHADER_NAME
        "${SHADER}"
        NAME_WE
    )

    # --------------------------------------------------------
    # One SPIR-V file
    # --------------------------------------------------------

    set(SPV_OUTPUT
        "${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv"
    )

    add_custom_command(
        OUTPUT
            "${SPV_OUTPUT}"

        COMMAND
            ${CMAKE_COMMAND}
            -E
            make_directory
            "${SHADER_OUTPUT_DIR}"

        COMMAND
            "${SLANGC}"
            "${SHADER}"
            -matrix-layout-column-major
            -target spirv
            -entry vertexMain
            -entry fragmentMain
            -o "${SPV_OUTPUT}"

        DEPENDS
            "${SHADER}"

        COMMENT
            "Compiling Slang shader: ${SHADER_NAME} -> SPIR-V"
    )

    list(APPEND
        COMPILED_SHADERS
        "${SPV_OUTPUT}"
    )

    # --------------------------------------------------------
    # Existing GLSL?
    # --------------------------------------------------------

    set(EXISTING_VERT_GLSL
        "${SHADER_DIR}/${SHADER_NAME}.vert.glsl"
    )

    set(EXISTING_FRAG_GLSL
        "${SHADER_DIR}/${SHADER_NAME}.frag.glsl"
    )

    if(EXISTS "${EXISTING_VERT_GLSL}" AND
       EXISTS "${EXISTING_FRAG_GLSL}")

        message(STATUS
            "Using existing GLSL for ${SHADER_NAME}"
        )

        # ----------------------------------------------------
        # Copy existing vertex GLSL
        # ----------------------------------------------------

        set(GLSL_VERT_OUTPUT
            "${SHADER_OUTPUT_DIR}/${SHADER_NAME}.vert.glsl"
        )

        add_custom_command(
            OUTPUT
                "${GLSL_VERT_OUTPUT}"

            COMMAND
                ${CMAKE_COMMAND}
                -E
                make_directory
                "${SHADER_OUTPUT_DIR}"

            COMMAND
                ${CMAKE_COMMAND}
                -E
                copy_if_different
                "${EXISTING_VERT_GLSL}"
                "${GLSL_VERT_OUTPUT}"

            DEPENDS
                "${EXISTING_VERT_GLSL}"

            COMMENT
                "Copying existing ${SHADER_NAME}.vert.glsl"
        )

        list(APPEND
            COMPILED_SHADERS
            "${GLSL_VERT_OUTPUT}"
        )

        # ----------------------------------------------------
        # Copy existing fragment GLSL
        # ----------------------------------------------------

        set(GLSL_FRAG_OUTPUT
            "${SHADER_OUTPUT_DIR}/${SHADER_NAME}.frag.glsl"
        )

        add_custom_command(
            OUTPUT
                "${GLSL_FRAG_OUTPUT}"

            COMMAND
                ${CMAKE_COMMAND}
                -E
                make_directory
                "${SHADER_OUTPUT_DIR}"

            COMMAND
                ${CMAKE_COMMAND}
                -E
                copy_if_different
                "${EXISTING_FRAG_GLSL}"
                "${GLSL_FRAG_OUTPUT}"

            DEPENDS
                "${EXISTING_FRAG_GLSL}"

            COMMENT
                "Copying existing ${SHADER_NAME}.frag.glsl"
        )

        list(APPEND
            COMPILED_SHADERS
            "${GLSL_FRAG_OUTPUT}"
        )

    else()

        # ----------------------------------------------------
        # No existing GLSL -> generate with SPIRV-Cross
        # ----------------------------------------------------

        set(STAGES
            vertexMain
            fragmentMain
        )

        set(STAGE_EXTS
            vert
            frag
        )

        list(LENGTH STAGES STAGE_COUNT)
        math(EXPR MAX_IDX "${STAGE_COUNT} - 1")

        foreach(IDX RANGE ${MAX_IDX})

            list(GET STAGES ${IDX} ENTRY_POINT)
            list(GET STAGE_EXTS ${IDX} STAGE_EXT)

            set(GLSL_OUTPUT
                "${SHADER_OUTPUT_DIR}/${SHADER_NAME}.${STAGE_EXT}.glsl"
            )

            add_custom_command(
                OUTPUT
                    "${GLSL_OUTPUT}"

                COMMAND
                    ${SPIRV_CROSS}
                    --version 410
                    --entry "${ENTRY_POINT}"
                    "${SPV_OUTPUT}"
                    --output "${GLSL_OUTPUT}"

                DEPENDS
                    "${SPV_OUTPUT}"

                COMMENT
                    "Transpiling ${SHADER_NAME} [${ENTRY_POINT}] -> GLSL 410"
            )

            list(APPEND
                COMPILED_SHADERS
                "${GLSL_OUTPUT}"
            )

        endforeach()

    endif()

endforeach()


# ------------------------------------------------------------
# Shader target
# ------------------------------------------------------------

add_custom_target(shaders ALL
    DEPENDS
        ${COMPILED_SHADERS}
)

add_dependencies(${PROJECT_NAME}
    shaders
)