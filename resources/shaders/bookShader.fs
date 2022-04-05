#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
    sampler2D texture_height1;

    //float shininess;
};

uniform float height_scale;
uniform sampler2D depthMap;
uniform Material material;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir);

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

    // get diffuse color
    vec3 color = texture(material.texture_diffuse1, texCoords).rgb;
    // ambient
    vec3 ambient = 0.2 * color;
    // diffuse
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.2) * spec;
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    float height =  texture(depthMap, fs_in.TexCoords).r;
    vec2 p = viewDir.xy / viewDir.z * (height * height_scale);
    return texCoords - p;
}