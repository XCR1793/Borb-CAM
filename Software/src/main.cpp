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
    auto backend_ptr = std::make_shared<backend>();
    frontend front(backend_ptr);

    // Optionally wait for window close in main thread (if needed)
    while (!WindowShouldClose()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
