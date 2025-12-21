#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 WorldPos;

void main()
{
    vec4 world = model * vec4(aPos, 1.0);
    WorldPos = world.xyz;

    gl_Position = projection * view * world;
}

// #version 330 core
// layout (location = 0) in vec3 aPos;
// layout (location = 1) in vec3 aNormal;
// layout (location = 2) in vec2 aTexCoord;

// out vec3 FragPos;
// out vec3 Normal;
// out vec2 TexCoord;

// uniform mat4 model;
// uniform mat4 view;
// uniform mat4 projection;
// uniform float time;

// void main()
// {
//     // 添加波浪效果
//     vec3 pos = aPos;
//     pos.y += sin(pos.x * 0.5 + time) * 0.3 + cos(pos.z * 0.5 + time * 0.7) * 0.2;
    
//     FragPos = vec3(model * vec4(pos, 1.0));
//     Normal = mat3(transpose(inverse(model))) * aNormal;
//     TexCoord = aTexCoord * 20.0;  // 重複貼圖
    
//     gl_Position = projection * view * vec4(FragPos, 1.0);
// }