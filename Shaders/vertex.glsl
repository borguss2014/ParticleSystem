#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aCol;
layout (location = 2) in vec3 aScale;

out vec4 Color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    Color = aCol;

    vec3 newPos = vec3(aPos.x * aScale.x, aPos.y * aScale.y, aPos.z * aScale.z);
    gl_Position = projection * view * model * vec4(newPos, 1.0);
}
