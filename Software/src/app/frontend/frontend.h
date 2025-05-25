#pragma once
#include <raylib.h>
#include "../backend/backend.h"
#include "../../core/appmanagement/appmanagement.h" // ✅ Include your app class

class Frontend {
public:
    void Initialize();
    void UpdateCamera();
    void Draw(Backend& backend);

private:
    Camera camera{};
    Shader shader{};
    app appInstance;  // ✅ Own instance of your app class
};
