#include "Engine.h"

#include "Engine/Command/CCommandManager.h"
#include "Engine/Command/CommandLists.h"
#include "Engine/Command/Commands.h"

#include "Engine/Render/Api/CApiTypes.h"
#include "Engine/Render/Api/IInputApi.h"
#include "Engine/Render/Api/IRenderApi.h"
#include "Engine/Render/Api/IWindowApi.h"

#include "Engine/Render/Types/CInputTypes.h"
#include "Engine/Render/Types/CRenderTypes.h"

#include "Engine/Render/gui/imgui.h"

#include "Engine/Render/Window/CGlfwWindow.h"
#include "Engine/Render/CVulkanRenderer.h"
#include "Engine/Render/COpenGLRenderer.h"

#include "Logger/Logger.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>

#include <cmath>
#include <cstdint>
#include <sys/_pthread/_pthread_t.h>


#define RGBA(r, g, b, a) \
    ImVec4( \
        (float)(r) / 255.0f, \
        (float)(g) / 255.0f, \
        (float)(b) / 255.0f, \
        (float)(a) / 255.0f \
    )


Vertex vertices[] =
{
    // Front (+Z)
    {{-0.5f, -0.5f,  0.5f}, {1, 0, 0, 1}, {0, 0}, {0, 0, 1}},
    {{ 0.5f, -0.5f,  0.5f}, {0, 1, 0, 1}, {1, 0}, {0, 0, 1}},
    {{ 0.5f,  0.5f,  0.5f}, {0, 0, 1, 1}, {1, 1}, {0, 0, 1}},
    {{-0.5f,  0.5f,  0.5f}, {1, 1, 0, 1}, {0, 1}, {0, 0, 1}},

    // Back (-Z)
    {{ 0.5f, -0.5f, -0.5f}, {1, 0, 0, 1}, {0, 0}, {0, 0, -1}},
    {{-0.5f, -0.5f, -0.5f}, {0, 1, 0, 1}, {1, 0}, {0, 0, -1}},
    {{-0.5f,  0.5f, -0.5f}, {0, 0, 1, 1}, {1, 1}, {0, 0, -1}},
    {{ 0.5f,  0.5f, -0.5f}, {1, 1, 0, 1}, {0, 1}, {0, 0, -1}},

    // Left (-X)
    {{-0.5f, -0.5f, -0.5f}, {1, 0, 0, 1}, {0, 0}, {-1, 0, 0}},
    {{-0.5f, -0.5f,  0.5f}, {0, 1, 0, 1}, {1, 0}, {-1, 0, 0}},
    {{-0.5f,  0.5f,  0.5f}, {0, 0, 1, 1}, {1, 1}, {-1, 0, 0}},
    {{-0.5f,  0.5f, -0.5f}, {1, 1, 0, 1}, {0, 1}, {-1, 0, 0}},

    // Right (+X)
    {{ 0.5f, -0.5f,  0.5f}, {1, 0, 0, 1}, {0, 0}, {1, 0, 0}},
    {{ 0.5f, -0.5f, -0.5f}, {0, 1, 0, 1}, {1, 0}, {1, 0, 0}},
    {{ 0.5f,  0.5f, -0.5f}, {0, 0, 1, 1}, {1, 1}, {1, 0, 0}},
    {{ 0.5f,  0.5f,  0.5f}, {1, 1, 0, 1}, {0, 1}, {1, 0, 0}},

    // Top (+Y)
    {{-0.5f,  0.5f,  0.5f}, {1, 0, 0, 1}, {0, 0}, {0, 1, 0}},
    {{ 0.5f,  0.5f,  0.5f}, {0, 1, 0, 1}, {1, 0}, {0, 1, 0}},
    {{ 0.5f,  0.5f, -0.5f}, {0, 0, 1, 1}, {1, 1}, {0, 1, 0}},
    {{-0.5f,  0.5f, -0.5f}, {1, 1, 0, 1}, {0, 1}, {0, 1, 0}},

    // Bottom (-Y)
    {{-0.5f, -0.5f, -0.5f}, {1, 0, 0, 1}, {0, 0}, {0, -1, 0}},
    {{ 0.5f, -0.5f, -0.5f}, {0, 1, 0, 1}, {1, 0}, {0, -1, 0}},
    {{ 0.5f, -0.5f,  0.5f}, {0, 0, 1, 1}, {1, 1}, {0, -1, 0}},
    {{-0.5f, -0.5f,  0.5f}, {1, 1, 0, 1}, {0, 1}, {0, -1, 0}}
};


