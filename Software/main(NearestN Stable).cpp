#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <raylib.h>
#include <raymath.h>
#include <algorithm>

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

const int FACE_NONE   = -1;
const int FACE_TOP    = 0;
const int FACE_BOTTOM = 1;
const int FACE_LEFT   = 2;
const int FACE_RIGHT  = 3;
const int FACE_FRONT  = 4;
const int FACE_BACK   = 5;

int GetBoxFace(Vector3 point, BoundingBox box, float epsilon = 0.001f) {
    if (fabs(point.y - box.max.y) < epsilon) return FACE_TOP;
    if (fabs(point.y - box.min.y) < epsilon) return FACE_BOTTOM;
    if (fabs(point.x - box.min.x) < epsilon) return FACE_LEFT;
    if (fabs(point.x - box.max.x) < epsilon) return FACE_RIGHT;
    if (fabs(point.z - box.max.z) < epsilon) return FACE_FRONT;
    if (fabs(point.z - box.min.z) < epsilon) return FACE_BACK;
    return FACE_NONE;
}

struct AABBEndpoints {
    Vector3 entry;
    Vector3 exit;
    bool valid;
};

AABBEndpoints ComputeAABBEndpoints(const std::vector<Line>& slice, BoundingBox box, bool flip, mesh& models) {
    std::vector<Line> oriented = models.Orient_Line_Group(slice);
    std::vector<Line> rotated = oriented;
    if (flip) {
        std::vector<Line> reversed;
        for (int i = (int)rotated.size() - 1; i >= 0; --i) {
            Line l = rotated[i];
            std::swap(l.startLinePoint, l.endLinePoint);
            reversed.push_back(l);
        }
        rotated = reversed;
        for (Line& l : rotated) std::swap(l.startLinePoint, l.endLinePoint);
    }

    Point entry = rotated.front().startLinePoint;
    Point exit  = rotated.back().endLinePoint;

    Vector3 dirIn  = Vector3Negate(Vector3Normalize(entry.Normal));
    Vector3 dirOut = Vector3Negate(Vector3Normalize(exit.Normal));

    Vector3 hitIn, hitOut;
    bool okIn  = models.RayIntersectsAABB(entry.Position, dirIn, box, &hitIn);
    bool okOut = models.RayIntersectsAABB(exit.Position, dirOut, box, &hitOut);

    if (okIn && okOut) {
        return { hitIn, hitOut, true };
    }
    return { {}, {}, false };
}

struct SliceEntry {
    int sliceIndex;
    bool flipped;
};

float CostBetween(const std::vector<SliceEntry>& seq, int a, int b,
                  const std::vector<std::vector<Line>>& slices,
                  BoundingBox box, mesh& models)
{
    if (a < 0 || b >= (int)seq.size()) return 0;

    const auto& sliceA = slices[seq[a].sliceIndex];
    const auto& sliceB = slices[seq[b].sliceIndex];

    AABBEndpoints epA = ComputeAABBEndpoints(sliceA, box, seq[a].flipped, models);
    AABBEndpoints epB = ComputeAABBEndpoints(sliceB, box, seq[b].flipped, models);

    if (!epA.valid || !epB.valid)
        return FLT_MAX;

    return Vector3Distance(epA.exit, epB.entry);
}

