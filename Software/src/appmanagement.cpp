#include "appmanagement.h"

void app::Initialise_Window(int height, int width, int fps, const char *title, const char*logo){
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(width, height, title);
    Image Icon = LoadImage(logo);
    SetWindowIcon(Icon);
    UnloadImage(Icon);
    SetTargetFPS(fps);
    SetExitKey(0);
}

Camera app::Initialise_Camera(Vector3 position, Vector3 target_pos, Vector3 rotation, float fov, int projection){
    Camera camera = {0};
    camera.position = position;
    camera.target = target_pos;
    camera.up = rotation;
    camera.fovy = fov;
    camera.projection = projection;
    return camera;
}


void app::Add_Button(int id){
    if(!ID_Check(id, buttons)){
        buttons.push_back((App_Button){});
    }
}

void app::Add_Button(int id, float height, float width, float xpos, float ypos, const char *text){
    if(!ID_Check(id, buttons)){
        buttons.push_back((App_Button){id, height, width, xpos, ypos, text, 0});
    }
}

void app::Rem_Button(int id){
    if(!buttons.empty()){
        for(std::vector<App_Button>::size_type it = 0; it < buttons.size(); it++){
            if(buttons.at(it).id == id){
                buttons.erase(buttons.begin() + it);
                it--;
            }
        }
    }
}

bool app::Ret_Button(int id){
    if(!buttons.empty()){
        for(std::vector<App_Button>::size_type it = 0; it < buttons.size(); it++){
            if(buttons.at(it).id == id){
                return buttons.at(it).state;
            }
        }
    }
    return 0;
}

int app::CNT_Buttons(){
    if(!buttons.empty()){
        return buttons.size();
    }
    return -1;
}

void app::Run_Buttons(){
    if(!buttons.empty()){
        for(auto &button : buttons){
            button.state = GuiButton((Rectangle){button.xpos, button.ypos, button.width, button.height}, button.text);
        }
    }
}

void app::Print(int value, int posx, int posy){
    char buffer[20];
    sprintf(buffer, "%d", value);
    DrawText(buffer, posx, posy, 30, RED);
}

bool app::ID_Check(int id, std::vector<App_Button> &button_array){
    if(!button_array.empty()){
        for(auto item : button_array){
            if(item.id == id){return true;}
        }
    }
    return false;
}