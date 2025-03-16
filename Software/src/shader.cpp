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
    static Light lights[MAX_LIGHTS]; // Ensure MAX_LIGHTS is defined properly

    float radius = diameter / 2.0f;
    float angleStep = 2.0f * PI / lightCount;

    for (int i = 0; i < lightCount; i++) {
        float angle = i * angleStep;
        float x = center.x + radius * cos(angle);
        float z = center.z + radius * sin(angle);
        float y = center.y; // Keeping lights at the same height

        float t = (float)i / (float)lightCount; // Normalize position in the ring (0 to 1)
        Color rainbowColor = GetRainbowColor(t); // Get smooth rainbow color

        lights[i] = CreateLight(LIGHT_POINT, (Vector3){ x, y, z }, Vector3Zero(), rainbowColor, shader);
    }

    return lights;
}

void shader::UpdateLights(Light* lights, Vector3 center, float diameter, int lightCount){
    if (!lights) return;

    float radius = diameter / 2.0f;
    float angleStep = 2.0f * PI / lightCount;

    for (int i = 0; i < lightCount; i++) {
        float angle = i * angleStep;
        float x = center.x + radius * cos(angle);
        float z = center.z + radius * sin(angle);
        float y = center.y; // Keeping height consistent

        float t = (float)i / (float)lightCount; // Normalize position in the ring (0 to 1)
        Color rainbowColor = GetRainbowColor(t);

        lights[i].position = (Vector3){ x, y, z };
        lights[i].color = rainbowColor; // Update color dynamically
    }
}