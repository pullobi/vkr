#pragma once
#include <glm/glm.hpp>

struct Vertex
{
    glm::vec3 position;
    glm::vec4 color;
    glm::vec2 uv;     //let me get where in the texture i'm supposed to be
    glm::vec3 normal; // let me get where i'm looking at
};

struct CameraUBO
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};