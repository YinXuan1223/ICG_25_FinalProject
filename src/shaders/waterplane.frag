#version 330 core

out vec4 FragColor;

in vec3 WorldPos;

uniform vec3 cameraPos;

// 水面顏色
uniform vec3 waterColor;   // e.g. vec3(0.0, 0.3, 0.4)
uniform vec3 fogColor;     // 通常接近天空顏色

// 控制邊界
uniform float maxDist;     // 水面半徑，例如 120.0
uniform float fadeWidth;   // 淡出寬度，例如 30.0

void main()
{
    // 只看 XZ 平面距離
    float dist = length(WorldPos.xz - cameraPos.xz);

    // 距離 fade（關鍵）
    float fade = smoothstep(maxDist, maxDist - fadeWidth, dist);

    vec3 color = waterColor;

    // 淡出到天空顏色
    color = mix(fogColor, color, fade);

    FragColor = vec4(color, 1.0);
}

// #version 330 core
// out vec4 FragColor;

// in vec3 FragPos;
// in vec3 Normal;
// in vec2 TexCoord;

// uniform vec3 viewPos;
// uniform float time;
// uniform samplerCube skybox;

// void main()
// {
//     // 基礎水色
//     vec3 waterColor = vec3(0.0, 0.3, 0.5);
    
//     // 計算反射
//     vec3 viewDir = normalize(viewPos - FragPos);
//     vec3 normal = normalize(Normal);
//     vec3 reflectDir = reflect(-viewDir, normal);
    
//     // 從 skybox 採樣反射
//     vec3 reflection = texture(skybox, reflectDir).rgb;
    
//     // 菲涅爾效果（邊緣更透明）
//     float fresnel = pow(1.0 - max(dot(viewDir, normal), 0.0), 3.0);
    
//     // 混合水色和反射
//     vec3 color = mix(waterColor, reflection, fresnel * 0.7);
    
//     // 添加一些波光粼粼的效果
//     float sparkle = sin(TexCoord.x * 50.0 + time) * cos(TexCoord.y * 50.0 + time);
//     sparkle = max(0.0, sparkle) * 0.3;
//     color += vec3(sparkle);
    
//     // 半透明
//     FragColor = vec4(color, 0.8);
// }