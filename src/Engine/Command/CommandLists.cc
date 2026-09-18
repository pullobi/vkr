#include "CommandLists.h"
#include "Engine/Command/CCommandManager.h"

#include <fstream>
#include <filesystem>
#include <string>
#include <string_view>

namespace fs = std::filesystem;


// ============================================================
// Global script root
// ============================================================

std::string g_BaseFolder = "";

std::string GetBaseFolder()
{
    return g_BaseFolder;
}

void SetBaseFolder(std::string s)
{
    g_BaseFolder = std::move(s);
}


// ============================================================
// CommandList
// ============================================================

CommandList::CommandList(
    std::string path,
    std::string file)
{
    /*
        'path' is the directory relative to which 'file'
        should be resolved.

        Root:

            CommandList(
                GetBaseFolder(),
                "bindings/main.sc"
            );

        gives:

            <BaseFolder>/bindings/main.sc


        Child:

            script misc.sc

        from:

            <BaseFolder>/bindings/main.sc

        gives:

            <BaseFolder>/bindings/misc.sc
    */

    fs::path basePath = fs::path(path);

    fs::path scriptPath;

    if (file.empty())
    {
        // No explicit file means the default CommandLists.txt.
        scriptPath = basePath / "CommandLists.txt";
    }
    else
    {
        fs::path requestedPath = fs::path(file);

        // Absolute paths are used directly.
        if (requestedPath.is_absolute())
        {
            scriptPath = requestedPath;
        }
        else
        {
            scriptPath = basePath / requestedPath;
        }
    }

    // Normalize things like:
    //
    // bindings/../misc.sc
    //
    // into:
    //
    // misc.sc
    //
    scriptPath = fs::weakly_canonical(scriptPath);

    FilePath = scriptPath.string();

    // This is the directory containing THIS script.
    //
    // For:
    //
    //   scripts/bindings/main.sc
    //
    // BaseFolder becomes:
    //
    //   scripts/bindings
    //
    BaseFolder = scriptPath.parent_path().string();

    commands = GetCommandListsFromFile(
        FilePath
    );

    // Find nested command lists.
    for (const std::string& command : commands)
    {
        constexpr std::string_view prefix = "commandList ";

        if (!command.starts_with(prefix))
            continue;

        std::string childFile =
            command.substr(prefix.size());

        // Child paths are relative to THIS script's directory.
        //
        // main.sc:
        //
        //     commandList misc.sc
        //
        // becomes:
        //
        //     bindings/misc.sc
        //
        children.push_back(
            std::make_unique<CommandList>(
                BaseFolder,
                childFile
            )
        );
    }
}


// ============================================================
// Execute
// ============================================================

void CommandList::Execute()
{
    for (const auto& child : children)
    {
        child->Execute();
    }

    for (const std::string& command : commands)
    {
        constexpr std::string_view prefix = "commandList ";

        // commandList entries are handled by the child
        // CommandList objects above.
        if (command.starts_with(prefix))
            continue;

        if (command.empty())
            continue;

        GetCommandManager().ExecuteFromString(command);
    }
}


// ============================================================
// Read command list
// ============================================================

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
        // Handle Windows CRLF files.
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        // Ignore empty lines.
        if (line.empty())
            continue;

        commands.push_back(line);
    }

    return commands;
}