#pragma once
#include <string>
enum class RenderBackend{
    None,
    Vulkan,
    OpenGL,
};

struct Size {
    unsigned int x;
    unsigned int y;
};


struct WindowOptions {
    Size size;
    std::string title;
};

enum class WindowBackend{
    GLFW,
    SDL,
};
