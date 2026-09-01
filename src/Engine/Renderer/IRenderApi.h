#pragma once
#include "IWindowApi.h"
#include "CApiTypes.h"



class IRenderApi {
public:
    virtual ~IRenderApi() = default;

    virtual void Init(IWindowApi* Window) = 0;
    virtual void RenderBegin() = 0;
    virtual void RenderEnd() = 0;
    virtual void Shutdown() = 0;

    virtual void ImGuiNewFrame() = 0;
    virtual void ImGuiRender() = 0;

    virtual RenderBackend getRenderBackend() = 0;
    IWindowApi* p_Window;    
};

