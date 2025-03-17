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

