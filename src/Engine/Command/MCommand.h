#pragma once

#include "CCommandManager.h"
#include "Engine/Command/CCommand.h"

#define COMMAND_JOIN_IMPL(a, b) a##b
#define COMMAND_JOIN(a, b) COMMAND_JOIN_IMPL(a, b)

#define RegisterCommand(name, description) \
    static CommandResult COMMAND_JOIN(Command_, __LINE__)( \
        const std::vector<std::string>& args \
    ); \
    \
    static struct COMMAND_JOIN(CommandRegistrar_, __LINE__) \
    { \
        COMMAND_JOIN(CommandRegistrar_, __LINE__)() \
        { \
            GetCommandManager().RegisterCommand( \
                name, \
                description, \
                COMMAND_JOIN(Command_, __LINE__) \
            ); \
        } \
    } COMMAND_JOIN(CommandRegistration_, __LINE__); \
    \
    static CommandResult COMMAND_JOIN(Command_, __LINE__)( \
        const std::vector<std::string>& args \
    )


