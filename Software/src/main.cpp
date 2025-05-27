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

    window.Add_Button(1,  20, 60,  30, 100, "X+");
    window.Add_Button(2,  20, 60, 120, 100, "X-");
    window.Add_Button(3,  20, 60,  30, 150, "Y+");
    window.Add_Button(4,  20, 60, 120, 150, "Y-");
    window.Add_Button(5,  20, 60,  30, 200, "Z+");
    window.Add_Button(6,  20, 60, 120, 200, "Z-");
    window.Add_Button(7,  20, 60,  30, 250, "S+");
    window.Add_Button(8,  20, 60, 120, 250, "S-");
    window.Add_Button(9,  20, 60,  30, 300, "A+");
    window.Add_Button(10, 20, 60, 120, 300, "A-");
    window.Add_Button(11, 20, 60,  30, 350, "B+");
    window.Add_Button(12, 20, 60, 120, 350, "B-");
    window.Add_Button(13, 20, 60,  30, 400, "C+");
    window.Add_Button(14, 20, 60, 120, 400, "C-");
    window.Add_Button(15, 20, 60,  30, 450, "O+");
    window.Add_Button(16, 20, 60, 120, 450, "O-");
    window.Add_Button(17, 20, 60,  75, 500, "Slice");
    window.Add_Button(18, 20, 120, 75, 525, "Display Model");
    // window.Add_Button(19, 20, 120, 75, 550, "Display Rays");
    window.Add_Button(20, 20, 100, 75, 575, "Slice to G-code");
    window.Add_Button(21, 20, 100, 75, 600, "Slice A+");
    window.Add_Button(22, 20, 100, 75, 625, "Slice A-");
    window.Add_Button(23, 20, 100, 75, 650, "Slice B+");
    window.Add_Button(24, 20, 100, 75, 675, "Slice B-");
    window.Add_Button(25, 20, 180, 75, 700, "Rot: ABC");
    window.Add_Button(26, 20, 160, 75, 725, "Show/Hide Gen");
    window.Add_Button(27, 20, 160, 75, 750, "Show/Hide Path");
    window.Add_Button(28, 20, 160, 75, 775, "Load GCode");

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
    auto lastUpdate = std::chrono::high_resolution_clock::now();
    std::vector<Line> flatToolpath; // Flattened toolpath used for animation

// === Model Transform & Slicing Config ===
static float scale = 0.1f;
static Vector3 offset = { 0, -0.5f, 0 };
static Vector2 sliceRot = { PI / 4.0f, 0 };
static float planeOffset = 0.0f;
int rotationMode = 0;
bool modelNeedsUpdate = true;


