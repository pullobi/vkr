#include "Engine/Render/gui/imgui_internal.h"
#include "Api/IRenderApi.h"
#include <cstdint>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <cassert>
#include <csignal>
#include <cstddef>
#include <vulkan/vulkan_core.h>
#include <fstream>
#include "Logger/Logger.h"
#include "Types/CRenderTypes.h"
#include "Api/CApiTypes.h"
#include "Api/IWindowApi.h"
#include "Engine/Render/gui/imgui.h"
#include "Logger/Logger.h"
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan_core.h>


#define RENDERAPI_VULKAN

struct SwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;

    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};



class CVulkanRenderer : public IRenderApi {
public:
    void Init(IWindowApi* Window) override;

    void RenderBegin() override;

    void Draw(const Vertex* vertices, uint32_t nVerts, const uint32_t* indices, uint32_t nIndices) override;

    void ImGuiNewFrame() override;
    void ImGuiRender() override;
    void RenderEnd() override;
    
    void Shutdown() override;

    RenderBackend getRenderBackend() override;


    // CameraUBO m_CameraUBO                   {};
    VkExtent2D getSwapchainExtent() override;
    void UpdateCameraBuffer() override;
private:
    VkInstance m_VkInstance                 = VK_NULL_HANDLE;
    VkPhysicalDevice m_VkPhysicalDevice     = VK_NULL_HANDLE;
    VkDevice m_VkDevice                     = VK_NULL_HANDLE;
    VkQueue m_GraphicsQueue                 = VK_NULL_HANDLE;
    VkQueue m_PresentQueue                  = VK_NULL_HANDLE;
    VkSurfaceKHR m_VkSurface                = VK_NULL_HANDLE;
    VkSwapchainKHR m_VkSwapchain            = VK_NULL_HANDLE;
    VkRenderPass m_VkRenderPass             = VK_NULL_HANDLE;
    VkShaderModule m_VkShaderModule         = VK_NULL_HANDLE;
    VkPipelineLayout m_VkPipelineLayout     = VK_NULL_HANDLE;
    VkPipeline m_VkGraphicsPipeline         = VK_NULL_HANDLE;
    VkCommandPool m_VkCommandPool           = VK_NULL_HANDLE;
    VkBuffer m_VkVertexBuffer               = VK_NULL_HANDLE;
    VkDeviceMemory m_VkVertexBufferMemory   = VK_NULL_HANDLE;
    uint32_t m_GraphicsQueueFamily          = UINT32_MAX;
    uint32_t m_PresentQueueFamily           = UINT32_MAX;
    VkFormat m_SwapchainImageFormat         {};
    VkExtent2D m_SwapchainExtent            {};
    VkBuffer m_VkIndexBuffer                = VK_NULL_HANDLE;
    VkDeviceMemory m_VkIndexBufferMemory    = VK_NULL_HANDLE;
    VkCommandBuffer m_CurrentCommandBuffer  = VK_NULL_HANDLE;
    VkSemaphore m_ImageAvailableSemaphore   = VK_NULL_HANDLE;
    VkSemaphore m_RenderFinishedSemaphore   = VK_NULL_HANDLE;
    VkFence m_InFlightFence                 = VK_NULL_HANDLE;
    bool m_VertexBufferCreated              = false;
    bool m_IndexBufferCreated               = false;
    uint32_t m_CurrentImageIndex            = 0;
    VkBuffer m_CameraBuffer                 = VK_NULL_HANDLE;
    VkDeviceMemory m_CameraBufferMemory     = VK_NULL_HANDLE;
    VkDescriptorSetLayout 
                m_CameraDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool m_DescriptorPool       = VK_NULL_HANDLE;
    VkDescriptorSet m_CameraDescriptorSet   = VK_NULL_HANDLE;
    VkImage m_DepthImage                    = VK_NULL_HANDLE;
    VkDeviceMemory m_DepthImageMemory       = VK_NULL_HANDLE;
    VkImageView m_DepthImageView            = VK_NULL_HANDLE;
    VkFormat m_DepthFormat                  = VK_FORMAT_D32_SFLOAT;

    std::vector<VkImage> m_SwapchainImages;
    std::vector<VkImageView> m_SwapchainImageViews;
    std::vector<VkFramebuffer> m_SwapchainFramebuffers;
    std::vector<VkCommandBuffer> m_CommandBuffers;
    std::vector<Vertex> m_Vertices;
    std::vector<uint32_t> m_Indices;
    ImGuiContext* m_ImGuiContext;

    void ImGuiInit();
    void CreateInstance();
    void SelectDevice();
    void CreateSurface();
    void FindQueueFamilies();
    void CreateDevice();
    void CreateSwapchain();
    void CreateImageViews();
    void CreateRenderPass();
    void CreateFramebuffers();
    void CreateGraphicsPipeline();
    void CreateCommandPool();
    void CreateCommandBuffers();
    void RecordCommandBuffers();
    void CreateVertexBuffer();
    void CreateIndexBuffer();
    void CreateSyncObjects();
    void CreateCameraBuffer();
    void CreateCameraDescriptorSetLayout();
    void CreateDescriptorPool();
    void CreateCameraDescriptorSet();
    void CreateDepthResources();
    
    void UpdateVertexBuffer();
    void UpdateIndexBuffer();
    

    // Helpers
    SwapchainSupportDetails QuerySwapchainSupport();
    VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& modes);
    VkShaderModule CreateShaderModule(const std::vector<char>& code);
    uint32_t FindMemoryType(uint32_t typeFilter,VkMemoryPropertyFlags properties);
};
