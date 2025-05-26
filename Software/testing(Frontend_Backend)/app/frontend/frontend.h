#ifndef FRONTEND_H
#define FRONTEND_H

#include <vector>
#include <raylib.h>
#include <raymath.h>
#include <appmanagement.h>
#include <meshmanagement.h>
#include <pathing.h>
#include <slice.h>
#include <backend.h>

class Frontend {
public:
    Frontend();
    ~Frontend();

    void Init();
    void Update();
    void Render();
    bool ShouldClose();
    void DebugInfo();

private:
    void AnimateToolpath(float deltaTime);
    void DrawToolpath();
    void ResetToolpath();

    app window_;
    Camera camera_;
    Shader shader_;
    mesh models_;
    Model currentModel_;
    Model newModel_;
    BoundingBox cullBox_;

    Backend backend_;
    bool backend_done_ = false;

    std::vector<Line> flatToolpath_;
    size_t currentLineIndex_ = 0;
    float segmentProgress_ = 0.0f;
    std::chrono::high_resolution_clock::time_point lastUpdate_;

    step currentStep_ = (step)-1;
    std::string currentStepText_ = "Idle";

    slice slicer;

    void SetupBackend();
};

#endif // FRONTEND_H
