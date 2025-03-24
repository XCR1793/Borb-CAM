#ifndef MESHMANAGMENT
#define MESHMANAGEMENT

#include <string>
#include <vector>
#include <raylib.h>
#include <raymath.h>

struct multimodel{
    int id;
    Model model;
    Vector3 pos = {0, 0, 0};
    Vector3 rot = {0, 0, 0};
    float scale = 1;
    Color colour = ORANGE;
};

class mesh{
    public:

    /**##########################################
     * #             Model Functions            #
     * ##########################################*/
    
    void Add_Model(int id, const char *model_path); // Add New Model

    void Rem_Model(int id); // Remove Model

    Model Ret_Model(int id); // Return Model at ID

    void Rep_Model(int id, Vector3 position); // Reposition Model

    void Reu_Model(int id, Model model); // Reupload Model
    
    void Sha_Model(Shader shader); // Apply shader to all models

    void Sha_Model(int id, Shader shader); // Apply shader to a specific model

    int CNT_Models(); // Return a count of Models Loaded

    void Run_Models(); // Run Models

    void Stop_Models(); // Unload all Models

    /**##########################################
     * #       Mesh Manipulation Functions      #
     * ##########################################*/

    Model Scale_Model(Model &model, float scale);

    Model Position_Model(Model &model, Vector3 position);

    Model Rotate_Model(Model &model, Vector3 rotatiton);

    Model SDF_Model(Model &model);

    Model FaceOffset_Model(Model &model, float offset);

    private:
    
    bool ID_Check(int id, std::vector<multimodel> &list);

    private:
    std::vector<multimodel> models;
};

#endif