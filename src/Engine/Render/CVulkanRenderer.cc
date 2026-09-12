#include "CVulkanRenderer.h"
#include "Engine/Render/Types/CRenderTypes.h"
#include "Engine/Render/gui/imgui_impl_glfw.h"
#include "Engine/Render/gui/imgui_impl_vulkan.h"
#include <glm/ext/vector_float2.hpp>
#include <set>
#include <vulkan/vulkan_core.h>

static CameraUBO m_CameraUBO {};

CameraUBO& GetCameraUBO(){
    return m_CameraUBO;
}

void CVulkanRenderer::Init(IWindowApi* window)
{
    LOGGER_ASSERT(window != nullptr, "window is invalid");

    p_Window = window;

    CreateInstance();
    CreateSurface();
    SelectDevice();
    FindQueueFamilies();
    CreateDevice();

    CreateCommandPool();

    CreateSwapchain();
    CreateImageViews();

    CreateDepthResources();

    CreateRenderPass();
    CreateFramebuffers();


    // -----------------------
    // Camera
    // -----------------------

    CreateCameraBuffer();
    CreateCameraDescriptorSetLayout();
    CreateDescriptorPool();
    CreateCameraDescriptorSet();


    // -----------------------
    // Shaders
    // -----------------------

    auto shaderCode = ReadFileVk(
        "assets/shaders/tri.spv"
    );
    m_VkShaderModule =
        CreateShaderModule(shaderCode);
    Logger().info("Shader module created");

    CreateGraphicsPipeline();
    CreateCommandBuffers();
    CreateSyncObjects();

    ImGuiInit();

    m_CameraUBO.model = glm::mat4(1.0f);
    
    m_CameraUBO.view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 3.0f), // camera position
        glm::vec3(0.0f, 0.0f, 0.0f), // looking at
        glm::vec3(0.0f, 1.0f, 0.0f)  // up
    );
    
    m_CameraUBO.projection = glm::perspective(
        glm::radians(60.0f),
        static_cast<float>(m_SwapchainExtent.width) /
            static_cast<float>(m_SwapchainExtent.height),
        0.1f,
        100.0f
    );
    
    // Vulkan's Y axis is opposite to OpenGL's.
    m_CameraUBO.projection[1][1] *= -1.0f;
    
    UpdateCameraBuffer();
}

void CVulkanRenderer::ImGuiInit()
{
    IMGUI_CHECKVERSION();
    m_ImGuiContext = ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    

    // Initialize the GLFW/platform side.
    // Do this once.
    p_Window->ImGuiInitForWindow(getRenderBackend());
    // Expands to ImGui_ImplGlfw_InitForVulkan(...)

    ImGui_ImplVulkan_InitInfo initInfo{};

    initInfo.ApiVersion = VK_API_VERSION_1_3;

    initInfo.Instance = m_VkInstance;
    initInfo.PhysicalDevice = m_VkPhysicalDevice;
    initInfo.Device = m_VkDevice;

    initInfo.QueueFamily = m_GraphicsQueueFamily;
    initInfo.Queue = m_GraphicsQueue;

    initInfo.DescriptorPoolSize = 1000;

    initInfo.MinImageCount =
        static_cast<uint32_t>(m_SwapchainImages.size());

    initInfo.ImageCount =
        static_cast<uint32_t>(m_SwapchainImages.size());

    initInfo.PipelineInfoMain.RenderPass = m_VkRenderPass;
    initInfo.PipelineInfoMain.Subpass = 0;

    ImGui_ImplVulkan_Init(&initInfo);
    ImGui::StyleColorsDark();
}

