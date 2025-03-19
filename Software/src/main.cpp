#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

int main() {
    // Initialization
    const int screenWidth = 600;
    const int screenHeight = 400;
    InitWindow(screenWidth, screenHeight, "Raylib + Raygui Example");
    SetTargetFPS(60);

    // GUI Elements
    bool buttonPressed = false;
    float sliderValue = 50.0f;
    char textInput[32] = "Type here";
    bool textBoxEditMode = false;

    // Main Loop
    while (!WindowShouldClose()) {
        // GUI Interaction
        if (GuiButton((Rectangle){ 50, 50, 100, 30 }, "Press Me")) {
            buttonPressed = !buttonPressed;
        }
        sliderValue = GuiSlider((Rectangle){ 50, 100, 200, 20 }, "Min", "Max", sliderValue, 0, 100);
        if (GuiTextBox((Rectangle){ 50, 150, 200, 30 }, textInput, 32, textBoxEditMode)) {
            textBoxEditMode = !textBoxEditMode;
        }

        // Drawing
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Display Outputs
        DrawText(buttonPressed ? "Button: ON" : "Button: OFF", 300, 50, 20, buttonPressed ? GREEN : RED);
        DrawText(TextFormat("Slider Value: %.2f", sliderValue), 300, 100, 20, DARKGRAY);
        DrawText(TextFormat("Text: %s", textInput), 300, 150, 20, DARKBLUE);

        EndDrawing();
    }

    // Cleanup
    CloseWindow();
    return 0;
}
