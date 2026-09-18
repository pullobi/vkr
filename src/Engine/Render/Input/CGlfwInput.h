#pragma once

#include "../Api/IInputApi.h"
#include "../Api/IWindowApi.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>

class GlfwInputApi : public IInputApi
{
public:
    explicit GlfwInputApi(GLFWwindow* window);

    void Update() override;

    void BindKey(
        Key key,
        PressType type,
        const std::string& command,
        uint16_t flags = 0
    ) override;

    void BindMouseButton(
        MouseButton button,
        PressType type,
        const std::string& command
    ) override;

    void UnbindKey(
        Key key,
        PressType type
    ) override;

    void UnbindMouseButton(
        MouseButton button,
        PressType type
    ) override;

    void GetMousePosition(
        double& x,
        double& y
    ) const override;

    void GetMouseDelta(
        double& x,
        double& y
    ) const override;

    double GetScrollX() const override;
    double GetScrollY() const override;

    float GetTime() override;


private:
    static void KeyCallback(
        GLFWwindow* window,
        int key,
        int scancode,
        int action,
        int mods
    );

    static void MouseButtonCallback(
        GLFWwindow* window,
        int button,
        int action,
        int mods
    );

    static void CursorPosCallback(
        GLFWwindow* window,
        double x,
        double y
    );

    static void ScrollCallback(
        GLFWwindow* window,
        double xOffset,
        double yOffset
    );

    static void CharCallback(GLFWwindow* window, unsigned int codepoint);

private:
    static Key FromGlfwKey(int key);
    static MouseButton FromGlfwMouseButton(int button);

    void ExecuteKeyBinding(
        Key key,
        PressType type
    );

    void ExecuteReleaseCommand(Key key);


    bool mouseLocked = false;
private:
    GLFWwindow* m_Window;

    std::unordered_map<
        Binding,
        std::string,
        BindingHash
    > m_KeyBindings;

    std::unordered_map<
        Binding,
        std::string,
        BindingHash
    > m_MouseBindings;

    std::unordered_set<Key> m_HeldKeys;

    

    double m_MouseX = 0.0;
    double m_MouseY = 0.0;

    double m_LastMouseX = 0.0;
    double m_LastMouseY = 0.0;

    double m_MouseDeltaX = 0.0;
    double m_MouseDeltaY = 0.0;

    double m_ScrollX = 0.0;
    double m_ScrollY = 0.0;
};


