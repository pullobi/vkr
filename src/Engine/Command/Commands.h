#pragma once
#include "CCommandManager.h"
#include "Engine/Command/CCommand.h"
#include "Logger/Logger.h"
#include "MCommand.h"
#include <iostream>

RegisterCommand("help", "Test Command")
{
    Logger().info("help called");
    auto& commandManager = GetCommandManager();
    

    if (!args.empty() && !args[0].empty())
    {
        CCommand* command =
            commandManager.FindCommand(args[0]);

        if (!command)
        {
            Logger().warn("Unknown command: {}", args[0]);
            return CommandResult::Failed;
        }

        Logger().info(
            "{}",
            command->GetDescription()
        );
    }
    else
    {   
        Logger().info("==== help page ====");
        for (auto& command : commandManager.m_Commands){
            Logger().info("{}",command.second.GetHelpDescription());
        }
        Logger().info("===================");
    }

    return CommandResult::Success;
}


RegisterCommand("echo", "prints first argument"){
    if (!args.empty() && !args[0].empty())
        Logger().info("{}", args[0]);
    else
        Logger().warn(" ");
    return CommandResult::Success;
}

CommandResult HandleBind(const std::vector<std::string>& args);
RegisterCommand("bind", "binds a command. bind <key> <command>"){
    return HandleBind(args);
}


void HandleForward();
RegisterCommand("+forward", "Starts Moving Forward"){
    std::cout << "+forward called\r\n";
    HandleForward();  
    return CommandResult::Success;
};

void HandleStopMovingForward();
RegisterCommand("-forward", "Stops moving forward"){
    std::cout << "-forward called\r\n";
    HandleStopMovingForward();
    return CommandResult::Success;
}