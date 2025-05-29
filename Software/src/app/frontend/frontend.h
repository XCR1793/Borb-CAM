    #ifndef FRONTEND_H
    #define FRONTEND_H

    #include <memory>
    #include <stdio.h>
    #include <stdlib.h>
    #include <chrono>
    #include <vector>
    #include <string>
    #include <raylib.h>
    #include <raygui.h>
    #include <raymath.h>
    #include <appmanagement.h>
    #include <backend.h>

    struct GUI_Settings{
        // Simple Window Setup
        int window_height = 1000;
        int window_width = 1500;
        int window_framerate = 60;
        const char *window_title = "Borb CAM Slicer";
        const char *logo_file_location = "src/assets/Logo-Light.png";

        // GUI Setup
        Font font;
        float panel_title_height = 30;
        float text_spacing_standard = 20;
        float button_height = 20;
        float button_width = 60;
        float button_spacing_x = 20;
        float button_spacing_y = 20;
    };

    enum Button_Type{
        Value, // Buttons with value that can increment
        Value_Clamped, // Same as value but has min and max values
        Toggle, // Togglable buttons
        Temp, // Buttons that are temporary toggle on but goes straight off
        Cycle, // Buttons that cycles through a list
    };

    struct per_button_Config{
        const char* title = "default";
        float increment = PI/8;
        float default_value = 0;
        Button_Type buttontype = Button_Type::Value;
        float Min = 0;
        float Max = 0;
        std::vector<std::string> string_values;
    };

    struct Button_Mini_Panel_config{
        const char* title;
        std::vector<int> button_ids;
        Button_Type buttontype = Button_Type::Value;
        float value = 0;
        float increment = PI/8;
        float Min = 0;
        float Max = 0;
        std::vector<std::string> string_values;
    };

    struct Mini_Panel_Config{
        bool Title = true; // Enable or disable the title of the panel before the panels generate
        Color Panel = RED; // colour in rgba
        const char* title = "some title";
        std::vector<Button_Mini_Panel_config> Button_Bar;// you can name the variable something better
        float start_x = 0;
        float start_y = 0;
        float increment_x = 0;
        float increment_y = 0;
        float max_height = 30;
        float max_width = 250;
        float button_heights = 30;
        float button_widths = 60;
        Color panel_color = {0, 255, 127, 255}; // some colour (hsl + a)
        Color color_increment = {5, 0, 0, 0}; // some colour to increment (hsl + a) per button_bar
    };

    struct Model_Config{
        Model base_model; // For Rendering & Base Non 
        Model modified_model; // Modified from base and transforms applied
        Vector3 translation = {0, 0, 0};
        float scale = 1.0;
        Vector3 rotation = {0, 0, 0};
    };

    enum class SliceState {
        Idle,
        Slicing,
        Completed
    };

    class frontend{
        public:
        frontend(std::shared_ptr<backend> b);
        ~frontend();

        /**##########################################
         * #           Frontend Functions           #
         * ##########################################*/

        void run(); // Frontend has its own thread

        private:
        /**##########################################
         * #            Helper Functions            #
         * ##########################################*/

        void reset_int(int value);
        int next_int();

        void set_float_increment(float increment_value);
        void reset_float();
        float next_float();
        float curr_float();

        Model DeepCopyModel(const Model& source);

        /**##########################################
         * #              GUI Helpers               #
         * ##########################################*/

        void Initialise_Mini_Panel(int starting_id, Mini_Panel_Config& config, const std::vector<per_button_Config>& button_configs);

        void Draw_Mini_Panel(const std::vector<Mini_Panel_Config>& panels);

        void Handle_Mini_Panel(std::vector<Mini_Panel_Config>& panels);

        /**##########################################
         * #           Frontend Functions           #
         * ##########################################*/

        void Initialise_Inputs();

        void Animate_Flat_Toolpath(std::vector<Line>& flatToolpath, size_t& currentLineIndex, std::chrono::high_resolution_clock::time_point& lastUpdate);

        /**##########################################
         * #            Button Functions            #
         * ##########################################*/

        void Slice();

        void Transform_Model();

        void Reset_Model_Transform();

        void Save_To_Gcode( bool Enable_Cull, 
                            int Choose_Angle_Cull, 
                            bool Enable_Reduce, 
                            int Choose_Angle_Reduce, 
                            float Reduce_Amount,
                            bool Enable_Offset,
                            int Choose_Angle_Offset,
                            float Offset_Amount);

        Vector3 Cull_Angles(int Choose_Angle, Vector3 Angles, bool bypass);

        Vector3 Reduce_Angles(int Choose_Angle, Vector3 Angles, float Amount, bool bypass);

        Vector3 Offset_Angles(int Choose_Angle, Vector3 Angles, float Amount, bool bypass);

        private:
        std::shared_ptr<backend> backend_;

        // Thread Variables
        std::thread mainThread;
        std::mutex dataMutex;

        // App Variables
        app window;
        path paths;
        slice slicing;
        mesh meshing;
        GUI_Settings gui_settings;
        int button_id = 0;
        float incremental = 0;
        int float_value_increment = 0;
        float Screen_Width = 0;
        float Screen_Height = 0;
        
        Mini_Panel_Config mini_panel_config;
        std::vector<Mini_Panel_Config> mini_panels;

        // Slicing Variables
        BoundingBox cullBox = {Vector3{-100, 100, -100}, Vector3{100, 50, 100}};
        Model_Config current_model;
        SliceState sliceState = SliceState::Idle;

        // Run Button Variables
        bool Model_Loaded = 0;
        bool Model_Visible = 1;
        Vector2 Rotation_Button_Plane = {0, 0};
        float Slice_Plane_Distance = 0;
        bool Toolpath_Loaded = 0;
        bool Animation_Visible = 1;
        bool Toolpath_Visible = 1;
        bool Slice_Status = 0;
        std::vector<std::vector<Line>> toolpath;
        std::vector<Line> Flat_Toolpath;
        size_t currentLineIndex = 0;std::chrono::high_resolution_clock::time_point lastUpdate = std::chrono::high_resolution_clock::now();
    };

    #endif