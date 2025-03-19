#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <raylib.h>
#include <raymath.h>
#include "appmanagement.h"

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

    window.Add_Button(1, 20, 60, 30, 100, "PRESS ME");

    int i = 0;
    
    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(DARKGRAY);

        window.Print(i, 500, 100);

        if(i % 2 == 0){
            window.Rem_Button(2);
            window.Add_Button(1, 20, 60, 30, 100, "PRESS ME");
            window.Print(1, 500, 50);
        }else{
            window.Rem_Button(1);
            window.Add_Button(2, 20, 60, 60, 100, "PRESS ME");
            window.Print(2, 500, 50);
        }

        if(window.Ret_Button(1)||window.Ret_Button(2)){
            i++;
        }

        window.Print(window.CNT_Button(), 200, 10);

        window.Run_Buttons();

        DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}