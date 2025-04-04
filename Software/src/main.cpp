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
    auto prev_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();


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
        if(epoch_seconds != prev_time){
            prev_time = epoch_seconds;
            pathPositions.clear();
            for(auto it : intersectionList){
                pathPositions.push_back(std::pair<Vector3, Vector3>(it.first, (Vector3){0, 0, 0}));
            }
            paths.Clear_File();
            paths.Path_to_Gcode1(pathPositions);
        }
        window.Print(pathPositions.size(), 500, 500);


        // window.PrintF(plane.x, 300, 30);
        // window.PrintF(plane.y, 300, 60);
        // window.PrintF(plane.z, 300, 90);
        
        // window.PrintF(intersection.second, 600, 30);
        // window.PrintF(intersection.first.x, 600, 60);
        // window.PrintF(intersection.first.y, 600, 90);
        // window.PrintF(intersection.first.z, 600, 120);

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

        // window.Print(models.Ret_Model(1).meshes->vertexCount, 600, 30);

        models.Reu_Model(1, currentModel);

        if(models.Ret_Model(1).meshes->indices != NULL){
            window.Print(1234, 600, 120);
        }

        BeginMode3D(camera);

        if(!intersectionList.empty()){
            // window.Print(1, 600, 60);
            for(long i = 0; i < intersectionList.size()-1; i++){
                DrawLine3D(intersectionList.at(i).first, intersectionList.at(i+1).first, BLUE);
            }
        }

        // DrawLine3D((Vector3){10, 1, 5}, (Vector3){10, -1, 5}, RED);

        // DrawPlane((Vector3){0, 0, 0}, (Vector2){10, 10}, RED);

        models.Run_Models();

        EndMode3D();

        DrawFPS(10, 10);
        EndDrawing();
    }
    models.Stop_Models();
    CloseWindow();
    return 0;
}
