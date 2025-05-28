#include "frontend.h"

frontend::frontend(std::shared_ptr<backend> b)
    : backend_(b), mainThread(&frontend::run, this) {}

frontend::~frontend() {
    if (mainThread.joinable()) {
        mainThread.join();
    }
}

Color LerpColor(Color a, Color b, float t) {
    Color result;
    result.r = (unsigned char)((1 - t) * a.r + t * b.r);
    result.g = (unsigned char)((1 - t) * a.g + t * b.g);
    result.b = (unsigned char)((1 - t) * a.b + t * b.b);
    result.a = 255;
    return result;
}

Vector3 RayRotConvert(Vector3 rot) {
    // rot.x = pitch (around X)
    // rot.y = yaw   (around Y)
    // rot.z = roll  (around Z)

    Vector3 converted;

    converted.x = rot.x;       // X rotation stays
    converted.y = -rot.z;      // Roll (around Z) becomes negative yaw in new system
    converted.z = rot.y;       // Yaw (around Y) becomes roll (around Z-up)

    return converted;
}

Vector3 RayPosConvert(Vector3 v) {
    Vector3 converted;
    converted.x = v.x;     // X remains the same
    converted.y = v.z;     // Z' back to Y
    converted.z = -v.y;    // Y' back to -Z
    return converted;
}

Model Rotate_Model(Model model, Vector3 rotation) {
    Matrix rotX = MatrixRotateX(rotation.x);
    Matrix rotY = MatrixRotateY(rotation.y);
    Matrix rotZ = MatrixRotateZ(rotation.z);
    Matrix rotationMatrix = MatrixMultiply(MatrixMultiply(rotZ, rotY), rotX); // ZYX order

    for (int i = 0; i < model.meshCount; i++) {
        Mesh *mesh = &model.meshes[i];
        for (int v = 0; v < mesh->vertexCount; v++) {
            Vector3 vertex = {
                mesh->vertices[v * 3 + 0],
                mesh->vertices[v * 3 + 1],
                mesh->vertices[v * 3 + 2]
            };
            vertex = Vector3Transform(vertex, rotationMatrix);
            mesh->vertices[v * 3 + 0] = vertex.x;
            mesh->vertices[v * 3 + 1] = vertex.y;
            mesh->vertices[v * 3 + 2] = vertex.z;
        }
        UploadMesh(mesh, false);
    }

    model.transform = MatrixIdentity(); // Reset transform (geometry is rotated)
    return model;
}

Model Scale_Model(Model model, float scale) {
    for (int i = 0; i < model.meshCount; i++) {
        Mesh *mesh = &model.meshes[i];
        for (int v = 0; v < mesh->vertexCount; v++) {
            mesh->vertices[v * 3 + 0] *= scale; // x
            mesh->vertices[v * 3 + 1] *= scale; // y
            mesh->vertices[v * 3 + 2] *= scale; // z
        }
        UploadMesh(mesh, false);
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
        UploadMesh(mesh, false);
    }
    model.transform = MatrixIdentity(); // Reset transform (mesh is already moved)
    return model;
}

