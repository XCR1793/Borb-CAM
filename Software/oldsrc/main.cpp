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
    Vector3 position = {0.0f, 0.0f, 0.0f};
    Vector3 rotation = {0.0f, 0.0f, 0.0f}; // Rotation in degrees
    float scale = 1.0f;
};

// Global Variables
std::vector<LoadedModel> loadedModels;
Rectangle dropBox = {50, 50, 300, 100};  // Drag-and-drop box position and size
bool dropBoxActive = false;

int selectedModelIndex = -1; // Currently selected model

// UI Input Buffers
char posX[10] = "0.0", posY[10] = "0.0", posZ[10] = "0.0";
char rotA[10] = "0.0", rotB[10] = "0.0", rotC[10] = "0.0";
char scaleInput[10] = "1.0";

// Function Declarations
void Initialise_Window();
Shader LoadLightingShader(void);
Model LoadObject(const char *filePath);
void DrawDropBox();
void DrawModelList();
void HandleFileDrop();
void RemoveModel(int index);
void DrawTransformUI();
void DrawBaseGrid();

// Function Implementations

void Initialise_Window()
{
    int screenWidth = 1200, screenHeight = 800;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Borb CAM Slicer");
    Image Icon = LoadImage("src/Logo-Light.png");
    SetWindowIcon(Icon);
    UnloadImage(Icon);
    SetTargetFPS(60);
}

// Load Lighting Shader
Shader LoadLightingShader(void)
{
    Shader shader = LoadShader(
        TextFormat("resources/shaders/glsl%i/lighting.vs", GLSL_VERSION),
        TextFormat("resources/shaders/glsl%i/lighting.fs", GLSL_VERSION));

    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
    
    // Set ambient lighting
    int ambientLoc = GetShaderLocation(shader, "ambient");
    float ambientLight[4] = {0.3f, 0.3f, 0.3f, 1.0f}; // Increased ambient intensity
    SetShaderValue(shader, ambientLoc, ambientLight, SHADER_UNIFORM_VEC4);

    return shader;
}

// Load 3D Object
Model LoadObject(const char *filePath)
{
    return LoadModel(filePath);
}

// Handle File Drop
void HandleFileDrop()
{
    if (IsFileDropped())
    {
        FilePathList droppedFiles = LoadDroppedFiles();

        if (droppedFiles.count > 0)
        {
            for (int i = 0; i < droppedFiles.count; i++)
            {
                if (IsFileExtension(droppedFiles.paths[i], ".obj"))
                {
                    Model newModel = LoadObject(droppedFiles.paths[i]);
                    loadedModels.push_back({newModel, GetFileName(droppedFiles.paths[i])});
                }
            }
        }

        UnloadDroppedFiles(droppedFiles);
    }
}

// Draw Drag and Drop Box
void DrawDropBox()
{
    Color boxColor = dropBoxActive ? BLUE : LIGHTGRAY;
    DrawRectangleRec(dropBox, Fade(boxColor, 0.5f));
    DrawRectangleLinesEx(dropBox, 2, boxColor);

    const char *message = dropBoxActive ? "Drop Here!" : "Drag and Drop Models Here";
    int textWidth = MeasureText(message, 20);
    DrawText(message, dropBox.x + (dropBox.width / 2 - textWidth / 2), dropBox.y + 35, 20, BLACK);

    if (CheckCollisionPointRec(GetMousePosition(), dropBox))
    {
        dropBoxActive = true;
    }
    else
    {
        dropBoxActive = false;
    }
}

// Draw List of Loaded Models
void DrawModelList()
{
    int startY = 180;
    for (size_t i = 0; i < loadedModels.size(); i++)
    {
        int yPos = startY + (30 * i);
        Color textColor = (i == selectedModelIndex) ? YELLOW : WHITE;

        Rectangle nameBox = {60, yPos, 200, 20};
        if (CheckCollisionPointRec(GetMousePosition(), nameBox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            selectedModelIndex = i;
            sprintf(posX, "%.2f", loadedModels[i].position.x);
            sprintf(posY, "%.2f", loadedModels[i].position.y);
            sprintf(posZ, "%.2f", loadedModels[i].position.z);
            sprintf(rotA, "%.2f", loadedModels[i].rotation.x);
            sprintf(rotB, "%.2f", loadedModels[i].rotation.y);
            sprintf(rotC, "%.2f", loadedModels[i].rotation.z);
            sprintf(scaleInput, "%.2f", loadedModels[i].scale);
        }

        DrawText(loadedModels[i].name.c_str(), nameBox.x, nameBox.y, 20, textColor);
    }
}

// Remove Model
void RemoveModel(int index)
{
    if (index >= 0 && index < loadedModels.size())
    {
        UnloadModel(loadedModels[index].model);
        loadedModels.erase(loadedModels.begin() + index);
    }
}

// Draw UI for Position, Rotation, and Scale
void DrawTransformUI()
{
    if (selectedModelIndex == -1)
        return;

    LoadedModel &model = loadedModels[selectedModelIndex];

    DrawText("Position", 400, 180, 20, WHITE);
    GuiTextBox((Rectangle){400, 210, 80, 20}, posX, 10, true);
    GuiTextBox((Rectangle){490, 210, 80, 20}, posY, 10, true);
    GuiTextBox((Rectangle){580, 210, 80, 20}, posZ, 10, true);

    DrawText("Rotation", 400, 250, 20, WHITE);
    GuiTextBox((Rectangle){400, 280, 80, 20}, rotA, 10, true);
    GuiTextBox((Rectangle){490, 280, 80, 20}, rotB, 10, true);
    GuiTextBox((Rectangle){580, 280, 80, 20}, rotC, 10, true);

    DrawText("Scale", 400, 320, 20, WHITE);
    GuiTextBox((Rectangle){400, 350, 80, 20}, scaleInput, 10, true);

    // Apply changes
    model.position = (Vector3){atof(posX), atof(posY), atof(posZ)};
    model.rotation = (Vector3){atof(rotA), atof(rotB), atof(rotC)};
    model.scale = atof(scaleInput);
}

// Draw the base grid
void DrawBaseGrid()
{
    DrawGrid(20, 1.0f); // 20 lines spaced 1 unit apart
}



// Main Function
int main()
{
    Initialise_Window();
    camview Camera;
    Camera.Initialise_Camera();

    Shader shader = LoadLightingShader();
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        HandleFileDrop();

        float zoomFactor = Vector3Distance(Camera.camera.position, Camera.camera.target) * 0.1f;
        Camera.UpdateCameraControls(&Camera.camera, zoomFactor);

        float cameraPos[3] = { Camera.camera.position.x, Camera.camera.position.y, Camera.camera.position.z };
        SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode3D(Camera.camera);
        BeginShaderMode(shader);

        DrawBaseGrid(); // Draws the grid on the ground

        for (auto &modelData : loadedModels)
        {
            DrawModelEx(modelData.model, modelData.position, (Vector3){1, 0, 0}, modelData.rotation.x, (Vector3){modelData.scale, modelData.scale, modelData.scale}, WHITE);
        }

        EndShaderMode();
        EndMode3D();

        DrawDropBox();
        DrawModelList();
        DrawTransformUI();
        DrawFPS(10, 10);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
