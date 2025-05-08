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

    window.Add_Button(1, 20, 60, 30, 100, "X+");
    window.Add_Button(2, 20, 60, 120, 100, "X-");
    window.Add_Button(3, 20, 60, 30, 150, "Y+");
    window.Add_Button(4, 20, 60, 120, 150, "Y-");
    window.Add_Button(5, 20, 60, 30, 200, "Z+");
    window.Add_Button(6, 20, 60, 120, 200, "Z-");
    window.Add_Button(7, 20, 60, 30, 250, "S+");
    window.Add_Button(8, 20, 60, 120, 250, "S-");
    window.Add_Button(9, 20, 60, 30, 300, "A+");
    window.Add_Button(10, 20, 60, 120, 300, "A-");
    window.Add_Button(11, 20, 60, 30, 350, "B+");
    window.Add_Button(12, 20, 60, 120, 350, "B-");
    window.Add_Button(13, 20, 60, 30, 400, "C+");
    window.Add_Button(14, 20, 60, 120, 400, "C-");
    window.Add_Button(15, 20, 60, 30, 450, "O+");
    window.Add_Button(16, 20, 60, 120, 450, "O-");
    window.Add_Button(17, 20, 60, 75, 500, "Run");

    mesh models;

    models.Add_Model(1, "src/model.obj");
    models.Add_Model(2, "src/model.obj");
    models.Add_Model(3, "src/model.obj");

    multimodel model2 = { .id = 2, .model = models.Ret_Model(2) };
    models.Position_Model(model2, (Vector3){5, 0, 0});
    models.Reu_Model(2, model2.model);
    
    multimodel model3 = { .id = 3, .model = models.Ret_Model(3) };
    models.Position_Model(model3, (Vector3){-5, 0, 0});
    models.Reu_Model(3, model3.model);
    

    models.Sha_Model(shader);

    float x = 0, xk = 0;
    float y = 0, yk = 0;
    float z = 0, zk = 0;
    float s = 1, sk = 1;
    float a = 0, ak = 0;
    float b = 0, bk = 0;
    float c = 0, ck = 0;
    float o = 0;

    path paths;
    paths.Create_File("src/OwO", "nc");

    std::vector<std::pair<Vector3, Vector3>> pathPositions;

    slice slicing;

    //     { {1.2f, 3.4f, 5.6f}, {7.8f, 9.0f, 2.1f} }
    // };

    // paths.Path_to_Gcode1(pathPositions);

    auto epoch_seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    auto prev_time = epoch_seconds;

    float slice_size = 0.1f;

    while(!WindowShouldClose()){

        window.Update_Camera(&camera);
        BeginDrawing();
        ClearBackground(DARKGRAY);

        // auto epoch_seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        // if((epoch_seconds - prev_time >= 15)){
            // prev_time = epoch_seconds;
            // pathPositions.clear();
            // for (auto& it : intersectionList) {
            //     pathPositions.push_back(std::make_pair(Vector3Zero(), models.NormalToRotation(it.startLinePoint.Normal)));
            // }
            // paths.Clear_File();
            // paths.Path_to_Gcode1(pathPositions);



        //     // window.Clear_File();
        //     // for(int i = 0; i < models.Ret_Model(1).meshCount; i++){
        //     //     auto vertexCount = models.Ret_Model(1).meshes[i].vertexCount;
        //     //     auto triangleCount = models.Ret_Model(1).meshes[i].triangleCount;
        //     //     window.Write_File_Last("src/Debug", "txt", "Mesh Number: " + std::to_string(i));
        //     //     window.Write_File_Last("src/Debug", "txt", "Vertex Count: " + std::to_string(vertexCount));
        //     //     window.Write_File_Last("src/Debug", "txt", "Triangle Count: " + std::to_string(triangleCount));

        //     //     for(int j = 0; j < 2418; j++){
        //     //         window.Write_File_Last("src/Debug", "txt",
        //     //             " V" + std::to_string(j) + ": " + 
        //     //             std::to_string(models.Ret_Model(1).meshes[i].vertices[(j * 3) + 0]) + " " +
        //     //             std::to_string(models.Ret_Model(1).meshes[i].vertices[(j * 3) + 1]) + " " +
        //     //             std::to_string(models.Ret_Model(1).meshes[i].vertices[(j * 3) + 2]) + " "
        //     //         );
        //     //     }
        //     // }
        // }

        if(window.Ret_Button(1)){x = x + 0.5; xk = 0.5;}
        if(window.Ret_Button(2)){x = x - 0.5; xk = -0.5;}
        if(window.Ret_Button(3)){y = y + 0.5; yk = 0.5;}
        if(window.Ret_Button(4)){y = y - 0.5; yk = -0.5;}
        if(window.Ret_Button(5)){z = z + 0.5; zk = 0.5;}
        if(window.Ret_Button(6)){z = z - 0.5; zk = -0.5;}
        if(window.Ret_Button(7)){s = s + 0.5; sk = 1.5;}
        if(window.Ret_Button(8)){s = s - 0.5; sk = 0.5;}
        if(window.Ret_Button(9 )){a = a + PI/10; ak = PI/10;}
        if(window.Ret_Button(10)){a = a - PI/10; ak = -PI/10;}
        if(window.Ret_Button(11)){b = b + PI/10; bk = PI/10;}
        if(window.Ret_Button(12)){b = b - PI/10; bk = -PI/10;}
        if(window.Ret_Button(13)){c = c + 0.5; ck = 0.5;}
        if(window.Ret_Button(14)){c = c - 0.5; ck = -0.5;}
        if(window.Ret_Button(15)){slice_size = slice_size + 0.01f;}
        if(window.Ret_Button(16)){slice_size = slice_size - 0.01f;}
        if(slice_size <= 0){slice_size = 0.01f;}

        multimodel currentmodel = { .id = 1, .model = models.Ret_Model(1) };

        models.Scale_Model(currentmodel, sk);
        models.Rotate_Model(currentmodel, (Vector3){ak, bk, ck});
        models.Position_Model(currentmodel, (Vector3){xk, yk, zk});
        models.Apply_Transformations(currentmodel);
        models.Reu_Model(1, currentmodel.model);

        std::vector<Line> intersectionList;

        for(float i = -4; i <= 4; i += slice_size) {
            Vector3 coefficients = slicing.rotation_coefficient(0,0);
            std::vector<Line> result = models.Intersect_Model(currentmodel.model, (Vector4){coefficients.x, coefficients.y, coefficients.z, i});
            if(!result.empty() && !intersectionList.empty()){
                auto lastIntersection = intersectionList.back();
                intersectionList.push_back((Line){
                    .startLinePoint = lastIntersection.endLinePoint,
                    .endLinePoint = result.front().startLinePoint,
                    .type = 2
                });
            }
            intersectionList.insert(intersectionList.end(), result.begin(), result.end());
        }

        xk = 0;
        yk = 0;
        zk = 0;
        ak = 0;
        bk = 0;
        ck = 0;
        sk = 1;        

        BeginMode3D(camera);

        o = models.Ret_Model(1).meshes[1].vertexCount;

        // if (!intersectionList.empty()) {
        //     for (auto& line : intersectionList) {
        //         if (line.type == 1) {
        //             Vector3 normal = line.startLinePoint.Normal;
        
        //             Color color = {
        //                 (unsigned char)((normal.x * 0.5f + 0.5f) * 255), // Red = X axis
        //                 (unsigned char)((normal.y * 0.5f + 0.5f) * 255), // Green = Y axis
        //                 (unsigned char)((normal.z * 0.5f + 0.5f) * 255), // Blue = Z axis
        //                 255
        //             };
        
        //             DrawLine3D(line.startLinePoint.Position, line.endLinePoint.Position, color);
        //         }
        //     }
        // }        

                        // Edge Offset
        // float offsetDistance = 0.05f; // adjust as needed

        // if (!intersectionList.empty()) {
        //     for (auto& line : intersectionList) {
        //         if (line.type == 1) {
        //             // Offset both start and end points along their normals
        //             Vector3 startOffset = MovePointAlongNormal3D(line.startLinePoint.Position, line.startLinePoint.Normal, offsetDistance);
        //             Vector3 endOffset = MovePointAlongNormal3D(line.endLinePoint.Position, line.endLinePoint.Normal, offsetDistance);
        
        //             // Color based on the start point normal
        //             Vector3 normal = Vector3Normalize(line.startLinePoint.Normal); // Ensure unit length
        //             Color color = {
        //                 (unsigned char)((normal.x * 0.5f + 0.5f) * 255), // Red = X
        //                 (unsigned char)((normal.y * 0.5f + 0.5f) * 255), // Green = Y
        //                 (unsigned char)((normal.z * 0.5f + 0.5f) * 255), // Blue = Z
        //                 255
        //             };
        
        //             DrawLine3D(startOffset, endOffset, color);
        //         }
        //     }
        // }

        // float debugLineLength = 1.0f; // adjust as needed

        // if (!intersectionList.empty()) {
        //     for (auto& line : intersectionList) {
        //         if (line.type == 1) {
        //             Vector3 start = line.startLinePoint.Position;
        //             Vector3 normal = Vector3Normalize(line.startLinePoint.Normal);
        
        //             // Skip if normal is zero-length
        //             if (Vector3Length(normal) < 0.0001f) continue;
        
        //             Vector3 end = Vector3Add(start, Vector3Scale(normal, debugLineLength));
        
        //             // Visualize normal direction
        //             Color color = {
        //                 (unsigned char)((normal.x * 0.5f + 0.5f) * 255),
        //                 (unsigned char)((normal.y * 0.5f + 0.5f) * 255),
        //                 (unsigned char)((normal.z * 0.5f + 0.5f) * 255),
        //                 255
        //             };
        
        //             // Optional: mark start and end with spheres
        //             DrawSphere(start, 0.01f, BLUE); // Start point
        //             DrawSphere(end, 0.01f, RED);    // End point
        
        //             // Draw the normal line
        //             DrawLine3D(start, end, color);
        //         }
        //     }
        // }
        
        // if (!intersectionList.empty()) {
        //     for (auto& line : intersectionList) {
        //         if (line.type == 1) {
        //             int meshIndex = line.meshNo;
        
        //             // Get the bounding box of the corresponding mesh
        //             BoundingBox bbox = GetMeshBoundingBox(currentmodel.model.meshes[meshIndex]);
        //             DrawBoundingBox(bbox, Fade(WHITE, 0.25f)); // Visualize bounding box
        
        //             // Ray start and REVERSED direction (inward)
        //             Vector3 rayOrigin = line.startLinePoint.Position;
        //             Vector3 rayDirection = Vector3Negate(Vector3Normalize(line.startLinePoint.Normal)); // Reversed normal
        
        //             // Skip invalid directions
        //             if (Vector3Length(rayDirection) < 0.0001f) continue;
        
        //             Vector3 hitPoint;
        //             bool hit = models.RayIntersectsAABB(rayOrigin, rayDirection, bbox, &hitPoint);
        
        //             if (hit) {
        //                 // Visualize the reversed normal with color
        //                 Color color = {
        //                     (unsigned char)((rayDirection.x * 0.5f + 0.5f) * 255),
        //                     (unsigned char)((rayDirection.y * 0.5f + 0.5f) * 255),
        //                     (unsigned char)((rayDirection.z * 0.5f + 0.5f) * 255),
        //                     255
        //                 };
        
        //                 DrawSphere(rayOrigin, 0.01f, BLUE);  // Ray origin
        //                 DrawSphere(hitPoint, 0.01f, RED);    // Ray hit
        //                 DrawLine3D(rayOrigin, hitPoint, color);
        //             }
        //         }
        //     }
        // }


        // // --- Define AABB for culling box (Y: 0 to -4, size: 4x4x4) ---
        // BoundingBox cullBox = {
        //     .min = Vector3{-2.0f, -4.0f, -2.0f},
        //     .max = Vector3{ 2.0f,  0.0f,  2.0f}
        // };

        // // --- Use AABB-based culling function ---
        // std::vector<Line> culledLines = models.Cull_Lines_ByBox(cullBox, intersectionList);

        // // --- Draw the culled lines ---
        // if (!culledLines.empty()) {
        //     for (auto& line : culledLines) {
        //         if (line.type == 1) {
        //             Vector3 normal = line.startLinePoint.Normal;
                
        //             Color color = {
        //                 (unsigned char)((normal.x * 0.5f + 0.5f) * 255),
        //                 (unsigned char)((normal.y * 0.5f + 0.5f) * 255),
        //                 (unsigned char)((normal.z * 0.5f + 0.5f) * 255),
        //                 255
        //             };
                
        //             DrawLine3D(line.startLinePoint.Position, line.endLinePoint.Position, color);
        //         }
        //     }
        // }

        // // --- Draw transparent culling box for debugging ---
        // Color transparentColor = { 255, 0, 0, 51 };  // 20% opacity red
        // Color wireColor = { 255, 0, 0, 255 };        // Full red for outline

        // DrawCube({0.0f, -2.0f, 0.0f}, 4.0f, 4.0f, 4.0f, transparentColor);  // Solid cube
        // DrawBoundingBox(cullBox, wireColor);                                // Wireframe box

        // --- Step 0: Cull box definition ---
        BoundingBox cullBox = { Vector3{-2, -4, -2}, Vector3{2, 0, 2} };

        // --- Step 1: Cull intersection lines against the cull box ---
        std::vector<Line> culledLines = models.Cull_Lines_ByBox(cullBox, intersectionList);

        // --- Step 2: Draw culled lines and collect ray lines ---
        std::vector<Line> rayLines;
        for(int i = 0; i < culledLines.size(); i++){
            Line &line = culledLines[i];
            if(line.type != 1) continue;
        
            DrawLine3D(line.startLinePoint.Position, line.endLinePoint.Position, BLUE);
        
            int meshIndex = line.meshNo;
            if(meshIndex < 0 || meshIndex >= currentmodel.model.meshCount) continue;
        
            BoundingBox bbox = GetMeshBoundingBox(currentmodel.model.meshes[meshIndex]);
            DrawBoundingBox(bbox, Fade(WHITE, 0.25f));
        
            Vector3 origin = line.startLinePoint.Position;
            Vector3 dir = Vector3Negate(Vector3Normalize(line.startLinePoint.Normal));
            if(Vector3Length(dir) < 0.0001f) continue;
        
            Vector3 hit;
            if(models.RayIntersectsAABB(origin, dir, bbox, &hit)){
                rayLines.push_back((Line){
                    .startLinePoint = { origin, line.startLinePoint.Normal },
                    .endLinePoint   = { hit,    line.startLinePoint.Normal },
                    .type           = 1,
                    .meshNo         = meshIndex,
                    .islandNo       = line.islandNo
                });
            }
        }

        // --- Step 3: Apply trimming logic per mesh to simulate occlusion ---
        std::vector<Line> processedRayLines;

        for(int i = 0; i < rayLines.size(); i++){
            std::vector<Line> raySet = { rayLines[i] };
        
            for(int j = 0; j < currentmodel.model.meshCount; j++){
                if(j == rayLines[i].meshNo) continue;
            
                BoundingBox otherBox = GetMeshBoundingBox(currentmodel.model.meshes[j]);
                raySet = models.Cull_Lines_ByBox(otherBox, raySet);
            
                if(raySet.empty()) break; // fully blocked
            }
        
            for(auto &trimmed : raySet){
                processedRayLines.push_back(trimmed);
            }
        }

        // --- Step 4: Final culling of ray lines using cull box ---
        std::vector<Line> finalRayLines = models.Cull_Lines_ByBox(cullBox, processedRayLines);
        

        // --- Step 5: Draw final rays and intersections ---
        for(int i = 0; i < finalRayLines.size(); i++){
            Line &line = finalRayLines[i];
            DrawSphere(line.startLinePoint.Position, 0.01f, BLUE);   // Ray origin
            DrawSphere(line.endLinePoint.Position,   0.01f, RED);    // Hit point
            DrawLine3D(line.startLinePoint.Position, line.endLinePoint.Position, GREEN);
        }

        // --- Step 6: Draw cull box for visual reference ---
        DrawCube((Vector3){0, -2, 0}, 4, 4, 4, (Color){255, 0, 0, 51});
        DrawBoundingBox(cullBox, RED);
        
// // --- Step 0: Cull box definition ---
// BoundingBox cullBox = { Vector3{-2, -4, -2}, Vector3{2, 0, 2} };

// // --- Step 1: Cull intersection lines against the cull box ---
// std::vector<Line> culledLines = models.Cull_Lines_ByBox(cullBox, intersectionList);

// // --- Step 2: Draw culled lines and collect ray lines ---
// std::vector<Line> rayLines;
// for(int i = 0; i < culledLines.size(); i++){
//     Line &line = culledLines[i];
//     if(line.type != 1) continue;

//     DrawLine3D(line.startLinePoint.Position, line.endLinePoint.Position, BLUE);

//     int meshIndex = line.meshNo;
//     if(meshIndex < 0 || meshIndex >= currentmodel.model.meshCount) continue;

//     Vector3 origin = line.startLinePoint.Position;
//     Vector3 dir = Vector3Negate(Vector3Normalize(line.startLinePoint.Normal));
//     if(Vector3Length(dir) < 0.0001f) continue;

//     Vector3 hit;
//     if(models.RayIntersectsAABB(origin, dir, GetMeshBoundingBox(currentmodel.model.meshes[meshIndex]), &hit)){
//         rayLines.push_back((Line){
//             .startLinePoint = { origin, line.startLinePoint.Normal },
//             .endLinePoint   = { hit,    line.startLinePoint.Normal },
//             .type           = 1,
//             .meshNo         = meshIndex,
//             .islandNo       = line.islandNo
//         });
//     }
// }

// // --- Step 3: Trim ray lines against all *other* meshes (triangle-based) ---
// std::vector<Line> processedRayLines;

// for(int i = 0; i < rayLines.size(); i++){
//     Line ray = rayLines[i];
//     std::vector<Line> raySet = { ray };

//     std::vector<Triangle> allTrisExceptSelf = models.Flatten_Triangles_Excluding(currentmodel.model, ray.meshNo);
//     std::vector<Line> trimmedSet;

//     for(Line& rline : raySet){
//         Vector3 a = rline.startLinePoint.Position;
//         Vector3 b = rline.endLinePoint.Position;
//         bool aInside = models.IsPointInsideMesh(a, allTrisExceptSelf);
//         bool bInside = models.IsPointInsideMesh(b, allTrisExceptSelf);

//         if(!aInside && !bInside){
//             trimmedSet.push_back(rline);
//         }else if(aInside && bInside){
//             continue;
//         }else{
//             Vector3 outside = aInside ? b : a;
//             Vector3 inside  = aInside ? a : b;
//             Vector3 dir = Vector3Subtract(inside, outside);
//             float minDist = FLT_MAX;
//             Point closestHit = {};

//             for(const Triangle &tri : allTrisExceptSelf){
//                 Vector3 normal = Vector3Normalize(Vector3CrossProduct(
//                     Vector3Subtract(tri.Vertex2.Position, tri.Vertex1.Position),
//                     Vector3Subtract(tri.Vertex3.Position, tri.Vertex1.Position)
//                 ));
//                 float d = Vector3DotProduct(normal, tri.Vertex1.Position);
//                 auto [hit, valid] = models.IntersectLinePlane({normal.x, normal.y, normal.z, d}, {outside, {}}, {inside, {}});
//                 if(!valid) continue;

//                 // Barycentric triangle check
//                 Vector3 v0 = Vector3Subtract(tri.Vertex2.Position, tri.Vertex1.Position);
//                 Vector3 v1 = Vector3Subtract(tri.Vertex3.Position, tri.Vertex1.Position);
//                 Vector3 v2 = Vector3Subtract(hit.Position, tri.Vertex1.Position);

//                 float d00 = Vector3DotProduct(v0, v0);
//                 float d01 = Vector3DotProduct(v0, v1);
//                 float d11 = Vector3DotProduct(v1, v1);
//                 float d20 = Vector3DotProduct(v2, v0);
//                 float d21 = Vector3DotProduct(v2, v1);

//                 float denom = d00 * d11 - d01 * d01;
//                 float v = (d11 * d20 - d01 * d21) / denom;
//                 float w = (d00 * d21 - d01 * d20) / denom;
//                 float u = 1.0f - v - w;

//                 if(u >= 0 && v >= 0 && w >= 0){
//                     float dist = Vector3LengthSqr(Vector3Subtract(hit.Position, outside));
//                     if(dist < minDist){
//                         minDist = dist;
//                         closestHit = hit;
//                     }
//                 }
//             }

//             if(minDist < FLT_MAX){
//                 Line trimmed = rline;
//                 if(aInside){
//                     trimmed.startLinePoint = closestHit;
//                     trimmed.endLinePoint   = rline.endLinePoint;
//                 }else{
//                     trimmed.startLinePoint = rline.startLinePoint;
//                     trimmed.endLinePoint   = closestHit;
//                 }
//                 trimmedSet.push_back(trimmed);
//             }
//         }
//     }

//     processedRayLines.insert(processedRayLines.end(), trimmedSet.begin(), trimmedSet.end());
// }

// // --- Step 4: Final culling of ray lines using cull box ---
// std::vector<Line> boxCulledRayLines = models.Cull_Lines_ByBox(cullBox, processedRayLines);

// // --- Step 5: Trim ray lines to the surface of the original model ---
// std::vector<Line> finalRayLines = models.Trim_Lines_ByModel(currentmodel.model, boxCulledRayLines);

// // --- Step 6: Draw final rays and intersections ---
// for(int i = 0; i < finalRayLines.size(); i++){
//     Line &line = finalRayLines[i];
//     DrawSphere(line.startLinePoint.Position, 0.01f, BLUE);
//     DrawSphere(line.endLinePoint.Position,   0.01f, RED);
//     DrawLine3D(line.startLinePoint.Position, line.endLinePoint.Position, GREEN);
// }

// // --- Step 7: Draw cull box for reference ---
// DrawCube((Vector3){0, -2, 0}, 4, 4, 4, (Color){255, 0, 0, 51});
// DrawBoundingBox(cullBox, RED);




        // models.Run_Models();

        // BoundingBox bbox1 = GetMeshBoundingBox(currentmodel.model.meshes[0]);
        // DrawBoundingBox(bbox1, RED);
        // BoundingBox bbox2 = GetMeshBoundingBox(currentmodel.model.meshes[1]);
        // DrawBoundingBox(bbox2, RED);

        EndMode3D();

        window.Run_Buttons();

        DrawFPS(10, 10);
        EndDrawing();
    }
    models.Stop_Models();
    CloseWindow();
    return 0;
}
