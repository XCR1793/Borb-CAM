#include "frontend.h"
#include <chrono>

// Utility functions
Model Scale_Model(Model model, float scale) {
    for (int i = 0; i < model.meshCount; i++) {
        Mesh* mesh = &model.meshes[i];
        for (int v = 0; v < mesh->vertexCount; v++) {
            mesh->vertices[v * 3 + 0] *= scale;
            mesh->vertices[v * 3 + 1] *= scale;
            mesh->vertices[v * 3 + 2] *= scale;
        }
    }
    model.transform = MatrixIdentity();
    return model;
}

Model Move_Model(Model model, Vector3 translation) {
    for (int i = 0; i < model.meshCount; i++) {
        Mesh* mesh = &model.meshes[i];
        for (int v = 0; v < mesh->vertexCount; v++) {
            mesh->vertices[v * 3 + 0] += translation.x;
            mesh->vertices[v * 3 + 1] += translation.y;
            mesh->vertices[v * 3 + 2] += translation.z;
        }
    }
    model.transform = MatrixIdentity();
    return model;
}

Color LerpColor(Color a, Color b, float t) {
    Color result;
    result.r = (unsigned char)((1 - t) * a.r + t * b.r);
    result.g = (unsigned char)((1 - t) * a.g + t * b.g);
    result.b = (unsigned char)((1 - t) * a.b + t * b.b);
    result.a = 255;
    return result;
}

Frontend::Frontend() {}
Frontend::~Frontend() {
    models_.Stop_Models();
    CloseWindow();
}

void Frontend::Init() {
    window_.Initialise_Window(1000, 1500, 60, "Borb CAM Slicer", "src/assets/Logo-Light.png");
    camera_ = window_.Initialise_Camera({20, 20, 20}, {0, 8, 0}, {0, 1.6f, 0}, 45.0f, CAMERA_PERSPECTIVE);
    shader_ = window_.Initialise_Shader();
    window_.Initialise_Lights(shader_);
    window_.Create_File("src/output/Debug", "txt");
    window_.Add_Button(1, 40, 100, 10, 50, "Slice");

if (!FileExists("src/models/model.obj")) {
    std::cerr << "[ERROR] File not found: src/models/model.obj\n";
}

    models_.Add_Model(1, "src/models/model.obj");
    currentModel_ = models_.Ret_Model(1);
    // newModel_ = Move_Model(Scale_Model(currentModel_, 0.1f), {0, -0.5f, 0});
    newModel_ = currentModel_;  // Skip transformation for testing
    cullBox_ = { {-5, -4, -5}, {5, 0, 5} };

    SetupBackend();
    lastUpdate_ = std::chrono::high_resolution_clock::now();
}

void Frontend::SetupBackend() {
    backend_.set_model(newModel_);

    Settings settings;
    settings.Starting_Position.mode = 0;
    settings.Starting_Position.value3D = {10, 10, 0};
    settings.Ending_Position.mode = 0;
    settings.Ending_Position.value3D = {0, 10, 10};
    settings.SlicingPlane = { PI / 4, 0, 0, 0 };
    settings.SlicingDistance = 0.1f;

    backend_.set_settings(settings);
    backend_.set_culling_boxes(0, GetModelBoundingBox(newModel_));

backend_.progress_callback([this](step s, int, bool success) {
    if (s == (step)-1) {
        currentStep_ = (step)-1;
        currentStepText_ = "✅ Done";
        backend_done_ = true;
        ResetToolpath();
    } else {
        currentStep_ = s;
        std::string stepName;
        switch (s) {
            case Generate_Surface_Toolpath:         stepName = "Generate Surface Toolpath"; break;
            case Cull_Toolpath_to_Obstacle:         stepName = "Cull Toolpath to Obstacle"; break;
            case Generate_Start_End_Rays:           stepName = "Generate AABB Rays"; break;
            case Optimise_Start_End_Positions:      stepName = "Optimize Start/End Positions"; break;
            case Optimise_Start_End_Linkages:       stepName = "Optimize Linkages"; break;
            case Add_Custom_Start_End_Positions:    stepName = "Add Custom Start/End"; break;
            case Restrict_Max_Angle_per_Move:       stepName = "Restrict Max Angle"; break;
            default: stepName = "Unknown Step"; break;
        }

        if (!success) {
            std::cerr << "[Frontend] ❌ Step failed: " << stepName << "\n";
            currentStepText_ = "❌ Failed: " + stepName;
        } else {
            std::cout << "[Frontend] ✅ Step completed: " << stepName << "\n";
            currentStepText_ = stepName;
        }
    }
});

}

void Frontend::ResetToolpath() {
    flatToolpath_.clear();
    auto toolpath_ptr = backend_.return_toolpath();
    if (!toolpath_ptr || toolpath_ptr->empty()) {
        currentStepText_ = "Toolpath empty!";
        return;
    }

    for (const auto& segment : *toolpath_ptr) {
        flatToolpath_.insert(flatToolpath_.end(), segment.begin(), segment.end());
    }

    currentLineIndex_ = 0;
    segmentProgress_ = 0.0f;
}

