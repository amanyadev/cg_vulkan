#version 450

layout(location = 0) in vec2 fragCoord;
layout(location = 0) out vec4 outColor;

// Simplified terrain-only uniform buffer
layout(binding = 0) uniform TerrainUniformData {
    vec3 cameraPos;
    float time;
    vec3 cameraTarget;
    float aspectRatio;
    mat4 viewMatrix;
    mat4 projMatrix;
    
    // Scene lighting
    vec3 sunDirection;
    float sunIntensity;
    vec3 sunColor;
    float padding1;
    vec3 ambientColor;
    float ambientIntensity;
    vec3 skyColorHorizon;
    float padding2;
    vec3 skyColorZenith;
    float padding3;
    vec3 fogColor;
    float fogDensity;
    
    // Terrain settings
    float terrainScale;
    float terrainHeight;
    float waterLevel;
    int enableWater;
    
    // Quality settings
    int qualityLevel;
    float viewDistance;
    float padding4[2];
} ubo;

// Fast noise functions
float hash21(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    
    float a = hash21(i);
    float b = hash21(i + vec2(1.0, 0.0));
    float c = hash21(i + vec2(0.0, 1.0));
    float d = hash21(i + vec2(1.0, 1.0));
    
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float fbm(vec2 p, int octaves) {
    float value = 0.0;
    float amplitude = 0.5;
    for (int i = 0; i < octaves; i++) {
        value += amplitude * noise(p);
        p *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

// Optimized terrain height
float getTerrainHeight(vec2 p) {
    p *= ubo.terrainScale;
    float height = 0.0;
    
    // Base terrain layers
    height += sin(p.x * 0.008) * 15.0 + cos(p.y * 0.01) * 12.0;
    height += sin(p.x * 0.02 + p.y * 0.015) * 8.0;
    height += cos(p.x * 0.025) * cos(p.y * 0.03) * 5.0;
    
    // Add detail based on quality
    if (ubo.qualityLevel >= 1) {
        height += fbm(p * 0.05, 2) * 4.0;
    }
    if (ubo.qualityLevel >= 2) {
        height += fbm(p * 0.1, 1) * 2.0;
    }
    
    // River valleys
    float river1 = abs(sin(p.x * 0.005 + cos(p.y * 0.003) * 2.0));
    float river2 = abs(sin(p.y * 0.004 + cos(p.x * 0.006) * 1.5));
    height -= smoothstep(0.0, 0.3, 1.0 - river1) * 10.0;
    height -= smoothstep(0.0, 0.2, 1.0 - river2) * 8.0;
    
    return height * ubo.terrainHeight * 0.01;
}

// Water height with waves
float getWaterHeight(vec2 p) {
    float waves = 0.0;
    waves += sin(p.x * 0.1 + ubo.time * 2.0) * 0.15;
    waves += sin(p.y * 0.08 + ubo.time * 1.5) * 0.1;
    waves += sin((p.x + p.y) * 0.05 + ubo.time * 2.5) * 0.08;
    return ubo.waterLevel + waves;
}

// Terrain normal
vec3 getTerrainNormal(vec2 p) {
    float eps = 0.1;
    float h = getTerrainHeight(p);
    float hx = getTerrainHeight(p + vec2(eps, 0.0));
    float hy = getTerrainHeight(p + vec2(0.0, eps));
    return normalize(vec3(h - hx, eps, h - hy));
}

// Enhanced terrain material
vec3 getTerrainMaterial(vec2 worldPos, float height, vec3 normal) {
    float slope = 1.0 - normal.y;
    
    // Base colors
    vec3 grass = vec3(0.3, 0.6, 0.2);
    vec3 dirt = vec3(0.5, 0.35, 0.25);
    vec3 rock = vec3(0.4, 0.4, 0.4);
    vec3 snow = vec3(0.9, 0.9, 0.95);
    
    vec3 color = grass;
    
    // Height-based blending
    color = mix(color, dirt, smoothstep(-2.0, 5.0, height));
    color = mix(color, rock, smoothstep(0.4, 0.8, slope));
    color = mix(color, snow, smoothstep(15.0, 20.0, height));
    
    // Add texture variation
    float variation = noise(worldPos * 20.0) * 0.3 + 0.7;
    color *= variation;
    
    return color;
}

// Simple ray-terrain intersection
bool intersectTerrain(vec3 ro, vec3 rd, out vec3 hitPoint, out bool isWater) {
    if (rd.y >= -0.001) return false;
    
    float t = 0.1;
    isWater = false;
    
    for (int i = 0; i < 32; i++) {
        vec3 p = ro + rd * t;
        float terrainH = getTerrainHeight(p.xz);
        float waterH = (ubo.enableWater == 1) ? getWaterHeight(p.xz) : -1000.0;
        
        // Check water first
        if (ubo.enableWater == 1 && p.y <= waterH && p.y > terrainH) {
            hitPoint = vec3(p.x, waterH, p.z);
            isWater = true;
            return true;
        }
        
        // Check terrain
        if (p.y <= terrainH) {
            hitPoint = vec3(p.x, terrainH, p.z);
            return true;
        }
        
        t += max(0.1, abs(p.y - max(terrainH, waterH)) * 0.5);
        if (t > ubo.viewDistance) break;
    }
    
    return false;
}

// Atmospheric fog
vec3 applyFog(vec3 color, float distance, vec3 rayDir) {
    float fogAmount = 1.0 - exp(-distance * ubo.fogDensity);
    vec3 skyColor = mix(ubo.skyColorHorizon, ubo.skyColorZenith, max(rayDir.y * 0.5 + 0.5, 0.0));
    return mix(color, skyColor, fogAmount);
}

void main() {
    // Screen coordinates
    vec2 uv = fragCoord;
    uv.y = -uv.y;
    uv.x *= ubo.aspectRatio;
    
    // Camera ray
    vec3 ro = ubo.cameraPos;
    vec3 forward = normalize(ubo.cameraTarget - ro);
    vec3 right = normalize(cross(forward, vec3(0.0, 1.0, 0.0)));
    vec3 up = cross(right, forward);
    vec3 rd = normalize(forward + uv.x * right + uv.y * up);
    
    vec3 color;
    vec3 hitPoint;
    bool isWater;
    
    if (intersectTerrain(ro, rd, hitPoint, isWater)) {
        float distance = length(hitPoint - ro);
        
        if (isWater) {
            // Water rendering
            vec3 waterColor = vec3(0.1, 0.3, 0.6);
            vec3 normal = vec3(0.0, 1.0, 0.0); // Simplified for performance
            
            // Simple reflection
            float fresnel = pow(1.0 - max(dot(normal, -rd), 0.0), 2.0);
            vec3 skyColor = mix(ubo.skyColorHorizon, ubo.skyColorZenith, 0.5);
            waterColor = mix(waterColor, skyColor * 0.8, fresnel);
            
            // Lighting
            float diff = max(dot(normal, normalize(ubo.sunDirection)), 0.0);
            color = waterColor * (ubo.sunColor * ubo.sunIntensity * diff * 0.3 + ubo.ambientColor * ubo.ambientIntensity);
            
        } else {
            // Terrain rendering
            vec3 normal = getTerrainNormal(hitPoint.xz);
            vec3 material = getTerrainMaterial(hitPoint.xz, hitPoint.y, normal);
            
            // Enhanced lighting
            float diff = max(dot(normal, normalize(ubo.sunDirection)), 0.0);
            vec3 diffuse = material * ubo.sunColor * ubo.sunIntensity * diff;
            vec3 ambient = material * ubo.ambientColor * ubo.ambientIntensity;
            
            color = diffuse + ambient;
        }
        
        // Apply atmospheric fog
        color = applyFog(color, distance, rd);
        
    } else {
        // Sky rendering
        float skyGradient = max(rd.y * 0.5 + 0.5, 0.0);
        color = mix(ubo.skyColorHorizon, ubo.skyColorZenith, skyGradient);
        
        // Simple sun
        float sunDot = max(dot(rd, normalize(ubo.sunDirection)), 0.0);
        color += ubo.sunColor * pow(sunDot, 128.0) * ubo.sunIntensity;
    }
    
    // Tone mapping and gamma correction
    color = color / (color + vec3(1.0)); // Simple Reinhard
    color = pow(color, vec3(1.0/2.2));   // Gamma correction
    
    outColor = vec4(color, 1.0);
}