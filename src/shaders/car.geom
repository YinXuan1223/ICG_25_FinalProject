#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec3 wPos;
    vec3 wNrm;
    vec2 uv;
} gin[];

out GS_OUT {
    vec2 uv;
    vec3 wPos;
    vec3 wNrm;
    vec3 bary;   // 用來做 wireframe
} gout;

uniform bool enableEffect;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

// 一點點隨機：讓每個面爆裂程度不同
float hash31(vec3 p){
    p = fract(p * 0.1031);
    p += dot(p, p.yzx + 33.33);
    return fract((p.x + p.y) * p.z);
}

void emitVertex(int i, vec3 wpos, vec3 wnrm, vec3 bary)
{
    gout.uv   = gin[i].uv;
    gout.wPos = wpos;
    gout.wNrm = wnrm;
    gout.bary = bary;

    gl_Position = projection * view * vec4(wpos, 1.0);
    EmitVertex();
}

void main()
{
    vec3 p0 = gin[0].wPos;
    vec3 p1 = gin[1].wPos;
    vec3 p2 = gin[2].wPos;

    vec3 faceN = normalize(cross(p1 - p0, p2 - p0));
    vec3 c = (p0 + p1 + p2) / 3.0;

    // 爆裂強度：0~1，隨 time 震盪，且每個面略不同
    float seed = hash31(c);
    float wave = 0.5 + 0.5 * sin(time * 2.5 + seed * 6.2831);

    // 調參：想更誇張就把 6.0 調大（單位是 world space）
    float explodeAmp = enableEffect? 3.0 * wave : 0.0;

    // 讓面沿著 face normal 整片往外推
    vec3 dp = faceN * explodeAmp;

    emitVertex(0, p0 + dp, gin[0].wNrm, vec3(1,0,0));
    emitVertex(1, p1 + dp, gin[1].wNrm, vec3(0,1,0));
    emitVertex(2, p2 + dp, gin[2].wNrm, vec3(0,0,1));
    EndPrimitive();
}
