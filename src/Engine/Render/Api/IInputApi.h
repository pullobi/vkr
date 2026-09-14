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
        const std::string& command
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