void CVulkanRenderer::RenderBegin()
{
    // Wait until the previous frame has finished.
    vkWaitForFences(
        m_VkDevice,
        1,
        &m_InFlightFence,
        VK_TRUE,
        UINT64_MAX
    );

    vkResetFences(
        m_VkDevice,
        1,
        &m_InFlightFence
    );

    UpdateCameraBuffer();

    // Get the next swapchain image.
    VkResult result = vkAcquireNextImageKHR(
        m_VkDevice,
        m_VkSwapchain,
        UINT64_MAX,
        m_ImageAvailableSemaphore,
        VK_NULL_HANDLE,
        &m_CurrentImageIndex
    );

    LOGGER_ASSERT(
        result == VK_SUCCESS ||
        result == VK_SUBOPTIMAL_KHR,
        "Failed to acquire swapchain image"
    );

    m_CurrentCommandBuffer =
        m_CommandBuffers[m_CurrentImageIndex];

    // Reset command buffer.
    result = vkResetCommandBuffer(
        m_CurrentCommandBuffer,
        0
    );

    LOGGER_ASSERT(
        result == VK_SUCCESS,
        "Failed to reset command buffer"
    );

    // Begin recording.
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    
    // Optimization: Tells Vulkan this buffer is re-recorded and submitted once per frame loop
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    result = vkBeginCommandBuffer(
        m_CurrentCommandBuffer,
        &beginInfo
    );

    LOGGER_ASSERT(
        result == VK_SUCCESS,
        "Failed to begin command buffer"
    );

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_VkRenderPass;
    renderPassInfo.framebuffer = m_SwapchainFramebuffers[m_CurrentImageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_SwapchainExtent;

    // Define clear properties for both color and depth layers
    // Note: If you do NOT use a depth buffer, reduce this array size to 1
    std::array<VkClearValue, 2> clearValues{};
    
    // Attachment 0: Color clear (R, G, B, A) - Alpha changed to 1.0f
    clearValues[0].color = {{
    0.0f,
    0.0f,
    0.0f,
    0.0f
    }};
    
    // Attachment 1: Depth/Stencil clear (1.0f = Max depth distance background)
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(
        m_CurrentCommandBuffer,
        &renderPassInfo,
        VK_SUBPASS_CONTENTS_INLINE
    );
}


void CVulkanRenderer::RenderEnd()
{
    // Finish the render pass.
    vkCmdEndRenderPass(
        m_CurrentCommandBuffer
    );

    // Finish recording the command buffer.
    VkResult result = vkEndCommandBuffer(
        m_CurrentCommandBuffer
    );

    LOGGER_ASSERT(
        result == VK_SUCCESS,
        "Failed to record command buffer"
    );
        VkSemaphore waitSemaphores[] =
    {
        m_ImageAvailableSemaphore
    };

    VkPipelineStageFlags waitStages[] =
    {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    VkSemaphore signalSemaphores[] =
    {
        m_RenderFinishedSemaphore
    };

    VkSubmitInfo submitInfo{};

    submitInfo.sType =
        VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.waitSemaphoreCount = 1;

    submitInfo.pWaitSemaphores =
        waitSemaphores;

    submitInfo.pWaitDstStageMask =
        waitStages;

    submitInfo.commandBufferCount = 1;

    submitInfo.pCommandBuffers =
        &m_CurrentCommandBuffer;

    submitInfo.signalSemaphoreCount = 1;

    submitInfo.pSignalSemaphores =
        signalSemaphores;

    result = vkQueueSubmit(
        m_GraphicsQueue,
        1,
        &submitInfo,
        m_InFlightFence
    );

    LOGGER_ASSERT(
        result == VK_SUCCESS,
        "Failed to submit draw command"
    );
        VkSwapchainKHR swapchains[] =
    {
        m_VkSwapchain
    };

    VkPresentInfoKHR presentInfo{};

    presentInfo.sType =
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;

    presentInfo.pWaitSemaphores =
        signalSemaphores;

    presentInfo.swapchainCount = 1;

    presentInfo.pSwapchains =
        swapchains;

    presentInfo.pImageIndices =
        &m_CurrentImageIndex;

    LOGGER_ASSERT(
        m_PresentQueue != VK_NULL_HANDLE,
    "m_PresentQueue is VK_NULL_HANDLE"
    );
    LOGGER_ASSERT(
        m_VkDevice != VK_NULL_HANDLE,
        "m_VkDevice is VK_NULL_HANDLE"
    );

    LOGGER_ASSERT(
        m_VkSwapchain != VK_NULL_HANDLE,
        "m_VkSwapchain is VK_NULL_HANDLE"
    );
    result = vkQueuePresentKHR(
        m_PresentQueue, 
        &presentInfo
    );

    LOGGER_ASSERT(
        result == VK_SUCCESS ||
        result == VK_SUBOPTIMAL_KHR,
        "Failed to present swapchain image"
    );
}

void CVulkanRenderer::Shutdown()
{
    // Make sure the GPU has finished using our resources.
    if (m_VkDevice != VK_NULL_HANDLE)
    {
        vkDeviceWaitIdle(m_VkDevice);
    }

    ImGui_ImplVulkan_Shutdown();
    p_Window->ImGuiImplWindowShutdown();
    ImGui::DestroyContext(m_ImGuiContext);
    
    // Synchronization
    

    if (m_ImageAvailableSemaphore != VK_NULL_HANDLE)
    {
        vkDestroySemaphore(
            m_VkDevice,
            m_ImageAvailableSemaphore,
            nullptr
        );

        m_ImageAvailableSemaphore = VK_NULL_HANDLE;
    }

    if (m_RenderFinishedSemaphore != VK_NULL_HANDLE)
    {
        vkDestroySemaphore(
            m_VkDevice,
            m_RenderFinishedSemaphore,
            nullptr
        );

        m_RenderFinishedSemaphore = VK_NULL_HANDLE;
    }

    if (m_InFlightFence != VK_NULL_HANDLE)
    {
        vkDestroyFence(
            m_VkDevice,
            m_InFlightFence,
            nullptr
        );

        m_InFlightFence = VK_NULL_HANDLE;
    }

    
    // Vertex buffer
    

    if (m_VkVertexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(
            m_VkDevice,
            m_VkVertexBuffer,
            nullptr
        );

        m_VkVertexBuffer = VK_NULL_HANDLE;
    }

    if (m_VkVertexBufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(
            m_VkDevice,
            m_VkVertexBufferMemory,
            nullptr
        );

        m_VkVertexBufferMemory = VK_NULL_HANDLE;
    }

    // Index buffer
    

    if (m_VkIndexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(
            m_VkDevice,
            m_VkIndexBuffer,
            nullptr
        );

        m_VkIndexBuffer = VK_NULL_HANDLE;
    }

    if (m_VkIndexBufferMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(
            m_VkDevice,
            m_VkIndexBufferMemory,
            nullptr
        );

        m_VkIndexBufferMemory = VK_NULL_HANDLE;
    }

    // Framebuffers

    for (VkFramebuffer framebuffer : m_SwapchainFramebuffers)
    {
        if (framebuffer != VK_NULL_HANDLE)
        {
            vkDestroyFramebuffer(
                m_VkDevice,
                framebuffer,
                nullptr
            );
        }
    }

    m_SwapchainFramebuffers.clear();

    // Graphics pipeline

    if (m_VkGraphicsPipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(
            m_VkDevice,
            m_VkGraphicsPipeline,
            nullptr
        );

        m_VkGraphicsPipeline = VK_NULL_HANDLE;
    }

    // Pipeline layout

    if (m_VkPipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(
            m_VkDevice,
            m_VkPipelineLayout,
            nullptr
        );

        m_VkPipelineLayout = VK_NULL_HANDLE;
    }

    
    // Shader module

    if (m_VkShaderModule != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(
            m_VkDevice,
            m_VkShaderModule,
            nullptr
        );

        m_VkShaderModule = VK_NULL_HANDLE;
    }

    // Render pass

    if (m_VkRenderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(
            m_VkDevice,
            m_VkRenderPass,
            nullptr
        );

        m_VkRenderPass = VK_NULL_HANDLE;
    }
    // Swapchain image views

    for (VkImageView imageView : m_SwapchainImageViews)
    {
        if (imageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(
                m_VkDevice,
                imageView,
                nullptr
            );
        }
    }

    m_SwapchainImageViews.clear();

    // Swapchain

    if (m_VkSwapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(
            m_VkDevice,
            m_VkSwapchain,
            nullptr
        );

        m_VkSwapchain = VK_NULL_HANDLE;
    }

    m_SwapchainImages.clear();

    // Command pool
    //
    // Destroying the pool also releases its command buffers.

    if (m_VkCommandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(
            m_VkDevice,
            m_VkCommandPool,
            nullptr
        );

        m_VkCommandPool = VK_NULL_HANDLE;
    }

    m_CommandBuffers.clear();
    m_CurrentCommandBuffer = VK_NULL_HANDLE;

    // Logical device

    if (m_VkDevice != VK_NULL_HANDLE)
    {
        vkDestroyDevice(
            m_VkDevice,
            nullptr
        );

        m_VkDevice = VK_NULL_HANDLE;
    }

    // Surface

    if (m_VkSurface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(
            m_VkInstance,
            m_VkSurface,
            nullptr
        );

        m_VkSurface = VK_NULL_HANDLE;
    }

    // Instance

    if (m_VkInstance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(
            m_VkInstance,
            nullptr
        );

        m_VkInstance = VK_NULL_HANDLE;
    }

    m_VkPhysicalDevice = VK_NULL_HANDLE;
    m_GraphicsQueue = VK_NULL_HANDLE;
    m_PresentQueue = VK_NULL_HANDLE;

    m_SwapchainImageFormat = {};
    m_SwapchainExtent = {};

    
}

RenderBackend CVulkanRenderer::getRenderBackend(){
    return RenderBackend::Vulkan;
}


// for some reason we dont have chk() in headers
void chk(VkResult result){
    if (result != VK_SUCCESS) {
        Logger().error("chk() -> {}", (uint32_t)result);
    }
    LOGGER_ASSERT(result == VK_SUCCESS, "chk failed");
}

void CVulkanRenderer::CreateInstance(){
    std::string appName = p_Window->GetWindowOptions().title;
    VkApplicationInfo appInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = appName.c_str(),
        .apiVersion = VK_API_VERSION_1_2, // 1, 2, eat my friggin poo
    };
    auto extensions = p_Window->GetExtensionsVulkan();

    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

    uint32_t extensionCount = extensions.size();

    LOGGER_ASSERT(extensionCount != 0, "Extension count is {}, did any of our extension getters work?", extensionCount);
    Logger().info("{} extensions supported.", extensionCount);
    Logger().info("Supported Extensions:");
    for (int i = 0; i < extensionCount; i++){
        Logger().info("{}: {}", i, extensions[i]);
    }

    VkInstanceCreateInfo instanceCI{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = extensionCount,
        .ppEnabledExtensionNames = extensions.data(),
    };
    Logger().info("Running vkCreateInstance...");
    chk(vkCreateInstance(&instanceCI, nullptr, &m_VkInstance));
};

void CVulkanRenderer::SelectDevice() {
    uint32_t deviceCount = 0;
    Logger().info("vkEnumuratePhysicalDevices");
    chk(vkEnumeratePhysicalDevices(
        m_VkInstance,
        &deviceCount,
        nullptr
    ));

    assert(deviceCount > 0);

    std::vector<VkPhysicalDevice> devices(deviceCount);
    Logger().info("vkEnumuratePhysicalDevices");
    chk(vkEnumeratePhysicalDevices(
        m_VkInstance,
        &deviceCount,
        devices.data()
    ));

    VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
    int bestScore = -1;

    for (VkPhysicalDevice device : devices) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);

        int score = 0;

        switch (properties.deviceType) {
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                score = 1000;
                break;

            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                score = 100;
                break;

            default:
                score = 0;
                break;
        }

        Logger().info(
            "GPU: {} (score: {})",
            properties.deviceName,
            score
        );

        if (score > bestScore) {
            bestScore = score;
            bestDevice = device;
        }
    }

    LOGGER_ASSERT(bestDevice != VK_NULL_HANDLE, "our best device is nullptr");

    m_VkPhysicalDevice = bestDevice;
}
void CVulkanRenderer::CreateSurface(){
    Logger().info("CreateVulkanSurface");
    VkResult err = p_Window->CreateVulkanSurface(m_VkInstance, &m_VkSurface);
    Logger().info("p_Window->CreateVulkanSurface() -> {}", (int)err);
};

