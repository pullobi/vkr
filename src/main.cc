#include "Engine/Engine.h"
#include "Engine/Render/Api/CApiTypes.h"
#include "Logger/Logger.h"
#include <cstring>

static bool* engine_KeepRunning = new bool(true);


#ifdef ENGINE_WINDOW_USE_GLFW
#define ENGINE_WINDOW_API WindowBackend::GLFW
#endif

#ifdef ENGINE_RENDER_USE_VULKAN
#define ENGINE_RENDER_API RenderBackend::Vulkan
#endif
#ifdef ENGINE_RENDER_USE_OPENGL
#define ENGINE_RENDER_API RenderBackend::OpenGL
#endif

int main(int argc, const char** argv)
{
    RenderBackend render = ENGINE_RENDER_API;
    WindowBackend window = ENGINE_WINDOW_API;
    for (int i = 0; i < argc; i++){
        if (strcmp("-vulkan", argv[i]) == 0){
            render = RenderBackend::Vulkan;
        }
        if (strcmp("-gl", argv[i]) == 0){
            render = RenderBackend::OpenGL;
        }
    }



    Engine* engine = new Engine({
        .windowOptions = {
            .size = {
                .x = 1024,
                .y = 768
            },
            .title = "My Engine"
        },

        .windowBackend = window,
        .renderBackend = render
    });


    
    Logger().warn("StartMainLoop");
    engine->MainLoop(engine_KeepRunning);
    Logger().warn("EndMainLoop");
    if (!*engine_KeepRunning == true){
        delete engine;
    };
    return 0;
}