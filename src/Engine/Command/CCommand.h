#pragma once

#include <cstdint>
#include <format>
#include <functional>
#include <string>
#include <vector>

enum class CommandResult
{
    Success,
    Failed,
    SyntaxError,
    NoCommand,
};

using CommandHandler =
    std::function<CommandResult(
        const std::vector<std::string>&
    )>;

class CCommand
{
public:
    CCommand(
        uint32_t id,
        std::string name,
        std::string description,
        CommandHandler handler
    );

    CommandResult Call(
        const std::vector<std::string>& args
    );

    std::string GetHelpDescription(){
        return std::format("{}: {} (command ID {})", m_Name, m_Description, m_Id);
    }
    uint32_t GetId() const;

    const std::string& GetName() const;
    const std::string& GetDescription() const;

private:
    uint32_t m_Id;
    std::string m_Name;
    std::string m_Description;

    CommandHandler m_Handler;
};