// === UI Toggles ===
bool modelVisible = true;
bool showPath = true;
bool showCombinedPath = true;


    while(!WindowShouldClose()){
        models.Sha_Model(1, shader);
        window.Update_Camera(&camera);
        BeginDrawing();
        ClearBackground(DARKGRAY);

// === Transform Controls ===
if (window.Ret_Button(1))  { offset.x += 0.1f; modelNeedsUpdate = true; }
if (window.Ret_Button(2))  { offset.x -= 0.1f; modelNeedsUpdate = true; }
if (window.Ret_Button(3))  { offset.y += 0.1f; modelNeedsUpdate = true; }
if (window.Ret_Button(4))  { offset.y -= 0.1f; modelNeedsUpdate = true; }
if (window.Ret_Button(5))  { offset.z += 0.1f; modelNeedsUpdate = true; }
if (window.Ret_Button(6))  { offset.z -= 0.1f; modelNeedsUpdate = true; }

if (window.Ret_Button(7))  { scale *= 1.1f; modelNeedsUpdate = true; }
if (window.Ret_Button(8))  { scale *= 0.9f; modelNeedsUpdate = true; }

if (window.Ret_Button(9))  { sliceRot.x += 0.05f; slicing.Set_Slicing_Plane(sliceRot, planeOffset); }
if (window.Ret_Button(10)) { sliceRot.x -= 0.05f; slicing.Set_Slicing_Plane(sliceRot, planeOffset); }
if (window.Ret_Button(11)) { sliceRot.y += 0.05f; slicing.Set_Slicing_Plane(sliceRot, planeOffset); }
if (window.Ret_Button(12)) { sliceRot.y -= 0.05f; slicing.Set_Slicing_Plane(sliceRot, planeOffset); }

if (window.Ret_Button(13)) {} // C+
if (window.Ret_Button(14)) {} // C-

if (window.Ret_Button(15)) { planeOffset += 0.05f; slicing.Set_Slicing_Plane(sliceRot, planeOffset); }
if (window.Ret_Button(16)) { planeOffset -= 0.05f; slicing.Set_Slicing_Plane(sliceRot, planeOffset); }

// === Slice Trigger ===
if (window.Ret_Button(17)) {
    slicing.Clear_Toolpath();
    slicing.Generate_Surface_Toolpath(newmodel);
    slicing.Cull_Toolpath_by_Box(cullBox);
    slicing.Optimise_Start_End_Positions();
    slicing.Apply_AABB_Rays(GetModelBoundingBox(newmodel));
    slicing.Optimise_Start_End_Linkages();
    slicing.Add_Start_End_Positions();
    slicing.Interpolate_Max_Angle_Displacement();
    toolpath = slicing.Return_Toolpath();

    flatToolpath.clear();
    for (const auto& segment : toolpath)
        flatToolpath.insert(flatToolpath.end(), segment.begin(), segment.end());
    currentLineIndex = 0;
    lastUpdate = std::chrono::high_resolution_clock::now();
}

// === Toggle Model Visibility ===
if (window.Ret_Button(18)) {
    modelVisible = !modelVisible;
    window.Rem_Button(18);
    window.Add_Button(18, 20, 120, 75, 525, modelVisible ? "Hide Model" : "Show Model");
}

// === G-code Save ===
if (window.Ret_Button(20)) {
    std::vector<Line> flat = slicing.Toolpath_Flattener();
    if (!flat.empty()) {
        paths.Clear_File();
        paths.Reset_N();
        for (const Line& line : flat) {
            Vector3 pos = line.endLinePoint.Position;
            Vector3 rot = models.NormalToRotation(line.endLinePoint.Normal);
            pathPositions.push_back({ pos, rot });
        }
        paths.Path_to_Gcode1(pathPositions);
    }
}

// === Slicing Angle Rotation ===
if (window.Ret_Button(21)) { sliceRot.x += PI / 20.0f; slicing.Set_Slicing_Plane(sliceRot, planeOffset); }
if (window.Ret_Button(22)) { sliceRot.x -= PI / 20.0f; slicing.Set_Slicing_Plane(sliceRot, planeOffset); }
if (window.Ret_Button(23)) { sliceRot.y += PI / 20.0f; slicing.Set_Slicing_Plane(sliceRot, planeOffset); }
if (window.Ret_Button(24)) { sliceRot.y -= PI / 20.0f; slicing.Set_Slicing_Plane(sliceRot, planeOffset); }

// === G-code Rotation Order Toggle ===
if (window.Ret_Button(25)) {
    rotationMode = (rotationMode + 1) % 6;
    slicing.Set_Gcode_Rotation_Order(static_cast<Gcode_Rotation_Order>(rotationMode));
}

// === Extra Toggles ===
if (window.Ret_Button(26)) { showPath = !showPath; }
if (window.Ret_Button(27)) { showCombinedPath = !showCombinedPath; }

if (window.Ret_Button(28)) {
    flatToolpath.clear();
    std::vector<std::pair<Vector3, Vector3>> pathData;
    if (paths.Gcode_to_Path("src/output/OwO", "nc", pathData)) {
        for (size_t i = 1; i < pathData.size(); ++i) {
            flatToolpath.push_back({
                .startLinePoint = { pathData[i - 1].first, pathData[i - 1].second },
                .endLinePoint   = { pathData[i].first,     pathData[i].second },
                .type = 1
            });
        }
    }
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
