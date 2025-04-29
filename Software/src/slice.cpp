#include "slice.h"

/**##########################################
 * #              Slicing Tools             #
 * ##########################################*/

// 



// Function to Slice
bool slice::Slice(){}

// Return Sliced Pathing
vectors_per_model slice::Return_Pathing(int id){
    for(auto p : path_list){
        if(p.id == id){
            return p.path_list;
        }
    }
    return vectors_per_model{};
}