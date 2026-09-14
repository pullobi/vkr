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
    glm::vec3 forward = GetForwardVec();

    view = glm::lookAt(
        pos,
        pos + forward,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
}

glm::vec3 CameraUBO::GetForwardVec() const
{
    float yaw = glm::radians(rot.x);
    float pitch = glm::radians(rot.y);

    glm::vec3 forward;

    forward.x = glm::cos(pitch) * glm::sin(yaw);
    forward.y = glm::sin(pitch);
    forward.z = -glm::cos(pitch) * glm::cos(yaw);

    return glm::normalize(forward);
}