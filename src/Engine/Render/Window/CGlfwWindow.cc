

#include "CGlfwWindow.h"
#include "Engine/Render/Api/CApiTypes.h"
#include "Engine/Render/Input/CGlfwInput.h"
#include "Engine/Render/Api/IInputApi.h"
#include "Engine/Render/gui/imgui_impl_glfw.h"
#include "Engine/const.h"
#include "Logger/Logger.h"
#include "glad/glad.h"
#include <cstddef>



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


void CGlfwWindow::Create(RenderBackend renderBackend) {
    // 1. Check if window already exists FIRST to prevent initialization leaks
    if (p_GlfwWindow)
        return;

    glfwInit();


    switch (renderBackend) {
        case RenderBackend::Vulkan:
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            break;
        case RenderBackend::OpenGL:
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
            break;
        default:
            break;
    }

    p_GlfwWindow = glfwCreateWindow(
        m_WindowOptions.size.x,
        m_WindowOptions.size.y,
        m_WindowOptions.title.c_str(),
        nullptr,
        nullptr
    );
    
    // Check if window creation actually succeeded
    if (!p_GlfwWindow) {
        // Handle initialization error (e.g., throw or log)
        glfwTerminate();
        return;
    }

    // 3. OpenGL requires making the context current on this thread
    if (renderBackend == RenderBackend::OpenGL)
        glfwMakeContextCurrent(p_GlfwWindow);


    m_ApiProvider = WindowBackend::GLFW;
    glfwSetWindowUserPointer(p_GlfwWindow, this);

    glfwSetWindowSizeCallback(
        p_GlfwWindow,
        [](GLFWwindow* window, int w, int h) {
            auto* self = static_cast<CGlfwWindow*>(glfwGetWindowUserPointer(window));
            if (self) {
                self->m_WindowOptions.size.x = w;
                self->m_WindowOptions.size.y = h;
            }
            self->SetWindowOptions(self->m_WindowOptions);
        }
    );

    // TODO: Consider using std::unique_ptr instead of raw 'new' to prevent memory leaks
    m_inputManager = new GlfwInputApi(p_GlfwWindow);
    SetGlobalInputApi(m_inputManager);
}

std::vector<const char*> CGlfwWindow::GetExtensionsVulkan() const{
    uint32_t count = 0;
    const char* const* extensions =
        glfwGetRequiredInstanceExtensions(&count);
    return std::vector<const char*>(extensions, extensions + count);
}

VkResult CGlfwWindow::CreateVulkanSurface(VkInstance instance, VkSurfaceKHR* surface){
    VkResult res = glfwCreateWindowSurface(instance, p_GlfwWindow, nullptr, surface);
    return res;
}

void CGlfwWindow::Destroy() {
    delete m_inputManager;
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

void CGlfwWindow::Update(){
    glfwPollEvents();
    m_inputManager->Update();
}


void CGlfwWindow::ImGuiInitForWindow(RenderBackend backend)
{
    switch (backend)
    {
        case RenderBackend::Vulkan:
            ImGui_ImplGlfw_InitForVulkan(p_GlfwWindow, false);
            break;

        case RenderBackend::OpenGL:
            ImGui_ImplGlfw_InitForOpenGL(p_GlfwWindow, false);
            break;
        case RenderBackend::None:
            Logger().fatal("RenderBackend cannot be None if you want to implement ImGui");
            break;
    }
}

void CGlfwWindow::ImGuiNewFrame(){
    ImGui_ImplGlfw_NewFrame();
}

void CGlfwWindow::ImGuiImplWindowShutdown(){
    ImGui_ImplGlfw_Shutdown();
}

IInputApi* CGlfwWindow::GetInputManager(){
    return (IInputApi*) m_inputManager;
}

float CGlfwWindow::GetTime(){
    return glfwGetTime();
}