void frontend::run(){
   window.Initialise_Window(gui_settings.window_height, gui_settings.window_width, gui_settings.window_framerate,
                             gui_settings.window_title, gui_settings.logo_file_location);
    Camera camera = window.Initialise_Camera({20.0f, 20.0f, 20.0f}, {0.0f, 8.0f, 0.0f}, {0.0f, 1.6f, 0.0f}, 45.0f, CAMERA_PERSPECTIVE);
    Shader shader = window.Initialise_Shader();
    window.Initialise_Lights(shader);
    gui_settings.font = LoadFont("src/assets/Inter_28pt-Light.ttf");
    paths.Create_File("src/output/OwO", "nc");
    Initialise_Inputs();

    while(!WindowShouldClose()){
        // Do Initial Processing Here
        backend_->run();
        window.Update_Camera(&camera);
        BeginDrawing();
        ClearBackground(DARKGRAY);
        BeginMode3D(camera);
        // All 3D stuff Here
        if(Model_Loaded){ // Dealing with Model Loading & Visibility
            meshing.Sha_Model(1, shader);
            if(Model_Visible){
                meshing.Run_Models();
            }
        }
        if(Toolpath_Loaded){
            if(Toolpath_Visible){
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
            }
            if(Animation_Visible){
                Animate_Flat_Toolpath(Flat_Toolpath, currentLineIndex, lastUpdate);
            }
        }

        slicing.Comp_Axis_Guides_3D();
        slicing.Comp_Ground_Grid();
        EndMode3D();

        // Draw GUI Here (As it is after the models)
        if( (!(mini_panels[0].Button_Bar[0].value == current_model.translation.x)) ||
            (!(mini_panels[0].Button_Bar[1].value == current_model.translation.y)) ||
            (!(mini_panels[0].Button_Bar[2].value == current_model.translation.z)) ||
            (!(mini_panels[0].Button_Bar[3].value == current_model.scale)) ||
            (!(mini_panels[0].Button_Bar[4].value == current_model.rotation.x)) ||
            (!(mini_panels[0].Button_Bar[5].value == current_model.rotation.y)) ||
            (!(mini_panels[0].Button_Bar[6].value == current_model.rotation.z))){ // Model Transformation
            
            float scale_temp = mini_panels[0].Button_Bar[3].value;
            Vector3 rotation_temp = {mini_panels[0].Button_Bar[4].value, mini_panels[0].Button_Bar[5].value, mini_panels[0].Button_Bar[6].value};
            Vector3 translation_temp = {mini_panels[0].Button_Bar[0].value, mini_panels[0].Button_Bar[1].value, mini_panels[0].Button_Bar[2].value};
            
            // Set Values
            Matrix scaleMat = MatrixScale(scale_temp, scale_temp, scale_temp);
            Matrix rotMat = MatrixRotateXYZ(rotation_temp);
            Matrix transMat = MatrixTranslate(translation_temp.x, translation_temp.y, translation_temp.z);

            // Write to Buffer
            current_model.scale = scale_temp;
            current_model.rotation = rotation_temp;
            current_model.translation = translation_temp;

            // Final transform: scale → rotate → translate
            Matrix finalTransform = MatrixMultiply(MatrixMultiply(transMat, rotMat), scaleMat);
            Model temp_model = meshing.Ret_Model(1);
            temp_model.transform = finalTransform;
            meshing.Reu_Model(1, temp_model);
        }
        if( (!(mini_panels[0].Button_Bar[7].value == Slice_Plane_Distance)) ||
            (!(mini_panels[0].Button_Bar[8].value == Rotation_Button_Plane.x)) ||
            (!(mini_panels[0].Button_Bar[9].value == Rotation_Button_Plane.y))) {
            
            // Update tracking vars
            Slice_Plane_Distance = mini_panels[0].Button_Bar[7].value;
            Rotation_Button_Plane.x = mini_panels[0].Button_Bar[8].value;
            Rotation_Button_Plane.y = mini_panels[0].Button_Bar[9].value;
            
            std::cout << "Set Values" << std::endl;
            // Apply to slicing settings
            backend_->slicing_tools
                .Set_Slicing_Plane({Rotation_Button_Plane.x, Rotation_Button_Plane.y}, 0)
                .Set_Slicing_Distance(Slice_Plane_Distance);
        }
        if(mini_panels[1].Button_Bar[0].value > 0.5f){ // Slice Button
            mini_panels[1].Button_Bar[0].value = 0.0f;
            Slice();
            sliceState = SliceState::Slicing;
        }
        if(mini_panels[1].Button_Bar[1].value > 0.5f){ // Load / Reload Model Button
            mini_panels[1].Button_Bar[1].value = 0.0f;
            if(Model_Loaded){meshing.Rem_Model(1);}

            meshing.Add_Model(1, "src/models/model.obj");
            current_model.base_model = meshing.Ret_Model(1);

            Model_Loaded = 1;
        }
        if(mini_panels[1].Button_Bar[2].value > 0.5f){ // Toggle Model Visibility Button
            mini_panels[1].Button_Bar[2].value = 0.0f;
            Model_Visible =! Model_Visible;
        }
        if(mini_panels[1].Button_Bar[3].value > 0.5f){ // Save Path to Gcode Button
            mini_panels[1].Button_Bar[3].value = 0.0f;
            bool enable = 0;
            if(mini_panels[2].Button_Bar[0].value > 0.5f){enable = 1;}
            Save_To_Gcode(enable, mini_panels[2].Button_Bar[2].value);
        }
        if(mini_panels[1].Button_Bar[4].value > 0.5f){ // Toggle Toolpath
            mini_panels[1].Button_Bar[4].value = 0.0f;
            Toolpath_Visible =! Toolpath_Visible;
        }
        if(mini_panels[1].Button_Bar[5].value > 0.5f){ // Toggle Animation
            mini_panels[1].Button_Bar[5].value = 0.0f;
            Toolpath_Loaded = 1;
            Animation_Visible =! Animation_Visible;
        }


        if(sliceState == SliceState::Slicing && !backend_->return_run_status()){
            sliceState = SliceState::Completed;
        
            Flat_Toolpath.clear();
            for(const auto& segment : backend_->return_config().Toolpath){
                Flat_Toolpath.insert(Flat_Toolpath.end(), segment.begin(), segment.end());
            }
            toolpath = backend_->return_config().Toolpath;

            Toolpath_Loaded = 1;
        
            std::cout << "[Frontend] Toolpath loaded after slicing!" << std::endl;
        }


        Draw_Mini_Panel(mini_panels);
        window.Run_Buttons();
        Handle_Mini_Panel(mini_panels);
        EndDrawing();
    }
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

Model frontend::DeepCopyModel(const Model& source) {
    Model copy = source;
    copy.meshes = (Mesh*)malloc(sizeof(Mesh) * source.meshCount);

    for (int i = 0; i < source.meshCount; i++) {
        Mesh src = source.meshes[i];
        Mesh dst = { 0 };

        dst.vertexCount = src.vertexCount;
        dst.triangleCount = src.triangleCount;

        size_t vertSize = sizeof(float) * 3 * dst.vertexCount;
        dst.vertices = (float*)malloc(vertSize);
        memcpy(dst.vertices, src.vertices, vertSize);

        if (src.normals) {
            dst.normals = (float*)malloc(vertSize);
            memcpy(dst.normals, src.normals, vertSize);
        }

        if (src.indices) {
            size_t indSize = sizeof(unsigned short) * dst.triangleCount * 3;
            dst.indices = (unsigned short*)malloc(indSize);
            memcpy(dst.indices, src.indices, indSize);
        }

        // You can add more attributes here if your toolpath uses tangents, texcoords, etc.

        UploadMesh(&dst, false);
        copy.meshes[i] = dst;
    }

    return copy;
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
    // reset_float();
    // set_float_increment(gs.button_spacing_x + gs.button_width);
    // window.Add_Button(0, gs.button_height, gs.button_width, next_float(), 0, "Model");
    // window.Add_Button(1, gs.button_height, gs.button_width, next_float(), 0, "Toolpath");
    // window.Add_Button(2, gs.button_height, gs.button_width, next_float(), 0, "Cullbox");
    // window.Add_Button(3, gs.button_height, gs.button_width, next_float(), 0, "Obstacles");

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
        { "Translate Y"   , 0.1f       , -1.0f     , Button_Type::Value}, // Model Y-axis translation (starts offset)
        { "Translate Z"   , 0.1f       , 0.0f      , Button_Type::Value}, // Model Z-axis translation
        { "Scale"         , 0.1f       , 0.1f      , Button_Type::Value}, // Model scale
        { "Rotation A"    , PI / 20.0f , 0.0f      , Button_Type::Value}, // Rotation A (toolpath)
        { "Rotation B"    , PI / 20.0f , 0.0f      , Button_Type::Value}, // Rotation B (toolpath)
        { "Rotation C"    , PI / 20.0f , 0.0f      , Button_Type::Value}, // Rotation C (toolpath)
        { "Plane Offset"  , 1.0f       , 5.0f      , Button_Type::Value}, // Slicing plane offset
        { "Slice A"       , PI / 20.0f , PI / 4.0f , Button_Type::Value}, // Slicing plane rotation X (theta)
        { "Slice B"       , PI / 20.0f , 0.0f      , Button_Type::Value}  // Slicing plane rotation Y (phi)
    };
    Initialise_Mini_Panel(4, mcg, button_inputs); // Setup IDs and logic
    mini_panels.push_back(mcg);

    std::vector<per_button_Config> system_buttons = {
        { "Slice"           , 0, 0, Temp  },
        { "Load Model"      , 0, 0, Temp  },
        { "Display Model"   , 0, 0, Toggle},
        { "Slice to G-code" , 0, 0, Temp  },
        { "Display Toolpath", 0, 0, Toggle},
        { "Display Animation",0, 0, Toggle}
        // { "Rot: ABC"        , 6, 1, Cycle , 1, 6, { "ABC", "ACB", "BAC", "BCA", "CAB", "CBA" } },
        // { "Rot: XYZ"        , 6, 1, Cycle , 1, 6, { "XYZ", "XZY", "YXZ", "YZX", "ZXY", "ZYX" } },
        // { "Show/Hide Gen"   , 0, 0, Toggle},
        // { "Show/Hide Path"  , 0, 0, Toggle},
        // { "Load GCode"      , 0, 0, Temp  }
    };
    mcg.Title = false;
    mcg.start_y = 500;
    Initialise_Mini_Panel(27, mcg, system_buttons);
    mini_panels.push_back(mcg);

    std::vector<per_button_Config> misc_buttons = {
        { "Enable Culling"  , 0, 0, Toggle},
        { "Enable Restriction",0,0, Toggle},
        { "Cull Angles"     , 6, 1, Cycle , 1, 6, { "A", "B", "C", "AB", "AC", "BC" } },
        { "Restrict Angles" , 6, 1, Cycle , 1, 6, { "A", "B", "C", "AB", "AC", "BC" } }
    };
    mcg.Title = true;
    mcg.title = "Restriction Settings";
    mcg.start_x = GetScreenWidth() - 250 - 20;
    mcg.start_y = 80;
    Initialise_Mini_Panel(33, mcg, misc_buttons);
    mini_panels.push_back(mcg);
}

