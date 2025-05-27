#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <vector>
#include <string>
#include <raylib.h>
#include <raymath.h>
#include <appmanagement.h>
#include <meshmanagement.h>
#include <pathing.h>
#include <slice.h>
#include <backend.h>

#ifndef RAYGUI_IMPLEMENTATION
    #define RAYGUI_IMPLEMENTATION
    #include <raygui.h>
#endif
#ifndef RLIGHTS_IMPLEMENTATION
    #define RLIGHTS_IMPLEMENTATION
    #include <rlights.h>
#endif

Model Scale_Model(Model model, float scale) {
    for (int i = 0; i < model.meshCount; i++) {
        Mesh *mesh = &model.meshes[i];
        for (int v = 0; v < mesh->vertexCount; v++) {
            mesh->vertices[v * 3 + 0] *= scale; // x
            mesh->vertices[v * 3 + 1] *= scale; // y
            mesh->vertices[v * 3 + 2] *= scale; // z
        }
    }
    model.transform = MatrixIdentity(); // Reset transform (mesh is already scaled)
    return model;
}

Model Move_Model(Model model, Vector3 translation) {
    for (int i = 0; i < model.meshCount; i++) {
        Mesh *mesh = &model.meshes[i];
        for (int v = 0; v < mesh->vertexCount; v++) {
            mesh->vertices[v * 3 + 0] += translation.x;
            mesh->vertices[v * 3 + 1] += translation.y;
            mesh->vertices[v * 3 + 2] += translation.z;
        }
    }
    model.transform = MatrixIdentity(); // Reset transform (mesh is already moved)
    return model;
}

Color LerpColor(Color a, Color b, float t) {
    Color result;
    result.r = (unsigned char)((1 - t) * a.r + t * b.r);
    result.g = (unsigned char)((1 - t) * a.g + t * b.g);
    result.b = (unsigned char)((1 - t) * a.b + t * b.b);
    result.a = 255;
    return result;
}

