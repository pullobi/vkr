#pragma once
#include "Engine/Render/Types/CRenderTypes.h"
#include "IWindowApi.h"
#include "CApiTypes.h"
#include "Tools.h"
#include <vulkan/vulkan_core.h>



class IRenderApi {
public:
    virtual ~IRenderApi() = default;

    virtual void Init(IWindowApi* Window) = 0;
    virtual void RenderBegin() = 0;
    virtual void RenderEnd() = 0;
    virtual void Shutdown() = 0;

    virtual void ImGuiNewFrame() = 0;
    virtual void ImGuiRender() = 0;

    virtual void UpdateCameraBuffer()=0;
    
    virtual void Draw(const Vertex* vertices, uint32_t nVerts, const uint32_t* indices, uint32_t nIndices) = 0;

    virtual RenderBackend getRenderBackend() = 0;
    // VkExtent2D doesn't really matter here since it's just a struct that has 2 uint32 values, 
    virtual VkExtent2D getSwapchainExtent() = 0;
    IWindowApi* p_Window;
};


CameraUBO& GetCameraUBO();


