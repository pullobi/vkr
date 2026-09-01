#include "CGlfwWindow.h"
#include "Engine/Renderer/CApiTypes.h"
#include "Engine/Renderer/gui/imgui_impl_glfw.h"
#include "Engine/Renderer/gui/imgui_impl_vulkan.h"
#include "Logger/Logger.h"
#include <cstddef>
#include <vulkan/vulkan_core.h>

void CGlfwWindow::SetWindowOptions(WindowOptions windowOptions) {
    SetTitle(windowOptions.title);
    Resize(windowOptions.size);
}

void CGlfwWindow::SetTitle(std::string title) {
    if (p_GlfwWindow)
        glfwSetWindowTitle(p_GlfwWindow, title.c_str());
    m_WindowOptions.title = title;
}

void CGlfwWindow::Resize(Size size) {
    if (p_GlfwWindow)
        glfwSetWindowSize(p_GlfwWindow, size.x, size.y);
    m_WindowOptions.size = size;
}


void CGlfwWindow::Create() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    if (p_GlfwWindow)
        return;

    p_GlfwWindow = glfwCreateWindow(
        m_WindowOptions.size.x,
        m_WindowOptions.size.y,
        m_WindowOptions.title.c_str(),
        nullptr,
        nullptr
    );
    
    m_ApiProvider = GLFW;
    glfwSetWindowUserPointer(p_GlfwWindow, this);

    glfwSetWindowSizeCallback(
    p_GlfwWindow,
    [](GLFWwindow* window, int w, int h)
    {
        auto* self = static_cast<CGlfwWindow*>(
            glfwGetWindowUserPointer(window)
        );

        self->m_WindowOptions.size.x = w;
        self->m_WindowOptions.size.y = h;
    }
);
    
}
std::vector<const char*> CGlfwWindow::GetExtensionsVulkan() const{
    uint32_t count = 0;
    const char* const* extensions =
        glfwGetRequiredInstanceExtensions(&count);
    return std::vector<const char*>(extensions, extensions + count);
};

VkResult CGlfwWindow::CreateVulkanSurface(VkInstance instance, VkSurfaceKHR* surface){
    VkResult res = glfwCreateWindowSurface(instance, p_GlfwWindow, nullptr, surface);
    
    return res;
}

void CGlfwWindow::Destroy() {
    if (p_GlfwWindow) {
        // VkSurfaceKHR* surface will automatically get publicly executed by CVulkanRenderer.cxx because they own it, not us.
        glfwDestroyWindow(p_GlfwWindow);
        p_GlfwWindow = nullptr;
    }
}

void* CGlfwWindow::GetNativeHandle(){
    return p_GlfwWindow;
}


bool CGlfwWindow::ShouldClose(){
    return glfwWindowShouldClose(p_GlfwWindow);
}

void CGlfwWindow::PollEvents(){
    glfwPollEvents();
}


void CGlfwWindow::ImGuiInitForWindow(RenderBackend backend)
{
    switch (backend)
    {
        case RenderBackend::Vulkan:
            ImGui_ImplGlfw_InitForVulkan(p_GlfwWindow, true);
            break;

        case RenderBackend::OpenGL:
            ImGui_ImplGlfw_InitForOpenGL(p_GlfwWindow, true);
            break;
        case RenderBackend::None:
            Logger().fatal("RenderBackend cannot be None if you want to implement ImGui");
            break;
    }
}

void CGlfwWindow::ImGuiNewFrame(){
    ImGui_ImplGlfw_NewFrame();
}