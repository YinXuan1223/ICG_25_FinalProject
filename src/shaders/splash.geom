#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in VS_OUT {
    vec3 pos;
    float birth;
} gin[];

out vec2 TexCoord;
out float Alpha;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPos;
uniform float time;

uniform float life = 0.5;
uniform float size0 = 7.0;
uniform float riseSpeed = 5.0;

void emitCorner(vec3 worldPos, vec2 uv, float alpha)
{
    TexCoord = uv;
    Alpha = alpha;
    gl_Position = projection * view * vec4(worldPos, 1.0);
    EmitVertex();
}

void main() {
    vec3 p = gin[0].pos;
    float age = time - gin[0].birth;

    if (age < 0.0 || age > life) return;

    // billboard axes
    vec3 viewDir = normalize(cameraPos - p);
    vec3 up = vec3(0, 1, 0);
    vec3 right = normalize(cross(viewDir, up));
    vec3 realUp = normalize(cross(right, viewDir));

    float t = age / life;                 
    float size = mix(size0, 0.05, t);
    float alpha = (1.0 - t) * 0.8;

    vec3 rise = vec3(0, riseSpeed * age, 0);
    vec3 c = p + rise;

    vec3 p0 = c + (-right - realUp) * size;
    vec3 p1 = c + ( right - realUp) * size;
    vec3 p2 = c + (-right + realUp) * size;
    vec3 p3 = c + ( right + realUp) * size;

    emitCorner(p0, vec2(0,0), alpha);
    emitCorner(p1, vec2(1,0), alpha);
    emitCorner(p2, vec2(0,1), alpha);
    emitCorner(p3, vec2(1,1), alpha);
    EndPrimitive();
}
