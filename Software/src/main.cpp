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

    // Mesh cubeMesh = GenMeshCube(2.0f, 2.0f, 2.0f); // width, height, length
    // Model cubeModel = LoadModelFromMesh(cubeMesh);

    models.Add_Model(1, "src/cone.obj");
    models.Add_Model(2, "src/model.obj");
    models.Add_Model(3, "src/model.obj");

    Model model2 = models.Ret_Model(2);
    models.Position_Model(model2, (Vector3){5, 0, 0});
    models.Reu_Model(2, model2);

    Model model3 = models.Ret_Model(3);
    models.Position_Model(model3, (Vector3){-5, 0, 0});
    models.Reu_Model(3, model3);

    models.Sha_Model(shader);

    float x = 0;
    float y = 0;
    float z = 0;
    float s = 1;
    float a = PI/2;
    float b = 0;
    float c = 0;
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

    bool is_pressed = 0;

    float slice_size = 1.0f;

    Model ActiveModel = models.Ret_Model(1);

    while(!WindowShouldClose()){

        window.Update_Camera(&camera);
        BeginDrawing();
        ClearBackground(DARKGRAY);

        // Vector3 plane = models.RotXYD_XYZ((Vector3){0, a, b});
        // Vector3 line_start = {x+10, y+10, z+10};
        // Vector3 line_end   = {x-10, y-10, z-10};
        // std::pair<Vector3, bool> intersection = models.IntersectLinePlane((Vector3){0, 1, 0}, (Vector3){10, 1, 5}, (Vector3){10, -1, 5});

        Model currentmodel = models.Ret_Model(1);

        std::vector<Line> intersectionList;

        for(float i = -100; i <= 100; i += slice_size) {
            Vector3 coefficients = slicing.rotation_coefficient(a, b);
            std::vector<Line> result = models.Intersect_Model(ActiveModel, (Vector4){coefficients.x, coefficients.y, coefficients.z, i});
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
        

        auto epoch_seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        if((epoch_seconds - prev_time >= 15)){
            prev_time = epoch_seconds;
            pathPositions.clear();
            for (auto& it : intersectionList) {
                pathPositions.push_back(std::make_pair(Vector3Zero(), models.NormalToRotation(it.startLinePoint.Normal)));
            }
            paths.Clear_File();
            paths.Path_to_Gcode1(pathPositions);



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
            o = 0;
        }
        
        if(window.Ret_Button(17)){
            o = 0.5;
        }

        if(window.Ret_Button(1)){x = x + 0.5;}
        if(window.Ret_Button(2)){x = x - 0.5;}
        if(window.Ret_Button(3)){y = y + 0.5;}
        if(window.Ret_Button(4)){y = y - 0.5;}
        if(window.Ret_Button(5)){z = z + 0.5;}
        if(window.Ret_Button(6)){z = z - 0.5;}
        if(window.Ret_Button(7)){s = s + 0.5;}
        if(window.Ret_Button(8)){s = s - 0.5;}
        if(window.Ret_Button(9 )){a = a + PI/10;}
        if(window.Ret_Button(10)){a = a - PI/10;}
        if(window.Ret_Button(11)){b = b + PI/10;}
        if(window.Ret_Button(12)){b = b - PI/10;}
        if(window.Ret_Button(13)){c = c + 0.5;}
        if(window.Ret_Button(14)){c = c - 0.5;}
        if(window.Ret_Button(15)){slice_size = slice_size + 0.01f;}
        if(window.Ret_Button(16)){slice_size = slice_size - 0.01f;}
        if(slice_size <= 0){slice_size = 0.01f;}

        models.Rep_Model(1, (Vector3){x, y, z});

        models.Scale_Model(currentmodel, s);

        models.Rotate_Model(currentmodel, (Vector3){a, b, c});

        models.Reu_Model(1, currentmodel);

        if(is_pressed){
            is_pressed = 0;
            ActiveModel = currentmodel;
        }

        int seg = 30;

        window.Print(models.Ret_Model(1).meshCount, 700, seg);
        seg += 30;
        for(int i = 0; i < models.Ret_Model(1).meshCount; i++){
            window.Print(i, 750, seg);
            window.Print(models.Ret_Model(1).meshes[i].vertexCount, 850, seg);
            window.Print(models.Ret_Model(1).meshes[i].triangleCount, 950, seg);
            seg += 30;
        }

        BeginMode3D(camera);

        // std::vector<std::vector<std::pair<int, Triangle>>> triangle_list = models.List_Triangles(models.Ret_Model(1));
        // for(auto it : triangle_list){
        //     for(auto ti : it){
        //         DrawTriangle3D(ti.second.Vertex1, ti.second.Vertex2, ti.second.Vertex3, BLUE);
        //     }
        // }



        o = models.Ret_Model(1).meshes[1].vertexCount;


        // if(!intersectionList.empty()){
        //     for(auto line : intersectionList){
        //         if(line.type == 1){
        //             if(line.meshNo == 0){
        //                 if(line.islandNo == 0){DrawLine3D(line.startLinePos, line.endLinePos, BLUE);}
        //                 if(line.islandNo == 1){DrawLine3D(line.startLinePos, line.endLinePos, RED);}
        //             }
        //             if(line.meshNo == 1){
        //                 if(line.islandNo == 0){DrawLine3D(line.startLinePos, line.endLinePos, PURPLE);}
        //                 if(line.islandNo == 1){DrawLine3D(line.startLinePos, line.endLinePos, GREEN);}
        //             }
        //         }
        //         // if(line.type == 2){DrawLine3D(line.startLinePos, line.endLinePos, ORANGE);}
        //     }
        // }

        if (!intersectionList.empty()) {
            for (auto& line : intersectionList) {
                if (line.type == 1) {
                    Vector3 normal = line.startLinePoint.Normal;
        
                    Color color = {
                        (unsigned char)((normal.x * 0.5f + 0.5f) * 255), // Red = X axis
                        (unsigned char)((normal.y * 0.5f + 0.5f) * 255), // Green = Y axis
                        (unsigned char)((normal.z * 0.5f + 0.5f) * 255), // Blue = Z axis
                        255
                    };
        
                    DrawLine3D(line.startLinePoint.Position, line.endLinePoint.Position, color);
                }
        
                // Optional: type 2 lines
                // if (line.type == 2) {
                //     DrawLine3D(line.startLinePoint.Position, line.endLinePoint.Position, ORANGE);
                // }
            }
        }
        

        // if (!intersectionList.empty()) {
        //     // Simple function to generate a unique color based on mesh and island number
        //     auto GenerateColor = [](int meshNo, int islandNo) -> Color {
        //         int seed = meshNo * 73856093 ^ islandNo * 19349663; // Combine numbers uniquely
        //         return (Color){
        //             (unsigned char)(50 + (seed * 97) % 200),
        //             (unsigned char)(50 + (seed * 53) % 200),
        //             (unsigned char)(50 + (seed * 29) % 200),
        //             255
        //         };
        //     };
        
        //     for (auto& line : intersectionList) {
        //         if (line.type == 1) {
        //             Color color = GenerateColor(line.meshNo, line.islandNo);
        //             DrawLine3D(line.startLinePoint.Position, line.endLinePoint.Position, color);
        //         }
        
        //         // Optional: type 2 lines if needed
        //         // if (line.type == 2) {
        //         //     DrawLine3D(line.startLinePos, line.endLinePos, ORANGE);
        //         // }
        //     }
        // }
        
        
        // const int NUM_ISLANDS = 2;
        // const int NUM_GRADIENT_LINES = 5;
        
        // std::vector<std::vector<Line>> islandLines(NUM_ISLANDS);
        
        // // Group lines by islandNo only
        // for (auto& line : intersectionList) {
        //     if (line.type == 1 && line.islandNo < NUM_ISLANDS) {
        //         islandLines[line.islandNo].push_back(line);
        //     }
        // }
        
        // // Draw per island
        // for (int island = 0; island < NUM_ISLANDS; ++island) {
        //     auto& lines = islandLines[island];
        //     int count = lines.size();
        
        //     for (int i = 0; i < count; ++i) {
        //         auto& line = lines[i];
        //         Color color;
        
        //         // Apply stepped gradient only to last NUM_GRADIENT_LINES lines
        //         int gradientStart = std::max(0, count - NUM_GRADIENT_LINES);
        
        //         if (i >= gradientStart) {
        //             int stepIndex = i - gradientStart;
        //             float t = (float)stepIndex / (NUM_GRADIENT_LINES - 1);
        
        //             // Red to Blue
        //             color = {
        //                 (unsigned char)(255 * (1.0f - t)),
        //                 0,
        //                 (unsigned char)(255 * t),
        //                 255
        //             };
        //         } else {
        //             color = WHITE; // Or any fallback/default color
        //         }
        
        //         DrawLine3D(line.startLinePos, line.endLinePos, color);
        //     }
        // }
        
        

// std::vector<std::vector<std::vector<std::pair<int, Triangle>>>> Triangle_List = models.Intersecting_Triangles(currentmodel, (Vector4){0,1,0,2});

// // Helper lambda to generate a pseudo-random color based on index
// auto GenerateUniqueColor = [](int index) -> Color {
//     // Simple color cycling using modulus to stay within 0-255
//     return (Color){
//         (unsigned char)(50 + (index * 97) % 200),  // R
//         (unsigned char)(50 + (index * 53) % 200),  // G
//         (unsigned char)(50 + (index * 29) % 200),  // B
//         255                                       // A
//     };
// };

// int islandCounter = 0;

// for (auto& perMesh : Triangle_List) {
//     for (auto& perIsland : perMesh) {
//         Color islandColor = GenerateUniqueColor(islandCounter++);

//         for (auto& perTriangle : perIsland) {
//             DrawLine3D(perTriangle.second.Vertex1, perTriangle.second.Vertex2, islandColor);
//             DrawLine3D(perTriangle.second.Vertex2, perTriangle.second.Vertex3, islandColor);
//             DrawLine3D(perTriangle.second.Vertex3, perTriangle.second.Vertex1, islandColor);
//         }
//     }
// }


// std::vector<std::vector<std::vector<std::pair<int, Triangle>>>> Triangle_List = models.Intersecting_Triangles(currentmodel, (Vector4){0,1,0,-0.5});

// const int NUM_GRADIENT_TRIANGLES = 5;

// int islandCounter = 0;

// for (auto& perMesh : Triangle_List) {
//     for (auto& perIsland : perMesh) {
//         int count = perIsland.size();
//         int gradientStart = std::max(0, count - NUM_GRADIENT_TRIANGLES);

//         for (int i = 0; i < count; ++i) {
//             auto& triangle = perIsland[i].second;

//             Color color;

//             if (i >= gradientStart) {
//                 // Compute step index in gradient
//                 int stepIndex = i - gradientStart;
//                 float t = (float)stepIndex / (NUM_GRADIENT_TRIANGLES - 1);

//                 color = {
//                     (unsigned char)(255 * (1.0f - t)), // red fades
//                     0,
//                     (unsigned char)(255 * t),         // blue increases
//                     255
//                 };
//             } else {
//                 // Use unique color for non-gradient triangles
//                 color = GenerateUniqueColor(islandCounter);
//             }

//             DrawLine3D(triangle.Vertex1, triangle.Vertex2, color);
//             DrawLine3D(triangle.Vertex2, triangle.Vertex3, color);
//             DrawLine3D(triangle.Vertex3, triangle.Vertex1, color);
//         }

//         islandCounter++;
//     }
// }



        models.Run_Models();

        EndMode3D();

        window.Run_Buttons();

        DrawFPS(10, 10);
        EndDrawing();
    }
    models.Stop_Models();
    CloseWindow();
    return 0;
}
