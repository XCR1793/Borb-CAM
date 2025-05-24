#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <vector>
#include <string>
#include <sys/stat.h>
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

bool FileExists(const std::string& path){
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

std::time_t getLastWriteTime(const std::string& path){
    struct stat result;
    if(stat(path.c_str(), &result) == 0){
        return result.st_mtime;
    }
    return 0;
}

Color HashColor(Vector3 pos) {
    // Simple hash using position components
    unsigned int seed = (unsigned int)(pos.x * 73856093) ^
                        (unsigned int)(pos.y * 19349663) ^
                        (unsigned int)(pos.z * 83492791);

    // Basic pseudo-random RGB from seed
    unsigned char r = 50 + (seed % 206);
    unsigned char g = 50 + ((seed / 2) % 206);
    unsigned char b = 50 + ((seed / 3) % 206);

    return (Color){ r, g, b, 255 };
}

int main(){
    app window;
    window.Initialise_Window(1000, 1500, 60, "Borb CAM Slicer", "src/Logo-Light.png");
    Camera camera = window.Initialise_Camera((Vector3){20.0f, 20.0f, 20.0f}, (Vector3){0.0f, 8.0f, 0.0f}, (Vector3){0.0f, 1.6f, 0.0f}, 45.0f, CAMERA_PERSPECTIVE);
    Shader shader = window.Initialise_Shader();
    window.Initialise_Lights(shader);
    window.Create_File("src/Debug", "txt");

    for(int i = 1; i <= 16; ++i){
        int row = (i - 1) / 2, col = (i - 1) % 2;
        const char* labels[] = { "X+", "X-", "Y+", "Y-", "Z+", "Z-", "S+", "S-", "A+", "A-", "B+", "B-", "C+", "C-", "O+", "O-" };
        window.Add_Button(i, 20, 60, 30 + 50 * row, 100 + 50 * col, labels[i - 1]);
    }
    window.Add_Button(17, 20, 60, 75, 500, "Run");
    window.Add_Button(18, 20, 120, 75, 550, "Hide Model");
    window.Add_Button(19, 20, 120, 75, 600, "Enable Rays");
    window.Add_Button(20, 20, 100, 75, 650, "Slice to G-code");
    window.Add_Button(21, 20, 100, 75, 700, "Slice A+");
    window.Add_Button(22, 20, 100, 75, 750, "Slice A-");
    window.Add_Button(23, 20, 100, 75, 800, "Slice B+");
    window.Add_Button(24, 20, 100, 75, 850, "Slice B-");
    window.Add_Button(25, 20, 180, 75, 900, "Rot: ABC");

    mesh models;
    path paths;
    slice slicing;
    paths.Create_File("src/OwO", "nc");

    const char* modelPath = "src/model.obj";
    int modelID = 1;
    bool modelLoaded = false;
    std::time_t lastModifiedTime = 0;

    bool modelVisible = true;
    bool runSlice = false;
    bool generateRays = false;
    std::vector<Line> finalRayLines;
    // std::vector<Lines> sliceSegments;
    std::vector<std::vector<Lines>> allSlices;
    auto lastRunTime = std::chrono::steady_clock::now() - std::chrono::seconds(2);

    float x = 0, xk = 0, y = 0, yk = 0, z = 0, zk = 0;
    float s = 1, sk = 1, a = 0, ak = 0, b = 0, bk = 0, c = 0, ck = 0;
    float slice_size = 0.1f;
    float sliceAngleA = 0.0f;
    float sliceAngleB = 0.0f;
    int rotationMode = 0;

    while(!WindowShouldClose()){
        if(FileExists(modelPath)){
            std::time_t modTime = getLastWriteTime(modelPath);
            if(!modelLoaded || modTime != lastModifiedTime){
                if(modelLoaded) models.Rem_Model(modelID);
                models.Add_Model(modelID, modelPath);
                models.Sha_Model(modelID, shader);
                modelLoaded = true;
                lastModifiedTime = modTime;
            }
        } else if(modelLoaded){
            models.Rem_Model(modelID);
            modelLoaded = false;
        }

        window.Update_Camera(&camera);
        BeginDrawing();
        ClearBackground(DARKGRAY);

        // GUI Buttons
        if(window.Ret_Button(1)) x += 0.5f, xk = 0.5f;
        if(window.Ret_Button(2)) x -= 0.5f, xk = -0.5f;
        if(window.Ret_Button(3)) y += 0.5f, yk = 0.5f;
        if(window.Ret_Button(4)) y -= 0.5f, yk = -0.5f;
        if(window.Ret_Button(5)) z += 0.5f, zk = 0.5f;
        if(window.Ret_Button(6)) z -= 0.5f, zk = -0.5f;
        if(window.Ret_Button(7)) s += 0.5f, sk = 1.5f;
        if(window.Ret_Button(8)) s -= 0.5f, sk = 0.5f;
        if(window.Ret_Button(9)) a += PI / 10, ak = PI / 10;
        if(window.Ret_Button(10)) a -= PI / 10, ak = -PI / 10;
        if(window.Ret_Button(11)) b += PI / 10, bk = PI / 10;
        if(window.Ret_Button(12)) b -= PI / 10, bk = -PI / 10;
        if(window.Ret_Button(13)) c += 0.5f, ck = 0.5f;
        if(window.Ret_Button(14)) c -= 0.5f, ck = -0.5f;
        if(window.Ret_Button(15)) slice_size += 0.01f;
        if(window.Ret_Button(16)) slice_size -= 0.01f;
        if(slice_size <= 0) slice_size = 0.01f;

        if(window.Ret_Button(21)) sliceAngleA += PI / 2.0f;
        if(window.Ret_Button(22)) sliceAngleA -= PI / 2.0f;
        if(window.Ret_Button(23)) sliceAngleB += PI / 2.0f;
        if(window.Ret_Button(24)) sliceAngleB -= PI / 2.0f;

        if(window.Ret_Button(25)){
            rotationMode = (rotationMode + 1) % 6;
            const char* labels[] = { "Rot: ABC", "Rot: CAB", "Rot: BCA", "Rot: CBA", "Rot: ACB", "Rot: BAC" };
            window.Rem_Button(25);
            window.Add_Button(25, 20, 180, 75, 900, labels[rotationMode]);
        }

        if(window.Ret_Button(17)){
            auto now = std::chrono::steady_clock::now();
            if(std::chrono::duration_cast<std::chrono::seconds>(now - lastRunTime).count() >= 1){
                runSlice = true;
                lastRunTime = now;
            }
        }

        if(window.Ret_Button(18)){
            modelVisible = !modelVisible;
            window.Rem_Button(18);
            window.Add_Button(18, 20, 120, 75, 550, modelVisible ? "Hide Model" : "Show Model");
        }

        if(window.Ret_Button(19)){
            generateRays = !generateRays;
            window.Rem_Button(19);
            window.Add_Button(19, 20, 120, 75, 600, generateRays ? "Disable Rays" : "Enable Rays");
        }

        if(modelLoaded){
            multimodel currentmodel = { .id = modelID, .model = models.Ret_Model(modelID) };

            models.Scale_Model(currentmodel, sk);
            models.Rotate_Model(currentmodel, (Vector3){ak, bk, ck});
            models.Position_Model(currentmodel, (Vector3){xk, yk, zk});
            models.Apply_Transformations(currentmodel);
            models.Reu_Model(modelID, currentmodel.model);

            xk = yk = zk = ak = bk = ck = 0;
            sk = 1;

            // NOTE: This is the regenerated core slicing section of main()
if (runSlice) {
    allSlices.clear();

    BoundingBox cullBox = { { -2, -4, -2 }, { 2, 0, 2 } };
    Vector3 coeff = slicing.rotation_coefficient(sliceAngleA, sliceAngleB);

    std::vector<Lines> previousSlice;

    for (float i = -4.0f; i <= 4.0f; i += slice_size) {
        // 1. Intersect model at slicing plane
        std::vector<Line> rawLines = models.Intersect_Model(currentmodel.model, (Vector4){ coeff.x, coeff.y, coeff.z, i });

        // 2. Cull and group
        std::vector<Lines> groupedLines = models.Cull_Lines_ByBox(cullBox, rawLines, false);

        // 3. Optionally bridge previous to current
        if (!groupedLines.empty() && !previousSlice.empty()) {
            Line connect = {
                .startLinePoint = previousSlice.back().endPosition,
                .endLinePoint   = groupedLines.front().startPosition,
                .type           = 2
            };

            Lines bridge;
            bridge.lineList = { connect };
            bridge.startPosition = connect.startLinePoint.Position;
            bridge.endPosition   = connect.endLinePoint.Position;
            bridge.distance = models.pointToPointDistance(connect.startLinePoint, connect.endLinePoint);

            groupedLines.insert(groupedLines.begin(), bridge);
        }

        // 4. Save slice
        allSlices.push_back(groupedLines);
        previousSlice = groupedLines;
    }
}


            BeginMode3D(camera);
            if(modelVisible) models.Run_Models();

            // for (const Lines& slice : sliceSegments) {
            //     Color col = HashColor(slice.startPosition);
            //     for (const Line& line : slice.lineList) {
            //         if (line.type == 1)
            //             DrawLine3D(line.startLinePoint.Position, line.endLinePoint.Position, col);
            //     }
            // }

if (!allSlices.empty()) {
    int totalSlices = (int)allSlices.size();

    srand(42); // Optional: consistent colors across runs; remove for full randomness
    
    for (const auto& sliceGroups : allSlices) {
        for (const auto& group : sliceGroups) {
            // 🎨 Random hue per group
            float hue = (float)(rand() % 360);
            Color col = ColorFromHSV(hue, 0.8f, 0.9f);
        
            for (const Line& line : group.lineList) {
                if (line.type == 1) {
                    DrawLine3D(line.startLinePoint.Position, line.endLinePoint.Position, col);
                }
            }
        
            // Optional visual markers at the start/end of group
            DrawCube(group.startPosition, 0.02f, 0.02f, 0.02f, GREEN);
            DrawCube(group.endPosition,   0.02f, 0.02f, 0.02f, RED);
        }
    }

}


            if(generateRays){
                for(const Line& line : finalRayLines){
                    DrawCube(line.startLinePoint.Position, 0.01f, 0.01f, 0.01f, BLUE);
                    DrawCube(line.endLinePoint.Position, 0.01f, 0.01f, 0.01f, RED);
                    DrawLine3D(line.startLinePoint.Position, line.endLinePoint.Position, GREEN);
                }
                DrawCube((Vector3){ 0, -2, 0 }, 4, 4, 4, (Color){ 255, 0, 0, 51 });
                DrawBoundingBox((BoundingBox){ { -2, -4, -2 }, { 2, 0, 2 } }, RED);
            }

            EndMode3D();
        }

        DrawText(TextFormat("Slicing Angle A: %.2f deg", sliceAngleA * RAD2DEG), 300, 700, 20, RED);
        DrawText(TextFormat("Slicing Angle B: %.2f deg", sliceAngleB * RAD2DEG), 300, 730, 20, RED);

        window.Run_Buttons();
        DrawFPS(10, 10);
        EndDrawing();
    }

    models.Stop_Models();
    CloseWindow();
    return 0;
}
