#ifndef SLICE
#define SLICE

#include <raylib.h>
#include <vector>
#include "meshmanagement.h"
#include "pathing.h"

struct path_point{Vector3 position; Vector3 rotation; int point_number;};

struct vector_set{std::vector<path_point> point_list;};

struct vectors_per_mesh{std::vector<vector_set> mesh_vector_list; int id;};

struct vectors_per_model{std::vector<vectors_per_mesh> model_vector_list;};

struct paths{
    int id;
    vectors_per_model path_list;
    Vector4 slice_equation; // First 3: Rotation X Y Z, Last Digit: slice height
};

enum TSP_Types{
    Nearest_Neighbor
};

class slice{
    public:
    /**##########################################
     * #               Maths Tools              #
     * ##########################################*/

    // Convert Rotation X Y to Coefficient A B C
    Vector3 rotation_coefficient(float Rotation_X, float Rotation_Y);

    /**##########################################
     * #            Algorithms Tools            #
     * ##########################################*/

    // TSP Algorithm: Selector
    std::vector<Line> TSP(std::vector<Point> points, TSP_Types tsp);

    // TSP Algorithm: Nearest Neighbor
    std::vector<Line> Generate_TSP_Lines_FromPoints(std::vector<Point> points);

    /**##########################################
     * #            Slicing Settings            #
     * ##########################################*/



    /**##########################################
     * #              Slicing Tools             #
     * ##########################################*/

    // 

    // Find Model Pathing
    vectors_per_model model_path(Model model);

    // Function to Slice
    bool Slice();

    // Return Sliced Pathing
    paths Return_Pathing(int id);

    private:
    std::vector<paths> path_list;
    mesh model_list;
    path pathing;
};

#endif