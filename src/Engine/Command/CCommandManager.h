#pragma once

#include "CCommand.h"

#include <cstdint>
#include <map>
#include <string>

class CCommandManager
{
public:
    CCommandManager();

    void RegisterCommand(CCommand command);
    void RegisterCommand(const std::string& name,const std::string& description,CommandHandler handler);
    CCommand* FindCommand(
        const std::string& name
    );

    CommandResult ExecuteFromString(std::string commandPrompt);



    std::map<std::string,CCommand> m_Commands;
private:
    uint32_t m_NextCommandId = 0;
    
    
};
CCommandManager& GetCommandManager();