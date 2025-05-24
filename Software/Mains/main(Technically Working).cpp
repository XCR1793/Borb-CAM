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

struct AABBEndpoints {
    Vector3 entry;
    Vector3 exit;
    bool valid;
};

// Simulate slice direction and compute outer box hit points
AABBEndpoints ComputeAABBEndpoints(const std::vector<Line>& slice, BoundingBox box, bool flip, mesh& models) {
    std::vector<Line> oriented = models.Orient_Line_Group(slice);

    std::vector<Line> rotated = oriented;
    if (flip) {
std::vector<Line> reversed;
for (int i = (int)rotated.size() - 1; i >= 0; --i) {
    Line l = rotated[i];
    std::swap(l.startLinePoint, l.endLinePoint);  // Flip direction
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
    bool flipped;  // ← new
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
        return FLT_MAX;  // Don't optimize across invalid connections

    return Vector3Distance(epA.exit, epB.entry);  // ← use only outer-to-outer ray distance
}

bool NextPermutation(std::vector<int>& seq) {
    int i = (int)seq.size() - 2;
    while (i >= 0 && seq[i] >= seq[i + 1]) --i;

    if (i < 0) return false;  // Last permutation reached

    int j = (int)seq.size() - 1;
    while (seq[j] <= seq[i]) --j;

    std::swap(seq[i], seq[j]);
    std::reverse(seq.begin() + i + 1, seq.end());
    return true;
}

size_t Factorial(int n) {
    size_t result = 1;
    for (int i = 2; i <= n; ++i) result *= i;
    return result;
}

void BruteForceBestSequence(
    const std::vector<std::vector<Line>>& slices,
    BoundingBox box,
    mesh& models,
    std::vector<SliceEntry>& bestSequence,
    float& bestCost)
{
    int n = slices.size();
    std::vector<int> indices(n);
    for (int i = 0; i < n; ++i) indices[i] = i;

    size_t totalPermutations = Factorial(n);
    size_t permCount = 0;

    bestCost = FLT_MAX;

    do {
        permCount++;
        if (permCount % 1000 == 0 || permCount == 1 || permCount == totalPermutations) {
            printf("Evaluating permutation %zu / %zu\r", permCount, totalPermutations);
            fflush(stdout);
        }

        // Try all flip combinations
        // for (int flipMask = 0; flipMask < (1 << n); ++flipMask) {
        for (int flipMask = 0; flipMask < 1; ++flipMask) {
            std::vector<SliceEntry> candidate;
            for (int i = 0; i < n; ++i) {
                bool flipped = (flipMask >> i) & 1;
                candidate.push_back({ indices[i], flipped });
            }

            float cost = 0.0f;
            bool valid = true;
            for (int i = 1; i < n; ++i) {
                float c = CostBetween(candidate, i - 1, i, slices, box, models);
                if (c == FLT_MAX) {
                    valid = false;
                    break;
                }
                cost += c;
            }

            if (valid && cost < bestCost) {
                bestCost = cost;
                bestSequence = candidate;
            }
        }
    } while (NextPermutation(indices));

    printf("\nBrute-force completed: %zu permutations evaluated.\n", permCount);
}

void NearestNeighborWithTwoOptSequence(
    const std::vector<std::vector<Line>>& slices,
    BoundingBox box,
    mesh& models,
    std::vector<SliceEntry>& sequence)
{
    int n = slices.size();
    std::vector<bool> visited(n, false);
    sequence.clear();

    int currentIndex = 0;
    visited[currentIndex] = true;
    sequence.push_back({ currentIndex, false });  // Start with flip = false

    for (int step = 1; step < n; ++step) {
        float bestCost = FLT_MAX;
        int bestIndex = -1;

        // Enforce alternation: next flip is opposite of last
        bool nextFlip = !sequence.back().flipped;

        for (int i = 0; i < n; ++i) {
            if (visited[i]) continue;

            SliceEntry trial = { i, nextFlip };
            std::vector<SliceEntry> test = { sequence.back(), trial };

            float cost = CostBetween(test, 0, 1, slices, box, models);
            if (cost < bestCost) {
                bestCost = cost;
                bestIndex = i;
            }
        }

        visited[bestIndex] = true;
        sequence.push_back({ bestIndex, nextFlip });
    }

    // Optional: skip 2-opt to preserve alternating pattern
    // If you *really* want to run 2-opt, you'll need to maintain the alternation manually (can be done, but adds complexity)
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
std::vector<Line> culled = models.Flatten_Culled_Lines(cullBox, result, false);

        if (!culled.empty()) {
            if (!intersectionList.empty()) {
                auto& prevSlice = allSlices.back();
                auto& lastPoint = prevSlice.back().endLinePoint;
                auto& firstPoint = culled.front().startLinePoint;
            
                // Add connector line between slice end and next slice start
                intersectionList.push_back((Line){
                    .startLinePoint = lastPoint,
                    .endLinePoint   = firstPoint,
                    .type = 2
                });
            
                // AABB ray from slice end
                Vector3 dirEnd = Vector3Negate(Vector3Normalize(lastPoint.Normal));
                Vector3 hitEnd;
                if (Vector3Length(dirEnd) > 0.001f && models.RayIntersectsAABB(lastPoint.Position, dirEnd, cullBox, &hitEnd)) {
                    intersectionList.push_back((Line){
                        .startLinePoint = lastPoint,
                        .endLinePoint = { hitEnd, lastPoint.Normal },
                        .type = 1
                    });
                }
            
                // AABB ray from slice start
                Vector3 dirStart = Vector3Negate(Vector3Normalize(firstPoint.Normal));
                Vector3 hitStart;
                if (Vector3Length(dirStart) > 0.001f && models.RayIntersectsAABB(firstPoint.Position, dirStart, cullBox, &hitStart)) {
                    intersectionList.push_back((Line){
                        .startLinePoint = firstPoint,
                        .endLinePoint = { hitStart, firstPoint.Normal },
                        .type = 1
                    });
                }
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
raySet = models.Flatten_Culled_Lines(otherBox, raySet, false);
            if (raySet.empty()) break;
        }
        processedRayLines.insert(processedRayLines.end(), raySet.begin(), raySet.end());
    }

finalRayLines = models.Flatten_Culled_Lines(cullBox, processedRayLines, false);

    // ─────────────────────────────────────────────────────────────────────────────
    // ░░ 2-OPT SEQUENCE OPTIMIZATION ░░
    // ─────────────────────────────────────────────────────────────────────────────


std::vector<SliceEntry> sequence;
float bestCost;
BoundingBox bbox = GetMeshBoundingBox(currentmodel.model.meshes[0]);
// NearestNeighborWithTwoOptSequence(allSlices, bbox, models, sequence);


for (int i = 0; i < (int)allSlices.size(); ++i)
    sequence.push_back({ i, false });


    std::vector<Point> pointCache(sequence.size());
    for (size_t i = 0; i < sequence.size(); ++i) {
        pointCache[i] = models.lastPoint(allSlices[sequence[i].sliceIndex], 0, true);
    }

    bool improved = true;
    int iter = 0;
    const int maxIter = 10000;
    const float minDelta = 0.01f;

    while (improved && iter++ < maxIter) {
        improved = false;

        for (size_t i = 1; i < sequence.size() - 2; ++i) {
            for (size_t k = i + 2; k < sequence.size() - 1; ++k) {
float bestCost = FLT_MAX;
bool bestFlipA = false, bestFlipB = false;

for (bool flipA : { false, true }) {
    for (bool flipB : { false, true }) {
        sequence[i].flipped = flipA;
        sequence[k].flipped = flipB;
        float cost = CostBetween(sequence, i - 1, i, allSlices, bbox, models)
           + CostBetween(sequence, k, k + 1, allSlices, bbox, models);

        if (cost < bestCost) {
            bestCost = cost;
            bestFlipA = flipA;
            bestFlipB = flipB;
        }
    }
}

// Restore best flips
sequence[i].flipped = bestFlipA;
sequence[k].flipped = bestFlipB;

float before = CostBetween(sequence, i - 1, i, allSlices, bbox, models)
             + CostBetween(sequence, k, k + 1, allSlices, bbox, models);

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

float after = CostBetween(temp, i - 1, i, allSlices, bbox, models)
            + CostBetween(temp, k, k + 1, allSlices, bbox, models);
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
intersectionList.clear();
Vector3 lastOuterExit = { 0 };
bool hasLastOuterExit = false;


for (size_t i = 0; i < sequence.size(); ++i) {
    const auto& slice = allSlices[sequence[i].sliceIndex];
        bool flip = sequence[i].flipped;
    int bestStart = 0;
    float bestScore = FLT_MAX;

    // Pick best starting point (nearest to last exit)
    for (int j = 0; j < (int)slice.size(); ++j) {
        Point candidate = models.lastPoint(slice, j, true);
        float dist = (i > 0) ? Vector3Distance(sliceLastPoints.back().Position, candidate.Position) : 0.0f;
        if (dist < bestScore) {
            bestScore = dist;
            bestStart = j;
        }
    }

    std::vector<Line> oriented = models.Orient_Line_Group(slice);

    // Rotate slice to start at bestStart
std::vector<Line> rotated;

if (!flip) {
    for (size_t j = bestStart; j < oriented.size(); ++j)
        rotated.push_back(oriented[j]);
    for (size_t j = 0; j < bestStart; ++j)
        rotated.push_back(oriented[j]);
} else {
    // Manually reverse the slice
    std::vector<Line> reversed;
    for (int j = (int)oriented.size() - 1; j >= 0; --j) {
        Line l = oriented[j];
        std::swap(l.startLinePoint, l.endLinePoint);  // flip direction
        reversed.push_back(l);
    }

    // Manual rotation to adjust start index
    int newStart = (int)reversed.size() - 1 - bestStart;
    for (size_t j = newStart; j < reversed.size(); ++j)
        rotated.push_back(reversed[j]);
    for (size_t j = 0; j < newStart; ++j)
        rotated.push_back(reversed[j]);
}



    // 🔹 Get entry and exit
    Point entryPoint = rotated.front().startLinePoint;
    Point exitPoint = rotated.back().endLinePoint;

// 🔹 ENTRY AABB ray: from outer box to slice entry
Vector3 entryDir = Vector3Negate(Vector3Normalize(entryPoint.Normal));
Vector3 outerEntryHit;
bool hasEntryHit = models.RayIntersectsAABB(entryPoint.Position, entryDir, bbox, &outerEntryHit);
if (hasEntryHit) {
    intersectionList.push_back(Line{
        .startLinePoint = { outerEntryHit, entryPoint.Normal },
        .endLinePoint   = entryPoint,
        .type = 2
    });
}

// 🔗 Add connection from previous AABB exit to this AABB entry (box → box)
if (hasLastOuterExit && hasEntryHit) {
    intersectionList.push_back(Line{
        .startLinePoint = { lastOuterExit, entryPoint.Normal },
        .endLinePoint   = { outerEntryHit, entryPoint.Normal },
        .type = 2
    });
}

// 🔸 Slice path
for (const Line& l : rotated) {
    intersectionList.push_back(l);
}

// 🔹 EXIT AABB ray: from slice end to outer box
Vector3 exitDir = Vector3Negate(Vector3Normalize(exitPoint.Normal));
Vector3 outerExitHit;
bool hasExitHit = models.RayIntersectsAABB(exitPoint.Position, exitDir, bbox, &outerExitHit);
if (hasExitHit) {
    intersectionList.push_back(Line{
        .startLinePoint = exitPoint,
        .endLinePoint   = { outerExitHit, exitPoint.Normal },
        .type = 2
    });
}

// Update for next loop
lastOuterExit = outerExitHit;
hasLastOuterExit = hasExitHit;

// Store entry/exit points for cubes
sliceLastPoints.push_back(entryPoint);
sliceLastPoints.push_back(exitPoint);


}

runSlice = false;
}


BeginMode3D(camera);

// Model
if (modelVisible) models.Run_Models();

// Path lines
size_t sliceColorIdx = 0;
for (const Line& line : intersectionList) {
    if (line.type == 0 || line.type == 1) {
        DrawLine3D(line.startLinePoint.Position, line.endLinePoint.Position, BLUE);
    } else if (line.type == 2) {
        Color rayColor = (sliceColorIdx % 2 == 0) ? GREEN : RED;
        DrawLine3D(line.startLinePoint.Position, line.endLinePoint.Position, rayColor);
        sliceColorIdx++;
    }
}


// Start/end cubes
for (size_t i = 0; i + 1 < sliceLastPoints.size(); i += 2) {
    const Point& entry = sliceLastPoints[i];
    const Point& exit  = sliceLastPoints[i + 1];

    DrawCube(entry.Position, 0.05f, 0.05f, 0.05f, GREEN);
    DrawCube(exit.Position,  0.05f, 0.05f, 0.05f, RED);
}



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
