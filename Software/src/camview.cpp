#include "camview.h"

void camview::Initialise_Camera(){
    camera.position = (Vector3){ 2.0f, 4.0f, 6.0f };
    camera.target = (Vector3){ 0.0f, 0.5f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
}

void camview::UpdateCameraControls(Camera *camera, float zoomFactor){
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)){
        Vector2 delta = GetMouseDelta();
        delta = Vector2Scale(delta, zoomFactor * 0.01f);
        Vector3 forward = Vector3Subtract(camera->target, camera->position);
        forward = Vector3Normalize(forward);
        Vector3 right = Vector3CrossProduct(forward, camera->up);
        camera->target = Vector3Add(camera->target, Vector3Scale(right, -delta.x));
        camera->target = Vector3Add(camera->target, Vector3Scale(camera->up, delta.y));
        camera->position = Vector3Add(camera->position, Vector3Scale(right, -delta.x));
        camera->position = Vector3Add(camera->position, Vector3Scale(camera->up, delta.y));
    }
    
    float wheelMove = GetMouseWheelMove();
    if (wheelMove != 0){
        Vector3 direction = Vector3Subtract(camera->target, camera->position);
        direction = Vector3Scale(direction, 1.0f + wheelMove * 0.1f);
        camera->position = Vector3Subtract(camera->target, direction);
    }
    
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)){
        Vector2 delta = GetMouseDelta();
        delta = Vector2Scale(delta, 0.005f);
        Vector3 right = Vector3CrossProduct(Vector3Subtract(camera->target, camera->position), camera->up);
        camera->position = Vector3RotateByAxisAngle(camera->position, camera->up, -delta.x);
        camera->position = Vector3RotateByAxisAngle(camera->position, right, -delta.y);
    }
}

// // Internal function to get mouse ray
// Ray camview::GetMouseRayInternal() {
//     return GetMouseRay(GetMousePosition(), camera);
// }

// // Transform bounding box based on position, rotation, and scale
// BoundingBox camview::TransformBoundingBox(BoundingBox bbox, Vector3 position, Vector3 rotationAxis, float angle, float scale) {
//     Matrix transform = MatrixMultiply(MatrixMultiply(MatrixScale(scale, scale, scale), MatrixRotate(rotationAxis, angle)), MatrixTranslate(position.x, position.y, position.z));

//     Vector3 minTransformed = Vector3Transform(bbox.min, transform);
//     Vector3 maxTransformed = Vector3Transform(bbox.max, transform);
    
//     return (BoundingBox){ minTransformed, maxTransformed };
// }

// // Perform ray collision checks
// RayHitResult camview::GetRayCollisionObjects(Ray ray, BoundingBox bbox, Model model, Vector3 quad[4], Vector3 triangle[3], Vector3 sphereCenter, float sphereRadius) {
//     RayHitResult result = { false, {0}, {0}, FLT_MAX };

//     // Ground collision
//     RayCollision groundHit = GetRayCollisionQuad(ray, quad[0], quad[1], quad[2], quad[3]);
//     if (groundHit.hit && groundHit.distance < result.distance) {
//         result.hit = true;
//         result.point = groundHit.point;
//         result.normal = groundHit.normal;
//         result.distance = groundHit.distance;
//     }

//     // Triangle collision
//     RayCollision triHit = GetRayCollisionTriangle(ray, triangle[0], triangle[1], triangle[2]);
//     if (triHit.hit && triHit.distance < result.distance) {
//         result.hit = true;
//         result.point = triHit.point;
//         result.normal = triHit.normal;
//         result.distance = triHit.distance;
//     }

//     // Sphere collision
//     RayCollision sphereHit = GetRayCollisionSphere(ray, sphereCenter, sphereRadius);
//     if (sphereHit.hit && sphereHit.distance < result.distance) {
//         result.hit = true;
//         result.point = sphereHit.point;
//         result.normal = sphereHit.normal;
//         result.distance = sphereHit.distance;
//     }

//     // Bounding Box Collision
//     RayCollision bboxHit = GetRayCollisionBox(ray, bbox);
//     if (bboxHit.hit && bboxHit.distance < result.distance) {
//         result.hit = true;
//         result.point = bboxHit.point;
//         result.normal = bboxHit.normal;
//         result.distance = bboxHit.distance;

//         // Mesh collision (if inside bounding box)
//         for (int i = 0; i < model.meshCount; i++) {
//             RayCollision meshHit = GetRayCollisionMesh(ray, model.meshes[i], model.transform);
//             if (meshHit.hit && meshHit.distance < result.distance) {
//                 result.hit = true;
//                 result.point = meshHit.point;
//                 result.normal = meshHit.normal;
//                 result.distance = meshHit.distance;
//             }
//         }
//     }

//     return result;
// }

// // Update function to check ray collisions each frame
// void camview::UpdateCameraView(Model model, BoundingBox bbox, Vector3 quad[4], Vector3 triangle[3], Vector3 sphereCenter, float sphereRadius, Vector3 modelPosition, Vector3 rotationAxis, float rotationAngle, float modelScale) {
//     Ray ray = GetMouseRayInternal();  // Get the ray from mouse position
    
//     // Transform bounding box to match model transformation
//     BoundingBox transformedBbox = TransformBoundingBox(bbox, modelPosition, rotationAxis, rotationAngle, modelScale);

//     RayHitResult collision = GetRayCollisionObjects(ray, bbox, model, quad, triangle, sphereCenter, sphereRadius);

//     // Draw collision points if any
//     DrawCollisionPoint(collision, BLUE);
// }

// // Function to draw the collision point and normal
// void camview::DrawCollisionPoint(RayHitResult collision, Color pointColor) {
//     if (collision.hit) {
//         DrawCube(collision.point, 0.3f, 0.3f, 0.3f, pointColor);
//         DrawCubeWires(collision.point, 0.3f, 0.3f, 0.3f, RED);

//         Vector3 normalEnd = Vector3Add(collision.point, collision.normal);
//         DrawLine3D(collision.point, normalEnd, RED);
//     }
// }