void CVulkanRenderer::FindQueueFamilies() {
    uint32_t queueFamilyCount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(
        m_VkPhysicalDevice,
        &queueFamilyCount,
        nullptr
    );

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);

    vkGetPhysicalDeviceQueueFamilyProperties(
        m_VkPhysicalDevice,
        &queueFamilyCount,
        queueFamilies.data()
    );

    for (uint32_t i = 0; i < queueFamilyCount; ++i) {

        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            m_GraphicsQueueFamily = i;
        }
        VkBool32 presentSupport = VK_FALSE;
        chk(vkGetPhysicalDeviceSurfaceSupportKHR(
            m_VkPhysicalDevice,
            i,
            m_VkSurface,
            &presentSupport
        ));
        if (presentSupport) {
            m_PresentQueueFamily = i;
        }
        if (m_GraphicsQueueFamily != UINT32_MAX &&
            m_PresentQueueFamily != UINT32_MAX) {
            break;
        }
    }
    LOGGER_ASSERT(
        m_GraphicsQueueFamily != UINT32_MAX,
        "No graphics queue family found"
    );
    LOGGER_ASSERT(
        m_PresentQueueFamily != UINT32_MAX,
        "No presentation queue family found"
    );

    Logger().info(
            "Graphics queue family: {}",
        m_GraphicsQueueFamily
    );

    Logger().info(
    "Present queue family: {}",
        m_PresentQueueFamily
    );
}

