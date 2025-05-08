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
    auto lastRunTime = std::chrono::steady_clock::now() - std::chrono::seconds(2);

    float x = 0, xk = 0, y = 0, yk = 0, z = 0, zk = 0;
    float s = 1, sk = 1, a = 0, ak = 0, b = 0, bk = 0, c = 0, ck = 0;
    float slice_size = 0.1f;
    float sliceAngleA = 0.0f;
    float sliceAngleB = 0.0f;
    int rotationMode = 0;

    std::vector<Line> intersectionList;

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

        if(window.Ret_Button(20)) {
            std::vector<std::pair<Vector3, Vector3>> pathPositions;
            for (const auto& line : finalRayLines) {
                if (Vector3Length(line.startLinePoint.Normal) < 0.0001f) continue;
                Vector3 position = line.startLinePoint.Position;
                Vector3 rotation = models.NormalToRotation(line.startLinePoint.Normal);
                float A = rotation.x, B = rotation.y, C = rotation.z;
                Vector3 reordered;

                switch(rotationMode){
                    case 0: reordered = { A, B, C }; break;
                    case 1: reordered = { C, A, B }; break;
                    case 2: reordered = { B, C, A }; break;
                    case 3: reordered = { C, B, A }; break;
                    case 4: reordered = { A, C, B }; break;
                    case 5: reordered = { B, A, C }; break;
                }

                pathPositions.push_back(std::make_pair(position, reordered));
            }

            if (!pathPositions.empty()) {
                paths.Clear_File();
                paths.Reset_N();
                paths.Path_to_Gcode1(pathPositions);
                TraceLog(LOG_INFO, "G-code written with %zu points", pathPositions.size());
            } else {
                TraceLog(LOG_WARNING, "No valid G-code points found.");
            }
        }

        // Continue with your modelLoaded block (Apply_Transformations, slicing, drawing, etc.)
        // unchanged from your working implementation.
        if(modelLoaded){
            multimodel currentmodel = {
                .id = modelID,
                .model = models.Ret_Model(modelID)
            };

            models.Scale_Model(currentmodel, sk);
            models.Rotate_Model(currentmodel, (Vector3){ak, bk, ck});
            models.Position_Model(currentmodel, (Vector3){xk, yk, zk});
            models.Apply_Transformations(currentmodel);
            models.Reu_Model(modelID, currentmodel.model);

            xk = yk = zk = ak = bk = ck = 0;
            sk = 1;

            if(runSlice){
                intersectionList.clear();
                for(float i = -4; i <= 4; i += slice_size){
                    Vector3 coefficients = slicing.rotation_coefficient(sliceAngleA, sliceAngleB);
                    std::vector<Line> result = models.Intersect_Model(currentmodel.model, (Vector4){coefficients.x, coefficients.y, coefficients.z, i});
                    if(!result.empty() && !intersectionList.empty()){
                        auto last = intersectionList.back();
                        intersectionList.push_back((Line){
                            .startLinePoint = last.endLinePoint,
                            .endLinePoint = result.front().startLinePoint,
                            .type = 2
                        });
                    }
                    intersectionList.insert(intersectionList.end(), result.begin(), result.end());
                }

                finalRayLines.clear();
                BoundingBox cullBox = { (Vector3){ -2, -4, -2 }, (Vector3){ 2, 0, 2 } };
                intersectionList = models.Cull_Lines_ByBox(cullBox, intersectionList);
                std::vector<Line> rayLines;

                for(Line& line : intersectionList){
                    if(line.type != 1) continue;
                    int meshIndex = line.meshNo;
                    if(meshIndex < 0 || meshIndex >= currentmodel.model.meshCount) continue;
                    Vector3 origin = line.startLinePoint.Position;
                    Vector3 dir = Vector3Negate(Vector3Normalize(line.startLinePoint.Normal));
                    if(Vector3Length(dir) < 0.0001f) continue;
                    Vector3 hit;
                    if(models.RayIntersectsAABB(origin, dir, GetMeshBoundingBox(currentmodel.model.meshes[meshIndex]), &hit)){
                        rayLines.push_back((Line){
                            .startLinePoint = { origin, line.startLinePoint.Normal },
                            .endLinePoint   = { hit,    line.startLinePoint.Normal },
                            .type           = 1,
                            .meshNo         = meshIndex,
                            .islandNo       = line.islandNo
                        });
                    }
                }

                std::vector<Line> processed;
                for(Line& ray : rayLines){
                    std::vector<Line> set = { ray };
                    for(int j = 0; j < currentmodel.model.meshCount; j++){
                        if(j == ray.meshNo) continue;
                        BoundingBox otherBox = GetMeshBoundingBox(currentmodel.model.meshes[j]);
                        set = models.Cull_Lines_ByBox(otherBox, set);
                        if(set.empty()) break;
                    }
                    for(Line& trimmed : set){
                        processed.push_back(trimmed);
                    }
                }

                finalRayLines = models.Cull_Lines_ByBox(cullBox, processed);
                runSlice = false;
            }

            BeginMode3D(camera);
            if(modelVisible) models.Run_Models();

            for(const Line& line : intersectionList){
                if(line.type == 1){
                    DrawLine3D(line.startLinePoint.Position, line.endLinePoint.Position, BLUE);
                }
            }

            if(generateRays){
                for(const Line& line : finalRayLines){
                    DrawSphere(line.startLinePoint.Position, 0.01f, BLUE);
                    DrawSphere(line.endLinePoint.Position, 0.01f, RED);
                    DrawLine3D(line.startLinePoint.Position, line.endLinePoint.Position, GREEN);
                }
                DrawCube((Vector3){ 0, -2, 0 }, 4, 4, 4, (Color){ 255, 0, 0, 51 });
                DrawBoundingBox((BoundingBox){ (Vector3){ -2, -4, -2 }, (Vector3){ 2, 0, 2 } }, RED);
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