void frontend::Animate_Flat_Toolpath(std::vector<Line>& flatToolpath, size_t& currentLineIndex, std::chrono::high_resolution_clock::time_point& lastUpdate) {
    if (flatToolpath.empty()) return;

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
    Vector3 endPos = Vector3Add(pos, Vector3Scale(norm, 100.0f));
    DrawCylinderEx(pos, endPos, 5.0f, 5.0f, 8, YELLOW);
    // DrawText(TextFormat("Line: %d / %d", (int)currentLineIndex, (int)flatToolpath.size()), 10, 50, 20, WHITE);
    // DrawText(TextFormat("Segment progress: %.2f", segmentProgress), 10, 70, 20, WHITE);
}

/**##########################################
 * #            Button Functions            #
 * ##########################################*/

void frontend::Slice(){
    if(!Model_Loaded) return;
    Transform_Model();
    if(!current_model.modified_model.meshCount || !current_model.modified_model.meshes->vertexCount){return;}\

    // backend_->set_settings(slicing.return_Settings());
    // std::cout << "Slicing Distance: " << slicing.return_Settings().SlicingDistance << std::endl;
    backend_->clear_schedule();
    backend_->set_model(current_model.modified_model);
    backend_->schedule(Generate_Surface_Toolpath);
    backend_->schedule(Cull_Toolpath_to_Obstacle, cullBox);
    backend_->schedule(Optimise_Start_End_Positions);
    // BoundingBox work_box;
    // work_box.max = {40, 80, 40};
    // work_box.min = {-40, 0, -40};
    // backend_->schedule(Generate_Start_End_Rays, work_box);
    backend_->schedule(Optimise_Start_End_Linkages);
    backend_->schedule(Add_Custom_Start_End_Positions);
    backend_->schedule(Restrict_Max_Angle_per_Move);
    backend_->run_schedule();
}

