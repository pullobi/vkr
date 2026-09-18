#include "CCommandManager.h"
#include "Engine/Command/CCommand.h"
#include "Logger/Logger.h"
#include <cctype>
#include <utility>
#include <sstream>
// #include <iostream>

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
    std::istringstream stream(commandPrompt);

    std::string commandName;
    stream >> commandName;

    if (commandName.empty())
        return CommandResult::NoCommand;

    CCommand* command = FindCommand(commandName);

    if (!command)
        return CommandResult::NoCommand;

    std::vector<std::string> args;
    args.push_back(commandName);

    std::string arg;

    while (stream >> arg)
        args.push_back(arg);

    return command->Call(args);
}

void CCommandManager::StartJob(std::string callerCmd, Job jobFunc){
    // Logger().warn("Starting Job with callercmd: {}, id: {}",callerCmd, jobFunc.id);
    jobs[callerCmd].push_back(jobFunc);
}
void CCommandManager::StopJob(std::string callerCmd, int id){
    for (size_t i = 0; i < jobs.size();)
    {
        if (!jobs[callerCmd][i].shouldKeepRunning)
        {
            jobs[callerCmd].erase(jobs[callerCmd].begin() + i);
        }
        else
        {
            jobs[callerCmd][i].job(jobs[callerCmd][i].shouldKeepRunning);
            ++i;
        }
    }
};

int nTicks = 0;

void CCommandManager::TickJob()
{
    
    for (auto& [command, commandJobs] : jobs)
    {
        for (auto it = commandJobs.begin(); it != commandJobs.end();)
        {
            Job& job = *it;
            // Logger().warn("(nTicks = {}) Ticking Job: id={}, shouldKeepRunning={}", ++nTicks, it->id, it->shouldKeepRunning ? "true":"false");

            if (job.shouldKeepRunning)
            {
                job.job(job.shouldKeepRunning);
            }

            if (!job.shouldKeepRunning)
            {
                Logger().warn("Job shouldnt keep running, destroying...");
                it = commandJobs.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}

/*
StartJob(self, [])
*/