std::vector<Line> GenerateUnifiedPath(const std::vector<std::vector<Line>>& slices, mesh& models, BoundingBox bbox){
    std::vector<Line> unified;
    if (slices.empty()) return unified;

    std::vector<std::vector<Line>> adjustedSlices = slices;
    std::vector<Point> rayEntries, rayExits;

    for (auto& slice : adjustedSlices) {
        slice = models.Orient_Line_Group(slice);
        Point entry = slice.front().startLinePoint;
        Point exit  = slice.back().endLinePoint;

        Vector3 dirIn  = Vector3Negate(Vector3Normalize(entry.Normal));
        Vector3 dirOut = Vector3Negate(Vector3Normalize(exit.Normal));
        Vector3 hitIn, hitOut;

        if (!models.RayIntersectsAABB(entry.Position, dirIn, bbox, &hitIn) ||
            !models.RayIntersectsAABB(exit.Position, dirOut, bbox, &hitOut)) {
            continue;
        }

        rayEntries.push_back({ hitIn, entry.Normal });
        rayExits.push_back({ hitOut, exit.Normal });
    }

    for (size_t i = 0; i < adjustedSlices.size(); ++i) {
        std::vector<Line>& slice = adjustedSlices[i];

        if (i > 0) {
            float distToStart = Vector3Distance(rayExits[i - 1].Position, rayEntries[i].Position);
            float distToEnd   = Vector3Distance(rayExits[i - 1].Position, rayExits[i].Position);

            if (distToEnd < distToStart) {
                std::reverse(slice.begin(), slice.end());
                for (Line& l : slice) std::swap(l.startLinePoint, l.endLinePoint);
                std::swap(rayEntries[i], rayExits[i]);
            }

            unified.push_back(Line{
                .startLinePoint = rayExits[i - 1],
                .endLinePoint   = rayEntries[i],
                .type = 2
            });
        }

        unified.push_back(Line{
            .startLinePoint = rayEntries[i],
            .endLinePoint   = slice.front().startLinePoint,
            .type = 2
        });

        for (const Line& l : slice)
            unified.push_back(l);

        unified.push_back(Line{
            .startLinePoint = slice.back().endLinePoint,
            .endLinePoint   = rayExits[i],
            .type = 2
        });
    }

    return unified;
}

