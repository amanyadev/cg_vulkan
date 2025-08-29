#version 450

layout(location = 0) in vec2 fragCoord;
layout(location = 0) out vec4 outColor;

// Uniform buffer
layout(binding = 0) uniform UniformBufferObject {
    vec3 cameraPos;
    float time;
    vec3 cameraTarget;
    float aspectRatio;
    mat4 viewMatrix;
    mat4 projMatrix;
    
    // Settings (keeping for compatibility)
    int maxSteps;
    float maxDistance;
    int enableTrees;
    int enableWater;
    int enableClouds;
    int qualityLevel;
    float treeDistance;
    float padding;
} ubo;

// Simple noise function
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

// Simple height-based terrain
float getHeight(vec2 p) {
    // Create gentle rolling hills
    float height = 0.0;
    height += sin(p.x * 0.02) * 3.0;
    height += cos(p.y * 0.03) * 2.0;
    height += sin(p.x * 0.1 + p.y * 0.1) * 1.0;
    height += noise(p * 0.05) * 2.0;
    return height;
}

// Get terrain normal
vec3 getNormal(vec2 p) {
    float eps = 0.1;
    float h = getHeight(p);
    float hx = getHeight(p + vec2(eps, 0.0));
    float hy = getHeight(p + vec2(0.0, eps));
    
    vec3 normal = normalize(vec3(h - hx, eps, h - hy));
    return normal;
}

void main() {
    // Screen coordinates
    vec2 uv = fragCoord;
    uv.y = -uv.y; // Flip Y coordinate
    uv.x *= ubo.aspectRatio;
    
    // Camera setup
    vec3 ro = ubo.cameraPos; // Ray origin
    vec3 forward = normalize(ubo.cameraTarget - ro);
    vec3 right = normalize(cross(forward, vec3(0.0, 1.0, 0.0)));
    vec3 up = cross(right, forward);
    
    // Calculate ray direction
    vec3 rd = normalize(forward + uv.x * right + uv.y * up);
    
    vec3 col = vec3(0.5, 0.7, 1.0); // Default sky color
    
    // Simple ray-plane intersection for terrain
    if (rd.y < -0.001) { // Ray going downward
        float t = -(ro.y - getHeight(ro.xz)) / rd.y;
        
        if (t > 0.0 && t < 200.0) {
            vec3 hitPoint = ro + rd * t;
            vec2 terrainPos = hitPoint.xz;
            
            // Refine the intersection
            for (int i = 0; i < 3; i++) {
                float expectedHeight = getHeight(terrainPos);
                float actualHeight = hitPoint.y;
                float error = actualHeight - expectedHeight;
                
                if (abs(error) < 0.1) break;
                
                // Adjust intersection point
                t -= error / rd.y;
                hitPoint = ro + rd * t;
                terrainPos = hitPoint.xz;
            }
            
            // Calculate lighting
            vec3 normal = getNormal(terrainPos);
            vec3 lightDir = normalize(vec3(0.5, 0.8, 0.3));
            
            float diffuse = max(dot(normal, lightDir), 0.0);
            float ambient = 0.3;
            
            // Simple terrain coloring
            vec3 grassColor = vec3(0.3, 0.6, 0.2);
            vec3 dirtColor = vec3(0.5, 0.4, 0.3);
            
            float slope = 1.0 - normal.y;
            col = mix(grassColor, dirtColor, smoothstep(0.2, 0.6, slope));
            
            // Apply lighting - NO FOG!
            col = col * (diffuse * 0.7 + ambient);
            
            // Add some variation
            col *= 0.9 + 0.2 * noise(terrainPos * 20.0);
        }
    }
    
    // Gamma correction
    col = pow(col, vec3(1.0/2.2));
    
    outColor = vec4(col, 1.0);
}