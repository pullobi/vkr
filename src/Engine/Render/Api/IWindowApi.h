#pragma once

#include "Engine/Render/Api/IInputApi.h"

// Include them anyway for the compile to shut up about vulkan stuff
// #ifdef ENGINE_RENDER_USE_VULKAN
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
// #endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <vector>
#include <string>

#include "../Api/CApiTypes.h"



class IWindowApi {
public:
    virtual ~IWindowApi() = default;

    std::string GetTitle() const;
    Size GetSize() const;
    WindowOptions GetWindowOptions() const;

    virtual void SetWindowOptions(WindowOptions windowOptions) = 0;
    virtual void SetTitle(std::string title) = 0;
    virtual void Resize(Size size) = 0;

    virtual void Create(RenderBackend renderBackend) = 0;
    virtual void Destroy() = 0;

    virtual void* GetNativeHandle() = 0;

    virtual bool ShouldClose() = 0;

    WindowBackend GetApiProvider() { return m_ApiProvider;};

    virtual void ImGuiInitForWindow(RenderBackend backend) = 0;
    virtual void ImGuiImplWindowShutdown() = 0;
    virtual void ImGuiNewFrame() = 0;
    

    virtual std::vector<const char*> GetExtensionsVulkan() const = 0;
    virtual VkResult CreateVulkanSurface(VkInstance instance, VkSurfaceKHR* surface) = 0;
    VkExtent2D getSwapchainExtent(); // compat
    
    virtual IInputApi* GetInputManager() = 0;


    virtual void Update() = 0;
    virtual float GetTime() = 0;
protected:
    WindowBackend m_ApiProvider;
    WindowOptions m_WindowOptions{};
};

IWindowApi* GetGlobalWindowApi();
void SetGlobalWindowApi(IWindowApi*);