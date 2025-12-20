#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 viewPos;
uniform samplerCube skybox;
uniform float AIR_coeff;
uniform float GLASS_coeff;

void main()
{ 

    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);

    vec3 Reflect = reflect(-V, N);

    float n1 = AIR_coeff;
    float n2 = GLASS_coeff;
    float eta = n1 / n2;
   
    vec3 Refract = refract(-V, N, eta);
	
    
    float r0 = pow( ((n1-n2)/(n1+n2)) , 2);
    float r_theta = r0 + (1-r0) * pow((1 + dot(-V, N)), 5);
    

    vec3 reflectColor = texture(skybox, Reflect).rgb;
    vec3 refractColor = texture(skybox, Refract).rgb;

    
    vec3 finalColor = mix(refractColor, reflectColor, r_theta);
    FragColor = vec4(finalColor, 1.0);

} 


