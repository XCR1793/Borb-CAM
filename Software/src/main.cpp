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
    window.Initialise_Window(800, 1200, 60, "Borb CAM Slicer", "src/Logo-Light.png");
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

    mesh models;

    // Mesh cubeMesh = GenMeshCube(2.0f, 2.0f, 2.0f); // width, height, length
    // Model cubeModel = LoadModelFromMesh(cubeMesh);

    models.Add_Model(1, "src/model.obj");
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
    float a = 0;
    float b = 0;
    float c = 0;
    float o = 0;

    path paths;
    paths.Create_File("src/OwO", "gcode");

    std::vector<std::pair<Vector3, Vector3>> pathPositions;
    //     { {1.2f, 3.4f, 5.6f}, {7.8f, 9.0f, 2.1f} }
    // };

    // paths.Path_to_Gcode1(pathPositions);

    auto epoch_seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    auto prev_time = epoch_seconds;



    while(!WindowShouldClose()){

        window.Update_Camera(&camera);
        BeginDrawing();
        ClearBackground(DARKGRAY);

        // Vector3 plane = models.RotXYD_XYZ((Vector3){0, a, b});
        // Vector3 line_start = {x+10, y+10, z+10};
        // Vector3 line_end   = {x-10, y-10, z-10};
        // std::pair<Vector3, bool> intersection = models.IntersectLinePlane((Vector3){0, 1, 0}, (Vector3){10, 1, 5}, (Vector3){10, -1, 5});

        Model currentmodel = models.Ret_Model(1);

        std::vector<std::pair<Vector3, Vector3>> intersectionList = models.Intersect_Model(currentmodel, (Vector3){0, 1, 0});

        auto epoch_seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        // if((o == 0.5) && (epoch_seconds != prev_time)){
        //     // prev_time = epoch_seconds;
        //     // pathPositions.clear();
        //     // for(auto it : intersectionList){
        //     //     pathPositions.push_back(std::pair<Vector3, Vector3>(it.first, (Vector3){0, 0, 0}));
        //     // }
        //     // paths.Clear_File();
        //     // paths.Path_to_Gcode1(pathPositions);

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
        //     // o = 0;
        // }
        // if(epoch_seconds != prev_time){
        //     prev_time = epoch_seconds;
            // o++;

        // }

        if(window.Ret_Button(1)){x = x + 0.5;}
        if(window.Ret_Button(2)){x = x - 0.5;}
        if(window.Ret_Button(3)){y = y + 0.5;}
        if(window.Ret_Button(4)){y = y - 0.5;}
        if(window.Ret_Button(5)){z = z + 0.5;}
        if(window.Ret_Button(6)){z = z - 0.5;}
        if(window.Ret_Button(7)){s = s + 0.5;}
        if(window.Ret_Button(8)){s = s - 0.5;}
        if(window.Ret_Button(9 )){a = a + PI/2;}
        if(window.Ret_Button(10)){a = a - PI/2;}
        if(window.Ret_Button(11)){b = b + PI/2;}
        if(window.Ret_Button(12)){b = b - PI/2;}
        if(window.Ret_Button(13)){c = c + 0.5;}
        if(window.Ret_Button(14)){c = c - 0.5;}
        if(window.Ret_Button(15)){o = o + 0.5;}
        if(window.Ret_Button(16)){o = o - 0.5;}

        window.Run_Buttons();

        models.Rep_Model(1, (Vector3){x, y, z});

        Model currentModel = models.Ret_Model(1);

        models.Scale_Model(currentModel, s);

        models.Rotate_Model(currentModel, (Vector3){a, b, c});

        models.Reu_Model(1, currentModel);

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

        // auto triangleCount = models.Ret_Model(1).meshes[1].triangleCount;
        for(int i = 0; i < o; i++){
            auto Vertex = models.Ret_Model(1).meshes[1].vertices;
            DrawSphere((Vector3){Vertex[(i*3)+0], Vertex[(i*3)+1], Vertex[(i*3)+2]}, 0.01, RED);
        }

        for(int i = 0; i < o; i++){
            if((static_cast<int>(i)%3) == 0){
                auto Vertex = models.Ret_Model(1).meshes[1].vertices;
                DrawTriangle3D( (Vector3){Vertex[(static_cast<int>(i)*3)+0], Vertex[(static_cast<int>(i)*3)+1], Vertex[(static_cast<int>(i)*3)+2]},
                                (Vector3){Vertex[(static_cast<int>(i)*3)+3], Vertex[(static_cast<int>(i)*3)+4], Vertex[(static_cast<int>(i)*3)+5]},
                                (Vector3){Vertex[(static_cast<int>(i)*3)+6], Vertex[(static_cast<int>(i)*3)+7], Vertex[(static_cast<int>(i)*3)+8]},
                                BLUE);
            }
        }

        // if(o > models.Ret_Model(1).meshes[1].vertexCount * 2.5){o = 0;}
        o = models.Ret_Model(1).meshes[1].vertexCount * 2.5;


        if(!intersectionList.empty()){
            for(std::size_t i = 0; i < intersectionList.size()-1; i++){
                DrawLine3D(intersectionList.at(i).first, intersectionList.at(i+1).first, BLUE);
            }
        }

        models.Run_Models();

        EndMode3D();

        DrawFPS(10, 10);
        EndDrawing();
    }
    models.Stop_Models();
    CloseWindow();
    return 0;
}
