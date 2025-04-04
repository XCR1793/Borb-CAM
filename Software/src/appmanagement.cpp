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

/**##########################################
 * #            Button Functions            #
 * ##########################################*/

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

/**##########################################
 * #            Camera Functions            #
 * ##########################################*/

Camera app::Initialise_Camera(Vector3 position, Vector3 target_pos, Vector3 rotation, float fov, int projection){
    Internal_Camera.position = position;
    Internal_Camera.target = target_pos;
    Internal_Camera.up = rotation;
    Internal_Camera.fovy = fov;
    Internal_Camera.projection = projection;
    return Internal_Camera;
}

void app::Update_Camera(Camera* camera){
    float zoomFactor = Vector3Distance(Internal_Camera.position, Internal_Camera.target) * 0.1f;

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)){
        Vector2 delta = Vector2Scale(GetMouseDelta(), zoomFactor * 0.01f);
        Vector3 forward = Vector3Normalize(Vector3Subtract(camera->target, camera->position));
        Vector3 right = Vector3CrossProduct(forward, camera->up);
        camera->position = Vector3Add(camera->position, Vector3Scale(right, -delta.x));
        camera->position = Vector3Add(camera->position, Vector3Scale(camera->up, delta.y));
        camera->target = Vector3Add(camera->target, Vector3Scale(right, -delta.x));
        camera->target = Vector3Add(camera->target, Vector3Scale(camera->up, delta.y));
    }

    float wheelMove = GetMouseWheelMove();
    if (wheelMove != 0){
        Vector3 direction = Vector3Scale(Vector3Subtract(camera->target, camera->position), 1.0f + wheelMove * 0.1f);
        camera->position = Vector3Subtract(camera->target, direction);
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)){
        Vector2 delta = Vector2Scale(GetMouseDelta(), 0.005f);
        Vector3 right = Vector3CrossProduct(Vector3Subtract(camera->target, camera->position), camera->up);
        camera->position = Vector3RotateByAxisAngle(camera->position, camera->up, -delta.x);
        camera->position = Vector3RotateByAxisAngle(camera->position, right, -delta.y);
    }
}

/**##########################################
 * #            Shader Functions            #
 * ##########################################*/

Shader app::Initialise_Shader(){
    int GLSL_VERSION = 330;
    Shader shader = LoadShader(TextFormat("resources/shaders/glsl%i/lighting.vs", GLSL_VERSION),
                               TextFormat("resources/shaders/glsl%i/lighting.fs", GLSL_VERSION));
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
    int ambientLoc = GetShaderLocation(shader, "ambient");
    SetShaderValue(shader, ambientLoc, (float[4]){ 0.1f, 0.1f, 0.1f, 1.0f }, SHADER_UNIFORM_VEC4);
    return shader;
}

Light* app::Initialise_Lights(Shader shader){
    Light* lights = (Light*)malloc(sizeof(Light) * 4);

    lights[0] = CreateLight(LIGHT_DIRECTIONAL, Vector3Zero(), (Vector3){-1.0f, -1.0f, 0.0f}, RED, shader);
    lights[1] = CreateLight(LIGHT_DIRECTIONAL, Vector3Zero(), (Vector3){1.0f, -1.0f, 0.0f}, GREEN, shader);
    lights[2] = CreateLight(LIGHT_DIRECTIONAL, Vector3Zero(), (Vector3){0.0f, -1.0f, -1.0f}, BLUE, shader);
    lights[3] = CreateLight(LIGHT_DIRECTIONAL, Vector3Zero(), (Vector3){0.0f, -1.0f, 1.0f}, YELLOW, shader);

    return lights;
}

/**##########################################
 * #             Misc Functions             #
 * ##########################################*/

void app::Print(int value, int posx, int posy){
    char buffer[20];
    sprintf(buffer, "%d", value);
    DrawText(buffer, posx, posy, 30, RED);
}

void app::PrintF(float value, int posx, int posy){
    char buffer[20];
    sprintf(buffer, "%.2f", value);
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

bool app::Create_File(const std::string& fileName, const std::string& extension){
    std::string filePath = fileName + "." + extension;

    if(FileExists(filePath.c_str())){TraceLog(LOG_INFO, "Overriding Existing File: %s", filePath.c_str());}

    Default_File(fileName, extension);

    return SaveFileData(filePath.c_str(), nullptr, 0);
}

bool app::Write_File(const std::string& fileName, const std::string& extension, long lineNumber, const std::string& content){
    std::string filePath = fileName + "." + extension;

    // Load existing lines
    std::vector<std::string> lines;
    std::ifstream infile(filePath);
    std::string line;
    while (std::getline(infile, line)) {
        lines.push_back(line);
    }
    infile.close();

    // Extend file if necessary
    if (lineNumber >= static_cast<long>(lines.size())) {
        lines.resize(lineNumber + 1, "");
    }

    // Set the line
    lines[lineNumber] = content;

    // Write lines back to file
    std::ofstream outfile(filePath);
    if (!outfile.is_open()) {
        TraceLog(LOG_WARNING, "Failed to open file for writing: %s", filePath.c_str());
        return false;
    }

    for (size_t i = 0; i < lines.size(); ++i) {
        outfile << lines[i];
        if (i != lines.size() - 1) {
            outfile << "\n";
        }
    }

    outfile.close();
    return true;
}

bool app::Write_File_Last(const std::string& fileName, const std::string& extension, const std::string& content) {
    std::string filePath = fileName + "." + extension;

    if(extension == ""){filePath = fileName;}

    std::ofstream outfile(filePath, std::ios::app); // Open in append mode
    if (!outfile.is_open()) {
        TraceLog(LOG_WARNING, "Failed to open file for writing: %s", filePath.c_str());
        return false;
    }

    // Always write content as a new line at the end
    outfile << content << "\n";
    outfile.close();
    return true;
}

bool app::Default_File(const std::string& fileName, const std::string& extension){
    default_file_path = fileName + "." + extension;
    return true;
}

bool app::Clear_File() {
    // Open file to clear its contents
    std::ofstream outfile(default_file_path, std::ios::trunc); // `std::ios::trunc` ensures the file is cleared

    // Since we're clearing the file, there's no need to write anything back
    outfile.close();
    return true;
}

bool app::Clear_File(const std::string& fileName, const std::string& extension) {
    std::string filePath = fileName + "." + extension;

    // Open file to clear its contents
    std::ofstream outfile(filePath, std::ios::trunc); // `std::ios::trunc` ensures the file is cleared

    // Since we're clearing the file, there's no need to write anything back
    outfile.close();
    return true;
}