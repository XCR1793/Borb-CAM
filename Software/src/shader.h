#ifndef SHADER
#define SHADER

#include <raylib.h>
#include <raymath.h>
#include <rlights.h>

class shader{
    public:
    Light* SetupLights(Shader shader, Vector3 center, float diameter, int lightCount);
    void UpdateLights(Light* lights, Vector3 center, float diameter, int lightCount);
    
};

#endif