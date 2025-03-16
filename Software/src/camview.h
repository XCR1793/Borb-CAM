#ifndef CAMVIEW
#define CAMVIEW
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <raylib.h>
#include <raymath.h>

class camview{
    public:

    Camera camera = { 0 };

    void Initialise_Camera();

    void UpdateCameraControls(Camera *camera, float zoomFactor);
};

#endif