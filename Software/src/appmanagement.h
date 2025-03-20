#ifndef APPMANAGEMENT
#define APPMANAGEMENT

#include <vector>
#include <stdio.h>
#include <raylib.h>
#include <raygui.h>

struct App_Button{
    int id;
    float height = 10.0f;
    float width = 30.0f;
    float xpos = 10.0f;
    float ypos = 10.0f;
    const char*text = "Click Me";
    bool state = 0;
};

class app{
    public:

    void Initialise_Window(int height, int width, int fps, const char *title, const char*logoName);

    Camera Initialise_Camera(Vector3 position, Vector3 target_pos, Vector3 rotation, float fov, int projection);

    Shader Initialise_Shader();

    void Load_3D_Environment();

    /**##########################################
     * #            Button Functions            #
     * ##########################################*/

    void Add_Button(int id); // Add Button

    void Add_Button(int id, float height, float width, float xpos, float ypos, const char *text); // Add Button

    void Rem_Button(int id); // Remove Button

    bool Ret_Button(int id); // Return Button State

    int CNT_Buttons(); // Return a count of Buttons Loaded

    void Run_Buttons(); // Run Buttons

    /**##########################################
     * #            Shader Functions            #
     * ##########################################*/


    /**##########################################
     * #             Misc Functions             #
     * ##########################################*/

    void Print(int value, int xpos, int ypos);

    private:
    std::vector<App_Button> buttons;

    /**##########################################
     * #            General Functions           #
     * ##########################################*/

    bool ID_Check(int id, std::vector<App_Button> &button_array);



};

#endif