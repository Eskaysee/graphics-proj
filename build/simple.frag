#version 330 core

in vec2 textCoord;
in vec3 myNormal;
//in vec4 myVertex;

uniform vec3 objectColor;
uniform sampler2D ourText;
uniform int isText;

uniform vec4 lightposn;
uniform vec4 lightcol;

uniform vec4 ambient;
uniform vec4 diffuse;
uniform vec4 specular;
uniform float shininess;

//out vec4 outColor;
out vec4 FragColor;

void main()
{
    //outColor = vec4(objectColor,1);
    if (isText>0) FragColor = texture(ourText, textCoord);
    else {
        FragColor = vec4(objectColor, 1.0f);
    }
}