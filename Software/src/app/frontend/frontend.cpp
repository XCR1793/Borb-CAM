#include "frontend.h"
#include <raygui.h>
#include <raylib.h> // Still needed for BeginDrawing etc.

void Frontend::Initialize() {
    appInstance.Initialise_Window(1000, 1500, 60, "Borb CAM Slicer", "src/assets/Logo-Light.png");

    camera = appInstance.Initialise_Camera(
        {20.0f, 20.0f, 20.0f},
        {0.0f, 8.0f, 0.0f},
        {0.0f, 1.6f, 0.0f},
        45.0f,
        CAMERA_PERSPECTIVE
    );

    shader = appInstance.Initialise_Shader();
    appInstance.Initialise_Lights(shader);
}

void Frontend::UpdateCamera() {
    appInstance.Update_Camera(&camera);  // ✅ Use your custom camera logic
}

void Frontend::Draw(Backend& backend) {
    BeginDrawing();
    ClearBackground(DARKGRAY);
    
    backend.UpdateToolpathIfRequested();

    BeginMode3D(camera);
    backend.DrawModelWithToolpath();
    EndMode3D();

    backend.DrawUI();
    DrawFPS(10, 10);
    EndDrawing();
}
