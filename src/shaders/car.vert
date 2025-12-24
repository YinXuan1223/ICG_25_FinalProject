// #version 330 core
// layout (location = 0) in vec3 aPos;
// layout (location = 1) in vec3 aNormal;
// layout (location = 2) in vec2 aTexCoord;

// out vec2 TexCoord;

// uniform mat4 model;
// uniform mat4 view;
// uniform mat4 projection;

// void main()
// {
// 	gl_Position = projection * view * model * vec4(aPos, 1.0);
// 	TexCoord = aTexCoord;
// }

#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out VS_OUT {
    vec3 wPos;
    vec3 wNrm;
    vec2 uv;
} vout;

uniform mat4 model;

void main()
{
    vec4 wp = model * vec4(aPos, 1.0);
    vout.wPos = wp.xyz;

    mat3 nMat = mat3(transpose(inverse(model)));
    vout.wNrm = normalize(nMat * aNormal);

    vout.uv = aTexCoord;

    // 先輸出 world space 給 GS 用
    gl_Position = wp;
}
