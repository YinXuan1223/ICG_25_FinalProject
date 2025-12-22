#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in float Alpha;

uniform vec3 splashColor = vec3(1.0, 1.0, 1.0);

void main() {
    vec2 uv = TexCoord * 2.0 - 1.0;
    float r2 = dot(uv, uv);
    float soft = exp(-r2 * 3.0);    

    float a = Alpha * soft;

    if (a < 0.01) discard;
    FragColor = vec4(splashColor, a);
}
