#include "frontend.h"

frontend::frontend(std::shared_ptr<backend> b)
    : backend_(b), mainThread(&frontend::run, this) {}

frontend::~frontend() {
    if (mainThread.joinable()) {
        mainThread.join();
    }
}

void frontend::run(){
    window.Initialise_Window(1000, 1500, 60, "Borb CAM Slicer", "src/assets/Logo-Light.png");
    Camera camera = window.Initialise_Camera((Vector3){20.0f, 20.0f, 20.0f}, (Vector3){0.0f, 8.0f, 0.0f}, (Vector3){0.0f, 1.6f, 0.0f}, 45.0f, CAMERA_PERSPECTIVE);
    Shader shader = window.Initialise_Shader();
    window.Initialise_Lights(shader);
    gui_settings.font = LoadFont("src/assets/Inter_28pt-Light.ttf");
    Initialise_Inputs();

    while(!WindowShouldClose()){
        window.Update_Camera(&camera);
        BeginDrawing();
        ClearBackground(DARKGRAY);
        Draw_Mini_Panel(mini_panels); // Actual rendering

        BeginMode3D(camera);
        EndMode3D();
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

/**##########################################
 * #              GUI Helpers               #
 * ##########################################*/

void frontend::Initialise_Mini_Panel(int starting_id, Mini_Panel_Config& cfg, const std::vector<per_button_Config>& button_configs) {
    float x = cfg.start_x;
    float y = cfg.start_y;

    reset_int(starting_id);

    cfg.Button_Bar.clear();

    for (const auto& btn_cfg : button_configs) {
        int id_pos = next_int();
        int id_neg = next_int();

        cfg.Button_Bar.push_back(Button_Mini_Panel_config{
            .title = btn_cfg.title,
            .button_ids = { id_pos, id_neg },
            .value = btn_cfg.default_value,
            .increment = btn_cfg.increment
        });

        y += cfg.increment_y;
    }
}


void frontend::Draw_Mini_Panel(const std::vector<Mini_Panel_Config>& panels){
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

            char label_text[128];
            snprintf(label_text, sizeof(label_text), "%s: %.2f", bar.title, bar.value);
            DrawTextEx(gs.font, label_text, { x + 10, y + (cfg.max_height / 2.0f - 10.0f) }, 18.0f, 1.0f, WHITE);

            float right_edge_x = x + cfg.max_width;
            float button_width = cfg.button_widths;
            float button_height = cfg.button_heights;
            float minus_button_x = right_edge_x - button_width;
            float plus_button_x = minus_button_x - (button_width + gs.button_spacing_x);
            float button_y = y + (cfg.max_height - button_height) / 2.0f;

            if (bar.button_ids.size() >= 2) {
                window.Add_Button(bar.button_ids[0], button_height, button_width, plus_button_x, button_y, "+");
                window.Add_Button(bar.button_ids[1], button_height, button_width, minus_button_x, button_y, "-");
            }

            y += cfg.increment_y;

            current_panel_color.r = std::min(current_panel_color.r + cfg.color_increment.r, 255);
            current_panel_color.g = std::min(current_panel_color.g + cfg.color_increment.g, 255);
            current_panel_color.b = std::min(current_panel_color.b + cfg.color_increment.b, 255);
            current_panel_color.a = std::min(current_panel_color.a + cfg.color_increment.a, 255);
        }
    }
}

void frontend::Handle_Mini_Panel(std::vector<Mini_Panel_Config>& panels){
    for(auto& cfg : panels) {
        for (auto& bar : cfg.Button_Bar) {
            if (bar.button_ids.size() >= 2) {
                int id_pos = bar.button_ids[0];
                int id_neg = bar.button_ids[1];

                if (window.Ret_Button(id_pos)) bar.value += bar.increment;
                if (window.Ret_Button(id_neg)) bar.value -= bar.increment;
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
    mcg.title = "Slicer & Pathing";
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
        { "Layer Height", 0.05f, 0.2f },
        { "Speed", 1.0f, 60.0f },
        { "Angle Limit", PI / 16, 0.0f },
        { "Max Travel", 5.0f, 100.0f }
    };

    Initialise_Mini_Panel(4, mcg, button_inputs); // Setup IDs and logic
    mini_panels.push_back(mcg);
}
