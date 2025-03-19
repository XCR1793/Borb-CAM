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

    /**##########################################
     * #            Button Functions            #
     * ##########################################*/

    void Add_Button(int id);

    void Add_Button(int id, float height, float width, float xpos, float ypos, const char *text);

    void Rem_Button(int id);

    bool Ret_Button(int id);

    int CNT_Buttons();

    void Run_Buttons();



    void Print(int value, int xpos, int ypos);

    private:
    std::vector<App_Button> buttons;

    /**##########################################
     * #            General Functions           #
     * ##########################################*/

    bool ID_Check(int id, std::vector<App_Button> &button_array);



};

#endif