#include "Commands.h"
#include "Engine/Command/CCommand.h"
#include "Engine/Render/Api/IInputApi.h"
#include "Engine/Render/Api/IRenderApi.h"
#include "Engine/Command/CCommandManager.h"
#include "Engine/Render/Api/IWindowApi.h"
#include "Engine/Render/Types/CInputTypes.h"
#include "Engine/const.h"
static bool g_ShouldStillMoveForward = false;

void HandleForward()
{
    Job j;
    j.id = 'a';
    j.shouldKeepRunning = &g_ShouldStillMoveForward;

    j.job = [](bool* b)
    {
        if (!b || !*b)
            return;

        auto& cameraUBO = GetCameraUBO();

        static double lastTime = GetGlobalInputApi()->GetTime();

        double currentTime = GetGlobalInputApi()->GetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);

        lastTime = currentTime;

        cameraUBO.AddPos(
            cameraUBO.GetForwardVec() *
            MOVE_SPEED *
            deltaTime
        );
        Logger().info("{} {} {}", cameraUBO.GetPos().x, cameraUBO.GetPos().y, cameraUBO.GetPos().z);
    };

    GetCommandManager().StartJob("+forward", j);
}
void HandleStopMovingForward(){
    g_ShouldStillMoveForward = false;
    GetCommandManager().StopJob("+forward", 'a');
}

CommandResult HandleBind(const std::vector<std::string>& args)
{
    if (args.size() < 3)
    {
        Logger().info(
            "Invalid command, usage: bind key presstype command arg1 arg2..."
        );
        return CommandResult::SyntaxError;
    }

    std::string key = args[0];
    std::string pressType = args[1];

    std::string command;

    for (size_t i = 2; i < args.size(); i++)
    {
        if (!command.empty())
            command += ' ';

        command += args[i];
    }

    GetGlobalInputApi()->BindKey(
        GetKeyFromString(key),
        GetPressTypeFromString(pressType),
        command
    );

    return CommandResult::Success;
}