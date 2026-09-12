#pragma once

#include "CCommand.h"

#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>

using Job =  struct {
    std::function<void(bool*)> job;
    uint32_t id;
    bool* shouldKeepRunning;
};

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

    void StartJob(std::string callerCmd, Job function);
    void StopJob(std::string callerCmd, int id);

    void TickJob();


    std::map<std::string,CCommand> m_Commands;
private:
    uint32_t m_NextCommandId = 0;

    std::map<std::string,std::vector<Job>> jobs;
    
    
};
CCommandManager& GetCommandManager();