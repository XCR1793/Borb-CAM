#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <raylib.h>
#include "raygui.h"
#include "components.h"
#include "pathplanner.h"

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

typedef struct {
    Vector3 *vertices;
    Vector3 *normals;
    int vertexCount;
} STLModel;

// Function to load a binary STL file
STLModel LoadSTL(const char *filename) {
    STLModel model = {0};

    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to open STL file: %s\n", filename);
        return model;
    }

    fseek(file, 80, SEEK_SET);  // Skip the STL header
    unsigned int numTriangles;
    fread(&numTriangles, sizeof(unsigned int), 1, file);

    model.vertexCount = numTriangles * 3;
    model.vertices = (Vector3 *)malloc(model.vertexCount * sizeof(Vector3));
    model.normals = (Vector3 *)malloc(numTriangles * sizeof(Vector3));

    for (unsigned int i = 0; i < numTriangles; i++) {
        fread(&model.normals[i], sizeof(Vector3), 1, file); // Normal
        fread(&model.vertices[i * 3], sizeof(Vector3), 3, file); // 3 Vertices
        fseek(file, 2, SEEK_CUR); // Skip attribute byte count
    }

    fclose(file);
    return model;
}

// Convert STLModel to raylib Mesh
Mesh ConvertToMesh(STLModel model) {
    Mesh mesh = {0};
    mesh.vertexCount = model.vertexCount;
    mesh.triangleCount = model.vertexCount / 3;

    // Allocate memory
    mesh.vertices = (float *)malloc(mesh.vertexCount * 3 * sizeof(float));
    mesh.normals = (float *)malloc(mesh.vertexCount * 3 * sizeof(float));

    // Copy data
    for (int i = 0; i < model.vertexCount; i++) {
        mesh.vertices[i * 3] = model.vertices[i].x;
        mesh.vertices[i * 3 + 1] = model.vertices[i].y;
        mesh.vertices[i * 3 + 2] = model.vertices[i].z;

        mesh.normals[i * 3] = model.normals[i / 3].x;
        mesh.normals[i * 3 + 1] = model.normals[i / 3].y;
        mesh.normals[i * 3 + 2] = model.normals[i / 3].z;
    }

    UploadMesh(&mesh, true);
    return mesh;
}

// Function to add lighting using raylib
void AddLightingToModel(Model *model) {
    Shader shader = LoadShader(0, TextFormat("resources/shaders/glsl%i/lighting.fs", GLSL_VERSION));
    model->materials[0].shader = shader;
    
    // Set light direction
    Vector3 lightDir = { -0.2f, -1.0f, -0.3f };
    int lightDirLoc = GetShaderLocation(shader, "lightDir");
    SetShaderValue(shader, lightDirLoc, &lightDir, SHADER_UNIFORM_VEC3);

    // Set ambient light
    float ambientColor[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
    int ambientLoc = GetShaderLocation(shader, "ambientColor");
    SetShaderValue(shader, ambientLoc, ambientColor, SHADER_UNIFORM_VEC4);
}

int main(){
    Initialise_Window();

    component item;
    comp_prop rect;
    item.add_component(rect, 0); // Title Bar
    item.add_component(rect, 1); // Side Bar

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 1.5f, 3.0f };
    camera.target = (Vector3){ 0.0f, 0.5f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    STLModel stlModel = LoadSTL("src/model.stl");
    Mesh mesh = ConvertToMesh(stlModel);
    Model model = LoadModelFromMesh(mesh);

    AddLightingToModel(&model);

    while(WindowShouldClose() == false){

        rect.width = item.global_properties.window_size_width;
        rect.height = 150;
        rect.colour = item.hsl_colour(0, 0, 67, 255);
        item.modify_component(rect, 0);

        rect.width = 300;
        rect.height = item.global_properties.window_size_height;
        rect.colour = item.hsl_colour(0, 0, 45, 255);
        item.modify_component(rect, 1);

        UpdateCamera(&camera, CAMERA_ORBITAL);

        BeginDrawing();
        ClearBackground(BLACK);

        // item.run_components();

        BeginMode3D(camera);
        DrawModel(model, (Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, LIGHTGRAY);
        DrawGrid(10, 1.0f);
        EndMode3D();

        DrawText("Use mouse to rotate", 10, 10, 20, DARKGRAY);

        EndDrawing();
        item.update_properties();
    }
    CloseWindow();
    return 0;
}
