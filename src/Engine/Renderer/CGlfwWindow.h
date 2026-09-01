#pragma once


#define WINDOWAPI_GLFW

#include "IWindowApi.h"
#include <vulkan/vulkan_core.h>

class CGlfwWindow : public IWindowApi {
public:
    void SetWindowOptions(WindowOptions windowOptions) override;
    void SetTitle(std::string title) override;
    void Resize(Size size) override;

    void Create() override;
    void Destroy() override;

    void* GetNativeHandle() override;
    bool ShouldClose() override;
    void PollEvents() override;

    void ImGuiInitForWindow(RenderBackend backend) override;
    void ImGuiNewFrame() override;

    std::vector<const char*> GetExtensionsVulkan() const override;
    VkResult CreateVulkanSurface(VkInstance instance, VkSurfaceKHR* surface) override;
private:
    GLFWwindow   *p_GlfwWindow = nullptr;
};