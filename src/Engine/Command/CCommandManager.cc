#include "CCommandManager.h"
#include "Engine/Command/CCommand.h"

#include <cctype>
#include <utility>


static CCommandManager g_CommandManager;

CCommandManager& GetCommandManager()
    {
        return g_CommandManager;
    }
CCommandManager::CCommandManager()
{
}

void CCommandManager::RegisterCommand(
    CCommand command
)
{
    m_Commands.emplace(
        command.GetName(),
        std::move(command)
    );
}

void CCommandManager::RegisterCommand(
    const std::string& name,
    const std::string& description,
    CommandHandler handler
)
{
    CCommand command(
        m_NextCommandId++,
        name,
        description,
        std::move(handler)
    );

    m_Commands.emplace(
        name,
        std::move(command)
    );
}

CCommand* CCommandManager::FindCommand(
    const std::string& name
)
{

    auto it = m_Commands.find(name);

    if (it == m_Commands.end())
        return nullptr;

    return &it->second;
}

CommandResult CCommandManager::ExecuteFromString(
    std::string commandPrompt
)
{
    // ---------------------------------------------------------
    // Find command name
    // ---------------------------------------------------------

    
    size_t openParen = commandPrompt.find('(');

    if (openParen == std::string::npos)
        return CommandResult::Failed;

    std::string commandName =
        commandPrompt.substr(0, openParen);

    // Remove whitespace from command name
    while (!commandName.empty() &&
           std::isspace(commandName.back()))
    {
        commandName.pop_back();
    }

    if (commandName.empty())
        return CommandResult::NoCommand;

    // ---------------------------------------------------------
    // Find command
    // ---------------------------------------------------------

    CCommand* command = FindCommand(commandName);

    if (!command)
        return CommandResult::NoCommand;

    // ---------------------------------------------------------
    // Parse arguments
    // ---------------------------------------------------------

    std::vector<std::string> args;

    size_t i = openParen + 1;

    while (i < commandPrompt.size())
    {
        // Skip whitespace
        while (
            i < commandPrompt.size() &&
            std::isspace(commandPrompt[i])
        )
        {
            i++;
        }

        // End of arguments
        if (
            i >= commandPrompt.size() ||
            commandPrompt[i] == ')'
        )
        {
            break;
        }

        // -----------------------------------------------------
        // Expect quoted argument
        // -----------------------------------------------------

        if (commandPrompt[i] != '"')
            return CommandResult::SyntaxError;

        i++;

        std::string argument;

        while (i < commandPrompt.size())
        {
            // Closing quote
            if (commandPrompt[i] == '"')
            {
                i++;
                break;
            }

            argument += commandPrompt[i];
            i++;
        }

        args.push_back(std::move(argument));

        // -----------------------------------------------------
        // Skip whitespace
        // -----------------------------------------------------

        while (
            i < commandPrompt.size() &&
            std::isspace(commandPrompt[i])
        )
        {
            i++;
        }

        // -----------------------------------------------------
        // Next argument
        // -----------------------------------------------------

        if (
            i < commandPrompt.size() &&
            commandPrompt[i] == ','
        )
        {
            i++;
            continue;
        }

        // -----------------------------------------------------
        // End
        // -----------------------------------------------------

        if (
            i < commandPrompt.size() &&
            commandPrompt[i] == ')'
        )
        {
            break;
        }

        // Invalid syntax
        return CommandResult::SyntaxError;
    }

    // ---------------------------------------------------------
    // Execute
    // ---------------------------------------------------------

    return command->Call(args);

}