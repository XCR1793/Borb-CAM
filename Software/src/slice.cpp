#include "slice.h"

/**##########################################
 * #               Maths Tools              #
 * ##########################################*/

// Convert Rotation X Y to Coefficient A B C
Vector3 slice::rotation_coefficient(float Rotation_X, float Rotation_Y){
    return (Vector3){sin(Rotation_Y), -sin(Rotation_X)*cos(Rotation_Y), cos(Rotation_X)*cos(Rotation_Y)};
}

// Distance to Plane
float slice::distance_to_plane(Vector3 point, Vector4 Coeff_abcd){
    return ((Coeff_abcd.x*point.x)+(Coeff_abcd.y*point.y)+(Coeff_abcd.z*point.z)-Coeff_abcd.w)/(sqrt((Coeff_abcd.x*Coeff_abcd.x)+(Coeff_abcd.y*Coeff_abcd.y)+(Coeff_abcd.z*Coeff_abcd.z)));
}

/**##########################################
 * #            Algorithms Tools            #
 * ##########################################*/

// TSP Selector
// std::vector<int> slice::TSP(std::vector<Point> points, TSP_Algorithm tsp){
//     switch(tsp){
//         case Nearest_Neighbor:
//             return Generate_TSP_Lines_FromPoints(points);
//         default:
//             std::cout << "Invalid TSP Selected" << std::endl;
//             return{};
//     }
// }

// // TSP Nearest Neighbor
// std::vector<Line> slice::NearestNeighborPath(std::vector<Line> lines) {
//     if (lines.empty()) return {};

//     std::vector<Line> result;
//     std::vector<bool> visited(lines.size(), false);

//     // Start from the first line
//     result.push_back(lines[0]);
//     visited[0] = true;

//     while (result.size() < lines.size()) {
//         const Line& current = result.back();
//         Vector3 currentPos = current.endLinePoint.Position;

//         float minDist = std::numeric_limits<float>::max();
//         int nextIndex = -1;
//         bool reverseNext = false;

//         for (size_t i = 0; i < lines.size(); ++i) {
//             if (visited[i]) continue;

//             float distToStart = mesh_Cl(currentPos, lines[i].startLinePoint.Position);
//             float distToEnd   = Distance(currentPos, lines[i].endLinePoint.Position);

//             if (distToStart < minDist) {
//                 minDist = distToStart;
//                 nextIndex = i;
//                 reverseNext = false;
//             }

//             if (distToEnd < minDist) {
//                 minDist = distToEnd;
//                 nextIndex = i;
//                 reverseNext = true;
//             }
//         }

//         if (nextIndex != -1) {
//             Line next = lines[nextIndex];
//             if (reverseNext) {
//                 std::swap(next.startLinePoint, next.endLinePoint);
//             }

//             result.push_back(next);
//             visited[nextIndex] = true;
//         } else {
//             break; // no more reachable segments
//         }
//     }

//     return result;
// }

/**##########################################
 * #            Helper Functions            #
 * ##########################################*/

Vector2 slice::Box_Corner_Distances(BoundingBox Box, Vector4 Coeff_abcd){
    Vector3 corners[8] = {
        {Box.min.x, Box.min.y, Box.min.z},
        {Box.max.x, Box.min.y, Box.min.z},
        {Box.min.x, Box.max.y, Box.min.z},
        {Box.max.x, Box.max.y, Box.min.z},
        {Box.min.x, Box.min.y, Box.max.z},
        {Box.max.x, Box.min.y, Box.max.z},
        {Box.min.x, Box.max.y, Box.max.z},
        {Box.max.x, Box.max.y, Box.max.z}
    };

    float minimum_distance = FLT_MAX;
    float maximum_distance = -FLT_MAX;

    for(int i = 0; i < 8; i++){
        float distance = distance_to_plane(corners[i], Coeff_abcd);
        if(distance < minimum_distance){minimum_distance = distance;}
        if(distance > maximum_distance){maximum_distance = distance;}
    }

    return (Vector2){minimum_distance, maximum_distance};
}

