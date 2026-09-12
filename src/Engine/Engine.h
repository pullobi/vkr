#pragma once
#include "Engine/Render/Api/IRenderApi.h"
#include "Engine/Render/Api/IWindowApi.h"
#include "Render/Api/CApiTypes.h"

struct EngineCreateInfo
{
    WindowOptions windowOptions;
    WindowBackend windowBackend;
    RenderBackend renderBackend;
};

class Engine {
public:
    Engine(EngineCreateInfo engineCreate);

    void MainLoop(bool* keepRunning);

    ~Engine();
private:
    WindowBackend m_windowBackend;
    RenderBackend m_renderBackend;

    IWindowApi* p_WindowApi;
    IRenderApi* p_RenderApi;
};