#ifndef APPMANAGEMENT
#define APPMANAGEMENT

#include <vector>
#include <stdio.h>
#include <raylib.h>
#include <raygui.h>
#include <raymath.h>
#include <rlights.h>
#include <rlgl.h>
#include <string>
#include <fstream>
#include <sstream>

struct App_Button{
    int id;
    float height = 10.0f;
    float width = 30.0f;
    float xpos = 10.0f;
    float ypos = 10.0f;
    const char*text = "Click Me";
    bool state = 0;
    float increment = 0;
};

class app{
    public:

    void Initialise_Window(int height, int width, int fps, const char *title, const char*logoName);

    /**##########################################
     * #            Button Functions            #
     * ##########################################*/

    void Add_Button(int id); // Add Button

    void Add_Button(int id, float xpos, float ypos, const char *text); // Add Button

    void Add_Button(int id, float xpos, float ypos, const char *text,  float increment); // Add Button

    void Add_Button(int id, float height, float width, float xpos, float ypos, const char *text); // Add Button

    void Add_Button(int id, float height, float width, float xpos, float ypos, const char *text, float increment); // Add Button

    void Add_Button_Pair(int id_pos, int id_neg, float height, float width, float xpos, float ypos,
                        const char *label_pos, const char *label_neg, float increment, float y_spacing = 5.0f); // Add Button Pair with Spacing

    bool Ret_Button_Pair(int id_pos, int id_neg, float &value, float &delta); // Return Button Pair State & Apply Increment

    void Rem_Button(int id); // Remove Button

    bool Ret_Button(int id); // Return Button State

    bool Ret_Button(int id, float &value); // Return Button State

    void Set_Button_Defaults(float height, float width, float xpos, float ypos); // Set Default Values for Buttons

    int CNT_Buttons(); // Return a count of Buttons Loaded

    void Run_Buttons(); // Run Buttons

    /**##########################################
     * #            Camera Functions            #
     * ##########################################*/
    Camera Initialise_Camera(Vector3 position, Vector3 target_pos, Vector3 rotation, float fov, int projection);

    void Update_Camera(Camera* camera);

    /**##########################################
     * #            Shader Functions            #
     * ##########################################*/
    Shader Initialise_Shader();

    Light* Initialise_Lights(Shader shader);

    /**##########################################
     * #             Misc Functions             #
     * ##########################################*/

    void Print(int value, int xpos, int ypos);
    void PrintF(float value, int xpos, int ypos);

    bool Create_File(const std::string& fileName, const std::string& extension);

    bool Write_File(const std::string& fileName, const std::string& extension, long lineNumber, const std::string& content);

    bool Write_File_Last(const std::string& fileName, const std::string& extension, const std::string& content);

    bool Default_File(const std::string& fileName, const std::string& extension);

    bool Clear_File();
    bool Clear_File(const std::string& fileName, const std::string& extension);

    private:
    std::vector<App_Button> buttons;
    Camera Internal_Camera = {0};
    std::string default_file_path;
    long line = 0;
    float default_height, default_width, default_xpos, default_ypos;

    /**##########################################
     * #            General Functions           #
     * ##########################################*/

    bool ID_Check(int id, std::vector<App_Button> &button_array);



};

#endif