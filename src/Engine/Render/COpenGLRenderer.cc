#include "COpenGLRenderer.h"
#include "Engine/Render/Api/IWindowApi.h"
#include "COpenGLRenderer.h"
#include "Engine/Render/Api/Tools.h"
#include "Logger/Logger.h"
#include "glad/include/glad/glad.h"
#include "COpenGLRenderer.h"
#include "Engine/Render/Api/Tools.h"
#include <GLFW/glfw3.h>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <string>
#include <vulkan/vulkan_core.h>

#include "gui/imgui_impl_opengl3.h"
#include "gui/imgui_impl_glfw.h"


void COpenGLRenderer::Init(IWindowApi* Window)
{
    assert(Window);

    p_Window = Window;

    auto* glfwWindow = (GLFWwindow*)p_Window->GetNativeHandle();

    LOGGER_ASSERT(glfwWindow != NULL, "expression");

    glfwMakeContextCurrent(glfwWindow);


    if (!gladLoadGLLoader(
        (GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return;
    }


    std::cout << "OpenGL Vendor:   "
              << reinterpret_cast<const char*>(glGetString(GL_VENDOR))
              << '\n';

    std::cout << "OpenGL Renderer: "
              << reinterpret_cast<const char*>(glGetString(GL_RENDERER))
              << '\n';

    std::cout << "OpenGL Version:  "
              << reinterpret_cast<const char*>(glGetString(GL_VERSION))
              << '\n';

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    glBlendFunc(
        GL_SRC_ALPHA,
        GL_ONE_MINUS_SRC_ALPHA
    );

    int framebufferWidth;
    int framebufferHeight;

    glfwGetFramebufferSize(
        glfwWindow,
        &framebufferWidth,
        &framebufferHeight
    );

    m_Width = framebufferWidth;
    m_Height = framebufferHeight;

    glViewport(
        0,
        0,
        framebufferWidth,
        framebufferHeight
    );

    m_ShaderProgram = CreateShaderProgram();

    if (!m_ShaderProgram)
    {
        std::cerr << "Failed to create OpenGL shader program\n";
        return;
    }

    CreateBuffers();
    SetupVertexAttributes();

    glGenBuffers(1, &m_CameraUBO);

    glBindBuffer(GL_UNIFORM_BUFFER, m_CameraUBO);

    glBufferData(
        GL_UNIFORM_BUFFER,
        sizeof(glm::mat4) * 3,
        nullptr,
        GL_DYNAMIC_DRAW
    );

    glBindBufferBase(
        GL_UNIFORM_BUFFER,
        0,
        m_CameraUBO
    );

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    
    ImGuiInit();

    m_Initialized = true;
}

void COpenGLRenderer::ImGuiInit()
{
    IMGUI_CHECKVERSION();
    p_ImGuiContext = ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();

    // Initialize the GLFW/platform side.
    // Do this once.
    p_Window->ImGuiInitForWindow(getRenderBackend());

    // Initialize the OpenGL renderer backend.
    ImGui_ImplOpenGL3_Init("#version 410");

    ImGui::StyleColorsDark();
}

void COpenGLRenderer::RenderBegin()
{
    if (!m_Initialized)
        return;

    GLFWwindow* glfwWindow =
        static_cast<GLFWwindow*>(p_Window->GetNativeHandle());

    int framebufferWidth;
    int framebufferHeight;

    glfwGetFramebufferSize(
        glfwWindow,
        &framebufferWidth,
        &framebufferHeight
    );

    if (m_Width != framebufferWidth || m_Height != framebufferHeight)
    {
        m_Width = framebufferWidth;
        m_Height = framebufferHeight;

        glViewport(
            0,
            0,
            framebufferWidth,
            framebufferHeight
        );
    }

    glClearColor(
        0.05f,
        0.05f,
        0.05f,
        1.0f
    );

    glClear(
        GL_COLOR_BUFFER_BIT |
        GL_DEPTH_BUFFER_BIT
    );
}


void COpenGLRenderer::Draw(
    const Vertex* vertices,
    uint32_t nVerts,
    const uint32_t* indices,
    uint32_t nIndices
)
{
    if (!m_Initialized)
        return;

    if (!vertices || !indices || nVerts == 0 || nIndices == 0)
        return;

    glUseProgram(m_ShaderProgram);

    UpdateCameraBuffer();

    glBindVertexArray(m_VAO);

    glBindBuffer(
        GL_ARRAY_BUFFER,
        m_VBO
    );

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(Vertex) * nVerts,
        vertices,
        GL_DYNAMIC_DRAW
    );

    glBindBuffer(
        GL_ELEMENT_ARRAY_BUFFER,
        m_EBO
    );

    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        sizeof(uint32_t) * nIndices,
        indices,
        GL_DYNAMIC_DRAW
    );

    glDrawElements(
        GL_TRIANGLES,
        static_cast<GLsizei>(nIndices),
        GL_UNSIGNED_INT,
        nullptr
    );

    glBindVertexArray(0);

    
}


void COpenGLRenderer::ImGuiNewFrame()
{
    if (!m_Initialized)
        return;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();
}


void COpenGLRenderer::ImGuiRender()
{
    if (!m_Initialized)
        return;

    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(
        ImGui::GetDrawData()
    );
}


void COpenGLRenderer::RenderEnd()
{
    if (!m_Initialized)
        return;

    auto* glfwWindow =
        static_cast<GLFWwindow*>(p_Window->GetNativeHandle());
    glfwSwapBuffers(glfwWindow);
}


