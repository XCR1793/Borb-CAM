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

float TotalTSPDistance(const std::vector<Point>& points) {
    float total = 0.0f;
    for (size_t i = 1; i < points.size(); ++i) {
        total += Vector3Distance(points[i - 1].Position, points[i].Position);
    }
    return total;
}

void TwoOptOptimizePoints(std::vector<Point>& path) {
    bool improved = true;
    while (improved) {
        improved = false;
        for (size_t i = 1; i < path.size() - 2; ++i) {
            for (size_t k = i + 1; k < path.size() - 1; ++k) {
                std::vector<Point> newPath = path;

                // Manual reverse between i and k
                int a = i, b = k;
                while (a < b) {
                    std::swap(newPath[a], newPath[b]);
                    a++;
                    b--;
                }

                if (TotalTSPDistance(newPath) < TotalTSPDistance(path)) {
                    path = newPath;
                    improved = true;
                }
            }
        }
    }
}

int main(){
    app window;
    window.Initialise_Window(1000, 1500, 60, "Borb CAM Slicer", "src/Logo-Light.png");
    Camera camera = window.Initialise_Camera((Vector3){20.0f, 20.0f, 20.0f}, (Vector3){0.0f, 8.0f, 0.0f}, (Vector3){0.0f, 1.6f, 0.0f}, 45.0f, CAMERA_PERSPECTIVE);
    Shader shader = window.Initialise_Shader();
    window.Initialise_Lights(shader);
    window.Create_File("src/Debug", "txt");

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
    auto lastRunTime = std::chrono::steady_clock::now() - std::chrono::seconds(2);

    float x = 0, xk = 0, y = 0, yk = 0, z = 0, zk = 0;
    float s = 1, sk = 1, a = 0, ak = 0, b = 0, bk = 0, c = 0, ck = 0;
    float slice_size = 0.1f;
    float sliceAngleA = 0.0f;
    float sliceAngleB = 0.0f;
    int rotationMode = 0;

    // Persistent cull box (used for intersection filtering, ray hit detection, etc.)
    BoundingBox cullBox = { Vector3{-2, -4, -2}, Vector3{2, 0, 2} };

    std::vector<Line> intersectionList;

    std::vector<Line> tspLines;

    
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

        if(window.Ret_Button(26)){
            showLastPoint = !showLastPoint;
            window.Rem_Button(26);
            window.Add_Button(26, 20, 160, 75, 950, showLastPoint ? "Hide lastPoint" : "Show lastPoint");
        }

        if(window.Ret_Button(27)){
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
        
            // ➕ Optimize the random starting points using 2-opt
            TwoOptOptimizePoints(sliceLastPoints);
        }


        if(window.Ret_Button(28)){
            // ➕ Optimize point order before generating TSP lines
            TwoOptOptimizePoints(sliceLastPoints);
            tspLines = slicing.Generate_TSP_Lines_FromPoints(sliceLastPoints);
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
    std::vector<float> sliceHeights;

    BoundingBox cullBox = { Vector3{-2, -4, -2}, Vector3{2, 0, 2} };

    for (float i = -4; i <= 4; i += slice_size) {
        Vector3 coefficients = slicing.rotation_coefficient(sliceAngleA, sliceAngleB);
        std::vector<Line> result = models.Intersect_Model(currentmodel.model, (Vector4){coefficients.x, coefficients.y, coefficients.z, i});
        std::vector<Line> culled = models.Cull_Lines_ByBox(cullBox, result);

        if (!culled.empty()) {
            if (!intersectionList.empty()) {
                auto last = intersectionList.back();
                intersectionList.push_back((Line){
                    .startLinePoint = last.endLinePoint,
                    .endLinePoint   = culled.front().startLinePoint,
                    .type = 2
                });
            }

            allSlices.push_back(culled);
            sliceHeights.push_back(i);
            intersectionList.insert(intersectionList.end(), culled.begin(), culled.end());
        }
    }

    std::vector<Line> rayLines;
    for (const Line& line : intersectionList) {
        if (line.type != 1) continue;

        int meshIndex = line.meshNo;
        if (meshIndex < 0 || meshIndex >= currentmodel.model.meshCount) continue;

        BoundingBox bbox = GetMeshBoundingBox(currentmodel.model.meshes[meshIndex]);
        Vector3 origin = line.startLinePoint.Position;
        Vector3 dir = Vector3Negate(Vector3Normalize(line.startLinePoint.Normal));
        if (Vector3Length(dir) < 0.0001f) continue;

        Vector3 hit;
        if (models.RayIntersectsAABB(origin, dir, bbox, &hit)) {
            rayLines.push_back((Line){
                .startLinePoint = { origin, line.startLinePoint.Normal },
                .endLinePoint   = { hit,    line.startLinePoint.Normal },
                .type           = 1,
                .meshNo         = meshIndex,
                .islandNo       = line.islandNo
            });
        }
    }

    std::vector<Line> processedRayLines;
    for (const Line& ray : rayLines) {
        std::vector<Line> raySet = { ray };
        for (int j = 0; j < currentmodel.model.meshCount; j++) {
            if (j == ray.meshNo) continue;
            BoundingBox otherBox = GetMeshBoundingBox(currentmodel.model.meshes[j]);
            raySet = models.Cull_Lines_ByBox(otherBox, raySet);
            if (raySet.empty()) break;
        }
        processedRayLines.insert(processedRayLines.end(), raySet.begin(), raySet.end());
    }

    finalRayLines = models.Cull_Lines_ByBox(cullBox, processedRayLines);

    // ─────────────────────────────────────────────────────────────────────────────
    // ░░ 2-OPT SEQUENCE OPTIMIZATION ░░
    // ─────────────────────────────────────────────────────────────────────────────

    struct SliceEntry {
        int sliceIndex;
        int startIndex;
    };

    std::vector<SliceEntry> sequence;
    for (int i = 0; i < (int)allSlices.size(); ++i)
        sequence.push_back({ i, 0 });

    BoundingBox bbox = GetMeshBoundingBox(currentmodel.model.meshes[0]);
    std::vector<Point> pointCache(sequence.size());
    for (size_t i = 0; i < sequence.size(); ++i) {
        pointCache[i] = models.lastPoint(allSlices[sequence[i].sliceIndex], 0, true);
    }

    auto CostBetween = [&](size_t idxA, size_t idxB) -> float {
        const Point& A = pointCache[idxA];
        const Point& B = pointCache[idxB];

        float dPos = Vector3Distance(A.Position, B.Position);

        Vector3 dirA = Vector3Negate(Vector3Normalize(A.Normal));
        Vector3 dirB = Vector3Negate(Vector3Normalize(B.Normal));

        Vector3 hitA, hitB;
        float dHit = (models.RayIntersectsAABB(A.Position, dirA, bbox, &hitA) &&
                      models.RayIntersectsAABB(B.Position, dirB, bbox, &hitB))
                    ? Vector3Distance(hitA, hitB)
                    : dPos * 2.0f;

        return dPos * 0.7f + dHit * 0.3f;
    };

    bool improved = true;
    int iter = 0;
    const int maxIter = 100;
    const float minDelta = 0.01f;

    while (improved && iter++ < maxIter) {
        improved = false;

        for (size_t i = 1; i < sequence.size() - 2; ++i) {
            for (size_t k = i + 2; k < sequence.size() - 1; ++k) {
                float before = CostBetween(i - 1, i) + CostBetween(k, k + 1);

                std::vector<SliceEntry> temp = sequence;
                int a = i, b = k;
                while (a < b) {
                    std::swap(temp[a], temp[b]);
                    a++;
                    b--;
                }

                for (size_t t = i; t <= k; ++t) {
                    pointCache[t] = models.lastPoint(allSlices[temp[t].sliceIndex], 0, true);
                }

                float after = CostBetween(i - 1, i) + CostBetween(k, k + 1);

                if (before - after > minDelta) {
                    sequence = temp;
                    improved = true;
                    goto restart_2opt;
                }
            }
        }

    restart_2opt:
        continue;
    }

    // ─────────────────────────────────────────────────────────────────────────────
    // ░░ Final best start point per optimized slice ░░
    // ─────────────────────────────────────────────────────────────────────────────
    sliceLastPoints.clear();
    Point prevPoint;

    for (size_t i = 0; i < sequence.size(); ++i) {
        const auto& slice = allSlices[sequence[i].sliceIndex];

        int bestStart = 0;
        float bestScore = FLT_MAX;

        for (int j = 0; j < (int)slice.size(); ++j) {
            Point candidate = models.lastPoint(slice, j, true);
            float distPos = (i > 0) ? Vector3Distance(prevPoint.Position, candidate.Position) : 0.0f;
            float distHit = 0.0f;

            if (i > 0) {
                Vector3 dirA = Vector3Negate(Vector3Normalize(prevPoint.Normal));
                Vector3 dirB = Vector3Negate(Vector3Normalize(candidate.Normal));

                Vector3 hitA, hitB;
                if (models.RayIntersectsAABB(prevPoint.Position, dirA, bbox, &hitA) &&
                    models.RayIntersectsAABB(candidate.Position, dirB, bbox, &hitB)) {
                    distHit = Vector3Distance(hitA, hitB);
                } else {
                    distHit = distPos * 2.0f;
                }
            }

            float score = distPos * 0.7f + distHit * 0.3f;
            if (score < bestScore) {
                bestScore = score;
                bestStart = j;
            }
        }

        Point best = models.lastPoint(slice, bestStart, true);
        sliceLastPoints.push_back(best);
        prevPoint = best;
    }

    TwoOptOptimizePoints(sliceLastPoints);
    runSlice = false;
}



            BeginMode3D(camera);
            if(modelVisible) models.Run_Models();

            for(const Line& line : intersectionList){
                if(line.type == 1){
                    DrawLine3D(line.startLinePoint.Position, line.endLinePoint.Position, BLUE);
                }
            }

            if (showLastPoint) {
                for (const Point& p : sliceLastPoints) {
                    DrawSphere(p.Position, 0.08f, GREEN);
                }
            }

            // for(const Line& line : tspLines){
            //     DrawLine3D(line.startLinePoint.Position, line.endLinePoint.Position, ORANGE);
            // }

        BoundingBox bbox = GetMeshBoundingBox(models.Ret_Model(modelID).meshes[0]);

        std::vector<Vector3> bboxHitPoints;

        // Snap points cleanly to bounding box faces to prevent precision errors
        auto SnapToBoxFace = [](Vector3 p, BoundingBox box, float eps = 0.01f) -> Vector3 {
            if (fabs(p.x - box.min.x) < eps) p.x = box.min.x;
            else if (fabs(p.x - box.max.x) < eps) p.x = box.max.x;
        
            if (fabs(p.y - box.min.y) < eps) p.y = box.min.y;
            else if (fabs(p.y - box.max.y) < eps) p.y = box.max.y;
        
            if (fabs(p.z - box.min.z) < eps) p.z = box.min.z;
            else if (fabs(p.z - box.max.z) < eps) p.z = box.max.z;
        
            return p;
        };

        auto IsPointNearFaceButInsideBox = [](Vector3 p, BoundingBox box, float epsilon = 0.005f) -> bool {
            bool insideX = p.x > box.min.x + epsilon && p.x < box.max.x - epsilon;
            bool insideY = p.y > box.min.y + epsilon && p.y < box.max.y - epsilon;
            bool insideZ = p.z > box.min.z + epsilon && p.z < box.max.z - epsilon;
        
            // Close to a face (within epsilon), but inside along at least one axis
            bool nearX = fabs(p.x - box.min.x) < epsilon || fabs(p.x - box.max.x) < epsilon;
            bool nearY = fabs(p.y - box.min.y) < epsilon || fabs(p.y - box.max.y) < epsilon;
            bool nearZ = fabs(p.z - box.min.z) < epsilon || fabs(p.z - box.max.z) < epsilon;
        
            return (insideX && nearY && nearZ) || (nearX && insideY && nearZ) || (nearX && nearY && insideZ);
        };

        // Cast rays to box and store hit points (snapped + fixed)
        for (const Point& tspPoint : sliceLastPoints) {
            Vector3 dir = Vector3Negate(Vector3Normalize(tspPoint.Normal));
            Vector3 hit;
        
            if (models.RayIntersectsAABB(tspPoint.Position, dir, bbox, &hit)) {
                hit = SnapToBoxFace(hit, bbox);
            
                if (IsPointNearFaceButInsideBox(hit, bbox)) {
                    Vector3 center = Vector3Lerp(bbox.min, bbox.max, 0.5f);
                    Vector3 pushDir = Vector3Normalize(Vector3Subtract(hit, center));
                    hit = Vector3Add(hit, Vector3Scale(pushDir, 0.01f));  // Push slightly out
                    hit = SnapToBoxFace(hit, bbox);                      // Re-snap to face
                
                    DrawSphere(hit, 0.03f, RED);  // Optional: mark problem point
                }
            
                DrawLine3D(tspPoint.Position, hit, RED);
                bboxHitPoints.push_back(hit);
            }
        }

        // Draw surface-adhering connection paths
        for (size_t i = 0; i + 1 < bboxHitPoints.size(); ++i) {
            Vector3 p1 = SnapToBoxFace(bboxHitPoints[i], bbox);
            Vector3 p2 = SnapToBoxFace(bboxHitPoints[i + 1], bbox);
        
            int face1 = GetBoxFace(p1, bbox);
            int face2 = GetBoxFace(p2, bbox);
        
            if (face1 == face2 && face1 != FACE_NONE) {
                DrawLine3D(p1, p2, DARKGREEN);
            } else {
                bool sameX = fabs(p1.x - p2.x) < 0.001f;
                bool sameY = fabs(p1.y - p2.y) < 0.001f;
                bool sameZ = fabs(p1.z - p2.z) < 0.001f;
            
                if (sameX) {
                    Vector3 mid = SnapToBoxFace({ p1.x, p2.y, p1.z }, bbox);
                    DrawLine3D(p1, mid, DARKGREEN);
                    DrawLine3D(mid, p2, DARKGREEN);
                } else if (sameY) {
                    Vector3 mid = SnapToBoxFace({ p2.x, p1.y, p1.z }, bbox);
                    DrawLine3D(p1, mid, DARKGREEN);
                    DrawLine3D(mid, p2, DARKGREEN);
                } else if (sameZ) {
                    Vector3 mid = SnapToBoxFace({ p2.x, p1.y, p1.z }, bbox);
                    DrawLine3D(p1, mid, DARKGREEN);
                    DrawLine3D(mid, p2, DARKGREEN);
                } else {
                    Vector3 corner = SnapToBoxFace({ p2.x, p1.y, p2.z }, bbox);
                    DrawLine3D(p1, corner, DARKGREEN);
                    DrawLine3D(corner, p2, DARKGREEN);
                }
            }
        }

// if (generateRays) {
//     for (const Line& line : finalRayLines) {
//         DrawSphere(line.startLinePoint.Position, 0.01f, BLUE);
//         DrawSphere(line.endLinePoint.Position, 0.01f, RED);
//         DrawLine3D(line.startLinePoint.Position, line.endLinePoint.Position, GREEN);
//     }
// }


        DrawCube((Vector3){0, -2, 0}, 4, 4, 4, (Color){255, 0, 0, 51});
        DrawBoundingBox(cullBox, RED);


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
