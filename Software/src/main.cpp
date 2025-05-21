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

Model Scale_Model(Model model, float scale) {
    for (int i = 0; i < model.meshCount; i++) {
        Mesh *mesh = &model.meshes[i];
        for (int v = 0; v < mesh->vertexCount; v++) {
            mesh->vertices[v * 3 + 0] *= scale; // x
            mesh->vertices[v * 3 + 1] *= scale; // y
            mesh->vertices[v * 3 + 2] *= scale; // z
        }
    }
    model.transform = MatrixIdentity(); // Reset transform (mesh is already scaled)
    return model;
}

Model Move_Model(Model model, Vector3 translation) {
    for (int i = 0; i < model.meshCount; i++) {
        Mesh *mesh = &model.meshes[i];
        for (int v = 0; v < mesh->vertexCount; v++) {
            mesh->vertices[v * 3 + 0] += translation.x;
            mesh->vertices[v * 3 + 1] += translation.y;
            mesh->vertices[v * 3 + 2] += translation.z;
        }
    }
    model.transform = MatrixIdentity(); // Reset transform (mesh is already moved)
    return model;
}

Color LerpColor(Color a, Color b, float t) {
    Color result;
    result.r = (unsigned char)((1 - t) * a.r + t * b.r);
    result.g = (unsigned char)((1 - t) * a.g + t * b.g);
    result.b = (unsigned char)((1 - t) * a.b + t * b.b);
    result.a = 255;
    return result;
}

int main(){
    app window;
    window.Initialise_Window(1000, 1500, 60, "Borb CAM Slicer", "src/Logo-Light.png");
    Camera camera = window.Initialise_Camera((Vector3){20.0f, 20.0f, 20.0f}, (Vector3){0.0f, 8.0f, 0.0f}, (Vector3){0.0f, 1.6f, 0.0f},45.0f, CAMERA_PERSPECTIVE);
    Shader shader = window.Initialise_Shader();
    window.Initialise_Lights(shader);
    window.Create_File("src/Debug", "txt");

    window.Add_Button(1, 10, 50, 10, 10, "Slice");

    mesh models;
    models.Add_Model(1, "src/model.obj");
    Model currentmodel = models.Ret_Model(1);
    Model newmodel = Move_Model(Scale_Model(currentmodel, 0.1), (Vector3){0, -0.5, 0});
    BoundingBox cullBox = { Vector3{-5, -4, -5}, Vector3{5, 0, 5} };

    path paths;
    paths.Create_File("src/OwO", "nc");

    std::vector<std::pair<Vector3, Vector3>> pathPositions;
    std::vector<std::vector<Line>> toolpath;

    slice slicing;
    slicing.Set_Slicing_Plane((Vector2){PI/4, 0}, 0);
    slicing.Set_Slicing_Distance(0.1f);

    while(!WindowShouldClose()){
        models.Sha_Model(1, shader);
        window.Update_Camera(&camera);
        BeginDrawing();
        ClearBackground(DARKGRAY);

        if(window.Ret_Button(1)){
            toolpath.clear();
            toolpath = slicing.Generate_Surface_Toolpath(newmodel);
            toolpath = slicing.Cull_Toolpath_by_Box(toolpath, cullBox);
            toolpath = slicing.Optimise_Start_End_Positions(toolpath);
            toolpath = slicing.Apply_AABB_Rays(toolpath, GetModelBoundingBox(currentmodel));
            toolpath = slicing.Optimise_Start_End_Linkages(toolpath);
        }

        BeginMode3D(camera);
        DrawBoundingBox(GetModelBoundingBox(newmodel), GREEN);

        // for(auto paths : toolpath){
        //     for(auto path : paths){
        //         DrawLine3D(path.startLinePoint.Position, path.endLinePoint.Position, BLUE);
        //     }
        // }

        Color gradientStart = BLUE;
        Color gradientEnd   = RED;

        for (const auto& paths : toolpath) {
            size_t count = paths.size();
            for (size_t i = 0; i < count; ++i) {
                float t = (count <= 1) ? 0.0f : (float)i / (float)(count - 1);
                Color lineColor = LerpColor(gradientStart, gradientEnd, t);
            
                DrawLine3D(paths[i].startLinePoint.Position, paths[i].endLinePoint.Position, lineColor);
            }
        }


        // models.Run_Models();
        EndMode3D();
        window.Run_Buttons();
        DrawFPS(10, 10);
        EndDrawing();
    }
    models.Stop_Models();
    CloseWindow();
    return 0;
}
