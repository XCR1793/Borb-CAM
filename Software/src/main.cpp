#include <stdio.h>
#include <stdlib.h>
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

    std::vector<std::pair<Vector3, Vector3>> pathPositions = {
        { {1.2f, 3.4f, 5.6f}, {7.8f, 9.0f, 2.1f} },
        { {4.3f, 6.7f, 8.9f}, {0.2f, 1.4f, 3.6f} },
        { {9.1f, 7.3f, 2.5f}, {4.8f, 6.9f, 1.0f} },
        { {5.5f, 2.2f, 3.3f}, {7.7f, 8.8f, 9.9f} },
        { {0.0f, 1.1f, 2.2f}, {3.3f, 4.4f, 5.5f} },
        { {6.6f, 7.7f, 8.8f}, {9.9f, 0.1f, 1.2f} },
        { {2.3f, 3.4f, 4.5f}, {5.6f, 6.7f, 7.8f} },
        { {8.9f, 9.0f, 0.1f}, {1.2f, 2.3f, 3.4f} },
        { {4.5f, 5.6f, 6.7f}, {7.8f, 8.9f, 9.0f} },
        { {0.1f, 1.2f, 2.3f}, {3.4f, 4.5f, 5.6f} },
        { {6.7f, 7.8f, 8.9f}, {9.0f, 0.1f, 1.2f} },
        { {2.2f, 3.3f, 4.4f}, {5.5f, 6.6f, 7.7f} },
        { {8.8f, 9.9f, 0.0f}, {1.1f, 2.2f, 3.3f} },
        { {4.4f, 5.5f, 6.6f}, {7.7f, 8.8f, 9.9f} },
        { {0.3f, 1.4f, 2.5f}, {3.6f, 4.7f, 5.8f} },
        { {6.9f, 7.0f, 8.1f}, {9.2f, 0.3f, 1.4f} },
        { {2.5f, 3.6f, 4.7f}, {5.8f, 6.9f, 7.0f} },
        { {8.1f, 9.2f, 0.3f}, {1.4f, 2.5f, 3.6f} },
        { {4.7f, 5.8f, 6.9f}, {7.0f, 8.1f, 9.2f} },
        { {0.4f, 1.5f, 2.6f}, {3.7f, 4.8f, 5.9f} },
        { {6.0f, 7.1f, 8.2f}, {9.3f, 0.4f, 1.5f} },
        { {2.6f, 3.7f, 4.8f}, {5.9f, 6.0f, 7.1f} },
        { {8.2f, 9.3f, 0.4f}, {1.5f, 2.6f, 3.7f} },
        { {4.8f, 5.9f, 6.0f}, {7.1f, 8.2f, 9.3f} },
        { {0.5f, 1.6f, 2.7f}, {3.8f, 4.9f, 6.0f} }
    };
    
    
    paths.Path_to_Gcode1(pathPositions);

    while(!WindowShouldClose()){

        window.Update_Camera(&camera);
        BeginDrawing();
        ClearBackground(DARKGRAY);

        Vector3 plane = models.RotXYD_XYZ((Vector3){x, y, z});
        Vector3 line_start = {10, 10, 10};
        Vector3 line_end = {-10, -10, -10};
        std::pair<Vector3, bool> intersection = models.IntersectLinePlane(plane, line_start, line_end);

        window.Print(plane.x, 300, 30);
        window.Print(plane.y, 300, 60);
        window.Print(plane.z, 300, 90);
        
        window.Print(intersection.second, 600, 30);
        window.Print(intersection.first.x, 600, 60);
        window.Print(intersection.first.y, 600, 90);
        window.Print(intersection.first.z, 600, 120);
        

        if(window.Ret_Button(1)){x = x + 1;}
        if(window.Ret_Button(2)){x = x - 1;}
        if(window.Ret_Button(3)){y = y + 1;}
        if(window.Ret_Button(4)){y = y - 1;}
        if(window.Ret_Button(5)){z = z + 1;}
        if(window.Ret_Button(6)){z = z - 1;}
        if(window.Ret_Button(7)){s = s + 0.5;}
        if(window.Ret_Button(8)){s = s - 0.5;}
        if(window.Ret_Button(9)){a = a + 0.5;}
        if(window.Ret_Button(10)){a = a - 0.5;}
        if(window.Ret_Button(11)){b = b + 0.5;}
        if(window.Ret_Button(12)){b = b - 0.5;}
        if(window.Ret_Button(13)){c = c + 0.5;}
        if(window.Ret_Button(14)){c = c - 0.5;}
        if(window.Ret_Button(15)){o = o + 0.5;}
        if(window.Ret_Button(16)){o = o - 0.5;}

        window.Run_Buttons();

        models.Rep_Model(1, (Vector3){x, y, z});

        Model currentModel = models.Ret_Model(1);

        models.Scale_Model(currentModel, s);

        models.Rotate_Model(currentModel, (Vector3){a, b, c});

        window.Print(o, 100, 10);

        models.Reu_Model(1, currentModel);

        BeginMode3D(camera);

        DrawLine3D(line_start, line_end, RED);

        models.Run_Models();

        EndMode3D();

        DrawFPS(10, 10);
        EndDrawing();
    }
    models.Stop_Models();
    CloseWindow();
    return 0;
}