void COpenGLRenderer::Shutdown()
{
    if (!m_Initialized)
        return;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    if (m_CameraUBO)
    {
        glDeleteBuffers(1, &m_CameraUBO);
        m_CameraUBO = 0;
    }

    if (m_EBO)
    {
        glDeleteBuffers(1, &m_EBO);
        m_EBO = 0;
    }

    if (m_VBO)
    {
        glDeleteBuffers(1, &m_VBO);
        m_VBO = 0;
    }

    if (m_VAO)
    {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }

    if (m_ShaderProgram)
    {
        glDeleteProgram(m_ShaderProgram);
        m_ShaderProgram = 0;
    }

    m_Initialized = false;
}


RenderBackend COpenGLRenderer::getRenderBackend()
{
    return RenderBackend::OpenGL;
}


VkExtent2D COpenGLRenderer::getSwapchainExtent()
{
    VkExtent2D extent{};

    extent.width = m_Width;
    extent.height = m_Height;

    return extent;
}

void COpenGLRenderer::UpdateCameraBuffer()
{
    if (!m_CameraUBO)
        return;

    CameraUBO& camera = GetCameraUBO();

    glBindBuffer(GL_UNIFORM_BUFFER, m_CameraUBO);

    glBufferSubData(
        GL_UNIFORM_BUFFER,
        0,
        sizeof(glm::mat4),
        &camera.model
    );

    glBufferSubData(
        GL_UNIFORM_BUFFER,
        sizeof(glm::mat4),
        sizeof(glm::mat4),
        &camera.view
    );

    glBufferSubData(
        GL_UNIFORM_BUFFER,
        sizeof(glm::mat4) * 2,
        sizeof(glm::mat4),
        &camera.projection
    );

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


GLuint COpenGLRenderer::CompileShader(
    GLenum type,
    const char* source
)
{
    GLuint shader = glCreateShader(type);

    glShaderSource(
        shader,
        1,
        &source,
        nullptr
    );

    glCompileShader(shader);

    GLint success = GL_FALSE;

    glGetShaderiv(
        shader,
        GL_COMPILE_STATUS,
        &success
    );

    if (!success)
    {
        GLint logLength = 0;

        glGetShaderiv(
            shader,
            GL_INFO_LOG_LENGTH,
            &logLength
        );

        std::string log(logLength, '\0');

        glGetShaderInfoLog(
            shader,
            logLength,
            nullptr,
            log.data()
        );

        Logger().error("OpenGL Shader Compilation failed on shader with contents\n{}\n://{}", source, log);

        glDeleteShader(shader);

        return 0;
    }

    return shader;
}


GLuint COpenGLRenderer::CreateShaderProgram()
{
    const auto vertexSource =
        ReadFileGL("assets/shaders/tri.vert.glsl");

    const auto fragmentSource =
        ReadFileGL("assets/shaders/tri.frag.glsl");

    if (vertexSource.empty() || fragmentSource.empty())
    {
        std::cerr << "Failed to read OpenGL shader files\n";
        return 0;
    }

    GLuint vertexShader = CompileShader(
        GL_VERTEX_SHADER,
        vertexSource.data()
    );

    GLuint fragmentShader = CompileShader(
        GL_FRAGMENT_SHADER,
        fragmentSource.data()
    );

    if (!vertexShader || !fragmentShader)
    {
        if (vertexShader)
            glDeleteShader(vertexShader);

        if (fragmentShader)
            glDeleteShader(fragmentShader);

        return 0;
    }

    GLuint program = glCreateProgram();

    glAttachShader(
        program,
        vertexShader
    );

    glAttachShader(
        program,
        fragmentShader
    );

    glLinkProgram(program);

    GLuint blockIndex = glGetUniformBlockIndex(program, "CameraUBO_std140");

    if (blockIndex == GL_INVALID_INDEX)
    {
        std::cerr << "CameraUBO_std140 not found in shader\n";
    }
    else
    {
        glUniformBlockBinding(program, blockIndex, 0);
    }

    GLint success = GL_FALSE;

    glGetProgramiv(
        program,
        GL_LINK_STATUS,
        &success
    );

    if (!success)
    {
        GLint logLength = 0;

        glGetProgramiv(
            program,
            GL_INFO_LOG_LENGTH,
            &logLength
        );

        std::string log(logLength, '\0');

        glGetProgramInfoLog(
            program,
            logLength,
            nullptr,
            log.data()
        );

        std::cerr
            << "OpenGL shader program linking failed:\n"
            << log
            << '\n';

        glDeleteProgram(program);

        program = 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}


void COpenGLRenderer::CreateBuffers()
{
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);
}


void COpenGLRenderer::SetupVertexAttributes()
{
    glBindVertexArray(m_VAO);

    glBindBuffer(
        GL_ARRAY_BUFFER,
        m_VBO
    );

    glBindBuffer(
        GL_ELEMENT_ARRAY_BUFFER,
        m_EBO
    );

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        reinterpret_cast<void*>(
            offsetof(Vertex, position)
        )
    );

    glEnableVertexAttribArray(1);

    glVertexAttribPointer(
        1,
        4,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        reinterpret_cast<void*>(
            offsetof(Vertex, color)
        )
    );

    glEnableVertexAttribArray(2);

    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        reinterpret_cast<void*>(
            offsetof(Vertex, uv)
        )
    );

    glEnableVertexAttribArray(3);

    glVertexAttribPointer(
        3,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        reinterpret_cast<void*>(
            offsetof(Vertex, normal)
        )
    );

    glBindVertexArray(0);
}


void COpenGLRenderer::UpdateCameraUniforms()
{
    UpdateCameraBuffer();
}