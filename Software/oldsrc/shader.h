#ifndef SHADER
#define SHADER

#include <raylib.h>
#include <raymath.h>
#include <rlights.h>
#include <rlgl.h>

typedef struct {
    Model model;
    Shader shader;
    Texture2D hdrTexture;
} HDRPhotosphere;

class shader{
    public:
    Light* SetupLights(Shader shader, Vector3 center, float diameter, int lightCount);
    void UpdateLights(Light* lights, Vector3 center, float diameter, int lightCount, float speed);

    // HDRPhotosphere LoadHDRPhotosphere(const char* hdrTexturePath, const char* shaderVert, const char* shaderFrag);
    // void DrawHDRPhotosphere(HDRPhotosphere sphere, Camera3D camera);
};

#endif