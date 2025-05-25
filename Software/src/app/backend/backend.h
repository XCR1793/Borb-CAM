#pragma once

#include <vector>
#include <chrono>
#include <raylib.h>
#include <appmanagement.h>
#include <meshmanagement.h>
#include <pathing.h>
#include <slice.h>

class Backend {
public:
    void Initialize();
    void Update(float deltaTime);
    void UpdateToolpathIfRequested();
    void DrawModelWithToolpath();
    void DrawUI();
    void Shutdown();

private:
    app window;
    mesh models;
    path paths;
    slice slicing;
    Model currentModel;
    Model transformedModel;
    std::vector<std::vector<Line>> toolpath;
    std::vector<Line> flatToolpath;
    size_t currentLineIndex = 0;
    float segmentProgress = 0.0f;
    std::chrono::high_resolution_clock::time_point lastUpdate;
    Shader cachedShader;

    Model Scale_Model(Model model, float scale);
    Model Move_Model(Model model, Vector3 translation);
    Color LerpColor(Color a, Color b, float t);
};
