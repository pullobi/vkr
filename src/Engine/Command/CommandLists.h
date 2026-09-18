#pragma once

#include <string>
#include <vector>
#include <memory>

class CommandList
{
public:
    CommandList(std::string path, std::string file = "");

    void Execute();

private:
    // Directory containing this command list.
    std::string BaseFolder;

    // Absolute path to the actual script file.
    std::string FilePath;

    std::vector<std::string> commands;
    std::vector<std::unique_ptr<CommandList>> children;
};

std::vector<std::string> GetCommandListsFromFile(
    std::string absolute_path
);

std::string GetBaseFolder();
void SetBaseFolder(std::string s);