std::vector<Line> slice::Flip_Vector(std::vector<Line>& Vector){
    if(Vector.empty()){return Vector;}    
    std::vector<Line> Flipped;
    for(auto Flipped_Line : Vector){
        Flipped.insert(Flipped.begin(), (Line){
            .startLinePoint = Flipped_Line.endLinePoint,
            .endLinePoint = Flipped_Line.startLinePoint,
            .type = Flipped_Line.type,
            .meshNo = Flipped_Line.meshNo,
            .islandNo = Flipped_Line.islandNo
        });
    }
    return Flipped;
}

std::vector<Line> slice::Modify_Starting_Pos(std::vector<Line>& Vector, int starting_id){
    if(Vector.empty()){return Vector;}

    std::vector<Line> result;
    int size = Vector.size();
    starting_id = ((starting_id % size) + size) % size;

    for(int i = 0; i < size; ++i){
        int index = (starting_id + i) % size;
        result.push_back(Vector[index]);
    }

    return result;
}

/**##########################################
 * #             System Settings            #
 * ##########################################*/

slice& slice::Set_Title(const std::string& New_Title){
    config.Title = New_Title;
    return *this;
}

slice& slice::Set_Description(const std::string& New_Description){
    config.Description = New_Description;
    return *this;
}

slice& slice::Set_Misc_Text(const std::string& New_Misc_Text){
    config.Misc = New_Misc_Text;
    return *this;
}

slice& slice::Set_Output_Directory(const std::string& Output_Directory){
    config.OutputDir = Output_Directory;
    return *this;
}

slice& slice::Set_Output_Name(const std::string& Output_Name){
    config.OutputFileName = Output_Name;
    return *this;
}

slice& slice::Set_Output_Type(const std::string& Output_Type){
    config.OutputFileType = Output_Type;
    return *this;
}

slice& slice::Set_Unit(const Setting_Units& Unit_Type){
    switch(Unit_Type){
        case Setting_Units::Metric:
            config.Unit = Unit_Type;
            config.Unit_Precision = Setting_Units::mm;
            break;
        
        case Setting_Units::Imperial:
            config.Unit = Unit_Type;
            config.Unit_Precision = Setting_Units::mil;
            break;

        default:
            std::cout << "Invalid unit type selected, choose: Metric or Imperial" << std::endl;
            break;
    }
    return *this;
}

slice& slice::Set_Unit_Precision(const Setting_Units& Unit_Precision){
    switch(Unit_Precision){
        case Setting_Units::mm:
        case Setting_Units::cm:
        case Setting_Units::m:
            config.Unit_Precision = Unit_Precision;
            config.Unit = Setting_Units::Metric;
            break;

        case Setting_Units::mil:
        case Setting_Units::inch:
        case Setting_Units::foot:
            config.Unit_Precision = Unit_Precision;
            config.Unit = Setting_Units::Imperial;
            break;
        
        default:
            std::cout << "Invalid unit precision selected, choose: mm, cm, m, mil, inch, foot" << std::endl;
            break;
    }
    return *this;
}

slice& slice::Set_Unit_Rotation(const Setting_Units& Unit_Rotation){
    switch(Unit_Rotation){
        case Setting_Units::rad:
            config.Unit_Rotation = Unit_Rotation;
            break;
        
        case Setting_Units::degrees:
            config.Unit_Rotation = Unit_Rotation;
            break;

        default:
            std::cout << "Invalid unit rotation selected, choose: rad or degrees" << std::endl;
            break;
    }
    return *this;
}

slice& slice::Set_Unit_Time(const Setting_Units& Unit_Time){
    switch(Unit_Time){
        case Setting_Units::milliseconds:
        case Setting_Units::seconds:
        case Setting_Units::minutes:
        case Setting_Units::hours:
            config.Unit_Time = Unit_Time;
            break;

        default:
            std::cout << "Invalid unit time selected, choose: milliseconds, seconds, minutes, hours" << std::endl;
            break;
    }
    return *this;
}

slice& slice::Set_Epsilon_Precision(const float& Epsilon_Precision){
    if(Epsilon_Precision > 0){}else{std::cout << "Epsilon cannot be negative" << std::endl; return *this;}
    if(Epsilon_Precision < 0.1){}else{std::cout << "Large epsilon precisions may lead to large collision checking errors. 0.001 is recommended";}
    config.Epsilon_Precision = Epsilon_Precision;
    return *this;
}

