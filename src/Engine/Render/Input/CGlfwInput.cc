#include "CGlfwInput.h"
#include "../Types/CInputTypes.h"
#include "Engine/Command/CCommandManager.h"
#include "Engine/Render/Api/IInputApi.h"
#include "Engine/Render/Api/IRenderApi.h"
#include "Engine/Render/Types/CRenderTypes.h"
#include "Engine/Render/gui/imgui_impl_glfw.h"
#include "Engine/const.h"

#include "Engine/Render/Api/BindFlags.h"
#include <cmath>
#include <cstdint>


// Gets used by Commands implementation to lock/unlock cursor

static bool g_mouseShouldLock = false;


void SetGlobalMouseShouldLock(bool v){
    g_mouseShouldLock = v;
}; 
bool GetGlobalMouseShouldLock(){
    return g_mouseShouldLock;
}
void ToggleGlobalMouseShouldLock(){
    g_mouseShouldLock = !g_mouseShouldLock;
}

GlfwInputApi::GlfwInputApi(GLFWwindow* window)
    : m_Window(window)
{
    glfwSetWindowUserPointer(window, this);

    glfwSetKeyCallback(
        window,
        KeyCallback
    );

    glfwSetMouseButtonCallback(
        window,
        MouseButtonCallback
    );

    glfwSetCursorPosCallback(
        window,
        CursorPosCallback
    );

    glfwSetScrollCallback(
        window,
        ScrollCallback
    );

    glfwGetCursorPos(
        window,
        &m_MouseX,
        &m_MouseY
    );

    glfwSetCharCallback(window, CharCallback);

    m_LastMouseX = m_MouseX;
    m_LastMouseY = m_MouseY;

    SetGlobalInputApi(this);
}


void GlfwInputApi::Update()
{
    m_MouseDeltaX = 0.0;
    m_MouseDeltaY = 0.0;

    m_ScrollX = 0.0;
    m_ScrollY = 0.0;


    for (const Key key : m_HeldKeys)
    {
        ExecuteKeyBinding(
            key,
            PressType::Held
        );
    }

    mouseLocked = g_mouseShouldLock;

}

void GlfwInputApi::BindKey(
    Key key,
    PressType type,
    const std::string& command,
    uint16_t flags
)
{
    if (type == PressType::Released){
        // Logger().warn("BindKey({}, {}, \"{}\")", (int)key, (int)type, command);
    }
    m_KeyBindings[
        Binding{ key, type }
    ] = command;
}

void GlfwInputApi::BindMouseButton(
    MouseButton button,
    PressType type,
    const std::string& command
)
{
    // If you want mouse buttons to share the same Binding
    // structure, make a separate MouseBinding structure.
}

void GlfwInputApi::UnbindKey(
    Key key,
    PressType type
)
{
    m_KeyBindings.erase(
        Binding{ key, type }
    );
}

void GlfwInputApi::UnbindMouseButton(
    MouseButton button,
    PressType type
)
{
    // Same idea as UnbindKey.
}

void GlfwInputApi::KeyCallback(
    GLFWwindow* window,
    int key,
    int scancode,
    int action,
    int mods
)
{
    auto* input =
        static_cast<GlfwInputApi*>(
            glfwGetWindowUserPointer(window)
        );

    if (!input)
        return;
    if (!input->mouseLocked)
    {
        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    }

    const Key engineKey =
        FromGlfwKey(key);

    if (engineKey == Key::Unknown)
        return;
    // Logger().info("Recv key {}", (int)engineKey);
    if (action == GLFW_PRESS)
    {
        input->m_HeldKeys.insert(engineKey);

        input->ExecuteKeyBinding(
            engineKey,
            PressType::Pressed
        );
    }
    else if (action == GLFW_RELEASE)
    {
        // Logger().warn("Key released: {}", key);
        input->m_HeldKeys.erase(engineKey);

        input->ExecuteReleaseCommand(
            engineKey
        );
    }
}

