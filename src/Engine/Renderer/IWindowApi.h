#pragma once

// we include this in our IWindowAPI for function access when we implement different window api's, our rendering api provider can call functions without needing to include both backends.
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vector>
#include <string>

#include "CApiTypes.h"
#include "gui/imgui_impl_glfw.h"
#include "gui/imgui_impl_vulkan.h"


class IWindowApi {
public:
    virtual ~IWindowApi() = default;

    std::string GetTitle() const;
    Size GetSize() const;
    WindowOptions GetWindowOptions() const;

    virtual void SetWindowOptions(WindowOptions windowOptions) = 0;
    virtual void SetTitle(std::string title) = 0;
    virtual void Resize(Size size) = 0;

    virtual void Create() = 0;
    virtual void Destroy() = 0;

    virtual void* GetNativeHandle() = 0;

    virtual bool ShouldClose() = 0;
    virtual void PollEvents() = 0;

    WindowApiProvider GetApiProvider() { return m_ApiProvider;};

    virtual void ImGuiInitForWindow(RenderBackend backend) = 0;
    virtual void ImGuiNewFrame() = 0;
    virtual std::vector<const char*> GetExtensionsVulkan() const = 0;
    virtual VkResult CreateVulkanSurface(VkInstance instance, VkSurfaceKHR* surface) = 0;

protected:
    WindowApiProvider m_ApiProvider;
    WindowOptions m_WindowOptions{};
};