slice& slice::Toggle_Axis_Guides_3D(const Setting_Actions Enable_Disable){
    switch(Enable_Disable){
        case Setting_Actions::Enable:
        case Setting_Actions::Disable:
            config.Axis_Guides_3D = Enable_Disable;
            break;
        default:
            std::cout << "Axis Guides can only be Enabled or Disabled" << std::endl;
            break;
    }
    return *this;
}

slice& slice::Set_Gcode_Rotation_Order(const Gcode_Rotation_Order Gcode_Rotation_Order){
    switch(Gcode_Rotation_Order){
        case Gcode_Rotation_Order::ABC:
        case Gcode_Rotation_Order::ACB:
        case Gcode_Rotation_Order::BAC:
        case Gcode_Rotation_Order::BCA:
        case Gcode_Rotation_Order::CAB:
        case Gcode_Rotation_Order::CBA:
            config.GcodeRotOrder = Gcode_Rotation_Order;
            break;
        default:
            std::cout << "Rotation order must be one of the following: ABC, ACB, BAC, BCA, CAB, CBA" << std::endl;
    }
    return *this;
}

/**##########################################
 * #             Slicer Settings            #
 * ##########################################*/

slice& slice::Set_Slicing_Plane(const Vector4& Plane){
    config.SlicingPlane = Plane;
    return *this;
}

slice& slice::Set_Slicing_Plane(const Vector2& Rotation_X_Y, const float& Plane_Distance){
    Vector3 Coeff_abc = rotation_coefficient(Rotation_X_Y.x, Rotation_X_Y.y);
    config.SlicingPlane = (Vector4){Coeff_abc.x, Coeff_abc.y, Coeff_abc.z, Plane_Distance};
    return *this;
}

slice& slice::Set_Slicing_Distance(const float& Distance){
    if(Distance < 0){std::cout << "Slicing distance cannot be negative." << std::endl; return *this;}
    if(Distance == 0){std::cout << "Slicing distance cannot be 0." << std::endl; return *this;}
    if(Distance > 0.01){std::cout << "Small slicing distances will cause lag, are you sure you need this level of precision?" << std::endl; return *this;}
    config.SlicingDistance = Distance;
    return *this;
}

slice& slice::Set_Slicing_Start_Distance(const Setting_Values& Start_Distance){
    config.SlicingStartDistance = Start_Distance;
    return *this;
}

slice& slice::Set_Slicing_End_Distance(const Setting_Values& End_Distance){
    config.SlicingEndDistance = End_Distance;
    return *this;
}

slice& slice::Set_Max_Speed(const float& Speed){
    if(Speed <= 0){std::cout << "Max speed must be greater than zero." << std::endl; return *this;}
    config.Max_Speed = Speed;
    return *this;
}

slice& slice::Set_Max_Acceleration(const float& Acceleration){
    if(Acceleration <= 0){std::cout << "Max acceleration must be greater than zero." << std::endl; return *this;}
    config.Max_Acceleration = Acceleration;
    return *this;
}

slice& slice::Set_Max_Angular_Increment(const float& AngularIncrement){
    if(AngularIncrement <= 0){std::cout << "Angular increment must be greater than zero." << std::endl; return *this;}
    config.Max_Angular_Increment = AngularIncrement;
    return *this;
}

slice& slice::Set_Starting_Position(const Setting_Values& Position){
    config.Starting_Position = Position;
    return *this;
}

slice& slice::Set_Starting_Rotation(const Setting_Values& Rotation){
    config.Starting_Rotation = Rotation;
    return *this;
}

slice& slice::Set_Finishing_Position(const Setting_Values& Position){
    config.Finishing_Position = Position;
    return *this;
}

slice& slice::Set_Finishing_Rotation(const Setting_Values& Rotation){
    config.Finishing_Rotation = Rotation;
    return *this;
}

/**##########################################
 * #              Slicing Tools             #
 * ##########################################*/

