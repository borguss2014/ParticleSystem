#version 410 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec4 instanceColor;
layout (location = 2) in mat4 instanceModel;

out vec4 Color;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    Color = instanceColor;
    gl_Position = projection * view * instanceModel * vec4(aPos, 0.0, 1.0);
}
