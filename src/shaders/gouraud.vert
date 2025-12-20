#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec3 vertexColor;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 viewPos;

uniform vec3 lightPos;
uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;
uniform vec3 lightSpecular;

uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float materialShininess;


void main()
{
	vec4 worldPos = model * vec4(aPos, 1.0);
    vec3 FragPos = worldPos.xyz;

    vec3 N = normalize(mat3(transpose(inverse(model))) * aNormal);
    vec3 L = normalize(lightPos - FragPos);
    vec3 V = normalize(viewPos - FragPos);
    vec3 H = normalize(L + V);

    float L_N = max(dot(N, L), 0.0);
    float N_H_alpha = pow(max(dot(N, H), 0.0), materialShininess); 

    vec3 ambient = lightAmbient * materialAmbient ;
    vec3 diffuse = lightDiffuse * materialDiffuse * L_N ;
    vec3 specular = lightSpecular * materialSpecular * N_H_alpha;

    vertexColor = ambient + diffuse + specular;
    TexCoord = aTexCoord;

    gl_Position = projection * view * worldPos;
}