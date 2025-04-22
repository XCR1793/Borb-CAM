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

struct Triangle{
    Vector3 Vertex1;
    Vector3 Vertex2;
    Vector3 Vertex3;
};

struct Line{
    Vector3 startLinePos;
    Vector3 startLineRot;
    Vector3 endLinePos;
    Vector3 endLineRot;
    int type; // 1 = Surface, 2 = Movement
    int meshNo;
    int islandNo;
};

class mesh{
    public:
    
    /**##########################################
     * #             Model Functions            #
     * ##########################################*/
    
    void Add_Model(int id, const char *model_path); // Add New Model
    void Add_Model(int id, Model model);

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
     * #             Maths Functions            #
     * ##########################################*/

    float Rad_Deg(float radians);
    Vector3 Rad_Deg(Vector3 angles);
    
    float Deg_Rad(float degrees);
    Vector3 Deg_Rad(Vector3 angles);

    Vector3 RotXYD_XYZ(Vector3 distance_xrot_yrot);

    std::pair<Vector3, bool> IntersectLinePlane(Vector4 planeNormal, Vector3 lineStart, Vector3 lineEnd);

    std::pair<Triangle, bool> IntersectTrianglePlane(Vector4 planeNormal, Triangle triangle);

    /**
     * Add point to point, to line equation
     * Add Line to Plane Intersection 3D Point equation
     * Run the intersection equation through all object points
     * Return an array with all intersection points
     */

    /**##########################################
     * #       Mesh Manipulation Functions      #
     * ##########################################*/

    Model Scale_Model(Model &model, float scale);

    Model Position_Model(Model &model, Vector3 position);

    Model Rotate_Model(Model &model, Vector3 rotatiton);

    std::vector<std::vector<std::pair<int, Triangle>>> List_Triangles(Model model);

    int Triangle_Touching(Triangle first, Triangle second);

    std::vector<std::pair<int, Triangle>> Sort_Triangles(std::vector<std::pair<int, Triangle>> Unsorted_Triangles);

    // Meshes, Islands, Triangle List(Triangle Number, Triangle)
    std::vector<std::vector<std::vector<std::pair<int, Triangle>>>> Intersecting_Triangles(Model &model, Vector4 Coeff_abcd);

    // Pair 1(Line Type [1 = Surface, 2 = Movement]), Pair 2(Start line, End line)
    std::vector<Line> Intersect_Model(Model &model, Vector4 Coeff_abcd);

    

    private:

    /**##########################################
     * #            Private Functions           #
     * ##########################################*/
    
    bool ID_Check(int id, std::vector<multimodel> &list);

    private:
    std::vector<multimodel> models;
};

#endif