void GlfwInputApi::ExecuteKeyBinding(
    Key key,
    PressType type
)
{
    const Binding binding{
        key,
        type
    };

    // TODO: fix keys with flags not being detected while mouse isn't locked
    
    const auto it =
        m_KeyBindings.find(binding);

    if (it == m_KeyBindings.end())
        return;

    if (!mouseLocked && !(it->first.flags & BINDFLAG_BYPASSGUI)){

        Logger().info("our mouse isn't locked and the binding's flags doesn't have BINDFLAG_BYPASSGUI, let's not execute.");
        return;
    }

    if (it == m_KeyBindings.end())
        return;


    GetCommandManager().ExecuteFromString(
        it->second
    );
}

void GlfwInputApi::ExecuteReleaseCommand(Key key)
{
    Binding binding{
        key,
        PressType::Pressed
    };

    auto it = m_KeyBindings.find(binding);

    // No Pressed binding? Try Released.
    if (it == m_KeyBindings.end())
    {
        binding.type = PressType::Released;
        it = m_KeyBindings.find(binding);
    }

    // Neither Pressed nor Released exists.
    if (it == m_KeyBindings.end())
    {
        // Logger().error("no such binding {}", (int)key);
        return;
    }

    // TODO: fix keys with flags not being detected while mouse isn't locked

    const std::string& command = it->second;

    if (command.empty())
    {
        // Logger().error("no such command");
        return;
    }
    if (!mouseLocked && !(it->first.flags & BINDFLAG_BYPASSGUI)){
        Logger().info("our mouse isn't locked and the binding's flags doesn't have BINDFLAG_BYPASSGUI, let's not execute.");
        return;
    }

    if (command[0] == '+')
    {
        std::string releaseCommand = command;
        releaseCommand[0] = '-';

        // Logger().warn(
        //     "Executing release command: {}",
        //     releaseCommand
        // );

        GetCommandManager().ExecuteFromString(releaseCommand);
    }
    else
    {
        GetCommandManager().ExecuteFromString(command);
    }
}

float GlfwInputApi::GetTime(){
    return glfwGetTime();
}

Key GlfwInputApi::FromGlfwKey(int key)
{
    switch (key)
    {
        case GLFW_KEY_A: return Key::A;
        case GLFW_KEY_B: return Key::B;
        case GLFW_KEY_C: return Key::C;
        case GLFW_KEY_D: return Key::D;
        case GLFW_KEY_E: return Key::E;
        case GLFW_KEY_F: return Key::F;
        case GLFW_KEY_G: return Key::G;
        case GLFW_KEY_H: return Key::H;
        case GLFW_KEY_I: return Key::I;
        case GLFW_KEY_J: return Key::J;
        case GLFW_KEY_K: return Key::K;
        case GLFW_KEY_L: return Key::L;
        case GLFW_KEY_M: return Key::M;
        case GLFW_KEY_N: return Key::N;
        case GLFW_KEY_O: return Key::O;
        case GLFW_KEY_P: return Key::P;
        case GLFW_KEY_Q: return Key::Q;
        case GLFW_KEY_R: return Key::R;
        case GLFW_KEY_S: return Key::S;
        case GLFW_KEY_T: return Key::T;
        case GLFW_KEY_U: return Key::U;
        case GLFW_KEY_V: return Key::V;
        case GLFW_KEY_W: return Key::W;
        case GLFW_KEY_X: return Key::X;
        case GLFW_KEY_Y: return Key::Y;
        case GLFW_KEY_Z: return Key::Z;

        case GLFW_KEY_0: return Key::Num0;
        case GLFW_KEY_1: return Key::Num1;
        case GLFW_KEY_2: return Key::Num2;
        case GLFW_KEY_3: return Key::Num3;
        case GLFW_KEY_4: return Key::Num4;
        case GLFW_KEY_5: return Key::Num5;
        case GLFW_KEY_6: return Key::Num6;
        case GLFW_KEY_7: return Key::Num7;
        case GLFW_KEY_8: return Key::Num8;
        case GLFW_KEY_9: return Key::Num9;

        case GLFW_KEY_ESCAPE:    return Key::Escape;
        case GLFW_KEY_ENTER:     return Key::Enter;
        case GLFW_KEY_TAB:       return Key::Tab;
        case GLFW_KEY_BACKSPACE: return Key::Backspace;
        case GLFW_KEY_SPACE:     return Key::Space;

        case GLFW_KEY_LEFT:  return Key::Left;
        case GLFW_KEY_RIGHT: return Key::Right;
        case GLFW_KEY_UP:    return Key::Up;
        case GLFW_KEY_DOWN:  return Key::Down;

        case GLFW_KEY_LEFT_SHIFT:    return Key::LeftShift;
        case GLFW_KEY_RIGHT_SHIFT:   return Key::RightShift;
        case GLFW_KEY_LEFT_CONTROL:  return Key::LeftControl;
        case GLFW_KEY_RIGHT_CONTROL: return Key::RightControl;
        case GLFW_KEY_LEFT_ALT:      return Key::LeftAlt;
        case GLFW_KEY_RIGHT_ALT:     return Key::RightAlt;

        default:
            // Logger().error("Unkwnown key: {}", key);
            return Key::Unknown;
    }
}

