#include "PlayerMovement.h"

#include "Engine/Command/CCommandManager.h"
#include "Engine/Render/Api/IInputApi.h"
#include "Engine/Render/Api/IRenderApi.h"
#include "Engine/const.h"

#include <glm/glm.hpp>


namespace
{
    constexpr uint32_t HandlerOwnerForward = 'P' + 1;

    constexpr const char* PlayerMoveHandlerJobName =
        "PlayerMoveHandlerJob";

    glm::vec3 g_PlayerMoveDesire{0.0f};

    bool g_PlayerMoveDesireNonZero = false;

    uint32_t g_PlayerMoveHandlerJobOwner = 0;
}


// ============================================================
// Movement update
// ============================================================

void PlayerMovement::Update()
{
    auto& cameraUBO = GetCameraUBO();

    static double lastTime =
        GetGlobalInputApi()->GetTime();

    const double currentTime =
        GetGlobalInputApi()->GetTime();

    const float deltaTime =
        static_cast<float>(currentTime - lastTime);

    lastTime = currentTime;

    if (deltaTime <= 0.0f)
        return;

    if (!g_PlayerMoveDesireNonZero)
        return;

    glm::vec3 forward =
        cameraUBO.GetForwardVec();

    // FPS-style movement:
    // Looking up/down should not make the player fly.
    forward.y = 0.0f;

    if (glm::length(forward) > 0.0f)
        forward = glm::normalize(forward);

    glm::vec3 right =
        glm::normalize(
            glm::cross(
                forward,
                glm::vec3(0.0f, 1.0f, 0.0f)
            )
        );

    glm::vec3 movement =
        right * g_PlayerMoveDesire.x +
        forward * g_PlayerMoveDesire.z;

    if (glm::length(movement) > 0.0f)
        movement = glm::normalize(movement);

    cameraUBO.AddPos(
        movement *
        MOVE_SPEED *
        deltaTime *
        0.5f
    );
}


// ============================================================
// Job management
// ============================================================

namespace
{
    void EnsureMovementJob()
    {
        if (g_PlayerMoveHandlerJobOwner != 0)
            return;

        Job job{
            .id = HandlerOwnerForward,
            .shouldKeepRunning = &g_PlayerMoveDesireNonZero
        };

        g_PlayerMoveHandlerJobOwner =
            HandlerOwnerForward;

        job.job = [](bool* shouldKeepRunning)
        {
            if (!g_PlayerMoveDesireNonZero)
            {
                *shouldKeepRunning = false;
            }

            if (g_PlayerMoveHandlerJobOwner !=
                HandlerOwnerForward)
            {
                *shouldKeepRunning = false;
            }

            if (!*shouldKeepRunning)
            {
                if (g_PlayerMoveHandlerJobOwner ==
                    HandlerOwnerForward)
                {
                    g_PlayerMoveHandlerJobOwner = 0;
                }

                return;
            }

            PlayerMovement::Update();
        };

        GetCommandManager().StartJob(
            PlayerMoveHandlerJobName,
            job
        );
    }
}


// ============================================================
// Forward / Back
// ============================================================

void PlayerMovement::Forward()
{
    g_PlayerMoveDesire.z = 1.0f;
    g_PlayerMoveDesireNonZero =
        g_PlayerMoveDesire != glm::vec3(0.0f);

    EnsureMovementJob();
}

void PlayerMovement::StopForward()
{
    g_PlayerMoveDesire.z = 0.0f;
    g_PlayerMoveDesireNonZero =
        g_PlayerMoveDesire != glm::vec3(0.0f);
}


void PlayerMovement::Back()
{
    g_PlayerMoveDesire.z = -1.0f;
    g_PlayerMoveDesireNonZero =
        g_PlayerMoveDesire != glm::vec3(0.0f);

    EnsureMovementJob();
}

void PlayerMovement::StopBack()
{
    g_PlayerMoveDesire.z = 0.0f;
    g_PlayerMoveDesireNonZero =
        g_PlayerMoveDesire != glm::vec3(0.0f);
}


// ============================================================
// Left / Right
// ============================================================

void PlayerMovement::Left()
{
    g_PlayerMoveDesire.x = -1.0f;
    g_PlayerMoveDesireNonZero =
        g_PlayerMoveDesire != glm::vec3(0.0f);

    EnsureMovementJob();
}

void PlayerMovement::StopLeft()
{
    g_PlayerMoveDesire.x = 0.0f;
    g_PlayerMoveDesireNonZero =
        g_PlayerMoveDesire != glm::vec3(0.0f);
}


void PlayerMovement::Right()
{
    g_PlayerMoveDesire.x = 1.0f;
    g_PlayerMoveDesireNonZero =
        g_PlayerMoveDesire != glm::vec3(0.0f);

    EnsureMovementJob();
}

void PlayerMovement::StopRight()
{
    g_PlayerMoveDesire.x = 0.0f;
    g_PlayerMoveDesireNonZero =
        g_PlayerMoveDesire != glm::vec3(0.0f);
}

void PlayerMovement::Up(){
    g_PlayerMoveDesire.y = 1.0f;
    g_PlayerMoveDesireNonZero =
        g_PlayerMoveDesire != glm::vec3(0.0f);

    EnsureMovementJob();
}

void PlayerMovement::StopUp()
{
    g_PlayerMoveDesire.y = 0.0f;
    g_PlayerMoveDesireNonZero =
        g_PlayerMoveDesire != glm::vec3(0.0f);
}

void PlayerMovement::Down(){
    g_PlayerMoveDesire.y = -1.0f;
    g_PlayerMoveDesireNonZero =
        g_PlayerMoveDesire != glm::vec3(0.0f);

    EnsureMovementJob();
}

void PlayerMovement::StopDown()
{
    g_PlayerMoveDesire.y = 0.0f;
    g_PlayerMoveDesireNonZero =
        g_PlayerMoveDesire != glm::vec3(0.0f);
}

