#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    vec3 TangentLightDir;
} vs_out;

// for spotlight
out SP_OUT {
    vec3 TangentSpotPos;
    vec3 TangentSpotDir;
} sp_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightDir;

uniform vec3 spotPosition;
uniform vec3 spotDirection;

void main()
{
    vs_out.FragPos   = vec3(model * vec4(aPos, 1.0));
    vs_out.TexCoords = aTexCoords;

    vec3 T   = normalize(mat3(model) * aTangent);
    vec3 B   = normalize(mat3(model) * aBitangent);
    vec3 N   = normalize(mat3(model) * aNormal);
    mat3 TBN = transpose(mat3(T, B, N));

    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos  = TBN * viewPos;
    vs_out.TangentFragPos  = TBN * vs_out.FragPos;
    vs_out.TangentLightDir = TBN * lightDir;

    sp_out.TangentSpotPos    = TBN * spotPosition;
    sp_out.TangentSpotDir    = TBN * spotDirection;

    gl_Position      = projection * view * model * vec4(aPos, 1.0);
}
