#pragma once

#include <cstdint>

enum class Key : uint16_t
{
    Unknown = 0,

    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

    Num0, Num1, Num2, Num3, Num4,
    Num5, Num6, Num7, Num8, Num9,

    Escape,
    Enter,
    Tab,
    Backspace,
    Space,

    Left,
    Right,
    Up,
    Down,

    LeftShift,
    RightShift,
    LeftControl,
    RightControl,
    LeftAlt,
    RightAlt
};


#include <unordered_map>
#include <string>

inline Key GetKeyFromString(const std::string& key)
{
    static const std::unordered_map<std::string, Key> keyMap =
    {
        {"A", Key::A},
        {"B", Key::B},
        {"C", Key::C},
        {"D", Key::D},
        {"E", Key::E},
        {"F", Key::F},
        {"G", Key::G},
        {"H", Key::H},
        {"I", Key::I},
        {"J", Key::J},
        {"K", Key::K},
        {"L", Key::L},
        {"M", Key::M},
        {"N", Key::N},
        {"O", Key::O},
        {"P", Key::P},
        {"Q", Key::Q},
        {"R", Key::R},
        {"S", Key::S},
        {"T", Key::T},
        {"U", Key::U},
        {"V", Key::V},
        {"W", Key::W},
        {"X", Key::X},
        {"Y", Key::Y},
        {"Z", Key::Z},

        {"0", Key::Num0},
        {"1", Key::Num1},
        {"2", Key::Num2},
        {"3", Key::Num3},
        {"4", Key::Num4},
        {"5", Key::Num5},
        {"6", Key::Num6},
        {"7", Key::Num7},
        {"8", Key::Num8},
        {"9", Key::Num9},

        {"ESCAPE", Key::Escape},
        {"ENTER", Key::Enter},
        {"TAB", Key::Tab},
        {"BACKSPACE", Key::Backspace},
        {"SPACE", Key::Space},

        {"LEFT", Key::Left},
        {"RIGHT", Key::Right},
        {"UP", Key::Up},
        {"DOWN", Key::Down},

        {"LSHIFT", Key::LeftShift},
        {"RSHIFT", Key::RightShift},
        {"LCTRL", Key::LeftControl},
        {"RCTRL", Key::RightControl},
        {"LALT", Key::LeftAlt},
        {"RALT", Key::RightAlt}
    };

    auto it = keyMap.find(key);

    if (it == keyMap.end())
        return Key::Unknown;

    return it->second;
}

enum class MouseButton : uint8_t
{
    Left,
    Right,
    Middle,
    Button4,
    Button5
};

enum class PressType : uint8_t
{
    Pressed,
    Held,
    Released
};

inline PressType GetPressTypeFromString(std::string& press){

    static const std::unordered_map<std::string, PressType> pressMap = {
        {"pressed", PressType::Pressed },
        {"held", PressType::Held },
        {"released", PressType::Released }
    };
    auto it = pressMap.find(press);
    if (it == pressMap.end()){
        return PressType::Released;
    }
    return it->second;
}