void frontend::Transform_Model() {
    current_model.modified_model = DeepCopyModel(current_model.base_model);
    current_model.modified_model = Scale_Model(current_model.modified_model, current_model.scale);
    current_model.modified_model = Rotate_Model(current_model.modified_model, current_model.rotation);
    current_model.modified_model = Move_Model(current_model.modified_model, current_model.translation);

    for (int i = 0; i < current_model.modified_model.meshCount; i++) {
        UploadMesh(&current_model.modified_model.meshes[i], false);  // Upload only once!
    }
}

void frontend::Reset_Model_Transform(){
    current_model.rotation = {0, 0, 0};
    current_model.translation = {0, 0, 0};
    current_model.scale = 1;
    Transform_Model();
}

void frontend::Save_To_Gcode(bool Enable_Restriction, int Choose_Angle){
    // Extract toolpath from backend
    std::vector<Line> flat_toolpath = backend_->slicing_tools.Toolpath_Flattener();
    if(flat_toolpath.empty()){ return; }

    std::vector<std::pair<Vector3, Vector3>> pathPositions;

    // Fetch GUI toggles
    bool cullingEnabled = (mini_panels[2].Button_Bar[0].value > 0.5f);  // Enable Culling
    // Enable_Restriction comes from Button_Bar[1], already passed in

    paths.Clear_File();
    paths.Reset_N();
    
    for(const Line& line : flat_toolpath){
        Vector3 pos = line.endLinePoint.Position;
        Vector3 rot = meshing.NormalToRotation(line.endLinePoint.Normal);
        rot.z += -90.0f;
        Vector3 rotConv = RayRotConvert(rot);
        rotConv = Cull_Angles(Choose_Angle, rotConv, !cullingEnabled); // Only apply if culling is enabled
        pathPositions.push_back({ RayPosConvert(pos), rotConv });
    }

    paths.Feedrate(400);
    paths.Path_to_Gcode1(pathPositions);
}

Vector3 frontend::Cull_Angles(int Choose_Angle, Vector3 Angles, bool bypass){
    if(bypass){return Angles;}

    switch (Choose_Angle) {
        case 1: // A → 0
            Angles.x = 0.0f;
            break;
        case 2: // B → 0
            Angles.y = 0.0f;
            break;
        case 3: // C → 0
            Angles.z = 0.0f;
            break;
        case 4: // A & B → 0
            Angles.x = 0.0f;
            Angles.y = 0.0f;
            break;
        case 5: // A & C → 0
            Angles.x = 0.0f;
            Angles.z = 0.0f;
            break;
        case 6: // B & C → 0
            Angles.y = 0.0f;
            Angles.z = 0.0f;
            break;
        default:
            // Do nothing if invalid value passed
            break;
    }
    return Angles;
}
