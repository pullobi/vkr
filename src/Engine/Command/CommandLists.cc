#include "CommandLists.h"
#include "Engine/Command/CCommandManager.h"

#include <fstream>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

std::string g_BaseFolder="";

std::string GetBaseFolder(){
    return g_BaseFolder;
};
void SetBaseFolder(std::string s){
    g_BaseFolder = s;
};

CommandList::CommandList(std::string path, std::string file)
{
    BaseFolder = std::move(path);

    std::string commandListName = "CommandLists.txt";

    if (!file.empty())
        commandListName = std::move(file);

    fs::path commandListPath =
        fs::path(BaseFolder) / commandListName;

    commands = GetCommandListsFromFile(
        commandListPath.string()
    );

    for (const std::string& command : commands)
    {
        constexpr std::string_view prefix = "commandList ";

        if (!command.starts_with(prefix))
            continue;

        std::string childFile =
            command.substr(prefix.size());

        children.push_back(
            std::make_unique<CommandList>(
                BaseFolder,
                childFile
            )
        );
    }
}

void CommandList::Execute()
{
    for (const auto& child : children)
    {
        child->Execute();
    }

    for (const std::string& command : commands)
    {
        constexpr std::string_view prefix = "commandList ";

        if (command.starts_with(prefix))
            continue;

        GetCommandManager().ExecuteFromString(command);
    }
}

std::vector<std::string> GetCommandListsFromFile(
    std::string absolute_path)
{
    std::vector<std::string> commands;

    std::ifstream file(absolute_path);

    if (!file.is_open())
        return commands;

    std::string line;

    while (std::getline(file, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (line.empty())
            continue;

        commands.push_back(line);
    }

    return commands;
}