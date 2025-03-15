#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <raylib.h>
#include "raygui.h"
#include "raymath.h"
#include "components.h"
#include "meshtools.h"
// #include "pathplanner.h"


#define GLSL_VERSION 330

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

    component item;
    comp_prop rect;
    item.add_component(rect, 0); // Title Bar
    item.add_component(rect, 1); // Side Bar


    // meshtools model;
    // model.loadModel("src/model.obj");

    // Model model = LoadModel("src/model.obj");
    // Model model = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));

    // Vector3 position = {0.0f, 0.0f, 0.0f};

    Model model = LoadModel("src/model.obj");
    // Texture2D tex = LoadTexture("Downloads/Unity/RubberDuck_AlbedoTransparency.png");
    // model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = tex;
    
    Camera cam = {0};
    cam.position = (Vector3){50.0f,50.0f,50.0f};
    cam.target = (Vector3){0.0f,0.0f,0.0f};
    cam.up = (Vector3){0.0f,1.0f,0.0f};
    cam.fovy = 90.f;
    cam.projection = CAMERA_PERSPECTIVE;
    
    Vector3 pos = {0.0f,0.0f,0.0f};
    Vector3 pos2 = {200.0f,1.0f,0.0f};
    BoundingBox bounds = GetMeshBoundingBox(model.meshes[0]);

    while(WindowShouldClose() == false){

        rect.width = item.global_properties.window_size_width;
        rect.height = 150;
        rect.colour = item.hsl_colour(0, 0, 67, 255);
        item.modify_component(rect, 0);

        rect.width = 300;
        rect.height = item.global_properties.window_size_height;
        rect.colour = item.hsl_colour(0, 0, 45, 255);
        item.modify_component(rect, 1);

        UpdateCamera(&cam, CAMERA_THIRD_PERSON);

        BeginDrawing();
        ClearBackground(BLACK);

        // item.run_components();
        

        BeginMode3D(cam);
        DrawModel(model, pos, 1.0f, WHITE);
        DrawModel(model, pos2, 1.0f, WHITE);
        DrawGrid(20, 10.0f);
        DrawBoundingBox(bounds, GREEN);
        EndMode3D();
        DrawText("Loading obj file", 10, GetScreenHeight()-25, 25, DARKGRAY);
        DrawFPS(10,10);

        EndDrawing();
        item.update_properties();
    }
    UnloadModel(model);
    CloseWindow();
    return 0;
}
