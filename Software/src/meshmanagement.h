#ifndef MESHMANAGMENT
#define MESHMANAGEMENT

#include <vector>
#include <raylib.h>

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
    
    void Add_Model(int id, const char *model_path);

    void Rem_Model(int id);

    Model Ret_Model(int id);

    void Rep_Model(int id, Vector3 position);

    int CNT_Models();

    void Run_Models();

    private:
    
    bool ID_Check(int id, std::vector<multimodel> &list);

    private:
    std::vector<multimodel> models;
};

#endif