int main(){
    app window;
    window.Initialise_Window(1000, 1500, 60, "Borb CAM Slicer", "src/assets/Logo-Light.png");
    Camera camera = window.Initialise_Camera((Vector3){20.0f, 20.0f, 20.0f}, (Vector3){0.0f, 8.0f, 0.0f}, (Vector3){0.0f, 1.6f, 0.0f},45.0f, CAMERA_PERSPECTIVE);
    Shader shader = window.Initialise_Shader();
    window.Initialise_Lights(shader);
    window.Create_File("src/output/Debug", "txt");

    window.Add_Button(1, 40, 100, 10, 50, "Slice");

    mesh models;
    models.Add_Model(1, "src/models/model.obj");
    Model currentmodel = models.Ret_Model(1);
    Model newmodel = Move_Model(Scale_Model(currentmodel, 0.1), (Vector3){0, -0.5, 0});
    BoundingBox cullBox = { Vector3{-5, -4, -5}, Vector3{5, 0, 5} };

    path paths;
    paths.Create_File("src/output/OwO", "nc");

    std::vector<std::pair<Vector3, Vector3>> pathPositions;
    std::vector<std::vector<Line>> toolpath;

    slice slicing;
    slicing
        .Set_Slicing_Plane((Vector2){PI/4, 0}, 0)
        .Set_Slicing_Distance(0.1f)
        .Set_Starting_Position((Setting_Values){.mode = 0, .value3D = (Vector3){10, 10, 0}})
        .Set_Ending_Position((Setting_Values){.mode = 0, .value3D = (Vector3){0, 10, 10}});
    

    size_t currentLineIndex = 0;
    float animationProgress = 0.0f;
    auto lastUpdate = std::chrono::high_resolution_clock::now();
    std::vector<Line> flatToolpath; // Flattened toolpath used for animation


    while(!WindowShouldClose()){
        models.Sha_Model(1, shader);
        window.Update_Camera(&camera);
        BeginDrawing();
        ClearBackground(DARKGRAY);

        if (window.Ret_Button(1)) {
            // Clear previous toolpath
            slicing.Clear_Toolpath();
        
            // Setup Backend
            Backend backend;
            backend.set_model(newmodel);
            backend.schedule(Generate_Surface_Toolpath);
        
            // Run the backend
            backend.run();
        
            // Wait for backend to finish (basic polling)
            while (backend.return_toolpath()->empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        
            // Load toolpath from backend into slicer
            auto generatedToolpath = *backend.return_toolpath();
            slicing.Load_Toolpath(generatedToolpath);
        
            // Continue processing with main application
            slicing.Cull_Toolpath_by_Box(cullBox);
            slicing.Optimise_Start_End_Positions();
            slicing.Apply_AABB_Rays(GetModelBoundingBox(currentmodel));
            slicing.Optimise_Start_End_Linkages();
            slicing.Add_Start_End_Positions();
            slicing.Interpolate_Max_Angle_Displacement();
            toolpath = slicing.Return_Toolpath();
        
            flatToolpath.clear();
            for (const auto& segment : toolpath) {
                flatToolpath.insert(flatToolpath.end(), segment.begin(), segment.end());
            }
        
            currentLineIndex = 0;
            animationProgress = 0.0f;
            lastUpdate = std::chrono::high_resolution_clock::now();
        }



        BeginMode3D(camera);
        DrawBoundingBox(GetModelBoundingBox(newmodel), GREEN);

        Color gradientStart = BLUE;
        Color gradientEnd   = RED;

        for (const auto& paths : toolpath) {
            size_t count = paths.size();
            for (size_t i = 0; i < count; ++i) {
                float t = (count <= 1) ? 0.0f : (float)i / (float)(count - 1);
                Color lineColor = LerpColor(gradientStart, gradientEnd, t);
            
                DrawLine3D(paths[i].startLinePoint.Position, paths[i].endLinePoint.Position, lineColor);
            }
        }

        // Animation
        if (flatToolpath.size() > 0) {
        const float worldSpeed = 5.0f; // units per second
        
        auto now = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(now - lastUpdate).count();
        lastUpdate = now;
        
        static float segmentProgress = 0.0f;
        
        while (deltaTime > 0.0f && !flatToolpath.empty()) {
            Line& line = flatToolpath[currentLineIndex];
            float segmentLength = Vector3Distance(line.startLinePoint.Position, line.endLinePoint.Position);
        
            // Protect against degenerate zero-length lines
            if (segmentLength < 0.0001f) {
                currentLineIndex = (currentLineIndex + 1) % flatToolpath.size();
                segmentProgress = 0.0f;
                continue;
            }
        
            float duration = segmentLength / worldSpeed;
            float progressIncrement = deltaTime / duration;
            segmentProgress += progressIncrement;
        
            if (segmentProgress >= 1.0f) {
                deltaTime = (segmentProgress - 1.0f) * duration; // leftover time carries into next line
                segmentProgress = 0.0f;
                currentLineIndex = (currentLineIndex + 1) % flatToolpath.size();
            } else {
                deltaTime = 0.0f; // finished for now
            }
        }

        // Render the current segment
        Line& activeLine = flatToolpath[currentLineIndex];
        Vector3 pos = Vector3Lerp(activeLine.startLinePoint.Position, activeLine.endLinePoint.Position, segmentProgress);
        Vector3 norm = Vector3Normalize(Vector3Lerp(activeLine.startLinePoint.Normal, activeLine.endLinePoint.Normal, segmentProgress));
        Vector3 endPos = Vector3Add(pos, Vector3Scale(norm, 0.3f));

        DrawCylinderEx(pos, endPos, 0.05f, 0.05f, 8, YELLOW);
        DrawText(TextFormat("Line: %d / %d", (int)currentLineIndex, (int)flatToolpath.size()), 10, 50, 20, WHITE);
        DrawText(TextFormat("Segment progress: %.2f", segmentProgress), 10, 70, 20, WHITE);
        }

        slicing.Comp_Axis_Guides_3D();
        slicing.Comp_Ground_Grid();


        // models.Run_Models();
        EndMode3D();
        window.Run_Buttons();
        DrawFPS(10, 10);
        EndDrawing();
    }
    models.Stop_Models();
    CloseWindow();
    return 0;
}