void Frontend::Update() {
    models_.Sha_Model(1, shader_);
    window_.Update_Camera(&camera_);

    if (window_.Ret_Button(1)) {
        backend_.clear_schedule();
        backend_.schedule(Generate_Surface_Toolpath);
        backend_.schedule(Cull_Toolpath_to_Obstacle, 0);
        backend_.schedule(Generate_Start_End_Rays, 0);
        backend_.schedule(Optimise_Start_End_Positions);
        backend_.schedule(Optimise_Start_End_Linkages);
        backend_.schedule(Add_Custom_Start_End_Positions);
        // backend_.schedule(Restrict_Max_Angle_per_Move);
        backend_done_ = false;
        backend_.run();
        currentStepText_ = "Starting...";
    }

    // 💡 Press Q for debug output
    if (IsKeyPressed(KEY_Q)) {
        DebugInfo();
    }
}


void Frontend::Render() {
    BeginDrawing();
    ClearBackground(DARKGRAY);

    BeginMode3D(camera_);
    DrawModel(newModel_, {0, 0, 0}, 1.0f, WHITE);                     // <--- NEW: Renders the model
    // DrawMeshTriangles(newModel_, SKYBLUE);  // or any color
    DrawBoundingBox(GetModelBoundingBox(newModel_), GREEN);
    DrawToolpath();

    if (!flatToolpath_.empty()) {
        auto now = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(now - lastUpdate_).count();
        lastUpdate_ = now;
        AnimateToolpath(deltaTime);
    }

    // Slice overlays
    slicer.Comp_Axis_Guides_3D();
    slicer.Comp_Ground_Grid();

    EndMode3D();
    window_.Run_Buttons();

    DrawFPS(10, 10);
    DrawText(TextFormat("Schedule State: %s", currentStepText_.c_str()), 10, 90, 20, LIGHTGRAY);

    EndDrawing();
}



void Frontend::DrawToolpath() {
    Color gradientStart = BLUE;
    Color gradientEnd = RED;
    auto toolpath_ptr = backend_.return_toolpath();

    for (const auto& segment : *toolpath_ptr) {
        size_t count = segment.size();
        for (size_t i = 0; i < count; ++i) {
            float t = (count <= 1) ? 0.0f : (float)i / (float)(count - 1);
            Color lineColor = LerpColor(gradientStart, gradientEnd, t);
            DrawLine3D(segment[i].startLinePoint.Position, segment[i].endLinePoint.Position, lineColor);
        }
    }
}

void Frontend::AnimateToolpath(float deltaTime) {
    const float worldSpeed = 5.0f;
    while (deltaTime > 0.0f && !flatToolpath_.empty()) {
        Line& line = flatToolpath_[currentLineIndex_];
        float length = Vector3Distance(line.startLinePoint.Position, line.endLinePoint.Position);
        if (length < 0.0001f) {
            currentLineIndex_ = (currentLineIndex_ + 1) % flatToolpath_.size();
            segmentProgress_ = 0.0f;
            continue;
        }

        float duration = length / worldSpeed;
        float progressInc = deltaTime / duration;
        segmentProgress_ += progressInc;

        if (segmentProgress_ >= 1.0f) {
            deltaTime = (segmentProgress_ - 1.0f) * duration;
            segmentProgress_ = 0.0f;
            currentLineIndex_ = (currentLineIndex_ + 1) % flatToolpath_.size();
        } else {
            deltaTime = 0.0f;
        }
    }

    Line& active = flatToolpath_[currentLineIndex_];
    Vector3 pos = Vector3Lerp(active.startLinePoint.Position, active.endLinePoint.Position, segmentProgress_);
    Vector3 norm = Vector3Normalize(Vector3Lerp(active.startLinePoint.Normal, active.endLinePoint.Normal, segmentProgress_));
    Vector3 endPos = Vector3Add(pos, Vector3Scale(norm, 0.3f));

    DrawCylinderEx(pos, endPos, 0.05f, 0.05f, 8, YELLOW);
    DrawText(TextFormat("Line: %d / %d", (int)currentLineIndex_, (int)flatToolpath_.size()), 10, 50, 20, WHITE);
    DrawText(TextFormat("Segment progress: %.2f", segmentProgress_), 10, 70, 20, WHITE);
}

bool Frontend::ShouldClose() {
    return WindowShouldClose();
}

void Frontend::DebugInfo() {
    std::cout << "\n====== FRONTEND DEBUG INFO ======\n";

    std::cout << "[Model] currentModel.meshCount = " << currentModel_.meshCount << "\n";
    std::cout << "[Model] newModel.meshCount     = " << newModel_.meshCount << "\n";

    BoundingBox bb = GetModelBoundingBox(newModel_);
    std::cout << "[Model] AABB min = (" << bb.min.x << ", " << bb.min.y << ", " << bb.min.z << ")\n";
    std::cout << "[Model] AABB max = (" << bb.max.x << ", " << bb.max.y << ", " << bb.max.z << ")\n";

    if (!backend_done_) {
        std::cout << "[Frontend] ⚠️ Backend still running. Toolpath access skipped.\n";
    } else {
        auto toolpath_ptr = backend_.return_toolpath();
        if (!toolpath_ptr) {
            std::cout << "[Backend] ❌ return_toolpath() is null\n";
        } else {
            std::cout << "[Backend] ✅ return_toolpath().size() = " << toolpath_ptr->size() << "\n";
            size_t lineCount = 0;
            for (const auto& seg : *toolpath_ptr) lineCount += seg.size();
            std::cout << "[Backend] Flattened lines: " << lineCount << "\n";
        }

        std::cout << "[Frontend] flatToolpath_.size() = " << flatToolpath_.size() << "\n";
    }

    std::cout << "[Frontend] currentStep_: " << currentStep_ << "\n";
    std::cout << "[Frontend] currentStepText_: " << currentStepText_ << "\n";
    std::cout << "=================================\n";
}
