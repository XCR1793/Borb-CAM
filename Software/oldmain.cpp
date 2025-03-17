#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <raylib.h>
#include <raymath.h>
#include "components.h"
#include "camview.h"
#include "shader.h"
#include "meshtools.h"
// #include "pathplanner.h"
#ifndef RAYGUI_IMPLEMENTATION
    #define RAYGUI_IMPLEMENTATION
    #include <raygui.h>
#endif
#ifndef RLIGHTS_IMPLEMENTATION
    #define RLIGHTS_IMPLEMENTATION
    #include <rlights.h>
#endif

#define MODEL_PATH "src/model.obj"

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

Shader LoadLightingShader(void)
{
    Shader shader = LoadShader(TextFormat("resources/shaders/glsl%i/lighting.vs", GLSL_VERSION),
                               TextFormat("resources/shaders/glsl%i/lighting.fs", GLSL_VERSION));
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
    int ambientLoc = GetShaderLocation(shader, "ambient");
    SetShaderValue(shader, ambientLoc, (float[4]){ 0.1f, 0.1f, 0.1f, 1.0f }, SHADER_UNIFORM_VEC4);
    return shader;
}

Model LoadObject(void)
{
    return LoadModel(MODEL_PATH);
}

void TransformModel(Vector3 *position, Vector3 *rotationAxis, float *rotationAngle, float *scale)
{
    if (IsKeyDown(KEY_W)) position->y += 0.1f;
    if (IsKeyDown(KEY_S)) position->y -= 0.1f;
    if (IsKeyDown(KEY_A)) position->x -= 0.1f;
    if (IsKeyDown(KEY_D)) position->x += 0.1f;
    if (IsKeyDown(KEY_Q)) position->z -= 0.1f;
    if (IsKeyDown(KEY_E)) position->z += 0.1f;

    if (IsKeyDown(KEY_UP)) *rotationAngle += 1.0f;
    if (IsKeyDown(KEY_DOWN)) *rotationAngle -= 1.0f;
    if (IsKeyDown(KEY_LEFT)) rotationAxis->y = 1.0f, rotationAxis->x = 0.0f, rotationAxis->z = 0.0f;
    if (IsKeyDown(KEY_RIGHT)) rotationAxis->y = 0.0f, rotationAxis->x = 1.0f, rotationAxis->z = 0.0f;

    if (IsKeyDown(KEY_PAGE_UP)) *scale += 0.01f;
    if (IsKeyDown(KEY_PAGE_DOWN)) *scale -= 0.01f;
}

Model TransformModel(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, float scale){
    // Create the transformation matrix
    Matrix transform = MatrixMultiply(
        MatrixMultiply(
            MatrixRotate(rotationAxis, rotationAngle),  // Rotation matrix
            MatrixScale(scale, scale, scale)           // Scaling matrix
        ),
        MatrixTranslate(position.x, position.y, position.z) // Translation matrix
    );

    // Apply transformation matrix to the model
    model.transform = transform;
    
    return model; // Return the transformed model
}



int main(){
    Initialise_Window();

    component item;
    comp_prop rect;
    item.add_component(rect, 0); // Title Bar
    item.add_component(rect, 1); // Side Bar

    camview Camera;
    Camera.Initialise_Camera();

    shader viewShader;


    SetConfigFlags(FLAG_MSAA_4X_HINT);

    Shader shader = LoadLightingShader();

    int Light_Count = 4;
    float radius = 1000;

    Light* lights = viewShader.SetupLights(shader, (Vector3){0.0f, 0.0f, 0.0f}, radius, Light_Count);
    Model model = LoadObject();
    model.materials[0].shader = shader;

    // BoundingBox bbox = GetModelBoundingBox(model);
    // Vector3 quad[4] = { {-1,0,1}, {1,0,1}, {1,0,-1}, {-1,0,-1} };
    // Vector3 triangle[3] = { {0,1,0}, {-1,0,0}, {1,0,0} };
    // Vector3 sphereCenter = {0, 1, 0};
    // float sphereRadius = 0.5f;

    Vector3 modelPosition = { 0.0f, 0.0f, 0.0f };
    Vector3 modelRotation = { 1.0f, 0.0f, 0.0f };
    float rotationAngle = -PI/2;
    float modelScale = 0.010f;

    // HDRPhotosphere sphere = viewShader.LoadHDRPhotosphere("cyclorama_hard_light_4k.hdr", "resources/shaders/glsl%i/skybox.vs", "resources/shaders/glsl%i/skybox.fs");

    // BoundingBox modelBoundingBox = GetMeshBoundingBox(model.meshes[0]);

    SetTargetFPS(60);

    while(WindowShouldClose() == false){

        rect.width = item.global_properties.window_size_width;
        rect.height = 150;
        rect.colour = item.hsl_colour(0, 0, 67, 255);
        item.modify_component(rect, 0);

        rect.width = 300;
        rect.height = item.global_properties.window_size_height;
        rect.colour = item.hsl_colour(0, 0, 45, 255);
        item.modify_component(rect, 1);

        float zoomFactor = Vector3Distance(Camera.camera.position, Camera.camera.target) * 0.1f;
        Camera.UpdateCameraControls(&Camera.camera, zoomFactor);

        float cameraPos[3] = { Camera.camera.position.x, Camera.camera.position.y, Camera.camera.position.z };
        SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

        for (int i = 0; i < Light_Count; i++) UpdateLightValues(shader, lights[i]);

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode3D(Camera.camera);
        // viewShader.DrawHDRPhotosphere(sphere, Camera.camera);

        BeginShaderMode(shader);
        DrawPlane(Vector3Zero(), (Vector2){ 10.0f, 10.0f }, WHITE);
        // DrawModelEx(model, modelPosition, modelRotation, rotationAngle, (Vector3){ modelScale, modelScale, modelScale }, WHITE);
        DrawModel(TransformModel(model, modelPosition, modelRotation, rotationAngle, modelScale), (Vector3){0, 0, 0}, 1, WHITE);
        
        EndShaderMode();
        for (int i = 0; i < Light_Count; i++) {
            if (lights[i].enabled) DrawSphereEx(lights[i].position, 0.2f, 8, 8, lights[i].color);
            else DrawSphereWires(lights[i].position, 0.2f, 8, 8, ColorAlpha(lights[i].color, 0.3f));
        }
        DrawGrid(10, 1.0f);

        // Camera.UpdateCameraView(model, bbox, quad, triangle, sphereCenter, sphereRadius, modelPosition, modelRotation, rotationAngle, modelScale);


        EndMode3D();
        // DrawFPS(10, 10);

        viewShader.UpdateLights(lights, (Vector3){0.0f, 300, 0.0f}, radius, Light_Count, 0.1);

        // DrawBoundingBox(modelBoundingBox, CYAN);

        // item.run_components();



        EndDrawing();
        item.update_properties();
    }
    // UnloadModel(sphere.model);
    // UnloadShader(sphere.shader);
    UnloadModel(model);
    CloseWindow();
    return 0;
}