int main(){
    app window;
    window.Initialise_Window(1000, 1500, 60, "Borb CAM Slicer", "src/Logo-Light.png");
    Camera camera = window.Initialise_Camera((Vector3){20.0f, 20.0f, 20.0f}, (Vector3){0.0f, 8.0f, 0.0f}, (Vector3){0.0f, 1.6f, 0.0f}, 45.0f, CAMERA_PERSPECTIVE);
    Shader shader = window.Initialise_Shader();
    window.Initialise_Lights(shader);
    window.Create_File("src/Debug", "txt");

    // UI buttons...
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
    window.Add_Button(17, 20, 60,  75, 500, "Run");
    window.Add_Button(18, 20, 120, 75, 525, "Hide Model");
    window.Add_Button(19, 20, 120, 75, 550, "Enable Rays");
    window.Add_Button(20, 20, 100, 75, 575, "Slice to G-code");
    window.Add_Button(21, 20, 100, 75, 600, "Slice A+");
    window.Add_Button(22, 20, 100, 75, 625, "Slice A-");
    window.Add_Button(23, 20, 100, 75, 650, "Slice B+");
    window.Add_Button(24, 20, 100, 75, 675, "Slice B-");
    window.Add_Button(25, 20, 180, 75, 700, "Rot: ABC");
    window.Add_Button(26, 20, 160, 75, 725, "Hide lastPoint");
    window.Add_Button(27, 20, 160, 75, 750, "Randomise Points");
    window.Add_Button(28, 20, 160, 75, 775, "Apply TSP");

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
    bool showLastPoint = true;
    std::vector<Line> finalRayLines;
    std::vector<Point> sliceLastPoints;
    std::vector<Line> combinedPath;
    auto lastRunTime = std::chrono::steady_clock::now() - std::chrono::seconds(2);

    float x = 0, xk = 0, y = 0, yk = 0, z = 0, zk = 0;
    float s = 1, sk = 1, a = 0, ak = 0, b = 0, bk = 0, c = 0, ck = 0;
    float slice_size = 0.1f;
    float sliceAngleA = 0.0f;
    float sliceAngleB = 0.0f;
    int rotationMode = 0;

    BoundingBox cullBox = { Vector3{-2, -4, -2}, Vector3{2, 0, 2} };
    std::vector<Line> intersectionList;
    std::vector<Line> tspLines;

    while (!WindowShouldClose()) {
        if (FileExists(modelPath)) {
            std::time_t modTime = getLastWriteTime(modelPath);
            if (!modelLoaded || modTime != lastModifiedTime) {
                if (modelLoaded) models.Rem_Model(modelID);
                models.Add_Model(modelID, modelPath);
                models.Sha_Model(modelID, shader);
                modelLoaded = true;
                lastModifiedTime = modTime;
            }
        } else if (modelLoaded) {
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

        if(window.Ret_Button(25)) {
            rotationMode = (rotationMode + 1) % 6;
            const char* labels[] = { "Rot: ABC", "Rot: CAB", "Rot: BCA", "Rot: CBA", "Rot: ACB", "Rot: BAC" };
            window.Rem_Button(25);
            window.Add_Button(25, 20, 180, 75, 900, labels[rotationMode]);
        }

        if(window.Ret_Button(26)) {
            showLastPoint = !showLastPoint;
            window.Rem_Button(26);
            window.Add_Button(26, 20, 160, 75, 950, showLastPoint ? "Hide lastPoint" : "Show lastPoint");
        }

        if(window.Ret_Button(27)) {
            sliceLastPoints.clear();
            float i = -4;
            Model modelCopy = models.Ret_Model(modelID);
            while (i <= 4) {
                Vector3 coefficients = slicing.rotation_coefficient(sliceAngleA, sliceAngleB);
                std::vector<Line> result = models.Intersect_Model(modelCopy, (Vector4){coefficients.x, coefficients.y, coefficients.z, i});
                if (!result.empty()) {
                    int randomStart = GetRandomValue(0, (int)result.size() - 1);
                    sliceLastPoints.push_back(models.lastPoint(result, randomStart, true));
                }
                i += slice_size;
            }
        }

        if(window.Ret_Button(28)) {
            tspLines = slicing.Generate_TSP_Lines_FromPoints(sliceLastPoints);
        }

        if(window.Ret_Button(17)) {
            auto now = std::chrono::steady_clock::now();
            if(std::chrono::duration_cast<std::chrono::seconds>(now - lastRunTime).count() >= 1) {
                runSlice = true;
                lastRunTime = now;
            }
        }

        if(window.Ret_Button(18)) {
            modelVisible = !modelVisible;
            window.Rem_Button(18);
            window.Add_Button(18, 20, 120, 75, 550, modelVisible ? "Hide Model" : "Show Model");
        }

        if(window.Ret_Button(19)) {
            generateRays = !generateRays;
            window.Rem_Button(19);
            window.Add_Button(19, 20, 120, 75, 600, generateRays ? "Disable Rays" : "Enable Rays");
        }

        if(modelLoaded) {
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

if (runSlice) {
    intersectionList.clear();
    sliceLastPoints.clear();

    std::vector<std::vector<Line>> allSlices;
    BoundingBox bbox = GetMeshBoundingBox(currentmodel.model.meshes[0]);

    Vector3 coefficients = slicing.rotation_coefficient(sliceAngleA, sliceAngleB);
    for (float i = -4; i <= 4; i += slice_size) {
        std::vector<Line> result = models.Intersect_Model(currentmodel.model, (Vector4){coefficients.x, coefficients.y, coefficients.z, i});
        std::vector<Line> culled = models.Flatten_Culled_Lines(cullBox, result, false);
        if (!culled.empty()) allSlices.push_back(culled);
    }

    std::vector<std::vector<Line>> orderedSlices;
    std::vector<Point> rayEntries, rayExits;

    // Compute entry/exit AABB rays for each slice, and optionally flip
    for (size_t i = 0; i < allSlices.size(); ++i) {
        std::vector<Line> slice = models.Orient_Line_Group(allSlices[i]);
        Point entry = slice.front().startLinePoint;
        Point exit  = slice.back().endLinePoint;

        Vector3 entryDir = Vector3Negate(Vector3Normalize(entry.Normal));
        Vector3 exitDir  = Vector3Negate(Vector3Normalize(exit.Normal));
        Vector3 hitEntry, hitExit;

        bool okIn = models.RayIntersectsAABB(entry.Position, entryDir, bbox, &hitEntry);
        bool okOut = models.RayIntersectsAABB(exit.Position, exitDir, bbox, &hitExit);

        if (!okIn || !okOut) continue;

        rayEntries.push_back({ hitEntry, entry.Normal });
        rayExits.push_back({ hitExit, exit.Normal });

        orderedSlices.push_back(slice);
    }

    for (size_t i = 0; i < orderedSlices.size(); ++i) {
        const std::vector<Line>& slice = orderedSlices[i];
        const Point& entry = rayEntries[i];
        const Point& exit = rayExits[i];

        if (i > 0) {
            const Point& prevExit = rayExits[i - 1];

            float distToStart = Vector3Distance(prevExit.Position, rayEntries[i].Position);
            float distToEnd   = Vector3Distance(prevExit.Position, rayExits[i].Position);

            bool shouldFlip = (distToEnd < distToStart);

            std::vector<Line> adjusted = slice;
            if (shouldFlip) {
                std::reverse(adjusted.begin(), adjusted.end());
                for (Line& l : adjusted) std::swap(l.startLinePoint, l.endLinePoint);
                std::swap(rayEntries[i], rayExits[i]);
            }

            intersectionList.push_back(Line{
                .startLinePoint = prevExit,
                .endLinePoint = rayEntries[i],
                .type = 2
            });

            orderedSlices[i] = adjusted;
        }

        intersectionList.push_back(Line{
            .startLinePoint = rayEntries[i],
            .endLinePoint = orderedSlices[i].front().startLinePoint,
            .type = 2
        });

        for (const Line& l : orderedSlices[i])
            intersectionList.push_back(l);

        intersectionList.push_back(Line{
            .startLinePoint = orderedSlices[i].back().endLinePoint,
            .endLinePoint = rayExits[i],
            .type = 2
        });

        sliceLastPoints.push_back(orderedSlices[i].front().startLinePoint);
        sliceLastPoints.push_back(orderedSlices[i].back().endLinePoint);
    }

    combinedPath = GenerateUnifiedPath(orderedSlices, models, bbox);

    runSlice = false;
}

}

BeginMode3D(camera);

if (modelVisible) models.Run_Models();

if (!combinedPath.empty()) {
    float totalLines = (float)combinedPath.size();

    for (size_t i = 0; i < combinedPath.size(); ++i) {
        const Line& line = combinedPath[i];

        // Elevate the line by +1 unit in Y
        Vector3 start = line.startLinePoint.Position;
        Vector3 end   = line.endLinePoint.Position;
        start.y += 1.0f;
        end.y   += 1.0f;

        // Gradient: red (start) → blue (end)
        float t = (float)i / (totalLines - 1);
        Color color = {
            (unsigned char)(255 * (1.0f - t)), // Red fades
            0,
            (unsigned char)(255 * t),         // Blue grows
            255
        };

        DrawLine3D(start, end, color);
    }
}

for (const Line& line : intersectionList) {
    if (line.type == 0 || line.type == 1) {
        DrawLine3D(line.startLinePoint.Position, line.endLinePoint.Position, BLUE);
    } else if (line.type == 2) {
        Vector3 dir = Vector3Normalize(Vector3Subtract(line.endLinePoint.Position, line.startLinePoint.Position));
        Vector3 normal = Vector3Normalize(line.startLinePoint.Normal);
        float alignment = Vector3DotProduct(dir, normal);
        Color color = (alignment >= 0.0f) ? GREEN : RED;
        DrawLine3D(line.startLinePoint.Position, line.endLinePoint.Position, color);
    }
}

for (size_t i = 0; i + 1 < sliceLastPoints.size(); i += 2) {
    const Point& entry = sliceLastPoints[i];
    const Point& exit  = sliceLastPoints[i + 1];

    Color entryColor = GREEN;
    Color exitColor  = RED;

    for (const Line& line : intersectionList) {
        if (line.type != 2) continue;

        // Match entry point to a ray's end
        if (Vector3Distance(entry.Position, line.endLinePoint.Position) < 0.0001f) {
            Vector3 dir = Vector3Normalize(Vector3Subtract(line.endLinePoint.Position, line.startLinePoint.Position));
            Vector3 normal = Vector3Normalize(line.startLinePoint.Normal);
            entryColor = (Vector3DotProduct(dir, normal) >= 0.0f) ? GREEN : RED;
        }

        // Match exit point to a ray's start
        if (Vector3Distance(exit.Position, line.startLinePoint.Position) < 0.0001f) {
            Vector3 dir = Vector3Normalize(Vector3Subtract(line.endLinePoint.Position, line.startLinePoint.Position));
            Vector3 normal = Vector3Normalize(line.startLinePoint.Normal);
            exitColor = (Vector3DotProduct(dir, normal) >= 0.0f) ? GREEN : RED;
        }
    }

    DrawCube(entry.Position, 0.05f, 0.05f, 0.05f, entryColor);
    DrawCube(exit.Position,  0.05f, 0.05f, 0.05f, exitColor);
}

DrawCube((Vector3){0, -2, 0}, 4, 4, 4, (Color){255, 0, 0, 51});
DrawBoundingBox(cullBox, RED);

EndMode3D();



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
