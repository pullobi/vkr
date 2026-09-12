#include "CGlfwInput.h"
#include "../Types/CInputTypes.h"
#include "Engine/Command/CCommandManager.h"
#include "Engine/Render/Api/IInputApi.h"
#include "Engine/Render/gui/imgui_impl_glfw.h"



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
}

void GlfwInputApi::BindKey(
    Key key,
    PressType type,
    const std::string& command
)
{
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
        return;
    }

    const Key engineKey =
        FromGlfwKey(key);

    if (engineKey == Key::Unknown)
        return;

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

    const auto it =
        m_KeyBindings.find(binding);

    if (it == m_KeyBindings.end())
        return;

    GetCommandManager().ExecuteFromString(
        it->second
    );
}

void GlfwInputApi::ExecuteReleaseCommand(Key key)
{
    const Binding binding{
        key,
        PressType::Pressed
    };

    const auto it =
        m_KeyBindings.find(binding);

    if (it == m_KeyBindings.end())
        return;

    const std::string& command =
        it->second;

    if (command.empty())
        return;

    if (command[0] != '+')
        return;

    std::string releaseCommand = command;

    releaseCommand[0] = '-';

    GetCommandManager().ExecuteFromString(
        releaseCommand
    );
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

    if (input->mouseLocked){
        ImGui_ImplGlfw_CursorPosCallback(window, x, y);
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