std::vector<std::vector<Line>> slice::Generate_Surface_Toolpath(Model model){
    // Store Local Values of Settings
    Settings Settings_Copy = config;

    // Variables
    float startDist = Settings_Copy.SlicingStartDistance.value, endDist = Settings_Copy.SlicingEndDistance.value;

    // Calculate Autos
    Vector2 Start_End_Distances = Box_Corner_Distances(GetModelBoundingBox(model), Settings_Copy.SlicingPlane);

    if(Settings_Copy.SlicingStartDistance.mode){}else{Start_End_Distances.x = startDist;}
    if(Settings_Copy.SlicingEndDistance.mode  ){}else{Start_End_Distances.y = endDist;  }
    
    // Slicing
    std::vector<std::vector<Line>> All_Slices;
    if(model.meshes->vertexCount > 2){
        for(float i = Start_End_Distances.x; i <= Start_End_Distances.y; i += Settings_Copy.SlicingDistance){
            Vector4 currentSlicingPlane = Settings_Copy.SlicingPlane;
            currentSlicingPlane.w = i;
            std::vector<Line> result = mesh_Class.Intersect_Model(model, currentSlicingPlane);
            if(!result.empty()){All_Slices.push_back(result);}
        }
    }

    // Apply Slice to self stored toolpath 
    Current_Toolpath.insert(Current_Toolpath.end(), All_Slices.begin(), All_Slices.end());

    // Return Surface Toolpath (Using this output RAW isnt recommended)
    return All_Slices;
}

std::vector<std::vector<Line>> slice::Cull_Toolpath_by_Box(std::vector<std::vector<Line>>& Toolpaths, BoundingBox cullBox){
    if(Toolpaths.empty()){return Toolpaths;}

    // Culling Toolpaths
    std::vector<std::vector<Line>> All_Slices;
    for(auto Slice_Set : Toolpaths){
        std::vector<Line> result = mesh_Class.Flatten_Culled_Lines(cullBox, Slice_Set, false);
        if(!result.empty()){All_Slices.push_back(result);}
    }

    // Return Culled Toolpath (Using this output RAW isnt recommended)
    return All_Slices;
}

std::vector<std::vector<Line>> slice::Apply_AABB_Rays(std::vector<std::vector<Line>>& ToolPaths, BoundingBox AABB_Box){
    if(ToolPaths.empty()){return ToolPaths;}

    // Applying AABB Rays to Start & End of toolpath
    std::vector<std::vector<Line>> AllSlices;
    for(auto& Slice : ToolPaths){
        if(Slice.empty()) continue;

        std::vector<Line> result = mesh_Class.Orient_Line_Group(Slice);

        Point entry = Slice.front().startLinePoint;
        Point exit  = Slice.back().endLinePoint;

        Vector3 entry_Direction = Vector3Negate(Vector3Normalize(entry.Normal));
        Vector3 exit_Direction  = Vector3Negate(Vector3Normalize(exit.Normal));
        Vector3 entry_Point = {0, 0, 0}, exit_Point = {0, 0, 0};

        bool entry_hit = mesh_Class.RayIntersectsAABB(entry.Position, entry_Direction, AABB_Box, &entry_Point);
        bool exit_hit  = mesh_Class.RayIntersectsAABB(exit.Position , exit_Direction , AABB_Box, &exit_Point );

        if(entry_hit && exit_hit){
            // Create front and end lines
            Line entryLine = {.startLinePoint = {entry_Point, entry.Normal}, .endLinePoint = entry};
            Line exitLine  = {.startLinePoint = exit, .endLinePoint = {exit_Point, exit.Normal}};

            // Insert at front and back
            result.insert(result.begin(), entryLine);
            result.push_back(exitLine);

            AllSlices.push_back(result);
        }
    }

    // Return Toolpath with AABB Rays attached to the start and end of every continuous path
    return AllSlices;
}

