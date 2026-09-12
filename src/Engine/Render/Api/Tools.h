#pragma once
#include "Logger/Logger.h"
#include <fstream>
#include <vector>
inline std::vector<char> ReadFileGL(const std::string& filename)
{
    std::ifstream file(
        filename,
        std::ios::ate | std::ios::binary
    );

    LOGGER_ASSERT(file.is_open(), "Failed to open shader file");

    size_t fileSize = static_cast<size_t>(file.tellg());

    std::vector<char> buffer(fileSize + 1);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    buffer[fileSize] = '\0';

    return buffer;
}

inline std::vector<char> ReadFileVk(const std::string& filename)
{
    std::ifstream file(
        filename,
        std::ios::ate | std::ios::binary
    );

    LOGGER_ASSERT(file.is_open(), "Failed to open shader file");

    size_t fileSize = static_cast<size_t>(file.tellg());

    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);


    return buffer;
}

