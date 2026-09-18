#pragma once

#include <glm/vec3.hpp>

namespace PlayerMovement
{
    void Forward();
    void StopForward();

    void Back();
    void StopBack();

    void Left();
    void StopLeft();

    void Right();
    void StopRight();

    void Up();
    void StopUp();

    void Down();
    void StopDown();

    
    void Update();
}