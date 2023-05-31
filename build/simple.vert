#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 vText;
//layout(location = 2) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

//out vec4 myVertex;
//out vec4 myNormal;
out vec2 textCoord;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
    textCoord = vText;
}
