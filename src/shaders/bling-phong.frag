#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 viewPos;

uniform vec3 lightPos;
uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;
uniform vec3 lightSpecular;

uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float materialShininess;

uniform sampler2D ourTexture;

void main()
{

    vec3 sampled_texture = texture(ourTexture, TexCoord).rgb;

	vec3 N = normalize(Normal);
    vec3 L = normalize(lightPos - FragPos);
    vec3 V = normalize(viewPos - FragPos);
    vec3 H = normalize(L + V);

    float L_N = max(dot(N, L), 0.0);
    float N_H_alpha = pow(max(dot(N, H), 0.0), materialShininess); 

    vec3 ambient = lightAmbient * materialAmbient * sampled_texture;
    vec3 diffuse = lightDiffuse * materialDiffuse * L_N * sampled_texture;
    vec3 specular = lightSpecular * materialSpecular * N_H_alpha * sampled_texture;;

    vec3 color = ambient + diffuse + specular;
    FragColor = vec4(color, 1.0);

}