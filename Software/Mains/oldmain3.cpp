#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
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

#define GLSL_VERSION 330

// Model Data Structure
struct LoadedModel {
    Model model;
    std::string name;
};

// Global Variables
std::vector<LoadedModel> loadedModels;
Rectangle dropBox = { 50, 50, 300, 100 };  // Drag-and-drop box position and size
bool dropBoxActive = false;

// Function Declarations
void Initialise_Window();
Shader LoadLightingShader(void);
Model LoadObject(const char* filePath);
void DrawDropBox();
void DrawModelList();
void HandleFileDrop();
void RemoveModel(int index);

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
Model LoadObject(const char* filePath) {
    return LoadModel(filePath);
}

// Handle File Drop
void HandleFileDrop() {
    if (IsFileDropped()) {
        FilePathList droppedFiles = LoadDroppedFiles();

        if (droppedFiles.count > 0) {
            for (int i = 0; i < droppedFiles.count; i++) {
                if (IsFileExtension(droppedFiles.paths[i], ".obj")) {
                    Model newModel = LoadObject(droppedFiles.paths[i]);
                    loadedModels.push_back({ newModel, GetFileName(droppedFiles.paths[i]) });
                }
            }
        }

        UnloadDroppedFiles(droppedFiles);
    }
}

// Draw Drag and Drop Box
void DrawDropBox() {
    // Highlight box if hovering or active
    Color boxColor = dropBoxActive ? BLUE : LIGHTGRAY;
    DrawRectangleRec(dropBox, Fade(boxColor, 0.5f));
    DrawRectangleLinesEx(dropBox, 2, boxColor);
    
    // Draw text inside box
    const char* message = dropBoxActive ? "Drop Here!" : "Drag and Drop Models Here";
    int textWidth = MeasureText(message, 20);
    DrawText(message, dropBox.x + (dropBox.width / 2 - textWidth / 2), dropBox.y + 35, 20, BLACK);
    
    // Activate drop area highlight
    if (CheckCollisionPointRec(GetMousePosition(), dropBox)) {
        dropBoxActive = true;
    } else {
        dropBoxActive = false;
    }
}

// Draw List of Loaded Models with Remove Button
void DrawModelList() {
    int startY = 180;
    
    for (size_t i = 0; i < loadedModels.size(); i++) {
        int yPos = startY + (30 * i);
        
        DrawText(loadedModels[i].name.c_str(), 60, yPos, 20, WHITE);
        
        Rectangle removeButton = { 270, yPos, 20, 20 };
        DrawRectangleRec(removeButton, RED);
        DrawText("X", removeButton.x + 5, removeButton.y + 2, 16, WHITE);

        // Remove if clicked
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), removeButton)) {
            RemoveModel(i);
            break;
        }
    }
}

// Remove Model from List
void RemoveModel(int index) {
    if (index >= 0 && index < loadedModels.size()) {
        UnloadModel(loadedModels[index].model);
        loadedModels.erase(loadedModels.begin() + index);
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
    Light* lights = viewShader.SetupLights(shader, (Vector3){ 0.0f, 0.0f, 0.0f }, radius, Light_Count);

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Handle File Drop
        HandleFileDrop();

        // Update Camera Controls
        float zoomFactor = Vector3Distance(Camera.camera.position, Camera.camera.target) * 0.1f;
        Camera.UpdateCameraControls(&Camera.camera, zoomFactor);

        float cameraPos[3] = { Camera.camera.position.x, Camera.camera.position.y, Camera.camera.position.z };
        SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

        for (int i = 0; i < Light_Count; i++) UpdateLightValues(shader, lights[i]);

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode3D(Camera.camera);
        BeginShaderMode(shader);

        DrawPlane(Vector3Zero(), (Vector2){ 10.0f, 10.0f }, WHITE);

        // Draw all loaded models
        for (const auto& modelData : loadedModels) {
            DrawModel(modelData.model, (Vector3){ 0, 0, 0 }, 1, WHITE);
        }

        EndShaderMode();

        for (int i = 0; i < Light_Count; i++) {
            if (lights[i].enabled) DrawSphereEx(lights[i].position, 0.2f, 8, 8, lights[i].color);
            else DrawSphereWires(lights[i].position, 0.2f, 8, 8, ColorAlpha(lights[i].color, 0.3f));
        }

        DrawGrid(10, 1.0f);
        EndMode3D();

        viewShader.UpdateLights(lights, (Vector3){ 0.0f, 300, 0.0f }, radius, Light_Count, 0.1);

        // UI Elements
        DrawDropBox();
        DrawModelList();
        DrawFPS(10, 10);

        EndDrawing();
    }

    // Unload all models before exiting
    for (auto& modelData : loadedModels) {
        UnloadModel(modelData.model);
    }

    CloseWindow();
    return 0;
}
