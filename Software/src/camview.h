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

    // void UpdateCameraView(Model model, BoundingBox bbox, Vector3 quad[4], Vector3 triangle[3], Vector3 sphereCenter, float sphereRadius, Vector3 modelPosition, Vector3 rotationAxis, float rotationAngle, float modelScale);
    // void DrawCollisionPoint(RayHitResult collision, Color pointColor);
    // Ray GetMouseRayInternal();
    // RayHitResult GetRayCollisionObjects(Ray ray, BoundingBox bbox, Model model, Vector3 quad[4], Vector3 triangle[3], Vector3 sphereCenter, float sphereRadius);
    // BoundingBox TransformBoundingBox(BoundingBox bbox, Vector3 position, Vector3 rotationAxis, float angle, float scale);

};

#endif