#include "frontend.h"

frontend::frontend(std::shared_ptr<backend> b)
    : backend_(b), mainThread(&frontend::run, this) {}

frontend::~frontend() {
    if (mainThread.joinable()) {
        mainThread.join();
    }
}

Model Scale_Model(Model model, float scale) {
    for (int i = 0; i < model.meshCount; i++) {
        Mesh *mesh = &model.meshes[i];
        for (int v = 0; v < mesh->vertexCount; v++) {
            mesh->vertices[v * 3 + 0] *= scale; // x
            mesh->vertices[v * 3 + 1] *= scale; // y
            mesh->vertices[v * 3 + 2] *= scale; // z
        }
    }
    model.transform = MatrixIdentity(); // Reset transform (mesh is already scaled)
    return model;
}

Model Move_Model(Model model, Vector3 translation) {
    for (int i = 0; i < model.meshCount; i++) {
        Mesh *mesh = &model.meshes[i];
        for (int v = 0; v < mesh->vertexCount; v++) {
            mesh->vertices[v * 3 + 0] += translation.x;
            mesh->vertices[v * 3 + 1] += translation.y;
            mesh->vertices[v * 3 + 2] += translation.z;
        }
    }
    model.transform = MatrixIdentity(); // Reset transform (mesh is already moved)
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

void frontend::run(){
    window.Initialise_Window(gui_settings.window_height, gui_settings.window_width, gui_settings.window_framerate,
                             gui_settings.window_title, gui_settings.logo_file_location);
    Camera camera = window.Initialise_Camera({20.0f, 20.0f, 20.0f}, {0.0f, 8.0f, 0.0f}, {0.0f, 1.6f, 0.0f}, 45.0f, CAMERA_PERSPECTIVE);
    Shader shader = window.Initialise_Shader();
    window.Initialise_Lights(shader);
    gui_settings.font = LoadFont("src/assets/Inter_28pt-Light.ttf");

    Initialise_Inputs();

    mesh models;
    models.Add_Model(1, "src/models/model.obj");
    Model currentmodel = models.Ret_Model(1);
    Model newmodel = Move_Model(Scale_Model(currentmodel, 0.1f), {0, -0.5f, 0});
    BoundingBox cullBox = { Vector3{-5, -4, -5}, Vector3{5, 0, 5} };

    path paths;
    paths.Create_File("src/output/OwO", "nc");

    std::vector<std::pair<Vector3, Vector3>> pathPositions;
    std::vector<std::vector<Line>> toolpath;
    std::vector<Line> flatToolpath;

    slice slicing;
    backend backend_;
    backend_.slicing_tools
        .Set_Slicing_Plane({PI/4, 0}, 0)
        .Set_Slicing_Distance(0.1f)
        .Set_Starting_Position({.mode = 0, .value3D = {10, 10, 0}})
        .Set_Ending_Position({.mode = 0, .value3D = {0, 10, 10}});

    // === Model Transform & Slicing Config ===
    static float scale = 0.1f;
    static Vector3 offset = { 0, -0.5f, 0 };
    static Vector2 sliceRot = { PI / 4.0f, 0 };
    static float planeOffset = 0.0f;
    int rotationMode = 0;
    bool modelNeedsUpdate = true;

    // === UI Toggles ===
    bool modelVisible = true;
    bool showPath = true;
    bool showCombinedPath = true;
    bool load_done_toolpath = false;

    size_t currentLineIndex = 0;
    auto lastUpdate = std::chrono::high_resolution_clock::now();

    while (!WindowShouldClose()) {
        models.Sha_Model(1, shader);
        window.Update_Camera(&camera);

        BeginDrawing();
        ClearBackground(DARKGRAY);

        Draw_Mini_Panel(mini_panels);

        // === Read GUI Inputs ===
        auto& modelPanel = mini_panels[0];     // Assuming index 0 is Model Settings
        auto& miscPanel  = mini_panels[1];     // Assuming index 1 is Misc Buttons

        offset.x     = modelPanel.Button_Bar[0].value;
        offset.y     = modelPanel.Button_Bar[1].value;
        offset.z     = modelPanel.Button_Bar[2].value;
        scale        = modelPanel.Button_Bar[3].value;
        Vector3 tool_rot = {
            modelPanel.Button_Bar[4].value,
            modelPanel.Button_Bar[5].value,
            modelPanel.Button_Bar[6].value
        };
        planeOffset  = modelPanel.Button_Bar[7].value;
        sliceRot.x   = modelPanel.Button_Bar[8].value;
        sliceRot.y   = modelPanel.Button_Bar[9].value;

        if (miscPanel.Button_Bar[0].value > 0.5f) { // Slice
            miscPanel.Button_Bar[0].value = 0.0f;

            backend_.clear_schedule();
            backend_.set_model(newmodel);
            backend_.schedule(Generate_Surface_Toolpath);
            backend_.schedule(Cull_Toolpath_to_Obstacle, cullBox);
            backend_.schedule(Optimise_Start_End_Positions);
            backend_.schedule(Generate_Start_End_Rays, GetModelBoundingBox(newmodel));
            backend_.schedule(Optimise_Start_End_Linkages);
            backend_.schedule(Add_Custom_Start_End_Positions);
            backend_.run_schedule();

            load_done_toolpath = false;
            currentLineIndex = 0;
            lastUpdate = std::chrono::high_resolution_clock::now();
        }

        if (miscPanel.Button_Bar[1].value > 0.5f) modelVisible = true;
        else modelVisible = false;

        if (miscPanel.Button_Bar[2].value > 0.5f) { // Slice to G-code
            miscPanel.Button_Bar[2].value = 0.0f;
            std::vector<Line> flat = backend_.slicing_tools.Toolpath_Flattener();
            if (!flat.empty()) {
                paths.Clear_File();
                paths.Reset_N();
                pathPositions.clear();
                for (const Line& line : flat) {
                    Vector3 pos = line.endLinePoint.Position;
                    Vector3 rot = models.NormalToRotation(line.endLinePoint.Normal);

                    // Get selected order
                    int rotIndex = static_cast<int>(miscPanel.Button_Bar[3].value) - 1;
                    std::string order = "ABC"; // Default fallback

                    if (!miscPanel.Button_Bar[3].string_values.empty()) {
                        int rotIndex = static_cast<int>(miscPanel.Button_Bar[3].value) - 1;
                        rotIndex = std::max(0, std::min(rotIndex, (int)miscPanel.Button_Bar[3].string_values.size() - 1));
                        order = miscPanel.Button_Bar[3].string_values[rotIndex];
                    }

                    Vector3 reordered = {0, 0, 0};

                    for (int i = 0; i < 3; ++i) {
                        float val = 0.0f;
                        switch (order[i]) {
                            case 'A': val = rot.x; break;
                            case 'B': val = rot.y; break;
                            case 'C': val = rot.z; break;
                        }
                    
                        if (i == 0) reordered.x = val;
                        else if (i == 1) reordered.y = val;
                        else if (i == 2) reordered.z = val;
                    }

                    pathPositions.push_back({ pos, reordered });

                }
                paths.Path_to_Gcode1(pathPositions);
            }
        }


        rotationMode = static_cast<int>(miscPanel.Button_Bar[3].value);
        slicing.Set_Gcode_Rotation_Order(static_cast<Gcode_Rotation_Order>(rotationMode));

        showCombinedPath = miscPanel.Button_Bar[4].value > 0.5f;
        showPath         = miscPanel.Button_Bar[5].value > 0.5f;

        if (miscPanel.Button_Bar[6].value > 0.5f) {
            miscPanel.Button_Bar[6].value = 0.0f;
            flatToolpath.clear();
            std::vector<std::pair<Vector3, Vector3>> pathData;
            if (paths.Gcode_to_Path("src/output/OwO", "nc", pathData)) {
                for (size_t i = 1; i < pathData.size(); ++i) {
                    flatToolpath.push_back({
                        .startLinePoint = { pathData[i - 1].first, pathData[i - 1].second },
                        .endLinePoint   = { pathData[i].first,     pathData[i].second },
                        .type = 1
                    });
                }
            }
        }

        backend_.run();

        if (!backend_.return_run_status() && backend_.return_worker_status() && !load_done_toolpath) {
            load_done_toolpath = true;
            flatToolpath.clear();
            for (const auto& segment : backend_.return_config().Toolpath) {
                flatToolpath.insert(flatToolpath.end(), segment.begin(), segment.end());
            }
            toolpath = backend_.return_config().Toolpath;
            std::cout << "Toolpath Loaded" << std::endl;
        }

        BeginMode3D(camera);
        if (modelVisible) DrawBoundingBox(GetModelBoundingBox(newmodel), GREEN);

        Color gradientStart = BLUE;
        Color gradientEnd   = RED;

        for (const auto& segment : toolpath) {
            size_t count = segment.size();
            for (size_t i = 0; i < count; ++i) {
                float t = (count <= 1) ? 0.0f : (float)i / (float)(count - 1);
                Color lineColor = LerpColor(gradientStart, gradientEnd, t);
                DrawLine3D(segment[i].startLinePoint.Position, segment[i].endLinePoint.Position, lineColor);
            }
        }

        if (!flatToolpath.empty()) {
            const float worldSpeed = 5.0f;
            auto now = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(now - lastUpdate).count();
            lastUpdate = now;
            static float segmentProgress = 0.0f;

            while (deltaTime > 0.0f) {
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

            Line& activeLine = flatToolpath[currentLineIndex];
            Vector3 pos = Vector3Lerp(activeLine.startLinePoint.Position, activeLine.endLinePoint.Position, segmentProgress);
            Vector3 norm = Vector3Normalize(Vector3Lerp(activeLine.startLinePoint.Normal, activeLine.endLinePoint.Normal, segmentProgress));
            Vector3 endPos = Vector3Add(pos, Vector3Scale(norm, 0.3f));
            DrawCylinderEx(pos, endPos, 0.05f, 0.05f, 8, YELLOW);
            DrawText(TextFormat("Line: %d / %d", (int)currentLineIndex, (int)flatToolpath.size()), 10, 50, 20, WHITE);
            DrawText(TextFormat("Segment progress: %.2f", segmentProgress), 10, 70, 20, WHITE);
        }

        slicing.Comp_Axis_Guides_3D();
        slicing.Comp_Ground_Grid();
        EndMode3D();

        window.Run_Buttons();
        Handle_Mini_Panel(mini_panels);
        DrawText(TextFormat("%d FPS", GetFPS()), GetScreenWidth() - MeasureText(TextFormat("%d FPS", GetFPS()), 20) - 10, 10, 20, CYAN);
        EndDrawing();
    }

    models.Stop_Models();
    UnloadFont(gui_settings.font);
    CloseWindow();
}

/**##########################################
 * #            Helper Functions            #
 * ##########################################*/

void frontend::reset_int(int value){
    button_id = value;
}

int frontend::next_int(){
    button_id++;
    return button_id - 1;
}

void frontend::set_float_increment(float increment_value){
    incremental = increment_value;
}

void frontend::reset_float(){
    float_value_increment = 0;
}

float frontend::next_float(){
    return (float_value_increment += incremental) - incremental;
}

float frontend::curr_float(){
    return float_value_increment;
}

/**##########################################
 * #              GUI Helpers               #
 * ##########################################*/

void frontend::Initialise_Mini_Panel(int starting_id, Mini_Panel_Config& cfg, const std::vector<per_button_Config>& button_configs) {
    float x = cfg.start_x;
    float y = cfg.start_y;

    reset_int(starting_id);
    cfg.Button_Bar.clear();

    for (const auto& btn_cfg : button_configs) {
        Button_Mini_Panel_config bar_cfg;
        bar_cfg.title = btn_cfg.title;
        bar_cfg.increment = btn_cfg.increment;
        bar_cfg.value = btn_cfg.default_value;
        bar_cfg.buttontype = btn_cfg.buttontype;
        bar_cfg.Min = btn_cfg.Min;
        bar_cfg.Max = btn_cfg.Max;
        bar_cfg.string_values = btn_cfg.string_values;

        if (btn_cfg.buttontype == Button_Type::Value_Clamped) {
            bar_cfg.value = std::max(bar_cfg.Min, std::min(bar_cfg.value, bar_cfg.Max));
        } else if (btn_cfg.buttontype == Button_Type::Cycle) {
            bar_cfg.increment = std::max(1.0f, btn_cfg.increment); // Treat increment as number of cycle steps
            bar_cfg.value = std::max(1.0f, std::min(bar_cfg.value, bar_cfg.increment));
        }

        switch (btn_cfg.buttontype) {
            case Button_Type::Toggle:
            case Button_Type::Temp: {
                int id = next_int();
                bar_cfg.button_ids = { id };

                float button_w = cfg.button_widths;
                float button_h = cfg.button_heights;
                float button_y = y + (cfg.max_height - button_h) / 2.0f;
                float button_x = x + cfg.max_width - button_w;

                window.Add_Button(id, button_h, button_w, button_x, button_y, "Toggle");
                break;
            }

            case Button_Type::Value:
            case Button_Type::Value_Clamped:
            case Button_Type::Cycle: {
                int id_pos = next_int();
                int id_neg = next_int();
                bar_cfg.button_ids = { id_pos, id_neg };

                float spacing = gui_settings.button_spacing_x;
                float button_w = cfg.button_widths;
                float button_h = cfg.button_heights;
                float button_y = y + (cfg.max_height - button_h) / 2.0f;

                float minus_x = x + cfg.max_width - button_w;
                float plus_x  = minus_x - button_w - spacing;

                float inc_val = (btn_cfg.buttontype == Button_Type::Cycle) ? 1.0f : btn_cfg.increment;

                window.Add_Button(id_pos, button_h, button_w, plus_x,  button_y, "+",  inc_val);
                window.Add_Button(id_neg, button_h, button_w, minus_x, button_y, "-", -inc_val);
                break;
            }
        }

        cfg.Button_Bar.push_back(bar_cfg);
        y += cfg.increment_y;
    }
}

void frontend::Draw_Mini_Panel(const std::vector<Mini_Panel_Config>& panels) {
    auto& gs = gui_settings;

    for (const auto& cfg : panels) {
        float x = cfg.start_x;
        float y = cfg.start_y;
        Color current_panel_color = cfg.panel_color;

        if (cfg.Title && cfg.title) {
            DrawTextEx(gs.font, cfg.title, { x, y - gs.panel_title_height }, 20.0f, 1.0f, WHITE);
        }

        for (const auto& bar : cfg.Button_Bar) {
            DrawRectangle(x, y, cfg.max_width, cfg.max_height, current_panel_color);

            Color labelColor = WHITE;

            // Label format
            char label_text[128];
            if (bar.button_ids.size() == 1) {
                snprintf(label_text, sizeof(label_text), "%s: %s", bar.title, bar.value > 0.5f ? "ON" : "OFF");
            } else if (bar.buttontype == Button_Type::Cycle && !bar.string_values.empty()) {
                int index = static_cast<int>(bar.value) - 1;
                index = std::max(0, std::min(index, (int)bar.string_values.size() - 1));
                snprintf(label_text, sizeof(label_text), "%s: %s", bar.title, bar.string_values[index].c_str());
            } else {
                snprintf(label_text, sizeof(label_text), "%s: %.2f", bar.title, bar.value);
            }


            DrawTextEx(gs.font, label_text, { x + 10, y + (cfg.max_height / 2.0f - 10.0f) }, 18.0f, 1.0f, labelColor);

            y += cfg.increment_y;

            // Increment panel color
            current_panel_color.r = std::min(current_panel_color.r + cfg.color_increment.r, 255);
            current_panel_color.g = std::min(current_panel_color.g + cfg.color_increment.g, 255);
            current_panel_color.b = std::min(current_panel_color.b + cfg.color_increment.b, 255);
            current_panel_color.a = std::min(current_panel_color.a + cfg.color_increment.a, 255);
        }
    }
}

void frontend::Handle_Mini_Panel(std::vector<Mini_Panel_Config>& panels) {
    for (auto& cfg : panels) {
        for (auto& bar : cfg.Button_Bar) {
            if (bar.button_ids.empty()) continue;

            switch (bar.buttontype) {
                case Button_Type::Temp: {
                    int id = bar.button_ids[0];
                    if (window.Ret_Button(id)) {
                        bar.value = 1.0f;
                    } else if (bar.value > 0.0f) {
                        bar.value = 0.0f;
                    }
                    break;
                }

                case Button_Type::Toggle: {
                    int id = bar.button_ids[0];
                    if (window.Ret_Button(id)) {
                        bar.value = (bar.value == 0.0f) ? 1.0f : 0.0f;
                    }
                    break;
                }

                case Button_Type::Value:
                case Button_Type::Value_Clamped:
                case Button_Type::Cycle: {
                    int id_pos = bar.button_ids[0];
                    int id_neg = bar.button_ids[1];

                    float change = 0.0f;

                    if (window.Ret_Button(id_pos)) {
                        change += (bar.buttontype == Button_Type::Cycle) ? 1.0f : bar.increment;
                    }
                    if (window.Ret_Button(id_neg)) {
                        change -= (bar.buttontype == Button_Type::Cycle) ? 1.0f : bar.increment;
                    }

                    bar.value += change;

                    if (bar.buttontype == Button_Type::Value_Clamped) {
                        bar.value = std::max(bar.Min, std::min(bar.value, bar.Max));
                    } else if (bar.buttontype == Button_Type::Cycle) {
                        float count = std::max(1.0f, bar.increment); // interpret increment as count
                        if (bar.value > count) bar.value = 1.0f;
                        if (bar.value < 1.0f)  bar.value = count;
                    }

                    break;
                }
            }
        }
    }
}

/**##########################################
 * #           Frontend Functions           #
 * ##########################################*/

void frontend::Initialise_Inputs(){
    auto& gs = gui_settings;
    auto& mcg = mini_panel_config;

    // Header Buttons
    reset_float();
    set_float_increment(gs.button_spacing_x + gs.button_width);
    window.Add_Button(0, gs.button_height, gs.button_width, next_float(), 0, "Model");
    window.Add_Button(1, gs.button_height, gs.button_width, next_float(), 0, "Toolpath");
    window.Add_Button(2, gs.button_height, gs.button_width, next_float(), 0, "Cullbox");
    window.Add_Button(3, gs.button_height, gs.button_width, next_float(), 0, "Obstacles");

    // === Slicer & Pathing Settings Mini Panel ===
    mcg.Title = true;
    mcg.title = "Model Settings";
    mcg.start_x = 20;
    mcg.start_y = 80;
    mcg.increment_x = 0;
    mcg.increment_y = 40;
    mcg.max_height = 30;
    mcg.max_width = 250;
    mcg.button_heights = 24;
    mcg.button_widths = 30;
    mcg.panel_color = {30, 30, 40, 255};
    mcg.color_increment = {5, 5, 5, 0};

    std::vector<per_button_Config> button_inputs = {
        { "Translate X"   , 0.1f       , 0.0f      , Button_Type::Value}, // Model X-axis translation
        { "Translate Y"   , 0.1f       , -0.5f     , Button_Type::Value}, // Model Y-axis translation (starts offset)
        { "Translate Z"   , 0.1f       , 0.0f      , Button_Type::Value}, // Model Z-axis translation
        { "Scale"         , 0.1f       , 0.1f      , Button_Type::Value}, // Model scale
        { "Rotation A"    , PI / 20.0f , PI / 4.0f , Button_Type::Value}, // Rotation A (toolpath)
        { "Rotation B"    , PI / 20.0f , 0.0f      , Button_Type::Value}, // Rotation B (toolpath)
        { "Rotation C"    , PI / 20.0f , 0.0f      , Button_Type::Value}, // Rotation C (toolpath)
        { "Plane Offset"  , 0.05f      , 0.0f      , Button_Type::Value}, // Slicing plane offset
        { "Slice A"       , PI / 20.0f , PI / 4.0f , Button_Type::Value}, // Slicing plane rotation X (theta)
        { "Slice B"       , PI / 20.0f , 0.0f      , Button_Type::Value}  // Slicing plane rotation Y (phi)
    };

    Initialise_Mini_Panel(4, mcg, button_inputs); // Setup IDs and logic
    mini_panels.push_back(mcg);

    std::vector<per_button_Config> misc_buttons = {
        { "Slice"           , 0, 0, Temp  },
        { "Display Model"   , 0, 0, Toggle},
        { "Slice to G-code" , 0, 0, Temp  },
        { "Rot: ABC"        , 6, 1, Cycle , 1, 6, { "ABC", "ACB", "BAC", "BCA", "CAB", "CBA" } },
        { "Show/Hide Gen"   , 0, 0, Toggle},
        { "Show/Hide Path"  , 0, 0, Toggle},
        { "Load GCode"      , 0, 0, Temp  }
    };

    
    mcg.start_y = 550;
    Initialise_Mini_Panel(27, mcg, misc_buttons);
    mini_panels.push_back(mcg);
}

