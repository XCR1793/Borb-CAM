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

void DrawMeshNormals(Model &model, float offset, Color normalColor) {
    if (!model.meshCount) return;

    for (int m = 0; m < model.meshCount; m++) {
        Mesh *mesh = &model.meshes[m];

        if (!mesh->normals) {
            GenMeshTangents(mesh);  // Generate normals if missing
        }

        for (int i = 0; i < mesh->vertexCount; i++) {
            // Get the vertex position
            Vector3 vertex = {
                mesh->vertices[i * 3 + 0],
                mesh->vertices[i * 3 + 1],
                mesh->vertices[i * 3 + 2]
            };

            // Get the normal direction
            Vector3 normal = {
                mesh->normals[i * 3 + 0],
                mesh->normals[i * 3 + 1],
                mesh->normals[i * 3 + 2]
            };

            // Compute the endpoint of the normal line
            Vector3 normalEnd = Vector3Add(vertex, Vector3Scale(normal, offset));

            // Draw the normal line
            DrawLine3D(vertex, normalEnd, normalColor);
        }
    }
}

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
    models.Sha_Model(shader);

    float x = 0;
    float y = 0;
    float z = 0;
    float s = 1;
    float a = 0;
    float b = 0;
    float c = 0;
    float o = 0;
    
    while(!WindowShouldClose()){

        window.Update_Camera(&camera);
        BeginDrawing();
        ClearBackground(DARKGRAY);

        if(window.Ret_Button(1)){x = x + 5;}
        if(window.Ret_Button(2)){x = x - 5;}
        if(window.Ret_Button(3)){y = y + 5;}
        if(window.Ret_Button(4)){y = y - 5;}
        if(window.Ret_Button(5)){z = z + 5;}
        if(window.Ret_Button(6)){z = z - 5;}
        if(window.Ret_Button(7)){s = s + 0.5;}
        if(window.Ret_Button(8)){s = s - 0.5;}
        if(window.Ret_Button(9)){a = a + 0.5;}
        if(window.Ret_Button(10)){a = a - 0.5;}
        if(window.Ret_Button(11)){b = b + 0.5;}
        if(window.Ret_Button(12)){b = b - 0.5;}
        if(window.Ret_Button(13)){c = c + 0.5;}
        if(window.Ret_Button(14)){c = c - 0.5;}
        if(window.Ret_Button(13)){o = o + 0.5;}
        if(window.Ret_Button(14)){o = o - 0.5;}

        window.Run_Buttons();

        models.Rep_Model(1, (Vector3){x, y, z});

        Model currentModel = models.Ret_Model(1);

        models.Scale_Model(currentModel, s);

        models.Rotate_Model(currentModel, (Vector3){a, b, c});

        models.FaceOffset_Model(currentModel, o);

        models.Reu_Model(1, currentModel);

        DrawMeshNormals(currentModel, o, BLUE);

        BeginMode3D(camera);

        models.Run_Models();

        EndMode3D();

        DrawFPS(10, 10);
        EndDrawing();
    }
    models.Stop_Models();
    CloseWindow();
    return 0;
}
