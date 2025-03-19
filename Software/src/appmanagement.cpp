#include "appmanagement.h"

void app::Initialise_Window(int height, int width, int fps, const char *title, const char*logo){
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(width, height, title);
    Image Icon = LoadImage(logo);
    SetWindowIcon(Icon);
    UnloadImage(Icon);
    SetTargetFPS(fps);
}