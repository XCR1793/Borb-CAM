#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <raylib.h>
#include <raymath.h>
#include "appmanagement.h"
#include "meshmanagement.h"

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

    window.Add_Button(1, 20, 60, 30, 100, "X+");
    window.Add_Button(2, 20, 60, 120, 100, "X-");
    window.Add_Button(3, 20, 60, 30, 150, "Y+");
    window.Add_Button(4, 20, 60, 120, 150, "Y-");
    window.Add_Button(5, 20, 60, 30, 200, "Z+");
    window.Add_Button(6, 20, 60, 120, 200, "Z-");

    mesh models;

    models.Add_Model(1, "src/model.obj");

    int x = 0;
    int y = 0;
    int z = 0;
    
    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(DARKGRAY);

        if(window.Ret_Button(1)){x = x + 5;}
        if(window.Ret_Button(2)){x = x - 5;}
        if(window.Ret_Button(3)){y = y + 5;}
        if(window.Ret_Button(4)){y = y - 5;}
        if(window.Ret_Button(5)){z = z + 5;}
        if(window.Ret_Button(6)){z = z - 5;}

        window.Run_Buttons();

        models.Rep_Model(1, (Vector3){x, y, z});

        BeginMode3D(camera);

        models.Run_Models();

        EndMode3D();

        DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
