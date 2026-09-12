#pragma once
#include "Engine/Render/Api/IRenderApi.h"
#include "Engine/Render/gui/imgui_internal.h"
#include "glad/include/glad/glad.h"
class COpenGLRenderer : public IRenderApi
{
public:
    void Init(IWindowApi* Window) override;

    void RenderBegin() override;

    void Draw(
        const Vertex* vertices,
        uint32_t nVerts,
        const uint32_t* indices,
        uint32_t nIndices
    ) override;

    void ImGuiNewFrame() override;
    void ImGuiRender() override;

    void RenderEnd() override;
    void Shutdown() override;

    RenderBackend getRenderBackend() override;
    void UpdateCameraBuffer() override;


    VkExtent2D getSwapchainExtent() override;

private:
    IWindowApi* p_Window = nullptr;

    GLuint m_ShaderProgram = 0;

    GLuint m_VAO = 0;
    GLuint m_VBO = 0;
    GLuint m_EBO = 0;

    GLuint m_CameraUBO = 0;


    uint32_t m_Width = 0;
    uint32_t m_Height = 0;

    bool m_Initialized = false;

    GLuint CompileShader(
        GLenum type,
        const char* source
    );

    GLuint CreateShaderProgram();

    void CreateBuffers();
    void SetupVertexAttributes();

    void UpdateCameraUniforms();

    void ImGuiInit();
    ImGuiContext* p_ImGuiContext;
};