uint32_t indices[] =
{
    // Front
    0, 1, 2,
    2, 3, 0,

    // Back
    4, 5, 6,
    6, 7, 4,

    // Left
    8, 9, 10,
    10, 11, 8,

    // Right
    12, 13, 14,
    14, 15, 12,

    // Top
    16, 17, 18,
    18, 19, 16,

    // Bottom
    20, 21, 22,
    22, 23, 20
};


Engine::Engine(EngineCreateInfo engineCreateInfo)
    : m_windowBackend(engineCreateInfo.windowBackend),
      m_renderBackend(engineCreateInfo.renderBackend)
{
    Logger().info("Start");

    // ============================================================
    // Window Backend
    // ============================================================

    engineCreateInfo.windowOptions.title += " ( ";

    switch (m_windowBackend)
    {
        case WindowBackend::GLFW:
        {
            p_WindowApi = new CGlfwWindow();

            engineCreateInfo.windowOptions.title += "GLFW";

            break;
        }

        case WindowBackend::SDL:
        {
            Logger().fatal("SDL not implemented yet.");
            break;
        }

        default:
        {
            Logger().fatal("Unknown Window Backend");
            break;
        }
    }

    // ============================================================
    // Render Backend
    // ============================================================

    engineCreateInfo.windowOptions.title += " - ";

    switch (m_renderBackend)
    {
        case RenderBackend::OpenGL:
        {
            p_RenderApi = new COpenGLRenderer();
            engineCreateInfo.windowOptions.title += "OpenGL";
            break;
        }

        case RenderBackend::Vulkan:
        {
            p_RenderApi = new CVulkanRenderer();

            engineCreateInfo.windowOptions.title += "Vulkan";

            break;
        }

        case RenderBackend::None:
        {
            Logger().fatal(
                "CPU rendering not implemented yet"
            );

            break;
        }

        default:
        {
            Logger().fatal("Unknown Render Backend");
            break;
        }
    }

    engineCreateInfo.windowOptions.title += " )";

    // ============================================================
    // Create Window
    // ============================================================

    p_WindowApi->SetWindowOptions(
        engineCreateInfo.windowOptions
    );

    p_WindowApi->Create(engineCreateInfo.renderBackend);

    const char* description = nullptr;
    LOGGER_ASSERT(p_WindowApi != NULL, "Window is NULL?");

    if (p_WindowApi->GetNativeHandle())
        Logger().info("GLFW window created");
    else
        Logger().info("GLFW window is null");


    Logger().info("create window");

    // ============================================================
    // Initialize Renderer
    // ============================================================

    p_RenderApi->Init(
        p_WindowApi
    );

    Logger().info("created renderer");

    // ============================================================
    // Commands
    // ============================================================

    auto& commandManager = GetCommandManager();
    RegisterEngineCommands();

    if (commandManager.FindCommand("help"))
    {
        Logger().info(
            "help command IS registered"
        );
    }
    else
    {
        Logger().error(
            "help command IS NOT registered"
        );
    }

    // ============================================================
    // Command Lists
    // ============================================================

    {
        SetBaseFolder("assets/CommandLists");
        CommandList mainCommandList(
            "assets/CommandLists",
            "main.txt"
        );

        mainCommandList.Execute();
    }

    // ============================================================
    // Input
    // ============================================================

    

    p_WindowApi->GetInputManager()->BindKey(
        Key::W,
        PressType::Pressed,
        "+forward"
    );

    

    p_WindowApi->GetInputManager()->BindKey(
        Key::Escape,
        PressType::Released,
        "cursor_toggle"
    );

    p_WindowApi->GetInputManager()->BindKey(Key::W, PressType::Pressed, "+forward");
    p_WindowApi->GetInputManager()->BindKey(Key:: A,PressType::Pressed, "+left");
    p_WindowApi->GetInputManager()->BindKey(Key:: S,PressType::Pressed, "+back");
    p_WindowApi->GetInputManager()->BindKey(Key:: D,PressType::Pressed, "+right");
    
    ToggleGlobalMouseShouldLock();

    SetGlobalWindowApi(
        p_WindowApi
    );
}


