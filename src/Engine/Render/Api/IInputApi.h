#pragma once

#include "../Types/CInputTypes.h"

#include <string>

class IInputApi
{
public:
    virtual ~IInputApi() = default;

    virtual void Update() = 0;

    virtual void BindKey(
        Key key,
        PressType type,
        const std::string& command,
        uint16_t flags = 0
    ) = 0;

    virtual void BindMouseButton(
        MouseButton button,
        PressType type,
        const std::string& command
    ) = 0;

    virtual void UnbindKey(
        Key key,
        PressType type
    ) = 0;

    virtual void UnbindMouseButton(
        MouseButton button,
        PressType type
    ) = 0;

    virtual void GetMousePosition(
        double& x,
        double& y
    ) const = 0;

    virtual void GetMouseDelta(
        double& x,
        double& y
    ) const = 0;

    virtual double GetScrollX() const = 0;
    virtual double GetScrollY() const = 0;

    virtual float GetTime() = 0;
};

void SetGlobalInputApi(IInputApi* inputApi);
IInputApi* GetGlobalInputApi();


void SetGlobalMouseShouldLock(bool);
void ToggleGlobalMouseShouldLock();
bool GetGlobalMouseShouldLock();


struct Binding
{
    Key key;
    PressType type;
    uint16_t flags;
    bool operator==(const Binding& other) const
    {
        return key == other.key &&
               type == other.type;
    }
};

struct BindingHash
{
    std::size_t operator()(const Binding& binding) const
    {
        const std::size_t keyHash =
            std::hash<int>{}(
                static_cast<int>(binding.key)
            );
        const std::size_t typeHash =
            std::hash<int>{}(
                static_cast<int>(binding.type)
            );
        return keyHash ^ (typeHash << 1);
    }
};