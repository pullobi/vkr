#include "Commands.h"
#include "Engine/Command/CCommand.h"
#include "Engine/Command/CommandLists.h"
#include "Engine/Render/Api/IInputApi.h"
#include "Engine/Render/Api/IRenderApi.h"
#include "Engine/Command/CCommandManager.h"
#include "Engine/Render/Types/CInputTypes.h"
#include "Engine/const.h"
#include <_stdlib.h>


#define PlayerMoveHandlerJobName "PlayerMoveHandlerJob"
#define HandlerOwnerPrefix 'P'
#define HandlerOwnerForward HandlerOwnerPrefix + 1
#define HandlerOwnerBack HandlerOwnerPrefix + 2
#define HandlerOwnerLeft HandlerOwnerPrefix + 3
#define HandlerOwnerRight HandlerOwnerPrefix + 4

static glm::vec3 PlayerMoveDesire{0.0f};
static bool g_bool_PlayerMoveDesireNonZero = false;
static uint32_t g_uint32_t_PlayerMoveHandlerJobOwner = 0;


// ============================================================
// Player Movement
// ============================================================

void HandlePlayerMoveVec()
{
    auto& cameraUBO = GetCameraUBO();

    static double lastTime =
        GetGlobalInputApi()->GetTime();

    double currentTime =
        GetGlobalInputApi()->GetTime();

    float deltaTime =
        static_cast<float>(currentTime - lastTime);

    lastTime = currentTime;

    if (deltaTime <= 0.0f)
        return;

    if (PlayerMoveDesire == glm::vec3(0.0f))
        return;

    glm::vec3 forward =
        cameraUBO.GetForwardVec();

    // FPS-style movement:
    // Looking up/down should not make us fly.
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
        right * PlayerMoveDesire.x +
        forward * PlayerMoveDesire.z;

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
// +forward / -forward
// ============================================================

void HandleForward()
{
    PlayerMoveDesire.z = 1.0f;

    g_bool_PlayerMoveDesireNonZero =
        PlayerMoveDesire != glm::vec3(0.0f);
}

void HandleStopMovingForward()
{
    PlayerMoveDesire.z = 0.0f;

    g_bool_PlayerMoveDesireNonZero =
        PlayerMoveDesire != glm::vec3(0.0f);
}


// ============================================================
// +back / -back
// ============================================================

void HandleBack()
{
    PlayerMoveDesire.z = -1.0f;

    g_bool_PlayerMoveDesireNonZero =
        PlayerMoveDesire != glm::vec3(0.0f);
}

void HandleStopMovingBack()
{
    PlayerMoveDesire.z = 0.0f;

    g_bool_PlayerMoveDesireNonZero =
        PlayerMoveDesire != glm::vec3(0.0f);
}


// ============================================================
// +left / -left
// ============================================================

void HandleLeft()
{
    PlayerMoveDesire.x = -1.0f;

    g_bool_PlayerMoveDesireNonZero =
        PlayerMoveDesire != glm::vec3(0.0f);
}

void HandleStopMovingLeft()
{
    PlayerMoveDesire.x = 0.0f;

    g_bool_PlayerMoveDesireNonZero =
        PlayerMoveDesire != glm::vec3(0.0f);
}


// ============================================================
// +right / -right
// ============================================================

void HandleRight()
{
    PlayerMoveDesire.x = 1.0f;

    g_bool_PlayerMoveDesireNonZero =
        PlayerMoveDesire != glm::vec3(0.0f);
}

void HandleStopMovingRight()
{
    PlayerMoveDesire.x = 0.0f;

    g_bool_PlayerMoveDesireNonZero =
        PlayerMoveDesire != glm::vec3(0.0f);
}

static void EnsurePlayerMoveHandler()
{
    if (g_uint32_t_PlayerMoveHandlerJobOwner != 0)
        return;

    Job j{
        .id = HandlerOwnerForward,
        .shouldKeepRunning = &g_bool_PlayerMoveDesireNonZero
    };

    g_uint32_t_PlayerMoveHandlerJobOwner =
        HandlerOwnerForward;

    j.job = [](bool* shouldKeepRunning)
    {
        if (!g_bool_PlayerMoveDesireNonZero)
        {
            *shouldKeepRunning = false;
        }

        if (g_uint32_t_PlayerMoveHandlerJobOwner != HandlerOwnerForward)
        {
            *shouldKeepRunning = false;
        }

        if (!*shouldKeepRunning)
        {
            if (g_uint32_t_PlayerMoveHandlerJobOwner == HandlerOwnerForward)
            {
                g_uint32_t_PlayerMoveHandlerJobOwner = 0;
            }

            return;
        }

        HandlePlayerMoveVec();
    };

    GetCommandManager().StartJob(
        PlayerMoveHandlerJobName,
        j
    );
}
// ============================================================
// bind
// ============================================================

CommandResult HandleBind(
    const std::vector<std::string>& args)
{   Logger().info("handle bind...\n{}", args[0]);
    if (args.size() < 3)
    {
        Logger().info(
            "Invalid command, usage: "
            "bind key presstype command arg1 arg2..."
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
    Logger().info("Bind key {}, pressType {}, {}",key, pressType, command);
    GetGlobalInputApi()->BindKey(
        GetKeyFromString(key),
        GetPressTypeFromString(pressType),
        command
    );

    return CommandResult::Success;
}


// ============================================================
// Commands
// ============================================================

static CommandResult HelpCommand(
    const std::vector<std::string>& args)
{
    Logger().info("help called");

    auto& commandManager = GetCommandManager();

    if (!args.empty() && !args[0].empty())
    {
        CCommand* command =
            commandManager.FindCommand(args[0]);

        if (!command)
        {
            Logger().warn(
                "Unknown command: {}",
                args[0]
            );

            return CommandResult::Failed;
        }

        Logger().info(
            "{}",
            command->GetDescription()
        );
    }
    else
    {
        Logger().info(
            "==== help page ===="
        );

        for (auto& command :
             commandManager.m_Commands)
        {
            Logger().info(
                "{}",
                command.second.GetHelpDescription()
            );
        }

        Logger().info(
            "==================="
        );
    }

    return CommandResult::Success;
}


static CommandResult EchoCommand(
    const std::vector<std::string>& args)
{
    if (!args.empty() && !args[0].empty())
    {
        Logger().info("{}", args[0]);
    }
    else
    {
        Logger().warn(" ");
    }

    return CommandResult::Success;
}


static CommandResult BindCommand(
    const std::vector<std::string>& args)
{
    return HandleBind(args);
}

/*

Update our player vectors and do the following
Check if our player move vec job does exist and has an owner, if so, do nothing.
If not, then start a job with id HandlerOwnerForward amd set our g_uint32_t_PlayerMoveHandlerJobOwner to it.
In that job that will tick for every frame in the game;
Check if the player move desire is zero (which means the player Stopped moving) and if it is, stop running.
Check if the job's Id we are currently running matches the one we set globally with g_uint32_t_PlayerMoveHandlerJobOwner,
if it Isn't, stop running, another job is taking care of it.

if we Shouldn't keep running, then we check if we are the owner by comparing g_uint32_t_PlayerMoveHandlerJobOwner to our HandlerOwnerForward.
If it matches, then we set it to zero.
If it doesn't match, do nothing and return.
If we set it to zero no matter what we would've ended all jobs controling the player when the goal was to leave only 1 running at a time.

*/
static CommandResult ForwardCommand(
    const std::vector<std::string>& args)
{
    HandleForward();
    EnsurePlayerMoveHandler();

    return CommandResult::Success;
}

static CommandResult StopForwardCommand(
    const std::vector<std::string>& args)
{
    HandleStopMovingForward();

    return CommandResult::Success;
}


static CommandResult BackCommand(
    const std::vector<std::string>& args)
{
    HandleBack();
    EnsurePlayerMoveHandler();

    return CommandResult::Success;
}

static CommandResult StopBackCommand(
    const std::vector<std::string>& args)
{
    HandleStopMovingBack();

    return CommandResult::Success;
}


static CommandResult LeftCommand(
    const std::vector<std::string>& args)
{
    HandleLeft();
    EnsurePlayerMoveHandler();

    return CommandResult::Success;
}

static CommandResult StopLeftCommand(
    const std::vector<std::string>& args)
{
    HandleStopMovingLeft();

    return CommandResult::Success;
}


static CommandResult RightCommand(
    const std::vector<std::string>& args)
{
    HandleRight();
    EnsurePlayerMoveHandler();

    return CommandResult::Success;
}

static CommandResult StopRightCommand(
    const std::vector<std::string>& args)
{
    HandleStopMovingRight();

    return CommandResult::Success;
}


static CommandResult CursorToggleCommand(
    const std::vector<std::string>& args)
{
    ToggleGlobalMouseShouldLock();

    return CommandResult::Success;
}



static CommandResult CommandListCommand(
    const std::vector<std::string>& args)
{
    if (args.size() < 2)
    {
        Logger().error("Usage: commandList <file>");
        return CommandResult::Failed;
    }

    CommandList list(
        GetBaseFolder(),
        args[1]
    );

    list.Execute();

    return CommandResult::Success;
}


// ============================================================
// REGISTER ALL ENGINE COMMANDS
// ============================================================

void RegisterEngineCommands()
{
    auto& commandManager = GetCommandManager();

    commandManager.RegisterCommand(
        "help",
        "Test Command",
        HelpCommand
    );

    commandManager.RegisterCommand(
        "echo",
        "prints first argument",
        EchoCommand
    );

    commandManager.RegisterCommand(
        "bind",
        "binds a command. bind <key> <command>",
        BindCommand
    );

    commandManager.RegisterCommand(
    "+forward",
    "Starts Moving Forward",
    ForwardCommand
    );

    commandManager.RegisterCommand(
        "-forward",
        "Stops moving forward",
        StopForwardCommand
    );

    commandManager.RegisterCommand(
        "+back",
        "Starts Moving Backward",
        BackCommand
    );

    commandManager.RegisterCommand(
        "-back",
        "Stops moving backward",
        StopBackCommand
    );

    commandManager.RegisterCommand(
        "+left",
        "Starts Moving Left",
        LeftCommand
    );

    commandManager.RegisterCommand(
        "-left",
        "Stops moving left",
        StopLeftCommand
    );

    commandManager.RegisterCommand(
        "+right",
        "Starts Moving Right",
        RightCommand
    );

    commandManager.RegisterCommand(
        "-right",
        "Stops moving right",
        StopRightCommand
    );

    commandManager.RegisterCommand(
        "cursor_toggle",
        "toggle cursor between ImGui and ",
        CursorToggleCommand
    );

    commandManager.RegisterCommand("script","script", CommandListCommand);
}