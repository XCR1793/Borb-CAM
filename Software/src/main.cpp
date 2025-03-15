#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <raylib.h>
#include "raygui.h"
#include "raymath.h"
#include "components.h"
// #include "meshtools.h"
// #include "pathplanner.h"
// #define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

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

Light* SetupLights(Shader shader)
{
    static Light lights[MAX_LIGHTS];
    lights[0] = CreateLight(LIGHT_POINT, (Vector3){ -2, 1, -2 }, Vector3Zero(), YELLOW, shader);
    lights[1] = CreateLight(LIGHT_POINT, (Vector3){ 2, 1, 2 }, Vector3Zero(), RED, shader);
    lights[2] = CreateLight(LIGHT_POINT, (Vector3){ -2, 1, 2 }, Vector3Zero(), GREEN, shader);
    lights[3] = CreateLight(LIGHT_POINT, (Vector3){ 2, 1, -2 }, Vector3Zero(), BLUE, shader);
    return lights;
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

void UpdateCameraControls(Camera *camera, float zoomFactor)
{
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 delta = GetMouseDelta();
        delta = Vector2Scale(delta, zoomFactor * 0.01f);
        Vector3 forward = Vector3Subtract(camera->target, camera->position);
        forward = Vector3Normalize(forward);
        Vector3 right = Vector3CrossProduct(forward, camera->up);
        camera->target = Vector3Add(camera->target, Vector3Scale(right, -delta.x));
        camera->target = Vector3Add(camera->target, Vector3Scale(camera->up, delta.y));
        camera->position = Vector3Add(camera->position, Vector3Scale(right, -delta.x));
        camera->position = Vector3Add(camera->position, Vector3Scale(camera->up, delta.y));
    }
    
    float wheelMove = GetMouseWheelMove();
    if (wheelMove != 0)
    {
        Vector3 direction = Vector3Subtract(camera->target, camera->position);
        direction = Vector3Scale(direction, 1.0f + wheelMove * 0.1f);
        camera->position = Vector3Subtract(camera->target, direction);
    }
    
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        Vector2 delta = GetMouseDelta();
        delta = Vector2Scale(delta, 0.005f);
        Vector3 right = Vector3CrossProduct(Vector3Subtract(camera->target, camera->position), camera->up);
        camera->position = Vector3RotateByAxisAngle(camera->position, camera->up, -delta.x);
        camera->position = Vector3RotateByAxisAngle(camera->position, right, -delta.y);
    }
}


int main(){
    Initialise_Window();

    component item;
    comp_prop rect;
    item.add_component(rect, 0); // Title Bar
    item.add_component(rect, 1); // Side Bar


    SetConfigFlags(FLAG_MSAA_4X_HINT);

    Camera camera = { 0 };
    camera.position = (Vector3){ 2.0f, 4.0f, 6.0f };
    camera.target = (Vector3){ 0.0f, 0.5f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Shader shader = LoadLightingShader();
    Light* lights = SetupLights(shader);
    Model model = LoadObject();
    model.materials[0].shader = shader;

    Vector3 modelPosition = { 0.0f, 0.0f, 0.0f };
    Vector3 modelRotation = {-90.0f, 0.0f, 0.0f }; // Default rotation axis (Y-axis)
    float rotationAngle = 90.0f;
    float modelScale = 0.010f;

    // float slider1 = 0.0f;
    // float slider2 = 0.0f;
    // float slider3 = 0.0f;

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

        float zoomFactor = Vector3Distance(camera.position, camera.target) * 0.1f;
        UpdateCameraControls(&camera, zoomFactor);

        float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
        SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);
        
        for (int i = 0; i < MAX_LIGHTS; i++) UpdateLightValues(shader, lights[i]);
        
        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode3D(camera);
        BeginShaderMode(shader);
        DrawPlane(Vector3Zero(), (Vector2){ 10.0f, 10.0f }, WHITE);
        DrawModelEx(model, modelPosition, modelRotation, rotationAngle, (Vector3){ modelScale, modelScale, modelScale }, WHITE);
        EndShaderMode();
        for (int i = 0; i < MAX_LIGHTS; i++) {
            if (lights[i].enabled) DrawSphereEx(lights[i].position, 0.2f, 8, 8, lights[i].color);
            else DrawSphereWires(lights[i].position, 0.2f, 8, 8, ColorAlpha(lights[i].color, 0.3f));
        }
        DrawGrid(10, 1.0f);
        EndMode3D();
        // DrawFPS(10, 10);

        // slider1 = GuiSliderBar((Rectangle){ 20, item.global_properties.window_size_height - 90, 200, 20 }, "X Pos", NULL, &slider1, -5.0f, 5.0f);
        // slider2 = GuiSliderBar((Rectangle){ 20, item.global_properties.window_size_height - 60, 200, 20 }, "Y Pos", NULL, &slider2, -5.0f, 5.0f);
        // slider3 = GuiSliderBar((Rectangle){ 20, item.global_properties.window_size_height - 30, 200, 20 }, "Z Pos", NULL, &slider3, -5.0f, 5.0f);
        

        item.run_components();

        EndDrawing();
        item.update_properties();
    }
    UnloadModel(model);
    CloseWindow();
    return 0;
}
