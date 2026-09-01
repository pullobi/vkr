#include "Engine/Command/CCommand.h"
#include "Engine/Command/CCommandManager.h"
#include "Engine/Command/MCommand.h"
#include "Engine/Renderer/gui/imgui.h"
#include "Logger/Logger.h"
#include "Engine/Renderer/CGlfwWindow.h"
#include "Engine/Renderer/CVulkanRenderer.h"
#include <csignal>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>

#define RGBA(r, g, b, a) \
    ImVec4((float)(r) / 255.0f, \
           (float)(g) / 255.0f, \
           (float)(b) / 255.0f, \
           (float)(a) / 255.0f)

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

RegisterCommand("help", "Test Command")
{
    Logger().info("help called");
    auto& commandManager = GetCommandManager();

    if (!args.empty() && !args[0].empty())
    {
        CCommand* command =
            commandManager.FindCommand(args[0]);

        if (!command)
        {
            Logger().warn("Unknown command: {}", args[0]);
            return CommandResult::Failed;
        }

        Logger().info(
            "{}",
            command->GetDescription()
        );
    }
    else
    {   
        Logger().info("==== help page ====");
        for (auto& command : commandManager.m_Commands){
            Logger().info("{}",command.second.GetHelpDescription());
        }
        Logger().info("===================");
    }

    return CommandResult::Success;
}



RegisterCommand("name", "description"){
    if (!args.empty() && !args[0].empty())
        Logger().info("{}", args[0]);
    else
        Logger().warn(" ");
    return CommandResult::Success;
}

int main()
{
    // ============================================================
    // Initialization
    // ============================================================
    signal(SIGTERM, [](int signal){
        goto stop;
    });
    {
        Logger().info("Start");

        CGlfwWindow window;

        window.SetWindowOptions({
            .size = {
                .x = 800,
                .y = 600
            },
            .title = "std::string title"
        });

        window.Create();

        Logger().info("create GLFW window");

        CVulkanRenderer renderer;

        renderer.Init(&window);

        Logger().info("created renderer");
        auto& commandManager = GetCommandManager();

        if (commandManager.FindCommand("help"))
        {
            Logger().info("help command IS registered");
        }
        else
        {
            Logger().error("help command IS NOT registered");
        }

        // ========================================================
        // Camera
        // ========================================================

        glm::vec3 camPos = {
            0.0f,
            0.0f,
            3.0f
        };

        float camYaw = -90.0f;
        float camPitch = 0.0f;


        // ========================================================
        // UI State
        // ========================================================

        bool b_LoggerOpen = true;
        bool b_CameraControlsOpen = true;

        char commandInput[512] = "";

        size_t previousLogCount =
            Logger().GetLogHistory().size();

        Logger().info(
            "log Count {}",
            previousLogCount
        );


        // ========================================================
        // Main Loop
        // ========================================================

        while (!window.ShouldClose())
        {
            // ====================================================
            // Camera
            // ====================================================

            {
                float yaw =
                    glm::radians(camYaw);

                float pitch =
                    glm::radians(camPitch);

                glm::vec3 forward;

                forward.x =
                    cos(pitch) * cos(yaw);

                forward.y =
                    sin(pitch);

                forward.z =
                    cos(pitch) * sin(yaw);

                forward =
                    glm::normalize(forward);

                glm::vec3 camTarget =
                    camPos + forward;


                // -----------------------------------------------
                // Update camera matrices
                // -----------------------------------------------

                renderer.m_CameraUBO.view =
                    glm::lookAt(
                        camPos,
                        camTarget,
                        glm::vec3(
                            0.0f,
                            1.0f,
                            0.0f
                        )
                    );

                renderer.m_CameraUBO.projection =
                    glm::perspective(
                        glm::radians(60.0f),

                        static_cast<float>(
                            renderer
                                .getSwapchainExtent()
                                .width
                        )
                        /
                        static_cast<float>(
                            renderer
                                .getSwapchainExtent()
                                .height
                        ),

                        0.1f,
                        100.0f
                    );

                renderer.m_CameraUBO.projection[1][1] *= -1.0f;

                renderer.UpdateCameraBuffer();
            }


            // ====================================================
            // Begin Frame
            // ====================================================

            {
                renderer.RenderBegin();
                renderer.ImGuiNewFrame();
            }


            // ====================================================
            // Keyboard Shortcuts
            // ====================================================

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


            // ====================================================
            // ImGui
            // ====================================================

            {

                // ==================================================
                // Camera Controls Window
                // ==================================================

                {
                    if (b_CameraControlsOpen)
                    {
                        ImGui::Begin(
                            "3D Camera",
                            &b_CameraControlsOpen
                        );

                        ImGui::Text("Camera");

                        ImGui::SliderFloat3(
                            "Position",
                            &camPos[0],
                            -10.0f,
                            10.0f
                        );

                        ImGui::SliderFloat(
                            "Yaw",
                            &camYaw,
                            -180.0f,
                            180.0f
                        );

                        ImGui::SliderFloat(
                            "Pitch",
                            &camPitch,
                            -89.0f,
                            89.0f
                        );


                        // ------------------------------------------
                        // Forward
                        // ------------------------------------------

                        float yaw =
                            glm::radians(camYaw);

                        float pitch =
                            glm::radians(camPitch);

                        glm::vec3 forward;

                        forward.x =
                            cos(pitch) * cos(yaw);

                        forward.y =
                            sin(pitch);

                        forward.z =
                            cos(pitch) * sin(yaw);

                        forward =
                            glm::normalize(forward);

                        ImGui::Text(
                            "Forward: %.2f %.2f %.2f",
                            forward.x,
                            forward.y,
                            forward.z
                        );

                        ImGui::End();
                    }
                }


                // ==================================================
                // Logger Window
                // ==================================================

                {
                    if (b_LoggerOpen)
                    {
                        ImGui::Begin(
                            "Vulkan",
                            &b_LoggerOpen,
                            ImGuiWindowFlags_MenuBar
                        );


                        // ------------------------------------------
                        // Menu Bar
                        // ------------------------------------------

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


                        // ------------------------------------------
                        // Logs
                        // ------------------------------------------

                        ImGui::Text("Logs");

                        ImGui::BeginChild(
                            "Scrolling",
                            ImVec2(0, -35.0f),
                            ImGuiChildFlags_None,
                            ImGuiWindowFlags_HorizontalScrollbar
                        );

                        for (const auto& logEntry :
                             Logger().GetLogHistory())
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


                        // ------------------------------------------
                        // Auto Scroll
                        // ------------------------------------------

                        if (Logger().GetLogHistory().size()
                            > previousLogCount)
                        {
                            ImGui::SetScrollHereY(1.0f);
                        }

                        previousLogCount =
                            Logger().GetLogHistory().size();

                        ImGui::EndChild();


                        // ------------------------------------------
                        // Command Prompt
                        // ------------------------------------------

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


                        // ------------------------------------------
                        // End Logger
                        // ------------------------------------------

                        ImGui::End();
                    }
                }
            }


            // ====================================================
            // Draw
            // ====================================================

            {
                renderer.Draw(
                    vertices,
                    24,
                    indices,
                    36
                );
            }


            // ====================================================
            // ImGui Render
            // ====================================================

            {
                renderer.ImGuiRender();
            }


            // ====================================================
            // End Frame
            // ====================================================

            {
                renderer.RenderEnd();
                window.PollEvents();
            }
        }


        // ========================================================
        // Shutdown
        // ========================================================
stop:
        {
            Logger().warn("destroying...");

            renderer.Shutdown();
            window.Destroy();
        }
    }
}