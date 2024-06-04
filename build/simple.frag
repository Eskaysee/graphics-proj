#version 330 core

in vec2 textCoord;
in vec3 myNormal;
in vec3 FragPos;
//in vec4 myVertex;

uniform vec3 objectColor;
uniform sampler2D ourText;
uniform int isText;

uniform vec3 lightPos;
uniform vec3 lightCol;

uniform vec3 ambient;
uniform vec3 diffuseCol;
uniform vec4 matSpecular;
uniform float matShininess;

out vec4 FragColor;

void main()
{
    if (isText>0) FragColor = texture(ourText, textCoord);
    else {
        vec3 normal = normalize(myNormal);
        const vec3 cameye = vec3(0.0f,0.0f,0.0f);
        vec3 lightDir = normalize(lightPos - FragPos);
        vec3 cameyeDir = normalize(cameye - FragPos);
        vec3 halfDir = normalize(lightDir+cameyeDir);
        vec3 reflectDir = reflect(-lightDir,normal);
        vec3 ambience =  ambient * lightCol;
        vec3 diffuse = max(dot(normal,lightDir), 0.0) * lightCol;
        vec3 res = (ambience + diffuse) * objectColor;
        FragColor = vec4(res, 1.0);
    }
}