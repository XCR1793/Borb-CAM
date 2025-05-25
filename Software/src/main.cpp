#include <frontend.h>
#include <backend.h>

#ifndef RAYGUI_IMPLEMENTATION
    #define RAYGUI_IMPLEMENTATION
    #include <raygui.h>
#endif

#ifndef RLIGHTS_IMPLEMENTATION
    #define RLIGHTS_IMPLEMENTATION
    #include <rlights.h>
#endif

int main() {
    Frontend frontend;
    Backend backend;

    frontend.Initialize();
    backend.Initialize();

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        frontend.UpdateCamera();
        backend.Update(deltaTime);
        frontend.Draw(backend);
    }

    backend.Shutdown();
    CloseWindow();
    return 0;
}