void GlfwInputApi::GetMousePosition(
    double& x,
    double& y
) const
{
    x = m_MouseX;
    y = m_MouseY;
}

void GlfwInputApi::GetMouseDelta(
    double& x,
    double& y
) const
{
    x = m_MouseDeltaX;
    y = m_MouseDeltaY;
}

double GlfwInputApi::GetScrollX() const
{
    return m_ScrollX;
}

double GlfwInputApi::GetScrollY() const
{
    return m_ScrollY;
}

void GlfwInputApi::CursorPosCallback(
    GLFWwindow* window,
    double x,
    double y
)
{
    auto* input =
        static_cast<GlfwInputApi*>(
            glfwGetWindowUserPointer(window)
        );

    if (!input)
        return;

    input->m_MouseDeltaX =
        x - input->m_MouseX;

    input->m_MouseDeltaY =
        y - input->m_MouseY;

    input->m_MouseX = x;
    input->m_MouseY = y;

    // Set mouse lock state.



    if (input->mouseLocked)
    {
        glfwSetInputMode(
            window,
            GLFW_CURSOR,
            GLFW_CURSOR_DISABLED
        );
        // int w, h;
        // glfwGetWindowSize(input->m_Window, &w, &h);
        // glfwSetCursorPos(input->m_Window, (double)w/2,(double)h/2);
    }
    else
    {
        glfwSetInputMode(
            window,
            GLFW_CURSOR,
            GLFW_CURSOR_NORMAL
        );

    
    }

    if (!input->mouseLocked){

        // if the mouse ISN'T locked we can pass it to ImGui's implementation
        ImGui_ImplGlfw_CursorPosCallback(window, x, y);
    } else {
        // if it IS locked we update our CameraUBO with pitch and yaw
        CameraUBO& cameraUBO = GetCameraUBO();
        glm::vec2 rot = cameraUBO.GetRot();
        // {yaw, pitch}
        rot.y += -1*input->m_MouseDeltaY * DEFAULT_SENSITIVITY;
        rot.y = glm::clamp(rot.y, -89.999f, 89.999f);

        rot.x += (input->m_MouseDeltaX * DEFAULT_SENSITIVITY );

        rot.x = std::fmod(rot.x, 360.0f); // Wrap around
        GetCameraUBO().SetRot(rot);
        
    }
    
}

void GlfwInputApi::ScrollCallback(
    GLFWwindow* window,
    double x,
    double y
)
{
    auto* input =
        static_cast<GlfwInputApi*>(
            glfwGetWindowUserPointer(window)
        );

    if (!input)
        return;

    input->m_ScrollX = x;
    input->m_ScrollY = y;

    if (!input->mouseLocked)
        ImGui_ImplGlfw_ScrollCallback(window, x, y);
}

void GlfwInputApi::MouseButtonCallback(
    GLFWwindow* window,
    int button,
    int action,
    int mods
)
{
    auto* input =
        static_cast<GlfwInputApi*>(
            glfwGetWindowUserPointer(window)
        );

    if (!input)
        return;
    
    if (!input->mouseLocked)
        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    // Mouse button handling can be implemented
    // once MouseButton bindings are defined.
}

void GlfwInputApi::CharCallback(GLFWwindow* window, unsigned int codepoint){
    auto* input =
        static_cast<GlfwInputApi*>(
            glfwGetWindowUserPointer(window)
        );
    if (!input)
        return;
    if (!input->mouseLocked){
        ImGui_ImplGlfw_CharCallback(window, codepoint);
    }
}