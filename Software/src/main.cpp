#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <raylib.h>
#include "raygui.h"
#include "components.h"

void Initialise_Window(){
    int screenWidth = 800, screenHeight = 800;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Borb CAM Slicer");
    Image Icon = LoadImage("src/Icon.png");
    SetWindowIcon(Icon);
    UnloadImage(Icon);
    SetTargetFPS(60);
}

int main(){
    Initialise_Window();
    
    // bool button_state = false;

    component item;
    comp_prop rect;
    // rect.colour = ORANGE;

    while(WindowShouldClose() == false){
        BeginDrawing();

        // if(GuiButton((Rectangle){24, 24, 120, 30}, "#191#Show Message"))button_state = true;
        rect.id = 0;
        rect.height = item.global_properties.window_height_ratio;
        rect.width = item.global_properties.window_width_ratio;
        rect.xpos = 200;
        rect.ypos = 50;
        item.add_component(rect);
        item.run_component(rect);
        item.remove_component(rect);
        // Rectangle rectangle;
        // rectangle.height = 50;
        // rectangle.width = 100;
        // rectangle.x = 100;
        // rectangle.y = 50;
        // DrawRectangleRounded(rectangle, 0, 0, ORANGE);

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
