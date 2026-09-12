#pragma once


#include "Engine/Render/Input/CGlfwInput.h"
#define WINDOWAPI_GLFW

#include "../Api/IWindowApi.h"



#include <vulkan/vulkan.h>



class CGlfwWindow : public IWindowApi {
public:
    void SetWindowOptions(WindowOptions windowOptions) override;
    void SetTitle(std::string title) override;
    void Resize(Size size) override;

    void Create(RenderBackend renderBackend) override;
    void Destroy() override;

    void* GetNativeHandle() override;
    bool ShouldClose() override;

    void ImGuiInitForWindow(RenderBackend backend) override;
    void ImGuiImplWindowShutdown() override;
    void ImGuiNewFrame() override;

    IInputApi* GetInputManager() override;

    std::vector<const char*> GetExtensionsVulkan() const override;
    VkResult CreateVulkanSurface(VkInstance instance, VkSurfaceKHR* surface) override;
    void Update() override;
    float GetTime() override;
private:
    GlfwInputApi* m_inputManager;
    GLFWwindow   *p_GlfwWindow = nullptr;
};