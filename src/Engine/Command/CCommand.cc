#include "CCommand.h"

CCommand::CCommand(
    uint32_t id,
    std::string name,
    std::string description,
    CommandHandler handler
)
    : m_Id(id),
      m_Name(std::move(name)),
      m_Description(std::move(description)),
      m_Handler(std::move(handler))
{
}

CommandResult CCommand::Call(
    const std::vector<std::string>& args
)
{
    if (!m_Handler)
        return CommandResult::Failed;

    return m_Handler(args);
}

uint32_t CCommand::GetId() const
{
    return m_Id;
}

const std::string& CCommand::GetName() const
{
    return m_Name;
}

const std::string& CCommand::GetDescription() const
{
    return m_Description;
}