void Engine::MainLoop(bool* keepRunning)
{
    // ============================================================
    // Camera
    // ============================================================

    auto& camera = GetCameraUBO();

    camera.SetPos({
        3.0f,
        0.0f,
        0.0f
    });

    camera.SetRot({
        -45.0f,
        0.0f
    });

    camera.model = glm::mat4(1.0f);

    // ============================================================
    // Projection
    // ============================================================

    {
        const auto extent =
            p_RenderApi->getSwapchainExtent();

        camera.projection =
            glm::perspective(
                glm::radians(60.0f),

                static_cast<float>(extent.width) /
                static_cast<float>(extent.height),

                0.1f,
                100.0f
            );

        if (m_renderBackend == RenderBackend::Vulkan){
            // Fix vulkan's weird reverse thingy;
            camera.projection[1][1] *= -1.0f;
        }
    }

    p_RenderApi->UpdateCameraBuffer();

    // ============================================================
    // UI State
    // ============================================================

    bool b_LoggerOpen = true;
    bool b_CameraControlsOpen = true;

    char commandInput[512] = "";

    size_t previousLogCount =
        Logger().GetLogHistory().size();

    Logger().info(
        "log Count {}",
        previousLogCount
    );

    // ============================================================
    // Main Loop
    // ============================================================

    while (
        *keepRunning &&
        !p_WindowApi->ShouldClose()
    )
    {
        // ========================================================
        // Camera
        // ========================================================

        {
            p_RenderApi->UpdateCameraBuffer();
        }

        // ========================================================
        // Begin Frame
        // ========================================================

        {
            p_RenderApi->RenderBegin();
            p_RenderApi->ImGuiNewFrame();
        }

        // ========================================================
        // Keyboard Shortcuts
        // ========================================================

        {
            if (ImGui::IsKeyPressed(ImGuiKey_X))
            {
                b_LoggerOpen =
                    !b_LoggerOpen;

                Logger().warn(
                    "logger toggle"
                );
            }

            if (ImGui::IsKeyPressed(ImGuiKey_C))
            {
                b_CameraControlsOpen =
                    !b_CameraControlsOpen;

                Logger().warn(
                    "camera control toggle"
                );
            }
        }

        // ========================================================
        // ImGui
        // ========================================================

        {
            // ====================================================
            // Camera Controls Window
            // ====================================================

            if (b_CameraControlsOpen)
            {
                ImGui::Begin(
                    "3D Camera",
                    &b_CameraControlsOpen
                );

                ImGui::Text("Camera");

                // ------------------------------------------------
                // Position
                // ------------------------------------------------

                glm::vec3 cameraPos =
                    camera.GetPos();

                if (ImGui::SliderFloat3(
                    "Position",
                    &cameraPos[0],
                    -10.0f,
                    10.0f
                ))
                {
                    camera.SetPos(
                        cameraPos
                    );
                }

                // ------------------------------------------------
                // Rotation
                // ------------------------------------------------

                glm::vec2 cameraRot =
                    camera.GetRot();

                float yaw =
                    cameraRot.x;

                float pitch =
                    cameraRot.y;

                if (ImGui::SliderFloat(
                    "Yaw",
                    &yaw,
                    -180.0f,
                    180.0f
                ))
                {
                    cameraRot.x = yaw;

                    camera.SetRot(
                        cameraRot
                    );
                }

                if (ImGui::SliderFloat(
                    "Pitch",
                    &pitch,
                    -89.0f,
                    89.0f
                ))
                {
                    cameraRot.y = pitch;

                    camera.SetRot(
                        cameraRot
                    );
                }

                // ------------------------------------------------
                // Forward
                // ------------------------------------------------

                const float yawRadians =
                    glm::radians(
                        cameraRot.x
                    );

                const float pitchRadians =
                    glm::radians(
                        cameraRot.y
                    );

                glm::vec3 forward;

                forward.x =
                    std::cos(pitchRadians) *
                    std::cos(yawRadians);

                forward.y =
                    std::sin(pitchRadians);

                forward.z =
                    std::cos(pitchRadians) *
                    std::sin(yawRadians);

                forward =
                    glm::normalize(
                        forward
                    );

                ImGui::Text(
                    "Forward: %.2f %.2f %.2f",
                    forward.x,
                    forward.y,
                    forward.z
                );

                ImGui::End();
            }

            // ====================================================
            // Logger Window
            // ====================================================

            if (b_LoggerOpen)
            {
                ImGui::Begin(
                    "Console",
                    &b_LoggerOpen,
                    ImGuiWindowFlags_MenuBar
                );

                // ------------------------------------------------
                // Menu Bar
                // ------------------------------------------------

                if (ImGui::BeginMenuBar())
                {
                    if (ImGui::BeginMenu("File"))
                    {
                        if (ImGui::MenuItem(
                            "Close Logger",
                            "X"
                        ))
                        {
                            b_LoggerOpen = false;
                        }

                        if (ImGui::MenuItem(
                            "Close Camera Controls",
                            "C"
                        ))
                        {
                            b_CameraControlsOpen = false;
                        }

                        ImGui::EndMenu();
                    }

                    ImGui::EndMenuBar();
                }

                // ------------------------------------------------
                // Logs
                // ------------------------------------------------

                ImGui::Text("Logs");

                ImGui::BeginChild(
                    "Scrolling",
                    ImVec2(0, -35.0f),
                    ImGuiChildFlags_None,
                    ImGuiWindowFlags_HorizontalScrollbar
                );

                for (
                    const auto& logEntry :
                    Logger().GetLogHistory()
                )
                {
                    switch (logEntry.logLevel)
                    {
                        case LogLevel::INFO:
                        {
                            ImGui::TextColored(
                                RGBA(
                                    80,
                                    80,
                                    80,
                                    255
                                ),
                                "%s",
                                logEntry.message.c_str()
                            );

                            break;
                        }

                        case LogLevel::ERROR:
                        {
                            ImGui::TextColored(
                                RGBA(
                                    160,
                                    80,
                                    80,
                                    255
                                ),
                                "%s",
                                logEntry.message.c_str()
                            );

                            break;
                        }

                        case LogLevel::WARN:
                        {
                            ImGui::TextColored(
                                RGBA(
                                    160,
                                    160,
                                    80,
                                    255
                                ),
                                "%s",
                                logEntry.message.c_str()
                            );

                            break;
                        }

                        default:
                        {
                            ImGui::TextUnformatted(
                                logEntry.message.c_str()
                            );

                            break;
                        }
                    }
                }

                // ------------------------------------------------
                // Auto Scroll
                // ------------------------------------------------

                if (
                    Logger().GetLogHistory().size() >
                    previousLogCount
                )
                {
                    ImGui::SetScrollHereY(
                        1.0f
                    );
                }

                previousLogCount =
                    Logger().GetLogHistory().size();

                ImGui::EndChild();

                // ------------------------------------------------
                // Command Prompt
                // ------------------------------------------------

                {
                    bool executeCommand =
                        ImGui::InputText(
                            "##CommandPrompt",
                            commandInput,
                            sizeof(commandInput),
                            ImGuiInputTextFlags_EnterReturnsTrue
                        );

                    ImGui::SameLine();

                    if (ImGui::Button("Run"))
                    {
                        executeCommand = true;
                    }

                    if (executeCommand)
                    {
                        if (commandInput[0] != '\0')
                        {
                            GetCommandManager()
                                .ExecuteFromString(
                                    commandInput
                                );

                            commandInput[0] = '\0';
                        }
                    }
                }

                ImGui::End();
            }
        }

        // ========================================================
        // Draw
        // ========================================================

        {
            p_RenderApi->Draw(
                vertices,
                24,
                indices,
                36
            );
        }

        // ========================================================
        // ImGui Render
        // ========================================================

        {
            p_RenderApi->ImGuiRender();
        }

        // ========================================================
        // End Frame
        // ========================================================

        {
            p_RenderApi->RenderEnd();

            GetCommandManager().TickJob();

            GetCameraUBO().UpdateView();

            p_WindowApi->Update();

            if (p_WindowApi->ShouldClose() || !*keepRunning){
                Logger().warn("Window SHOULD close");
            }
        }
    }
}


Engine::~Engine()
{
    Logger().warn(
        "destroying..."
    );

    // ============================================================
    // Renderer Shutdown
    // ============================================================

    if (p_RenderApi)
    {
        p_RenderApi->Shutdown();

        delete p_RenderApi;

        p_RenderApi = nullptr;
    }

    // ============================================================
    // Window Shutdown
    // ============================================================

    if (p_WindowApi)
    {
        p_WindowApi->Destroy();

        delete p_WindowApi;

        p_WindowApi = nullptr;
    }
}