#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

struct Pyramid {
    sampler2D diffuse;
    vec3 specular;

    float shininess;
};

struct Light {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    vec3 position;
};

uniform vec3 viewPosition;
uniform Pyramid pyramid;
uniform Light light;

void main()
{
    vec3 ambient = light.ambient * texture(pyramid.diffuse, TexCoord).rgb;

    vec3 norm = normalize(Normal);
    vec3 lightDirection = normalize(light.position - FragPos);
    float diff = max(dot(norm, -lightDirection), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(pyramid.diffuse, TexCoord).rgb;

    vec3 viewDirection = normalize(viewPosition - FragPos);
    vec3 reflectDir = reflect(-lightDirection, norm);
    float spec = pow(max(dot(viewDirection, reflectDir), 0.0), pyramid.shininess);
    vec3 specular = light.specular * spec * pyramid.specular;
//     vec3 specular = light.specular * spec * texture(pyramid.specular, TexCoord).rgb;

    vec3 result = ambient + diffuse + specular;

    FragColor = vec4(result, 1.0f);
}