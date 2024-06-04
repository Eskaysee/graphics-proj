#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 vText;
layout(location = 2) in vec3 norms;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int isText;

//out vec4 myVertex;
out vec3 FragPos;
out vec3 myNormal;
out vec2 textCoord;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
    textCoord = vec2(0.0,0.0);
    if (isText>0) textCoord = vText;
    myNormal = mat3(transpose(inverse(model))) * norms;
    FragPos = vec3(view * model * vec4(position, 1.0f));
}
