#version 410

layout(std140) uniform CameraUBO_std140
{
    mat4 model;
    mat4 view;
    mat4 projection;
} camera;

layout(location = 0) in vec3 input_position;
layout(location = 1) in vec4 input_color;
layout(location = 2) in vec2 input_uv;
layout(location = 3) in vec3 input_normal;

layout(location = 0) out vec4 color;
layout(location = 1) out vec2 uv;
layout(location = 2) out vec3 normal;

void main()
{
    gl_Position =
        camera.projection *
        camera.view *
        camera.model *
        vec4(input_position, 1.0);

    color = input_color;
    uv = input_uv;
    normal = input_normal;
}