void CVulkanRenderer::CreateDevice()
{
    float queuePriority = 1.0f;

    const char* deviceExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    // We need one queue from each unique family.
    std::set<uint32_t> uniqueQueueFamilies = {
        m_GraphicsQueueFamily,
        m_PresentQueueFamily
    };

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCI{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = queueFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };

        queueCreateInfos.push_back(queueCI);
    }

    VkDeviceCreateInfo deviceCI{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,

        .queueCreateInfoCount =
            static_cast<uint32_t>(queueCreateInfos.size()),

        .pQueueCreateInfos =
            queueCreateInfos.data(),

        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = deviceExtensions,
    };

    chk(vkCreateDevice(
        m_VkPhysicalDevice,
        &deviceCI,
        nullptr,
        &m_VkDevice
    ));

    // Get graphics queue
    vkGetDeviceQueue(
        m_VkDevice,
        m_GraphicsQueueFamily,
        0,
        &m_GraphicsQueue
    );

    // Get presentation queue
    vkGetDeviceQueue(
        m_VkDevice,
        m_PresentQueueFamily,
        0,
        &m_PresentQueue
    );

    LOGGER_ASSERT(
        m_GraphicsQueue != VK_NULL_HANDLE,
        "Failed to get graphics queue"
    );

    LOGGER_ASSERT(
        m_PresentQueue != VK_NULL_HANDLE,
        "Failed to get presentation queue"
    );
}

SwapchainSupportDetails CVulkanRenderer::QuerySwapchainSupport()
{
    SwapchainSupportDetails details{};

    chk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        m_VkPhysicalDevice,
        m_VkSurface,
        &details.capabilities
    ));

    uint32_t formatCount = 0;

    chk(vkGetPhysicalDeviceSurfaceFormatsKHR(
        m_VkPhysicalDevice,
        m_VkSurface,
        &formatCount,
        nullptr
    ));

    if (formatCount > 0)
    {
        details.formats.resize(formatCount);

        chk(vkGetPhysicalDeviceSurfaceFormatsKHR(
            m_VkPhysicalDevice,
            m_VkSurface,
            &formatCount,
            details.formats.data()
        ));
    }

    uint32_t presentModeCount = 0;

    chk(vkGetPhysicalDeviceSurfacePresentModesKHR(
        m_VkPhysicalDevice,
        m_VkSurface,
        &presentModeCount,
        nullptr
    ));

    if (presentModeCount > 0)
    {
        details.presentModes.resize(presentModeCount);

        chk(vkGetPhysicalDeviceSurfacePresentModesKHR(
            m_VkPhysicalDevice,
            m_VkSurface,
            &presentModeCount,
            details.presentModes.data()
        ));
    }

    return details;
}

VkSurfaceFormatKHR CVulkanRenderer::ChooseSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& formats)
{
    for (const auto& format : formats)
    {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return format;
        }
    }

    return formats[0];
}

VkExtent2D CVulkanRenderer::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }

    auto size = p_Window->GetWindowOptions().size;

    VkExtent2D extent{
        static_cast<uint32_t>(size.x),
        static_cast<uint32_t>(size.y)
    };

    extent.width = std::clamp(
        extent.width,
        capabilities.minImageExtent.width,
        capabilities.maxImageExtent.width
    );

    extent.height = std::clamp(
        extent.height,
        capabilities.minImageExtent.height,
        capabilities.maxImageExtent.height
    );

    return extent;
}

