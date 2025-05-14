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

    /**##########################################
     * #          Mesh Helper Functions         #
     * ##########################################*/

    float pointToPointDistance(Vector3 StartPoint, Vector3 EndPoint);

    float pointToPointDistance(Point StartPoint, Point EndPoint);

    int Triangle_Touching(Triangle first, Triangle second);

    bool CheckCollisionPointBox(Vector3 point, BoundingBox box);

    bool Line_Touching(const Line& a, const Line& b);

    Line Flip_Line(const Line& line);

    std::pair<std::pair<Point, Point>, bool> Intersect_Line_Box(Point startPoint, Point endPoint, BoundingBox box);

    Line Clip_Line_To_Box(const Line& line, BoundingBox box, bool in_out);

    std::vector<Line> Orient_Line_Group(const std::vector<Line>& lines);

    std::vector<Line> Chain_Walker(const std::vector<Line>& unordered);

    std::vector<Line> Flatten_Culled_Lines(BoundingBox box, const std::vector<Line>& lines, bool in_out);

    /**##########################################
     * #       Mesh Manipulation Functions      #
     * ##########################################*/

    multimodel Scale_Model(multimodel &model, float scale);

    multimodel Position_Model(multimodel &model, Vector3 position);

    multimodel Rotate_Model(multimodel &model, Vector3 rotatiton);

    multimodel Apply_Transformations(multimodel &model);

    std::vector<std::vector<std::pair<int, Triangle>>> List_Triangles(Model model);

    std::vector<std::pair<int, Triangle>> Sort_Triangles(std::vector<std::pair<int, Triangle>> Unsorted_Triangles);

    // Meshes, Islands, Triangle List(Triangle Number, Triangle)
    std::vector<std::vector<std::vector<std::pair<int, Triangle>>>> Intersecting_Triangles(Model &model, Vector4 Coeff_abcd);

    // Pair 1(Line Type [1 = Surface, 2 = Movement]), Pair 2(Start line, End line)
    std::vector<Line> Intersect_Model(Model &model, Vector4 Coeff_abcd);

    std::vector<Lines> Group_Continuous(const std::vector<Line>& lines);

    std::vector<Lines> Cull_Lines_ByBox(BoundingBox box, const std::vector<Line>& lines, bool in_out);

    Point lastPoint(std::vector<Line> lineList, int startNo, bool direction);

    private:

    /**##########################################
     * #            Private Functions           #
     * ##########################################*/
    
    bool ID_Check(int id, std::vector<multimodel> &list);

    private:
    std::vector<multimodel> models;
    float epsilon = 0.01f;
};
#endif