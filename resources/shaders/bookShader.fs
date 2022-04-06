#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    vec3 TangentLightDir;
} fs_in;

in SP_OUT {
    vec3 TangentSpotPos;
    vec3 TangentSpotDir;
} sp_in;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
    sampler2D texture_height1;

    float shininess;
};

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform bool flashLight;
uniform float height_scale;
uniform sampler2D depthMap;

uniform Material material;
uniform PointLight pointLight;
uniform DirLight dirLight;
uniform SpotLight spotLight;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 color, vec2 texCoords);
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 color, vec2 texCoords);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 color, vec2 texCoords);

void main()
{
    // offset texture coordinates with Parallax Mapping
    vec3 viewDir   = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec2 texCoords = ParallaxMapping(fs_in.TexCoords,  viewDir);

     texCoords = ParallaxMapping(fs_in.TexCoords,  viewDir);
     if (texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
        discard;

    // then sample textures with new texture coords
    vec3 normal  = texture(material.texture_normal1, texCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);

    vec3 color = texture(material.texture_diffuse1, texCoords).rgb;

    vec3 result = CalcDirLight(dirLight, normal, viewDir, color, texCoords);
    result += CalcPointLight(pointLight, normal, fs_in.TangentFragPos, viewDir, color, texCoords);

    if (flashLight) {
        result += CalcSpotLight(spotLight, normal, fs_in.TangentFragPos, viewDir, color, texCoords);
    }

    FragColor = vec4(result, 1.0);
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    float height =  texture(depthMap, fs_in.TexCoords).r;
    vec2 p = viewDir.xy / viewDir.z * (height * height_scale);
    return texCoords - p;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 color, vec2 texCoords)
{
//     vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    vec3 lightDir = normalize(fs_in.TangentLightPos - fragPos);

    float diff = max(dot(lightDir, normal), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

    float distance    = length(light.position - fs_in.TangentFragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient = light.ambient * color;
    vec3 diffuse = light.diffuse * diff * color;
    vec3 specular = light.specular * spec * texture(material.texture_specular1, texCoords).rgb;


     ambient  *= attenuation;
     diffuse   *= attenuation;
     specular *= attenuation;

     return (ambient + diffuse + specular);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 color, vec2 texCoords)
{
    vec3 lightDir = normalize(-fs_in.TangentLightDir);
//     vec3 lightDir = normalize(light.direction);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 ambient = light.ambient * color;
    vec3 diffuse = light.diffuse * diff * color;
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, texCoords));

    return (ambient + diffuse + specular);

}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 color, vec2 texCoords)
{
    //vec3 lightDir = normalize(fs_in.TangentLightPos - fragPos);
    vec3 lightDir = normalize(sp_in.TangentSpotPos - fragPos);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

//     float distance = length(light.position - fragPos);
    float distance = length(sp_in.TangentSpotPos - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    float theta = dot(lightDir, normalize(-sp_in.TangentSpotDir));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 ambient = light.ambient * color;
    vec3 diffuse = light.diffuse * diff * color;
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, texCoords));

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular);
}