void CVulkanRenderer::CreateSwapchain()
{
    auto support = QuerySwapchainSupport();

    VkSurfaceFormatKHR surfaceFormat =
        ChooseSurfaceFormat(support.formats);

    VkPresentModeKHR presentMode =
        ChoosePresentMode(support.presentModes);

    VkExtent2D extent =
        ChooseSwapExtent(support.capabilities);

    uint32_t imageCount =
        support.capabilities.minImageCount + 1;

    if (support.capabilities.maxImageCount > 0 &&
        imageCount > support.capabilities.maxImageCount)
    {
        imageCount = support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchainCI{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = m_VkSurface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = support.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
    };

    chk(vkCreateSwapchainKHR(
        m_VkDevice,
        &swapchainCI,
        nullptr,
        &m_VkSwapchain
    ));

    m_SwapchainImageFormat = surfaceFormat.format;
    m_SwapchainExtent = extent;

    uint32_t swapchainImageCount = 0;

    chk(vkGetSwapchainImagesKHR(
        m_VkDevice,
        m_VkSwapchain,
        &swapchainImageCount,
        nullptr
    ));

    m_SwapchainImages.resize(swapchainImageCount);

    chk(vkGetSwapchainImagesKHR(
        m_VkDevice,
        m_VkSwapchain,
        &swapchainImageCount,
        m_SwapchainImages.data()
    ));

    Logger().info(
        "Swapchain images: {}",
        swapchainImageCount
    );
    
}

VkPresentModeKHR CVulkanRenderer::ChoosePresentMode(
    const std::vector<VkPresentModeKHR>& modes)
{
    for (const auto mode : modes)
    {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

void CVulkanRenderer::CreateImageViews()
{
    m_SwapchainImageViews.resize(m_SwapchainImages.size());

    for (size_t i = 0; i < m_SwapchainImages.size(); i++)
    {
        VkImageViewCreateInfo viewCI{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = m_SwapchainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = m_SwapchainImageFormat,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        chk(vkCreateImageView(
            m_VkDevice,
            &viewCI,
            nullptr,
            &m_SwapchainImageViews[i]
        ));
    }

    Logger().info(
        "Swapchain image views: {}",
        m_SwapchainImageViews.size()
    );
}

void CVulkanRenderer::CreateRenderPass()
{
    // -----------------------
    // Color attachment
    // -----------------------

    VkAttachmentDescription colorAttachment{
        .format = m_SwapchainImageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference colorAttachmentRef{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };


    // -----------------------
    // Depth attachment
    // -----------------------

    VkAttachmentDescription depthAttachment{
        .format = m_DepthFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout =
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    VkAttachmentReference depthAttachmentRef{
        .attachment = 1,
        .layout =
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };


    // -----------------------
    // Subpass
    // -----------------------

    VkSubpassDescription subpass{
        .pipelineBindPoint =
            VK_PIPELINE_BIND_POINT_GRAPHICS,

        .colorAttachmentCount = 1,
        .pColorAttachments =
            &colorAttachmentRef,

        .pDepthStencilAttachment =
            &depthAttachmentRef
    };


    // -----------------------
    // Render pass
    // -----------------------

    std::array<VkAttachmentDescription, 2> attachments{
        colorAttachment,
        depthAttachment
    };

    VkRenderPassCreateInfo renderPassCI{
        .sType =
            VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,

        .attachmentCount =
            static_cast<uint32_t>(attachments.size()),

        .pAttachments =
            attachments.data(),

        .subpassCount = 1,
        .pSubpasses = &subpass
    };

    chk(vkCreateRenderPass(
        m_VkDevice,
        &renderPassCI,
        nullptr,
        &m_VkRenderPass
    ));

    Logger().info("Render pass created");
}

void CVulkanRenderer::CreateFramebuffers()
{
    m_SwapchainFramebuffers.resize(
        m_SwapchainImageViews.size()
    );

    for (size_t i = 0;
         i < m_SwapchainImageViews.size();
         i++)
    {
        VkImageView attachments[] = {
            m_SwapchainImageViews[i], // color
            m_DepthImageView          // depth
        };

        VkFramebufferCreateInfo framebufferCI{
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = m_VkRenderPass,
            .attachmentCount = 2,
            .pAttachments = attachments,
            .width = m_SwapchainExtent.width,
            .height = m_SwapchainExtent.height,
            .layers = 1
        };

        chk(vkCreateFramebuffer(
            m_VkDevice,
            &framebufferCI,
            nullptr,
            &m_SwapchainFramebuffers[i]
        ));
    }

    Logger().info(
        "Swapchain framebuffers: {}",
        m_SwapchainFramebuffers.size()
    );
}


VkShaderModule CVulkanRenderer::CreateShaderModule(
    const std::vector<char>& code)
{
    LOGGER_ASSERT(
        !code.empty(),
        "SPIR-V file is empty"
    );

    LOGGER_ASSERT(
        code.size() % 4 == 0,
        "SPIR-V file size is not divisible by 4"
    );

    LOGGER_ASSERT(
        code.size() >= 4,
        "SPIR-V file is too small"
    );

    const uint32_t* words =
        reinterpret_cast<const uint32_t*>(code.data());

    LOGGER_ASSERT(
        words[0] == 0x07230203,
        "Invalid SPIR-V magic number"
    );

    VkShaderModuleCreateInfo shaderCI{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = code.size(),
        .pCode = words
    };

    VkShaderModule shaderModule = VK_NULL_HANDLE;

    chk(vkCreateShaderModule(
        m_VkDevice,
        &shaderCI,
        nullptr,
        &shaderModule
    ));

    return shaderModule;
}


void CVulkanRenderer::CreateGraphicsPipeline()
{
    VkPipelineShaderStageCreateInfo vertexStage{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = m_VkShaderModule,
        .pName = "vertexMain"
    };

    VkPipelineShaderStageCreateInfo fragmentStage{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = m_VkShaderModule,
        .pName = "fragmentMain"
    };

    VkPipelineShaderStageCreateInfo shaderStages[] = {
        vertexStage,
        fragmentStage
    };

    VkVertexInputBindingDescription binding{
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    VkVertexInputAttributeDescription attributes[] = {
        {
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, position)
        },
        {
            .location = 1,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32A32_SFLOAT,
            .offset = offsetof(Vertex, color)
        },
        {
            .location = 2,
            .binding = 0,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(Vertex, uv)
        },
        {
            .location = 3,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, normal)
        }
    };

    VkPipelineVertexInputStateCreateInfo vertexInput{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &binding,
        .vertexAttributeDescriptionCount = 4,
        .pVertexAttributeDescriptions = attributes
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkViewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(m_SwapchainExtent.width),
        .height = static_cast<float>(m_SwapchainExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D scissor{
        .offset = {0, 0},
        .extent = m_SwapchainExtent
    };

    VkPipelineViewportStateCreateInfo viewportState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor
    };

    VkPipelineRasterizationStateCreateInfo rasterizer{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.0f,
    };

    VkPipelineMultisampleStateCreateInfo multisampling{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE
    };
    VkPipelineColorBlendAttachmentState colorBlendAttachment{
        .blendEnable = VK_FALSE,
        .colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo colorBlending{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment
    };
    VkPipelineLayoutCreateInfo pipelineLayoutCI{};

    pipelineLayoutCI.sType =
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    
    pipelineLayoutCI.setLayoutCount = 1;
    pipelineLayoutCI.pSetLayouts =
        &m_CameraDescriptorSetLayout;
    
    pipelineLayoutCI.pushConstantRangeCount = 0;

    chk(vkCreatePipelineLayout(
        m_VkDevice,
        &pipelineLayoutCI,
        nullptr,
        &m_VkPipelineLayout
    ));
    VkGraphicsPipelineCreateInfo pipelineCI{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInput,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depthStencil,
        .pColorBlendState = &colorBlending,
        .layout = m_VkPipelineLayout,
        .renderPass = m_VkRenderPass,
        .subpass = 0
    };
    Logger().info("CreateGraphicsPipeline");
    chk(
vkCreateGraphicsPipelines(
        m_VkDevice,
        VK_NULL_HANDLE,
        1,
        &pipelineCI,
        nullptr,
        &m_VkGraphicsPipeline
    ));
}

void CVulkanRenderer::CreateCommandPool()
{
    VkCommandPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = m_GraphicsQueueFamily
    };

    chk(vkCreateCommandPool(
        m_VkDevice,
        &poolInfo,
        nullptr,
        &m_VkCommandPool
    ));
}

void CVulkanRenderer::CreateCommandBuffers()
{
    m_CommandBuffers.resize(m_SwapchainFramebuffers.size());

    VkCommandBufferAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_VkCommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount =
            static_cast<uint32_t>(m_CommandBuffers.size())
    };

    chk(vkAllocateCommandBuffers(
        m_VkDevice,
        &allocInfo,
        m_CommandBuffers.data()
    ));
}

void CVulkanRenderer::RecordCommandBuffers()
{
    for (size_t i = 0; i < m_CommandBuffers.size(); ++i)
    {
        VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
        };

        chk(vkBeginCommandBuffer(
            m_CommandBuffers[i],
            &beginInfo
        ));

        VkClearValue clearColor{};
        clearColor.color = {
            0.0f, 0.0f, 0.0f, 1.0f
        };

        VkRenderPassBeginInfo renderPassInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = m_VkRenderPass,
            .framebuffer = m_SwapchainFramebuffers[i],
            .renderArea = {
                {0, 0},
                m_SwapchainExtent
            },
            .clearValueCount = 1,
            .pClearValues = &clearColor
        };

        vkCmdBeginRenderPass(
            m_CommandBuffers[i],
            &renderPassInfo,
            VK_SUBPASS_CONTENTS_INLINE
        );

        vkCmdBindPipeline(
            m_CommandBuffers[i],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_VkGraphicsPipeline
        );

        vkCmdDraw(
            m_CommandBuffers[i],
            3,
            1,
            0,
            0
        );

        vkCmdEndRenderPass(m_CommandBuffers[i]);

        chk(vkEndCommandBuffer(m_CommandBuffers[i]));
    }
}

uint32_t CVulkanRenderer::FindMemoryType(
    uint32_t typeFilter,
    VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memoryProperties{};

    vkGetPhysicalDeviceMemoryProperties(
        m_VkPhysicalDevice,
        &memoryProperties
    );

    for (uint32_t i = 0;
         i < memoryProperties.memoryTypeCount;
         ++i)
    {
        if ((typeFilter & (1 << i)) &&
            (memoryProperties.memoryTypes[i].propertyFlags & properties)
                == properties)
        {
            return i;
        }
    }

    LOGGER_ASSERT(false, "Failed to find suitable memory type");

    return UINT32_MAX;
}

void CVulkanRenderer::CreateVertexBuffer()
{
    VkDeviceSize bufferSize =
        sizeof(Vertex) * m_Vertices.size();

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateBuffer(
        m_VkDevice,
        &bufferInfo,
        nullptr,
        &m_VkVertexBuffer
    );

    LOGGER_ASSERT(
        result == VK_SUCCESS,
        "Failed to create vertex buffer"
    );

    VkMemoryRequirements memoryRequirements{};

    vkGetBufferMemoryRequirements(
        m_VkDevice,
        m_VkVertexBuffer,
        &memoryRequirements
    );

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType =
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    allocInfo.allocationSize =
        memoryRequirements.size;

    allocInfo.memoryTypeIndex =
        FindMemoryType(
            memoryRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

    result = vkAllocateMemory(
        m_VkDevice,
        &allocInfo,
        nullptr,
        &m_VkVertexBufferMemory
    );

    LOGGER_ASSERT(
        result == VK_SUCCESS,
        "Failed to allocate vertex buffer memory"
    );

    result = vkBindBufferMemory(
        m_VkDevice,
        m_VkVertexBuffer,
        m_VkVertexBufferMemory,
        0
    );

    LOGGER_ASSERT(
        result == VK_SUCCESS,
        "Failed to bind vertex buffer memory"
    );
}

void CVulkanRenderer::CreateIndexBuffer()
{
    VkDeviceSize bufferSize =
        sizeof(uint32_t) * m_Indices.size();

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateBuffer(
        m_VkDevice,
        &bufferInfo,
        nullptr,
        &m_VkIndexBuffer
    );

    LOGGER_ASSERT(
        result == VK_SUCCESS,
        "Failed to create index buffer"
    );

    VkMemoryRequirements memoryRequirements{};

    vkGetBufferMemoryRequirements(
        m_VkDevice,
        m_VkIndexBuffer,
        &memoryRequirements
    );

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType =
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    allocInfo.allocationSize =
        memoryRequirements.size;

    allocInfo.memoryTypeIndex =
        FindMemoryType(
            memoryRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

    result = vkAllocateMemory(
        m_VkDevice,
        &allocInfo,
        nullptr,
        &m_VkIndexBufferMemory
    );

    LOGGER_ASSERT(
        result == VK_SUCCESS,
        "Failed to allocate index buffer memory"
    );

    result = vkBindBufferMemory(
        m_VkDevice,
        m_VkIndexBuffer,
        m_VkIndexBufferMemory,
        0
    );

    LOGGER_ASSERT(
        result == VK_SUCCESS,
        "Failed to bind index buffer memory"
    );
}

void CVulkanRenderer::UpdateVertexBuffer()
{
    VkDeviceSize bufferSize =
        sizeof(Vertex) * m_Vertices.size();

    void* data = nullptr;

    VkResult result = vkMapMemory(
        m_VkDevice,
        m_VkVertexBufferMemory,
        0,
        bufferSize,
        0,
        &data
    );

    LOGGER_ASSERT(
        result == VK_SUCCESS,
        "Failed to map vertex buffer memory"
    );

    std::memcpy(
        data,
        m_Vertices.data(),
        static_cast<size_t>(bufferSize)
    );

    vkUnmapMemory(
        m_VkDevice,
        m_VkVertexBufferMemory
    );
}

void CVulkanRenderer::UpdateIndexBuffer()
{
    VkDeviceSize bufferSize =
        sizeof(uint32_t) * m_Indices.size();

    void* data = nullptr;

    VkResult result = vkMapMemory(
        m_VkDevice,
        m_VkIndexBufferMemory,
        0,
        bufferSize,
        0,
        &data
    );

    LOGGER_ASSERT(
        result == VK_SUCCESS,
        "Failed to map index buffer memory"
    );

    std::memcpy(
        data,
        m_Indices.data(),
        static_cast<size_t>(bufferSize)
    );

    vkUnmapMemory(
        m_VkDevice,
        m_VkIndexBufferMemory
    );
}

void CVulkanRenderer::Draw(
    const Vertex* vertices,
    uint32_t nVerts,
    const uint32_t* indices,
    uint32_t nIndices)
{
    m_Vertices.assign(vertices, vertices + nVerts);
    m_Indices.assign(indices, indices + nIndices);

    if (!m_VertexBufferCreated)
    {
        CreateVertexBuffer();
        m_VertexBufferCreated = true;
    }

    if (!m_IndexBufferCreated)
    {
        CreateIndexBuffer();
        m_IndexBufferCreated = true;
    }

    UpdateVertexBuffer();
    UpdateIndexBuffer();

    // -----------------------
    // Pipeline
    // -----------------------

    vkCmdBindPipeline(
        m_CurrentCommandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_VkGraphicsPipeline
    );

    // -----------------------
    // Camera descriptor set
    // -----------------------

    vkCmdBindDescriptorSets(
        m_CurrentCommandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_VkPipelineLayout,
        0, // set = 0
        1,
        &m_CameraDescriptorSet,
        0,
        nullptr
    );

    // -----------------------
    // Vertex buffer
    // -----------------------

    VkBuffer vertexBuffers[] = {
        m_VkVertexBuffer
    };

    VkDeviceSize offsets[] = {
        0
    };

    vkCmdBindVertexBuffers(
        m_CurrentCommandBuffer,
        0,
        1,
        vertexBuffers,
        offsets
    );

    // -----------------------
    // Index buffer
    // -----------------------

    vkCmdBindIndexBuffer(
        m_CurrentCommandBuffer,
        m_VkIndexBuffer,
        0,
        VK_INDEX_TYPE_UINT32
    );

    // -----------------------
    // Draw
    // -----------------------

    vkCmdDrawIndexed(
        m_CurrentCommandBuffer,
        static_cast<uint32_t>(m_Indices.size()),
        1,
        0,
        0,
        0
    );
}

void CVulkanRenderer::CreateSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType =
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType =
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    // Start signaled so the first frame doesn't wait forever.
    fenceInfo.flags =
        VK_FENCE_CREATE_SIGNALED_BIT;

    VkResult result = vkCreateSemaphore(
        m_VkDevice,
        &semaphoreInfo,
        nullptr,
        &m_ImageAvailableSemaphore
    );

    LOGGER_ASSERT(
        result == VK_SUCCESS,
        "Failed to create image available semaphore"
    );

    result = vkCreateSemaphore(
        m_VkDevice,
        &semaphoreInfo,
        nullptr,
        &m_RenderFinishedSemaphore
    );

    LOGGER_ASSERT(
        result == VK_SUCCESS,
        "Failed to create render finished semaphore"
    );

    result = vkCreateFence(
        m_VkDevice,
        &fenceInfo,
        nullptr,
        &m_InFlightFence
    );

    LOGGER_ASSERT(
        result == VK_SUCCESS,
        "Failed to create in-flight fence"
    );
}

void CVulkanRenderer::ImGuiNewFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}
void CVulkanRenderer::ImGuiRender()
{
    ImGui::Render();

    ImGui_ImplVulkan_RenderDrawData(
        ImGui::GetDrawData(),
        m_CurrentCommandBuffer
    );
}

void CVulkanRenderer::CreateCameraBuffer()
{
    VkDeviceSize bufferSize = sizeof(CameraUBO);

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    chk(vkCreateBuffer(
        m_VkDevice,
        &bufferInfo,
        nullptr,
        &m_CameraBuffer
    ));

    VkMemoryRequirements memoryRequirements{};

    vkGetBufferMemoryRequirements(
        m_VkDevice,
        m_CameraBuffer,
        &memoryRequirements
    );

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;

    allocInfo.memoryTypeIndex = FindMemoryType(
        memoryRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    chk(vkAllocateMemory(
        m_VkDevice,
        &allocInfo,
        nullptr,
        &m_CameraBufferMemory
    ));

    chk(vkBindBufferMemory(
        m_VkDevice,
        m_CameraBuffer,
        m_CameraBufferMemory,
        0
    ));
}

void CVulkanRenderer::UpdateCameraBuffer()
{
    void* data = nullptr;

    chk(vkMapMemory(
        m_VkDevice,
        m_CameraBufferMemory,
        0,
        sizeof(CameraUBO),
        0,
        &data
    ));

    std::memcpy(
        data,
        &m_CameraUBO,
        sizeof(CameraUBO)
    );

    vkUnmapMemory(
        m_VkDevice,
        m_CameraBufferMemory
    );
}

void CVulkanRenderer::CreateDescriptorPool()
{
    VkDescriptorPoolSize poolSize{};

    poolSize.type =
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};

    poolInfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;

    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;

    chk(vkCreateDescriptorPool(
        m_VkDevice,
        &poolInfo,
        nullptr,
        &m_DescriptorPool
    ));
}

void CVulkanRenderer::CreateCameraDescriptorSet()
{
    VkDescriptorSetAllocateInfo allocInfo{};

    allocInfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

    allocInfo.descriptorPool = m_DescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts =
        &m_CameraDescriptorSetLayout;

    chk(vkAllocateDescriptorSets(
        m_VkDevice,
        &allocInfo,
        &m_CameraDescriptorSet
    ));

    VkDescriptorBufferInfo bufferInfo{};

    bufferInfo.buffer = m_CameraBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(CameraUBO);

    VkWriteDescriptorSet write{};

    write.sType =
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

    write.dstSet = m_CameraDescriptorSet;
    write.dstBinding = 0;
    write.dstArrayElement = 0;
    write.descriptorType =
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.descriptorCount = 1;
    write.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(
        m_VkDevice,
        1,
        &write,
        0,
        nullptr
    );
}

void CVulkanRenderer::CreateDepthResources()
{
    VkImageCreateInfo imageInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = m_DepthFormat,
        .extent = {
            m_SwapchainExtent.width,
            m_SwapchainExtent.height,
            1
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    chk(vkCreateImage(
        m_VkDevice,
        &imageInfo,
        nullptr,
        &m_DepthImage
    ));

    VkMemoryRequirements memRequirements{};

    vkGetImageMemoryRequirements(
        m_VkDevice,
        m_DepthImage,
        &memRequirements
    );

    VkMemoryAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = FindMemoryType(
            memRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        )
    };

    chk(vkAllocateMemory(
        m_VkDevice,
        &allocInfo,
        nullptr,
        &m_DepthImageMemory
    ));

    chk(vkBindImageMemory(
        m_VkDevice,
        m_DepthImage,
        m_DepthImageMemory,
        0
    ));

    VkImageViewCreateInfo viewInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = m_DepthImage,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = m_DepthFormat,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    chk(vkCreateImageView(
        m_VkDevice,
        &viewInfo,
        nullptr,
        &m_DepthImageView
    ));

    Logger().info("Depth resources created");
}

void CVulkanRenderer::CreateCameraDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding cameraBinding{};

    cameraBinding.binding = 0;
    cameraBinding.descriptorType =
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    cameraBinding.descriptorCount = 1;

    cameraBinding.stageFlags =
        VK_SHADER_STAGE_VERTEX_BIT;

    cameraBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};

    layoutInfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &cameraBinding;

    chk(vkCreateDescriptorSetLayout(
        m_VkDevice,
        &layoutInfo,
        nullptr,
        &m_CameraDescriptorSetLayout
    ));

    Logger().info("Camera descriptor set layout created");
}

VkExtent2D CVulkanRenderer::getSwapchainExtent(){
    return m_SwapchainExtent;
}