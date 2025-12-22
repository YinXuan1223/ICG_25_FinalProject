#version 330 core

out vec4 FragColor;

in vec3 WorldPos;

uniform vec3 waterColor;   // e.g. vec3(0.0, 0.3, 0.4)

void main()
{

    FragColor = vec4(waterColor, 1.0);
}

