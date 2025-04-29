#ifndef SLICE
#define SLICE

#include <raylib.h>
#include <vector>
#include <meshmanagement.h>
#include <pathing.h>

struct path_point{
    Vector3 position;
    Vector3 rotation;
    int point_number;
};

struct vector_set{std::vector<path_point> point_list;};

struct vectors_per_mesh{std::vector<vector_set> mesh_vector_list; int id;};

struct vectors_per_model{std::vector<vectors_per_mesh> model_vector_list;};

struct path_model_params{
    Model model;
    Vector4 slice_plane_equation; // First 3: Coeff X Y Z, Last Digit: slice height
};

struct paths{
    int id;
    vectors_per_model path_list;
    path_model_params model_list;
};


class slice{
    public:
    /**##########################################
     * #              Slicing Tools             #
     * ##########################################*/

    // 



    // Function to Slice
    bool Slice();

    // Return Sliced Pathing
    vectors_per_model Return_Pathing(int id);




    private:
    std::vector<paths> path_list;
};

#endif