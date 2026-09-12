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
    
    glm::vec3 pos;
    glm::vec2 rot;
    
    glm::vec3 GetPos() const;
    glm::vec2 GetRot() const;
    glm::vec3 GetForwardVec() const;
    void SetPos(glm::vec3 pos);
    void AddPos(glm::vec3 pos);
    void SetRot(glm::vec2 newRot);


    void UpdateView();
};
