#include "slice.h"

/**##########################################
 * #               Maths Tools              #
 * ##########################################*/

// Convert Rotation X Y to Coefficient A B C
Vector3 slice::rotation_coefficient(float Rotation_X, float Rotation_Y){
    return (Vector3){sin(Rotation_Y), -sin(Rotation_X)*cos(Rotation_Y), cos(Rotation_X)*cos(Rotation_Y)};
}




/**##########################################
 * #              Slicing Tools             #
 * ##########################################*/

// Find Model Pathing
vectors_per_model slice::model_path(Model model){
    return vectors_per_model{};
}



// Function to Slice
bool slice::Slice(){};

// Return Sliced Pathing
paths slice::Return_Pathing(int id){
    for(auto p : path_list){
        if(p.id == id){
            return p;
        }
    }
    return paths{};
}