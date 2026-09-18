#include "Commands.h"

#include "Engine/Command/CCommand.h"
#include "Engine/Command/CommandLists.h"
#include "Engine/Command/CCommandManager.h"
#include "Engine/Player/PlayerMovement.h"
#include "Engine/Render/Api/IInputApi.h"
#include "Engine/Render/Types/CInputTypes.h"

#include <string>
#include <vector>


// ============================================================
// bind
// ============================================================

static CommandResult BindCommand(
    const std::vector<std::string>& args)
{
    // args[0] = "bind"
    // args[1] = key
    // args[2] = presstype
    // args[3...] = command

    if (args.size() < 4)
    {
        return CommandResult::SyntaxError;
    }

    std::string command;

    for (size_t i = 3; i < args.size(); i++)
    {
        if (!command.empty())
            command += ' ';

        command += args[i];
    }

    GetGlobalInputApi()->BindKey(
        Key::W,
        PressType::Pressed,
        command
    );

    return CommandResult::Success;
}


// ============================================================
// help
// ============================================================

static CommandResult HelpCommand(
    const std::vector<std::string>& args)
{
    Logger().info("help called");

    auto& commandManager = GetCommandManager();

    // args[0] = "help"
    // args[1] = command to get help for

    if (args.size() > 1 && !args[1].empty())
    {
        CCommand* command =
            commandManager.FindCommand(args[1]);

        if (!command)
        {
            Logger().warn(
                "Unknown command: {}",
                args[1]
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


// ============================================================
// echo
// ============================================================

static CommandResult EchoCommand(
    const std::vector<std::string>& args)
{
    // args[0] = "echo"
    // args[1...] = text

    if (args.size() > 1)
    {
        for (size_t i = 1; i < args.size(); ++i)
        {
            Logger().info("{}", args[i]);
        }
    }
    else
    {
        Logger().warn(" ");
    }

    return CommandResult::Success;
}


// ============================================================
// cursor_toggle
// ============================================================

static CommandResult CursorToggleCommand(
    const std::vector<std::string>& args)
{
    ToggleGlobalMouseShouldLock();

    return CommandResult::Success;
}


// ============================================================
// script
// ============================================================


static CommandResult CommandListCommand(
    const std::vector<std::string>& args)
{
    // args[0] = "script"
    // args[1] = file

    if (args.size() < 2)
    {
        Logger().error("Usage: script <file>");
        return CommandResult::Failed;
    }

    CommandList list(
        GetBaseFolder(),
        args[1]
    );

    list.Execute();

    return CommandResult::Success;
}

static CommandResult Nothing(const std::vector<std::string>& args){
    return CommandResult::Success;
};

// ============================================================
// REGISTER ALL ENGINE COMMANDS
// ============================================================

void RegisterEngineCommands()
{
    auto& commandManager = GetCommandManager();

    // --------------------------------------------------------
    // General
    // --------------------------------------------------------

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
        "binds a command. bind <key> <presstype> <command>",
        BindCommand
    );

    commandManager.RegisterCommand(
        "cursor_toggle",
        "toggle cursor between ImGui and game",
        CursorToggleCommand
    );

    commandManager.RegisterCommand(
        "script",
        "execute a command script",
        CommandListCommand
    );


    // --------------------------------------------------------
    // Player Movement
    // --------------------------------------------------------

    commandManager.RegisterCommand(
        "+forward",
        "Starts Moving Forward",
        [](const std::vector<std::string>& args){
            PlayerMovement::Forward();
            return CommandResult::Success;
        }
    );

    commandManager.RegisterCommand(
        "-forward",
        "Stops moving forward",
        [](const std::vector<std::string>& args){
            PlayerMovement::StopForward();
            return CommandResult::Success;
        }
    );


    commandManager.RegisterCommand(
        "+back",
        "Starts Moving Backward",
        [](const std::vector<std::string>& args){
            PlayerMovement::Back();
            return CommandResult::Success;
        }
    );

    commandManager.RegisterCommand(
        "-back",
        "Stops moving backward",
        [](const std::vector<std::string>& args){
            PlayerMovement::StopBack();
            return CommandResult::Success;
        }
    );


    commandManager.RegisterCommand(
        "+left",
        "Starts Moving Left",
        [](const std::vector<std::string>& args){
            PlayerMovement::Left();
            return CommandResult::Success;
        }
    );

    commandManager.RegisterCommand(
        "-left",
        "Stops moving left",
        [](const std::vector<std::string>& args){
            PlayerMovement::StopLeft();
            return CommandResult::Success;
        }
    );


    commandManager.RegisterCommand(
        "+right",
        "Starts Moving Right",
        [](const std::vector<std::string>& args){
            PlayerMovement::Right();
            return CommandResult::Success;
        }
    );

    commandManager.RegisterCommand(
        "-right",
        "Stops moving right",
        [](const std::vector<std::string>& args){
            PlayerMovement::StopRight();
            return CommandResult::Success;
        }
    );

    commandManager.RegisterCommand(
        "+up",
        "Starts moving up",
        [](const std::vector<std::string>& args){
            PlayerMovement::Up();
            return CommandResult::Success;
        }
    );

    commandManager.RegisterCommand(
        "-up",
        "Stops moving up",
        [](const std::vector<std::string>& args){
            PlayerMovement::StopUp();
            return CommandResult::Success;
        }
    );

    commandManager.RegisterCommand(
        "+down",
        "Starts moving down",
        [](const std::vector<std::string>& args){
            PlayerMovement::Down();
            return CommandResult::Success;
        }
    );

    commandManager.RegisterCommand(
        "-down",
        "Stops moving down",
        [](const std::vector<std::string>& args){
            PlayerMovement::StopDown();
            return CommandResult::Success;
        }
    );

    commandManager.RegisterCommand("#","Comment block", Nothing);
    commandManager.RegisterCommand("//","Comment block", Nothing);
}