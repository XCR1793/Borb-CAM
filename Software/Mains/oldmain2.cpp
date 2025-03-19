#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <float.h>   // Fix for FLT_MAX
#include <raylib.h>
#include <raymath.h>
#include "components.h"
#include "camview.h"
#include "shader.h"
#include "meshtools.h"

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

// Function Declarations
void Initialise_Window();
Shader LoadLightingShader(void);
Model LoadObject(void);
RayCollision CheckMouseCollision(Ray ray, Model model);
void DrawMouseCollision(RayCollision collision);
Model TransformModel(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, float scale);
Matrix MatrixRotateFromQuaternion(Quaternion q); // Custom function

// Initialize Window
void Initialise_Window() {
    int screenWidth = 1200, screenHeight = 800;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Borb CAM Slicer");
    Image Icon = LoadImage("src/Logo-Light.png");
    SetWindowIcon(Icon);
    UnloadImage(Icon);
    SetTargetFPS(60);
}

// Load Lighting Shader
Shader LoadLightingShader(void) {
    Shader shader = LoadShader(
        TextFormat("resources/shaders/glsl%i/lighting.vs", GLSL_VERSION),
        TextFormat("resources/shaders/glsl%i/lighting.fs", GLSL_VERSION)
    );
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
    int ambientLoc = GetShaderLocation(shader, "ambient");
    SetShaderValue(shader, ambientLoc, (float[4]){ 0.1f, 0.1f, 0.1f, 1.0f }, SHADER_UNIFORM_VEC4);
    return shader;
}

// Load 3D Object
Model LoadObject(void) {
    return LoadModel(MODEL_PATH);
}

// Apply Transformations to Model
Model TransformModel(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, float scale) {
    Matrix transform = MatrixMultiply(
        MatrixMultiply(
            MatrixRotate(rotationAxis, rotationAngle),  // Rotation
            MatrixScale(scale, scale, scale)           // Scale
        ),
        MatrixTranslate(position.x, position.y, position.z) // Translation
    );

    model.transform = transform;
    return model;
}

// Convert Quaternion to Rotation Matrix
Matrix MatrixRotateFromQuaternion(Quaternion q) {
    return QuaternionToMatrix(q);  // Use raymath's function to convert Quaternion to Matrix
}

// Check Mouse Collision (With Model Surface)
RayCollision CheckMouseCollision(Ray ray, Model model) {
    RayCollision closestCollision = { 0 };
    closestCollision.hit = false;
    closestCollision.distance = FLT_MAX;  // Fix: Use float.h for FLT_MAX

    // Iterate over each mesh in the model
    for (int i = 0; i < model.meshCount; i++) {
        RayCollision collision = GetRayCollisionMesh(ray, model.meshes[i], model.transform);

        if (collision.hit && collision.distance < closestCollision.distance) {
            closestCollision = collision;  // Store closest hit
        }
    }

    return closestCollision;
}

// Draw Mouse Collision Effects (Cube aligns with normal)
void DrawMouseCollision(RayCollision collision) {
    if (collision.hit) {
        // Define cube size
        float cubeSize = 0.3f;

        // Compute rotation from default cube orientation to normal direction
        Vector3 up = { 0.0f, 1.0f, 0.0f };  // Default "up" direction for the cube
        Quaternion rotationQuat = QuaternionFromVector3ToVector3(up, collision.normal);
        Matrix rotationMatrix = MatrixRotateFromQuaternion(rotationQuat); // Fix: Use custom function

        // Create transformation matrix for the cube
        Matrix cubeTransform = MatrixMultiply(rotationMatrix, MatrixTranslate(collision.point.x, collision.point.y, collision.point.z));

        // Apply transformation to draw the cube correctly
        rlPushMatrix();
        rlMultMatrixf(MatrixToFloat(cubeTransform));  // Apply transformation
        DrawCube(Vector3Zero(), cubeSize, cubeSize, cubeSize, ORANGE);
        DrawCubeWires(Vector3Zero(), cubeSize, cubeSize, cubeSize, RED);
        rlPopMatrix();

        // Draw normal as a red line
        Vector3 normalEnd = Vector3Add(collision.point, collision.normal);
        DrawLine3D(collision.point, normalEnd, RED);

        // Display Debug Info
        DrawText(TextFormat("Hit Position: %.2f, %.2f, %.2f", 
                            collision.point.x, 
                            collision.point.y, 
                            collision.point.z), 10, 70, 10, WHITE);

        DrawText(TextFormat("Normal: %.2f, %.2f, %.2f", 
                            collision.normal.x, 
                            collision.normal.y, 
                            collision.normal.z), 10, 85, 10, WHITE);
    }
}

// Main Function
int main() {
    Initialise_Window();

    camview Camera;
    Camera.Initialise_Camera();

    shader viewShader;
    Shader shader = LoadLightingShader();

    int Light_Count = 4;
    float radius = 1000;
    Light* lights = viewShader.SetupLights(shader, (Vector3){0.0f, 0.0f, 0.0f}, radius, Light_Count);

    Model model = LoadObject();
    model.materials[0].shader = shader;

    Vector3 modelPosition = { 0.0f, 0.0f, 0.0f };
    Vector3 modelRotation = { 1.0f, 0.0f, 0.0f };
    float rotationAngle = -PI / 2;
    float modelScale = 0.010f;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Update Camera Controls
        float zoomFactor = Vector3Distance(Camera.camera.position, Camera.camera.target) * 0.1f;
        Camera.UpdateCameraControls(&Camera.camera, zoomFactor);

        float cameraPos[3] = { Camera.camera.position.x, Camera.camera.position.y, Camera.camera.position.z };
        SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

        for (int i = 0; i < Light_Count; i++) UpdateLightValues(shader, lights[i]);

        // Transform the Model
        model = TransformModel(model, modelPosition, modelRotation, rotationAngle, modelScale);

        // Get Mouse Ray
        Ray ray = GetScreenToWorldRay(GetMousePosition(), Camera.camera);

        // Check Collision with Model Surface
        RayCollision collision = CheckMouseCollision(ray, model);

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode3D(Camera.camera);
        
        BeginShaderMode(shader);
        DrawPlane(Vector3Zero(), (Vector2){ 10.0f, 10.0f }, WHITE);
        DrawModel(model, (Vector3){0, 0, 0}, 1, WHITE);
        EndShaderMode();

        for (int i = 0; i < Light_Count; i++) {
            if (lights[i].enabled) DrawSphereEx(lights[i].position, 0.2f, 8, 8, lights[i].color);
            else DrawSphereWires(lights[i].position, 0.2f, 8, 8, ColorAlpha(lights[i].color, 0.3f));
        }

        DrawGrid(10, 1.0f);

        // Draw Mouse Collision (on actual model surface)
        DrawMouseCollision(collision);

        EndMode3D();

        viewShader.UpdateLights(lights, (Vector3){0.0f, 300, 0.0f}, radius, Light_Count, 0.1);

        DrawFPS(10, 10);

        EndDrawing();
    }

    UnloadModel(model);
    CloseWindow();
    return 0;
}