std::vector<std::vector<Line>> slice::Optimise_Start_End_Positions(std::vector<std::vector<Line>>& ToolPaths){
    if(ToolPaths.empty()){return ToolPaths;}

    std::vector<std::vector<Line>> Optimised;

    // Start with the first slice unchanged
    Optimised.push_back(ToolPaths[0]);
    Point lastExit = ToolPaths[0].back().endLinePoint;
    for(size_t i = 1; i < ToolPaths.size(); ++i){
        std::vector<Line>& current = ToolPaths[i];

        if(current.empty()){
            Optimised.push_back(current);
            continue;
        }

        Point loopStart = current.front().startLinePoint;
        Point loopEnd   = current.back().endLinePoint;

        // Check if it's loopable (i.e., first point == last point)
        bool isLoop = Vector3Distance(loopStart.Position, loopEnd.Position) < config.Epsilon_Precision;

        if(!isLoop){
            Optimised.push_back(current); // can't rotate, keep as-is
            lastExit = current.back().endLinePoint;
            continue;
        }

        // Try all rotation positions and pick the one closest to lastExit
        int bestIndex = 0;
        float bestDistance = std::numeric_limits<float>::max();

        for(size_t j = 0; j < current.size(); ++j){
            Point candidateStart = current[j].startLinePoint;
            float dist = Vector3Distance(lastExit.Position, candidateStart.Position);
            if(dist < bestDistance){
                bestDistance = dist;
                bestIndex = j;
            }
        }

        std::vector<Line> rotated = Modify_Starting_Pos(current, bestIndex);
        Optimised.push_back(rotated);
        lastExit = rotated.back().endLinePoint;
    }

    return Optimised;
}

std::vector<std::vector<Line>> slice::Optimise_Start_End_Linkages(std::vector<std::vector<Line>>& ToolPaths){
    if(ToolPaths.empty()){return ToolPaths;}

    std::vector<std::vector<Line>> OptimisedPaths;

    // Start with the first path
    OptimisedPaths.push_back(ToolPaths[0]);
    Point lastExit = ToolPaths[0].back().endLinePoint;

    for(size_t i = 1; i < ToolPaths.size(); ++i){
        std::vector<Line> currentPath = ToolPaths[i];
        Point entry = currentPath.front().startLinePoint;
        Point exit  = currentPath.back().endLinePoint;

        float distToStart = Vector3Distance(lastExit.Position, entry.Position);
        float distToEnd   = Vector3Distance(lastExit.Position, exit.Position);

        bool shouldFlip = (distToEnd < distToStart);
        if(shouldFlip){
            currentPath = Flip_Vector(currentPath);
            std::swap(entry, exit);
        }

        // Create the linkage line from lastExit to current entry
        Line linkageLine = {
            .startLinePoint = lastExit,
            .endLinePoint = entry,
            .type = 2
        };

        // Add the linkage as a separate inner vector
        OptimisedPaths.push_back({linkageLine});

        // Then add the actual path
        OptimisedPaths.push_back(currentPath);

        lastExit = exit;
    }

    return OptimisedPaths;
}

/**##########################################
 * #        Slicing Tools (Buffered)        #
 * ##########################################*/

std::vector<std::vector<Line>> slice::Clear_Toolpath(){
    Current_Toolpath.clear();
    return Current_Toolpath;
}

std::vector<std::vector<Line>> slice::Load_Toolpath(std::vector<std::vector<Line>>& ToolPaths){
    if(ToolPaths.empty()){return ToolPaths;}
    Current_Toolpath.clear();
    Current_Toolpath = ToolPaths;
    return Current_Toolpath;
}

std::vector<std::vector<Line>> slice::Return_Toolpath(){
    return Current_Toolpath;
}

std::vector<std::vector<Line>> slice::Cull_Toolpath_by_Box(BoundingBox cullBox){
    Current_Toolpath = Cull_Toolpath_by_Box(Current_Toolpath, cullBox);
    return Current_Toolpath;
}

std::vector<std::vector<Line>> slice::Apply_AABB_Rays(BoundingBox AABB_Box){
    Current_Toolpath = Apply_AABB_Rays(Current_Toolpath, AABB_Box);
    return Current_Toolpath;
}

std::vector<std::vector<Line>> slice::Optimise_Start_End_Positions(){
    Current_Toolpath = Optimise_Start_End_Positions(Current_Toolpath);
    return Current_Toolpath;
}

std::vector<std::vector<Line>> slice::Optimise_Start_End_Linkages(){
    Current_Toolpath = Optimise_Start_End_Linkages(Current_Toolpath);
    return Current_Toolpath;
}