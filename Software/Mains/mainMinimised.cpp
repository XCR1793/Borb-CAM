#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <raymath.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#define RLIGHTS_IMPLEMENTATION
#include <rlights.h>

#define MODEL_PATH "src/model.obj"
#define GLSL_VERSION 330

void Initialise_Window(){
    int screenWidth = 1200, screenHeight = 800;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Borb CAM Slicer");
    Image Icon = LoadImage("src/Logo-Light.png");
    SetWindowIcon(Icon);
    UnloadImage(Icon);
    SetTargetFPS(60);
}

Shader LoadLightingShader(void){
    Shader shader = LoadShader(
        TextFormat("resources/shaders/glsl%i/lighting.vs", GLSL_VERSION),
        TextFormat("resources/shaders/glsl%i/lighting.fs", GLSL_VERSION)
    );
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
    int ambientLoc = GetShaderLocation(shader, "ambient");
    SetShaderValue(shader, ambientLoc, (float[4]){0.3f, 0.3f, 0.3f, 1.0f}, SHADER_UNIFORM_VEC4);
    return shader;
}

Model TransformModel(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, float scale){
    model.transform = MatrixMultiply(
        MatrixMultiply(
            MatrixRotate(rotationAxis, rotationAngle),
            MatrixScale(scale, scale, scale)
        ),
        MatrixTranslate(position.x, position.y, position.z)
    );
    return model;
}

void Initialise_Camera(Camera* camera){
    camera->position = (Vector3){2.0f, 4.0f, 6.0f};
    camera->target = (Vector3){0.0f, 0.5f, 0.0f};
    camera->up = (Vector3){0.0f, 1.0f, 0.0f};
    camera->fovy = 45.0f;
    camera->projection = CAMERA_PERSPECTIVE;
}

void UpdateCameraControls(Camera* camera, float zoomFactor){
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)){
        Vector2 delta = Vector2Scale(GetMouseDelta(), zoomFactor * 0.01f);
        Vector3 forward = Vector3Normalize(Vector3Subtract(camera->target, camera->position));
        Vector3 right = Vector3CrossProduct(forward, camera->up);
        camera->position = Vector3Add(camera->position, Vector3Scale(right, -delta.x));
        camera->position = Vector3Add(camera->position, Vector3Scale(camera->up, delta.y));
        camera->target = Vector3Add(camera->target, Vector3Scale(right, -delta.x));
        camera->target = Vector3Add(camera->target, Vector3Scale(camera->up, delta.y));
    }

    float wheelMove = GetMouseWheelMove();
    if (wheelMove != 0){
        Vector3 direction = Vector3Scale(Vector3Subtract(camera->target, camera->position), 1.0f + wheelMove * 0.1f);
        camera->position = Vector3Subtract(camera->target, direction);
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)){
        Vector2 delta = Vector2Scale(GetMouseDelta(), 0.005f);
        Vector3 right = Vector3CrossProduct(Vector3Subtract(camera->target, camera->position), camera->up);
        camera->position = Vector3RotateByAxisAngle(camera->position, camera->up, -delta.x);
        camera->position = Vector3RotateByAxisAngle(camera->position, right, -delta.y);
    }
}

int main(){
    Initialise_Window();

    Camera camera = { 0 };
    Initialise_Camera(&camera);

    Shader shader = LoadLightingShader();

    // Define 4 directional lights (front, back, left, right)
    Light lights[4] = {
        CreateLight(LIGHT_DIRECTIONAL, Vector3Zero(), (Vector3){-1.0f, -1.0f, 0.0f}, RED, shader),    // Right-side sunlight
        CreateLight(LIGHT_DIRECTIONAL, Vector3Zero(), (Vector3){1.0f, -1.0f, 0.0f}, GREEN, shader),   // Left-side sunlight
        CreateLight(LIGHT_DIRECTIONAL, Vector3Zero(), (Vector3){0.0f, -1.0f, -1.0f}, BLUE, shader),   // Front-side sunlight
        CreateLight(LIGHT_DIRECTIONAL, Vector3Zero(), (Vector3){0.0f, -1.0f, 1.0f}, YELLOW, shader)   // Back-side sunlight
    };

    Model model = LoadModel(MODEL_PATH);
    model.materials[0].shader = shader;

    Vector3 modelPosition = {0.0f, 0.0f, 0.0f};
    Vector3 modelRotation = {1.0f, 0.0f, 0.0f};
    float rotationAngle = -PI / 2;
    float modelScale = 1.0f;

    while (!WindowShouldClose()){
        UpdateCameraControls(&camera, Vector3Distance(camera.position, camera.target) * 0.1f);
        SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], &camera.position, SHADER_UNIFORM_VEC3);

        // Update all directional lights
        for (int i = 0; i < 4; i++)
            UpdateLightValues(shader, lights[i]);

        BeginDrawing();
            ClearBackground(BLACK);
            BeginMode3D(camera);
                BeginShaderMode(shader);
                    DrawPlane(Vector3Zero(), (Vector2){10.0f, 10.0f}, GRAY);
                    DrawModel(TransformModel(model, modelPosition, modelRotation, rotationAngle, modelScale), Vector3Zero(), 1, WHITE);
                EndShaderMode();
                DrawGrid(10, 1.0f);
            EndMode3D();
        EndDrawing();
    }

    UnloadModel(model);
    CloseWindow();
    return 0;
}
