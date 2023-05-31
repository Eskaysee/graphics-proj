#version 330 core

in vec2 textCoord;

uniform vec3 objectColor;
uniform sampler2D ourText;

//out vec4 outColor;
out vec4 FragColor;

void main()
{
    //outColor = vec4(objectColor,1);
    FragColor = texture(ourText, textCoord);
}