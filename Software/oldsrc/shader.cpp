#include "shader.h"

Color GetRainbowColor(float t) {
    t = fmodf(t, 1.0f); // Ensure t is between 0 and 1

    float r, g, b;
    
    if (t < 1.0f / 6.0f) {       // Red → Yellow
        r = 1.0f;
        g = t * 6.0f;
        b = 0.0f;
    } else if (t < 2.0f / 6.0f) { // Yellow → Green
        r = (2.0f / 6.0f - t) * 6.0f;
        g = 1.0f;
        b = 0.0f;
    } else if (t < 3.0f / 6.0f) { // Green → Cyan
        r = 0.0f;
        g = 1.0f;
        b = (t - 2.0f / 6.0f) * 6.0f;
    } else if (t < 4.0f / 6.0f) { // Cyan → Blue
        r = 0.0f;
        g = (4.0f / 6.0f - t) * 6.0f;
        b = 1.0f;
    } else if (t < 5.0f / 6.0f) { // Blue → Magenta
        r = (t - 4.0f / 6.0f) * 6.0f;
        g = 0.0f;
        b = 1.0f;
    } else {                      // Magenta → Red
        r = 1.0f;
        g = 0.0f;
        b = (6.0f / 6.0f - t) * 6.0f;
    }

    return (Color){ (unsigned char)(r * 255), (unsigned char)(g * 255), (unsigned char)(b * 255), 255 };
}

Light* shader::SetupLights(Shader shader, Vector3 center, float diameter, int lightCount) {
    if (lightCount <= 0) return nullptr; // Prevent invalid allocations

    // Dynamically allocate memory for lights
    Light* lights = new Light[lightCount]; 

    float radius = diameter / 2.0f;
    float angleStep = 2.0f * PI / lightCount;

    for (int i = 0; i < lightCount; i++) {
        float angle = i * angleStep;
        float x = center.x + radius * cosf(angle);
        float z = center.z + radius * sinf(angle);
        float y = center.y; // Keeping lights at the same height

        float t = (float)i / (float)lightCount; // Normalize position in the ring (0 to 1)
        Color rainbowColor = GetRainbowColor(t); // Get smooth rainbow color

        lights[i] = CreateLight(LIGHT_POINT, (Vector3){ x, y, z }, Vector3Zero(), rainbowColor, shader);
    }

    return lights;
}

void shader::UpdateLights(Light* lights, Vector3 center, float diameter, int lightCount, float speed) {
    if (!lights) return;

    float radius = diameter / 2.0f;
    float angleStep = 2.0f * PI / lightCount;
    
    float rotationAngle = GetTime() * speed; // Rotate over time

    for (int i = 0; i < lightCount; i++) {
        float angle = i * angleStep + rotationAngle; // Add rotation angle
        float x = center.x + radius * cos(angle);
        float z = center.z + radius * sin(angle);
        float y = center.y; // Keeping height consistent

        float t = (float)i / (float)lightCount; // Normalize position in the ring (0 to 1)
        Color rainbowColor = GetRainbowColor(t);

        lights[i].position = (Vector3){ x, y, z };
        lights[i].color = rainbowColor; // Update color dynamically
    }
}


// HDRPhotosphere shader::LoadHDRPhotosphere(const char* hdrTexturePath, const char* shaderVert, const char* shaderFrag) {
//     HDRPhotosphere sphere;

//     // Load HDR texture
//     sphere.hdrTexture = LoadTexture(hdrTexturePath);
//     sphere.hdrTexture.width = sphere.hdrTexture.height; // Ensure it's square if needed

//     // Generate a unit sphere mesh (scaled in rendering)
//     Mesh sphereMesh = GenMeshSphere(1.0f, 64, 64); // Small sphere to be scaled infinitely
//     sphere.model = LoadModelFromMesh(sphereMesh);

//     // Load HDR shader
//     sphere.shader = LoadShader(shaderVert, shaderFrag);

//     // Assign HDR texture to model
//     sphere.model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = sphere.hdrTexture; 
//     sphere.model.materials[0].shader = sphere.shader;

//     return sphere;
// }

// void shader::DrawHDRPhotosphere(HDRPhotosphere sphere, Camera3D camera) {
//     rlDisableBackfaceCulling(); // Render inside the sphere
//     rlDisableDepthMask();       // Prevent depth issues

//     Vector3 cameraPos = camera.position; // Ensure it moves with the camera

//     // Draw sphere at camera position with large scale
//     DrawModel(sphere.model, cameraPos, 1000.0f, WHITE);

//     rlEnableDepthMask();
//     rlEnableBackfaceCulling();
// }