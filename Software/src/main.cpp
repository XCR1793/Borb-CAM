#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <raylib.h>
#include "raygui.h"
#include "components.h"

void Initialise_Window(){
    int screenWidth = 1200, screenHeight = 800;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Borb CAM Slicer");
    Image Icon = LoadImage("src/Logo-Light.png");
    SetWindowIcon(Icon);
    UnloadImage(Icon);
    SetTargetFPS(60);
}

void ray_print(int value, int posx, int posy){
    char buffer[20];
    sprintf(buffer, "%d", value);
    DrawText(buffer, posx, posy, 30, RED);
}

int main(){
    Initialise_Window();
    
    // bool button_state = false;

    component item;
    item.update_properties();

    comp_prop rect;
    rect.xpos = 100;
    item.add_component(rect, 0);

    comp_prop rect2;
    rect2.xpos = 200;
    item.add_component(rect2, 1);

    // int x = 0, y = 0;

    while(WindowShouldClose() == false){
        BeginDrawing();
        ClearBackground(BLACK);

        // if(GuiButton((Rectangle){24, 24, 120, 30}, "#191#Show Message"))button_state = true;
        // item.run_components();
        // item.update_properties();
        item.run_components();
        // item.modify_component(item.position_component(rect, 100, 10));
        // item.modify_component(item.position_component(rect2, 200, 10));

        ray_print(item.component_list.size(), 100, 50);
        item.numerical_output_component(390857, 100, 100, 20, BLUE, 1);
        // ray_print(static_cast<int>(item.component_list.begin()), 300, 50);

        // x++; y++;

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
