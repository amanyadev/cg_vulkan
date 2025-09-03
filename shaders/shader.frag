#version 450

// Inputs from vertex shader
layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec4 fragColor;
layout(location = 4) in vec3 fragTangent;
layout(location = 5) in vec3 fragBitangent;

layout(location = 0) out vec4 outColor;

// Uniform buffer (matches vertex shader)
layout(binding = 0) uniform UniformBufferObject {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projMatrix;
    mat4 normalMatrix;
    
    vec3 cameraPos;
    float time;
    
    // Primary light
    vec3 lightDirection;
    float lightIntensity;
    vec3 lightColor;
    float padding1;
    
    // Secondary light
    vec3 light2Direction;
    float light2Intensity;
    vec3 light2Color;
    float padding2;
    
    // Ambient lighting
    vec3 ambientColor;
    float ambientIntensity;
    
    // IBL and environment
    float exposure;
    float gamma;
    float iblIntensity;
    float shadowIntensity;
    
    // Material override
    float metallicFactor;
    float roughnessFactor;
    int renderMode;
    float padding3;
} ubo;

// PBR Material textures
layout(binding = 1) uniform sampler2D albedoMap;
layout(binding = 2) uniform sampler2D normalMap;
layout(binding = 3) uniform sampler2D metallicRoughnessMap;
layout(binding = 4) uniform sampler2D emissiveMap;
layout(binding = 5) uniform sampler2D aoMap;

const float PI = 3.14159265359;

// Utility functions
vec3 getNormalFromMap() {
    vec3 tangentNormal = texture(normalMap, fragTexCoord).xyz * 2.0 - 1.0;
    
    vec3 N = normalize(fragNormal);
    vec3 T = normalize(fragTangent);
    vec3 B = normalize(fragBitangent);
    mat3 TBN = mat3(T, B, N);
    
    return normalize(TBN * tangentNormal);
}

float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return num / denom;
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return num / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 calculatePBR(vec3 albedo, vec3 normal, vec3 viewDir, vec3 lightDir, vec3 lightColor, float metallic, float roughness) {
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    // Cook-Torrance BRDF
    float NDF = distributionGGX(normal, halfwayDir, roughness);
    float G = geometrySmith(normal, viewDir, lightDir, roughness);
    
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    vec3 F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
    
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    float NdotL = max(dot(normal, lightDir), 0.0);
    return (kD * albedo / PI + specular) * lightColor * NdotL;
}

void main() {
    // Sample material properties from textures
    vec4 albedoSample = texture(albedoMap, fragTexCoord);
    vec3 albedo = albedoSample.rgb * fragColor.rgb;
    float alpha = albedoSample.a * fragColor.a;
    
    // Get normal from normal map or use vertex normal
    vec3 normal = normalize(fragNormal);
    if (textureSize(normalMap, 0).x > 1) {
        normal = getNormalFromMap();
    }
    
    // Sample metallic/roughness map (metallic in B channel, roughness in G channel)
    vec3 metallicRoughnessSample = texture(metallicRoughnessMap, fragTexCoord).rgb;
    float metallic = metallicRoughnessSample.b * ubo.metallicFactor;
    float roughness = metallicRoughnessSample.g * ubo.roughnessFactor;
    
    // Clamp values
    metallic = clamp(metallic, 0.0, 1.0);
    roughness = clamp(roughness, 0.04, 1.0); // Prevent roughness from being too low
    
    // Sample AO map
    float ao = texture(aoMap, fragTexCoord).r;
    if (textureSize(aoMap, 0).x <= 1) ao = 1.0; // No AO map
    
    // Sample emissive map
    vec3 emissive = texture(emissiveMap, fragTexCoord).rgb;
    if (textureSize(emissiveMap, 0).x <= 1) emissive = vec3(0.0); // No emissive map
    
    vec3 viewDir = normalize(ubo.cameraPos - fragWorldPos);
    
    // Primary light contribution
    vec3 lightDir1 = normalize(-ubo.lightDirection);
    vec3 Lo = calculatePBR(albedo, normal, viewDir, lightDir1, ubo.lightColor * ubo.lightIntensity, metallic, roughness);
    
    // Secondary light contribution
    if (ubo.light2Intensity > 0.0) {
        vec3 lightDir2 = normalize(-ubo.light2Direction);
        Lo += calculatePBR(albedo, normal, viewDir, lightDir2, ubo.light2Color * ubo.light2Intensity, metallic, roughness);
    }
    
    // Ambient lighting with improved IBL approximation
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    vec3 F = fresnelSchlickRoughness(max(dot(normal, viewDir), 0.0), F0, roughness);
    
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    
    // Enhanced ambient lighting
    vec3 ambient = (kD * albedo + kS * 0.1) * ubo.ambientColor * ubo.ambientIntensity * ubo.iblIntensity * ao;
    
    // Add emissive
    vec3 color = ambient + Lo + emissive;
    
    // Handle render modes
    if (ubo.renderMode == 1) { // Wireframe
        color = mix(color, vec3(1.0), 0.8);
    } else if (ubo.renderMode == 2) { // Points
        color = vec3(1.0, 0.0, 0.0);
    } else if (ubo.renderMode == 3) { // Normals
        color = normal * 0.5 + 0.5;
    } else if (ubo.renderMode == 4) { // Albedo only
        color = albedo;
    } else if (ubo.renderMode == 5) { // Metallic only
        color = vec3(metallic);
    } else if (ubo.renderMode == 6) { // Roughness only
        color = vec3(roughness);
    } else if (ubo.renderMode == 7) { // AO only
        color = vec3(ao);
    }
    
    // HDR tone mapping
    color = color * ubo.exposure;
    
    // Reinhard tone mapping
    color = color / (color + vec3(1.0));
    
    // Gamma correction
    color = pow(color, vec3(1.0 / ubo.gamma));
    
    outColor = vec4(color, alpha);
}