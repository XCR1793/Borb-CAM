#ifndef MESHMANAGEMENT
#define MESHMANAGEMENT

#include <string>
#include <vector>
#include <raylib.h>
#include <raymath.h>
#include <cfloat>

struct multimodel{
    int id;
    Model model;
    Vector3 pos = {0, 0, 0};
    Vector3 rot = {0, 0, 0};
    float scale = 1;
    Color colour = ORANGE;
};

struct Point{
    Vector3 Position;
    Vector3 Normal;
};

struct Triangle{
    Point Vertex1;
    Point Vertex2;
    Point Vertex3;
};

struct Line{
    Point startLinePoint;
    Point endLinePoint;
    int type; // 1 = Surface, 2 = Movement
    int meshNo;
    int islandNo;
};

struct Lines{
    std::vector<Line> lineList;
    Vector3 startPosition;
    Vector3 endPosition;
    float distance;
};

struct LineList{
    std::vector<Lines> allLines;
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

    Vector3 NormalToRotation(Vector3 normal); // Currently Not Working Properly

    std::pair<Point, bool> IntersectLinePlane(Vector4 planeNormal, Point lineStart, Point lineEnd);

    std::pair<Triangle, bool> IntersectTrianglePlane(Vector4 planeNormal, Triangle triangle);

    Vector3 MovePointAlongNormal3D(Vector3 startPoint, Vector3 normal, float distance);
    
    bool RayIntersectsAABB(Vector3 rayOrigin, Vector3 rayDir, BoundingBox box, Vector3* out);

    Vector3 PointToVec3(Point point);

    /**
     * Add point to point, to line equation
     * Add Line to Plane Intersection 3D Point equation
     * Run the intersection equation through all object points
     * Return an array with all intersection points
     */

    /**##########################################
     * #       Mesh Manipulation Functions      #
     * ##########################################*/

    multimodel Scale_Model(multimodel &model, float scale);

    multimodel Position_Model(multimodel &model, Vector3 position);

    multimodel Rotate_Model(multimodel &model, Vector3 rotatiton);

    multimodel Apply_Transformations(multimodel &model);

    std::vector<std::vector<std::pair<int, Triangle>>> List_Triangles(Model model);

    int Triangle_Touching(Triangle first, Triangle second);

    std::vector<std::pair<int, Triangle>> Sort_Triangles(std::vector<std::pair<int, Triangle>> Unsorted_Triangles);

    // Meshes, Islands, Triangle List(Triangle Number, Triangle)
    std::vector<std::vector<std::vector<std::pair<int, Triangle>>>> Intersecting_Triangles(Model &model, Vector4 Coeff_abcd);

    // Pair 1(Line Type [1 = Surface, 2 = Movement]), Pair 2(Start line, End line)
    std::vector<Line> Intersect_Model(Model &model, Vector4 Coeff_abcd);

    bool CheckCollisionPointBox(Vector3 point, BoundingBox box);

    std::vector<Line> Cull_Lines_ByBox(BoundingBox box, const std::vector<Line>& lines); // REDO

    float pointToPointDistance(Vector3 StartPoint, Vector3 EndPoint);

    Point lastPoint(std::vector<Line> lineList, int startNo, bool direction);

    private:

    /**##########################################
     * #            Private Functions           #
     * ##########################################*/
    
    bool ID_Check(int id, std::vector<multimodel> &list);

    private:
    std::vector<multimodel> models;
};
#endif