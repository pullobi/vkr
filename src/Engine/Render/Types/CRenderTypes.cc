#include "CRenderTypes.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include "Engine/Render/Api/IWindowApi.h"

glm::vec3 CameraUBO::GetPos() const
{
    return pos;
}

void CameraUBO::SetPos(glm::vec3 newPos)
{
    pos = newPos;
    UpdateView();
}

void CameraUBO::AddPos(glm::vec3 offset)
{
    pos += offset;
    UpdateView();
}

glm::vec2 CameraUBO::GetRot() const
{
    return rot;
}

void CameraUBO::SetRot(glm::vec2 newRot)
{
    rot = newRot;
    UpdateView();
}

void CameraUBO::UpdateView()
{
    float yaw = glm::radians(rot.x);
    float pitch = glm::radians(rot.y);

    glm::vec3 forward;
    forward.x = cos(pitch) * sin(yaw);
    forward.y = sin(pitch);
    forward.z = -cos(pitch) * cos(yaw);

    view = glm::lookAt(
        pos,
        pos + glm::normalize(forward),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    
    
}

glm::vec3 CameraUBO::GetForwardVec() const
{
    return glm::normalize(glm::vec3(
        glm::cos(rot.y) * glm::cos(rot.x),
        glm::sin(rot.y),
        glm::cos(rot.y) * glm::sin(rot.x)
    ));
}