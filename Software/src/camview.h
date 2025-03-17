#ifndef CAMVIEW
#define CAMVIEW
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <raylib.h>
#include <raymath.h>

// #ifndef FLT_MAX
// #define FLT_MAX 3.402823466e+38F  // Correct value for max float
// #endif

typedef struct {
    bool hit;
    Vector3 point;
    Vector3 normal;
    float distance;
} RayHitResult;

class camview{
    public:

    Camera camera = { 0 };

    void Initialise_Camera();

    void UpdateCameraControls(Camera *camera, float zoomFactor);


};

#endif