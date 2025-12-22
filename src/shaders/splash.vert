#version 330 core
layout (location = 0) in vec3 aPos;     // world space position
layout (location = 1) in float aBirth;  // spawn time

out VS_OUT {
    vec3 pos;
    float birth;
} vout;

void main() {
    vout.pos = aPos;
    vout.birth = aBirth;

    gl_Position = vec4(aPos, 1.0);
}
