// #version 330 core
// out vec4 FragColor;

// in vec2 TexCoord; 

// uniform sampler2D ourTexture;

// void main()
// {
//     FragColor = texture(ourTexture, TexCoord);
    
// } 


#version 330 core
out vec4 FragColor;

in GS_OUT {
    vec2 uv;
    vec3 wPos;
    vec3 wNrm;
    vec3 bary;
} fin;

uniform bool enableEffect;
uniform sampler2D ourTexture;
uniform vec3 cameraPos;
uniform float time;

float wireFactor(vec3 bary)
{
    // 線粗：越大線越細（可調）
    float thickness = 1.5;

    vec3 d = fwidth(bary);
    vec3 a3 = smoothstep(vec3(0.0), d * thickness, bary);
    return 1.0 - min(min(a3.x, a3.y), a3.z); // 1=線, 0=面
}

void main()
{
    vec4 base = texture(ourTexture, fin.uv);

    // wireframe
    float w = wireFactor(fin.bary);

    // 掃描線（y 方向掃過）
    float scan = 0.5 + 0.5 * sin(fin.wPos.y * 0.12 + time * 6.0);
    scan = smoothstep(0.2, 1.0, scan);

    // rim glow（邊緣發光）
    vec3 N = normalize(fin.wNrm);
    vec3 V = normalize(cameraPos - fin.wPos);
    float rim = pow(1.0 - max(dot(N, V), 0.0), 3.0);

    // 組合：底色 + 線框發光 + 掃描 + 邊緣光
    vec3 col = base.rgb;

    if(enableEffect){
        // 線框偏亮
        col += vec3(0.2, 0.8, 1.2) * w * (0.6 + 0.4 * scan);

        // rim
        col += vec3(0.1, 0.6, 1.0) * rim * 0.8;
    }
    

    FragColor = vec4(col, base.a);
}
