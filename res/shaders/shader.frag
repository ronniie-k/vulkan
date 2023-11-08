#version 450

layout(location = 0) in vec3 iWorldPos;
layout(location = 1) in vec3 iNormal;
layout(location = 2) in vec2 iTexCoord;

layout(location = 0) out vec4 oColor;

layout(set = 0, binding = 1) uniform sampler2D depthTexture;
layout(set = 1, binding = 0) uniform sampler2D albedoTexture;
layout(set = 1, binding = 1) uniform sampler2D normalTexture;
layout(set = 1, binding = 2) uniform sampler2D metallicRoughnessTexture;


const float PI = 3.14159265359;

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 normal;
    vec4 lightPos;
    vec4 cameraPos;
} ubo;


vec3 getNormal()
{
    vec3 normal = 2 * texture(normalTexture, iTexCoord).rgb - 1;
	vec3 q1 = dFdx(iWorldPos);
	vec3 q2 = dFdy(iWorldPos);
	vec2 st1 = dFdx(iTexCoord);
	vec2 st2 = dFdy(iTexCoord);

	vec3 N = normalize(iNormal);
	vec3 T = normalize(q1 * st2.t - q2 * st1.t);
	vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);
    
    return normalize(TBN * normal);
}

float geometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = geometrySchlickGGX(NdotV, roughness);
    float ggx2 = geometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

float distributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    //float depth = texture(depthTexture, iTexCoord).r;
    float depth = 0;
    vec3 lightColor = vec3(23.47, 21.31, 20.79) * 0.2;
    //vec3 lightColor = vec3(0.95);

    vec3 albedo = texture(albedoTexture, iTexCoord).rgb;
    vec4 metallicRoughness = texture(metallicRoughnessTexture, iTexCoord);
    float metallic = metallicRoughness.b;
    float roughness = metallicRoughness.g;
    
    vec3 N = getNormal();
    vec3 L = normalize(ubo.lightPos.xyz - iWorldPos);
    vec3 V = normalize(ubo.cameraPos.xyz - iWorldPos);
    vec3 H = normalize(L + V);

    float distance = length(ubo.lightPos.xyz - iWorldPos);
    float attenuation = 1.0f / (1.0f + 0.09f * distance + 0.032f * (distance * distance));
    vec3  radiance    = lightColor * attenuation;

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    //brdf
    float NDF = distributionGGX(N, H, roughness);   
    float G   = geometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float deno = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
    vec3 specular = numerator / deno;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);
    vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;
    vec3 ambient = vec3(0.03) * albedo;
    vec3 color = Lo + ambient;

    // HDR tonemapping
    //color = color / (color + vec3(1.0));
    // gamma correct
    //color = pow(color, vec3(1.0/2.2));

    oColor = vec4(color, 1.0) * (1 - depth);
    //oColor = vec4(vec3(depth), 1);
}