#version 450

// Vertex attributes from glTF model
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec4 inColor;

// Uniform buffer
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

// Outputs to fragment shader
layout(location = 0) out vec3 fragWorldPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec4 fragColor;
layout(location = 4) out vec3 fragTangent;
layout(location = 5) out vec3 fragBitangent;

void main() {
    vec4 worldPos = ubo.modelMatrix * vec4(inPosition, 1.0);
    gl_Position = ubo.projMatrix * ubo.viewMatrix * worldPos;
    
    fragWorldPos = worldPos.xyz;
    fragNormal = normalize((ubo.normalMatrix * vec4(inNormal, 0.0)).xyz);
    fragTexCoord = inTexCoord;
    fragColor = inColor;
    
    // Calculate tangent space for normal mapping
    // Simple tangent calculation (for proper normal mapping, tangents should come from the model)
    vec3 c1 = cross(fragNormal, vec3(0.0, 0.0, 1.0));
    vec3 c2 = cross(fragNormal, vec3(0.0, 1.0, 0.0));
    fragTangent = normalize(length(c1) > length(c2) ? c1 : c2);
    fragBitangent = normalize(cross(fragNormal, fragTangent));
}
