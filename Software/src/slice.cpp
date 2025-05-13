#include "slice.h"

/**##########################################
 * #               Maths Tools              #
 * ##########################################*/

// Convert Rotation X Y to Coefficient A B C
Vector3 slice::rotation_coefficient(float Rotation_X, float Rotation_Y){
    return (Vector3){sin(Rotation_Y), -sin(Rotation_X)*cos(Rotation_Y), cos(Rotation_X)*cos(Rotation_Y)};
}

/**##########################################
 * #            Algorithms Tools            #
 * ##########################################*/

// TSP Selector
std::vector<Line> slice::TSP(std::vector<Point> points, TSP_Types tsp){
    switch(tsp){
        case Nearest_Neighbor:
            return Generate_TSP_Lines_FromPoints(points);
        default:
            return{};
    }
}

// TSP Nearest Neighbor
std::vector<Line> slice::Generate_TSP_Lines_FromPoints(std::vector<Point> points){
    std::vector<Line> tspLines;
    int n = points.size();
    if(n < 2) return tspLines;

    std::vector<bool> visited(n, false);
    std::vector<int> path;
    path.push_back(0);
    visited[0] = true;

    for(int step = 1; step < n; ++step){
        int last = path.back();
        float minDist = FLT_MAX;
        int nextIdx = -1;

        for(int i = 0; i < n; ++i){
            if(!visited[i]){
                float dist = model_list.pointToPointDistance(points[last].Position, points[i].Position);
                if(dist < minDist){
                    minDist = dist;
                    nextIdx = i;
                }
            }
        }

        if(nextIdx >= 0){
            visited[nextIdx] = true;
            path.push_back(nextIdx);
        }
    }

    for (size_t i = 0; i < path.size() - 1; ++i) {
        Line tspLine;
        tspLine.startLinePoint = points[path[i]];
        tspLine.endLinePoint = points[path[i + 1]];
        tspLine.type = 2;
        tspLines.push_back(tspLine);
    }

    return tspLines;
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