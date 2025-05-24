#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <vector>
#include <string>
#include <raylib.h>
#include <raymath.h>
#include "appmanagement.h"
#include "meshmanagement.h"
#include "pathing.h"
#include "slice.h"

#ifndef RAYGUI_IMPLEMENTATION
    #define RAYGUI_IMPLEMENTATION
    #include <raygui.h>
#endif
#ifndef RLIGHTS_IMPLEMENTATION
    #define RLIGHTS_IMPLEMENTATION
    #include <rlights.h>
#endif

int main(){
    app window;
    window.Initialise_Window(1000, 1500, 60, "Borb CAM Slicer", "src/Logo-Light.png");
    Camera camera = window.Initialise_Camera((Vector3){20.0f, 20.0f, 20.0f}, (Vector3){0.0f, 8.0f, 0.0f}, (Vector3){0.0f, 1.6f, 0.0f},45.0f, CAMERA_PERSPECTIVE);

    Shader shader = window.Initialise_Shader();
    window.Initialise_Lights(shader);
    window.Create_File("src/Debug", "txt");

    mesh models;
    models.Add_Model(1, "src/model.obj");

    // Bounding box to test against
    BoundingBox box = {
        .min = {4.0f, 0.0f, -2.0f},
        .max = {10.0f, 7.0f, 2.0f}
    };

    std::vector<Line> edgeCases;

    // Line 1: touches edge
    edgeCases.push_back({
        .startLinePoint = {{4.0f, 2.0f, 0.0f}, {0,1,0}},
        .endLinePoint   = {{3.0f, 2.0f, 0.0f}, {0,1,0}},
        .type = 1
    });

    // Line 2: parallel to face (just above top)
    edgeCases.push_back({
        .startLinePoint = {{4.0f, 6.01f, 0.0f}, {0,1,0}},
        .endLinePoint   = {{10.0f, 6.01f, 0.0f}, {0,1,0}},
        .type = 1
    });

    // Line 3: crosses corner diagonally
    edgeCases.push_back({
        .startLinePoint = {{3.0f, -1.0f, -3.0f}, {0,1,0}},
        .endLinePoint   = {{11.0f, 7.0f, 3.0f}, {0,1,0}},
        .type = 1
    });

    // Line 4: starts inside, ends outside
    edgeCases.push_back({
        .startLinePoint = {{5.0f, 2.0f, 0.0f}, {0,1,0}},
        .endLinePoint   = {{12.0f, 2.0f, 0.0f}, {0,1,0}},
        .type = 1
    });

    // Line 5: fully inside
    edgeCases.push_back({
        .startLinePoint = {{5.0f, 2.0f, 0.0f}, {0,1,0}},
        .endLinePoint   = {{6.0f, 2.0f, 0.0f}, {0,1,0}},
        .type = 1
    });

    // Line 6: fully outside
    edgeCases.push_back({
        .startLinePoint = {{12.0f, 8.0f, 5.0f}, {0,1,0}},
        .endLinePoint   = {{13.0f, 9.0f, 6.0f}, {0,1,0}},
        .type = 1
    });

    // Line 7: along box edge
    edgeCases.push_back({
        .startLinePoint = {{4.0f, 0.0f, -2.0f}, {0,1,0}},
        .endLinePoint   = {{4.0f, 6.0f, -2.0f}, {0,1,0}},
        .type = 1
    });

    std::vector<Line> culled = models.Cull_Lines_ByBox(box, edgeCases);
    std::vector<bool> lineEnabled(edgeCases.size(), true); // toggle flags for each line

    while (!WindowShouldClose()) {
        // Toggle lines with number keys
        if (IsKeyPressed(KEY_ONE))   { lineEnabled[0] = !lineEnabled[0]; printf("Line 1 %s\n", lineEnabled[0] ? "Enabled" : "Disabled"); }
        if (IsKeyPressed(KEY_TWO))   { lineEnabled[1] = !lineEnabled[1]; printf("Line 2 %s\n", lineEnabled[1] ? "Enabled" : "Disabled"); }
        if (IsKeyPressed(KEY_THREE)) { lineEnabled[2] = !lineEnabled[2]; printf("Line 3 %s\n", lineEnabled[2] ? "Enabled" : "Disabled"); }
        if (IsKeyPressed(KEY_FOUR))  { lineEnabled[3] = !lineEnabled[3]; printf("Line 4 %s\n", lineEnabled[3] ? "Enabled" : "Disabled"); }
        if (IsKeyPressed(KEY_FIVE))  { lineEnabled[4] = !lineEnabled[4]; printf("Line 5 %s\n", lineEnabled[4] ? "Enabled" : "Disabled"); }
        if (IsKeyPressed(KEY_SIX))   { lineEnabled[5] = !lineEnabled[5]; printf("Line 6 %s\n", lineEnabled[5] ? "Enabled" : "Disabled"); }
        if (IsKeyPressed(KEY_SEVEN)) { lineEnabled[6] = !lineEnabled[6]; printf("Line 7 %s\n", lineEnabled[6] ? "Enabled" : "Disabled"); }

        window.Update_Camera(&camera);
        BeginDrawing();
        ClearBackground(DARKGRAY);
        BeginMode3D(camera);

        DrawBoundingBox(box, RED);

        // Draw box corners
        Vector3 corners[8] = {
            box.min,
            {box.min.x, box.min.y, box.max.z},
            {box.min.x, box.max.y, box.min.z},
            {box.min.x, box.max.y, box.max.z},
            {box.max.x, box.min.y, box.min.z},
            {box.max.x, box.min.y, box.max.z},
            {box.max.x, box.max.y, box.min.z},
            box.max
        };
        for (int i = 0; i < 8; i++) {
            Vector3 pos = Vector3Add(corners[i], (Vector3){0, 0.1f, 0});
            DrawCube(pos, 0.1f, 0.1f, 0.1f, RED);
        }

        // Draw enabled edge case lines
        for (size_t i = 0; i < edgeCases.size(); i++) {
            if (!lineEnabled[i]) continue;
            Vector3 offset = {0, 2.0f, 0};
            Vector3 start = Vector3Add(edgeCases[i].startLinePoint.Position, offset);
            Vector3 end   = Vector3Add(edgeCases[i].endLinePoint.Position, offset);
            DrawLine3D(start, end, BLUE);
            DrawCube(start, 0.1f, 0.1f, 0.1f, DARKBLUE);
            DrawCube(end,   0.1f, 0.1f, 0.1f, DARKBLUE);

            // Label
            Vector3 mid = Vector3Lerp(start, end, 0.5f);
            Vector2 screenPos = GetWorldToScreen(mid, camera);
            DrawText(TextFormat("L%zu", i), screenPos.x, screenPos.y, 10, SKYBLUE);
        }

        // Draw culled lines and labels
        for (size_t i = 0; i < culled.size(); i++) {
            Vector3 offset = {0, 4.0f, 0};
            Vector3 start = Vector3Add(culled[i].startLinePoint.Position, offset);
            Vector3 end   = Vector3Add(culled[i].endLinePoint.Position, offset);
            DrawLine3D(start, end, YELLOW);
            DrawCube(start, 0.2f, 0.2f, 0.2f, GOLD);
            DrawCube(end,   0.2f, 0.2f, 0.2f, GOLD);

            Vector3 mid = Vector3Lerp(start, end, 0.5f);
            Vector2 screenPos = GetWorldToScreen(mid, camera);
            DrawText(TextFormat("C%zu", i), screenPos.x, screenPos.y, 10, GOLD);
        }

        EndMode3D();
        window.Run_Buttons();
        DrawFPS(10, 10);
        EndDrawing();
    }

    models.Stop_Models();
    CloseWindow();
    return 0;
}
