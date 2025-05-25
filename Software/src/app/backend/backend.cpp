#include "backend.h"
#include <iostream>

void Backend::Initialize() {
    // window.Initialise_Window(1000, 1500, 60, "Borb CAM Slicer", "src/assets/Logo-Light.png");
    cachedShader = window.Initialise_Shader();
    window.Initialise_Lights(cachedShader);
    window.Create_File("src/output/Debug", "txt");
    window.Add_Button(1, 40, 100, 10, 50, "Slice");

    models.Add_Model(1, "src/models/model.obj");
    currentModel = models.Ret_Model(1);
    transformedModel = Move_Model(Scale_Model(currentModel, 0.1f), { 0, -0.5f, 0 });

    slicing
        .Set_Slicing_Plane({ PI / 4, 0 }, 0)
        .Set_Slicing_Distance(0.1f)
        .Set_Starting_Position({ .mode = 0, .value3D = { 10, 10, 0 } })
        .Set_Ending_Position({ .mode = 0, .value3D = { 0, 10, 10 } });

    paths.Create_File("src/output/OwO", "nc");
    lastUpdate = std::chrono::high_resolution_clock::now();
}

void Backend::Update(float deltaTime) {
    const float worldSpeed = 5.0f;

    if (flatToolpath.empty()) return;

    auto now = std::chrono::high_resolution_clock::now();
    deltaTime = std::chrono::duration<float>(now - lastUpdate).count();
    lastUpdate = now;

    while (deltaTime > 0.0f && !flatToolpath.empty()) {
        Line& line = flatToolpath[currentLineIndex];
        float segmentLength = Vector3Distance(line.startLinePoint.Position, line.endLinePoint.Position);
        if (segmentLength < 0.0001f) {
            currentLineIndex = (currentLineIndex + 1) % flatToolpath.size();
            segmentProgress = 0.0f;
            continue;
        }

        float duration = segmentLength / worldSpeed;
        float progressIncrement = deltaTime / duration;
        segmentProgress += progressIncrement;

        if (segmentProgress >= 1.0f) {
            deltaTime = (segmentProgress - 1.0f) * duration;
            segmentProgress = 0.0f;
            currentLineIndex = (currentLineIndex + 1) % flatToolpath.size();
        } else {
            deltaTime = 0.0f;
        }
    }
}

void Backend::UpdateToolpathIfRequested() {
    if (window.Ret_Button(1)) {
        slicing.Clear_Toolpath();
        slicing.Generate_Surface_Toolpath(transformedModel);
        slicing.Cull_Toolpath_by_Box({ { -5, -4, -5 }, { 5, 0, 5 } });
        slicing.Optimise_Start_End_Positions();
        slicing.Apply_AABB_Rays(GetModelBoundingBox(currentModel));
        slicing.Optimise_Start_End_Linkages();
        slicing.Add_Start_End_Positions();
        slicing.Interpolate_Max_Angle_Displacement();
        toolpath = slicing.Return_Toolpath();

        flatToolpath.clear();
        for (const auto& segment : toolpath)
            flatToolpath.insert(flatToolpath.end(), segment.begin(), segment.end());

        currentLineIndex = 0;
        segmentProgress = 0.0f;
        lastUpdate = std::chrono::high_resolution_clock::now();
    }
}

void Backend::DrawModelWithToolpath() {
    models.Sha_Model(1, cachedShader);
    DrawBoundingBox(GetModelBoundingBox(transformedModel), GREEN);

    Color startColor = BLUE;
    Color endColor = RED;

    for (const auto& paths : toolpath) {
        size_t count = paths.size();
        for (size_t i = 0; i < count; ++i) {
            float t = (count <= 1) ? 0.0f : (float)i / (float)(count - 1);
            DrawLine3D(paths[i].startLinePoint.Position, paths[i].endLinePoint.Position, LerpColor(startColor, endColor, t));
        }
    }

    if (!flatToolpath.empty()) {
        Line& activeLine = flatToolpath[currentLineIndex];
        Vector3 pos = Vector3Lerp(activeLine.startLinePoint.Position, activeLine.endLinePoint.Position, segmentProgress);
        Vector3 norm = Vector3Normalize(Vector3Lerp(activeLine.startLinePoint.Normal, activeLine.endLinePoint.Normal, segmentProgress));
        Vector3 endPos = Vector3Add(pos, Vector3Scale(norm, 0.3f));
        DrawCylinderEx(pos, endPos, 0.05f, 0.05f, 8, YELLOW);
    }

    slicing.Comp_Axis_Guides_3D();
    slicing.Comp_Ground_Grid();
}

void Backend::DrawUI() {
    window.Run_Buttons();
}

void Backend::Shutdown() {
    models.Stop_Models();
}

Model Backend::Scale_Model(Model model, float scale) {
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

Model Backend::Move_Model(Model model, Vector3 translation) {
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

Color Backend::LerpColor(Color a, Color b, float t) {
    return {
        (unsigned char)((1 - t) * a.r + t * b.r),
        (unsigned char)((1 - t) * a.g + t * b.g),
        (unsigned char)((1 - t) * a.b + t * b.b),
        255
    };
}
