#version 330 core

out vec4 FragColor;

in vec3 vertexColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;

void main()
{
	vec3 sampled_texture = texture(ourTexture, TexCoord).rgb;
    FragColor = vec4(clamp(vertexColor * sampled_texture, 0.0, 1.0),1.0);
}