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

Vector3 slice::rotate_Vector(Vector3 A, Vector3 B, float rotation_radians){
    // Normalize both input vectors
    A = Vector3Normalize(A);
    B = Vector3Normalize(B);

    // Compute angle between A and B
    float cosPhi = Clamp(Vector3DotProduct(A, B), -1.0f, 1.0f);
    float phi = acosf(cosPhi); // Total angle between A and B

    // Handle edge cases
    if(phi == 0.0f){return A;}             // Already aligned
    if(rotation_radians >= phi){return B;} // Go fully to B

    // SLERP weights
    float sinPhi = sinf(phi);
    float weightA = sinf(phi - rotation_radians) / sinPhi;
    float weightB = sinf(rotation_radians) / sinPhi;

    // Interpolated result
    Vector3 result = {
        weightA * A.x + weightB * B.x,
        weightA * A.y + weightB * B.y,
        weightA * A.z + weightB * B.z
    };

    return Vector3Normalize(result); // Ensure it's still a unit vector
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

std::vector<Line> slice::Interpolate_Normal_Angles(Line A, Line B, const float Max_Angle_Displacement){
    float angle_displacement = Vector3Angle(A.endLinePoint.Normal, B.startLinePoint.Normal);
    if(angle_displacement <= Max_Angle_Displacement){return {};}

    std::vector<Line> Interpolated_Angles;

    int steps = (int)(angle_displacement / Max_Angle_Displacement);
    if(steps <= 0){return {};}

    Line Increments;
    Increments.startLinePoint.Position = A.endLinePoint.Position;
    Increments.endLinePoint.Position   = A.startLinePoint.Position;
    Increments.type     = 3; // Temp for Testing
    Increments.meshNo   = A.meshNo;
    Increments.islandNo = A.islandNo;

    for(int i = 1; i <= steps; ++i){
        Increments.startLinePoint.Normal = rotate_Vector(A.endLinePoint.Normal, B.startLinePoint.Normal, Max_Angle_Displacement * i);
        Interpolated_Angles.push_back(Increments);
    }

    for(size_t j = 0; j < Interpolated_Angles.size(); ++j){
        if(j < Interpolated_Angles.size() - 1){
            Interpolated_Angles[j].endLinePoint = Interpolated_Angles[j + 1].startLinePoint;
        }else{
            Interpolated_Angles[j].endLinePoint = B.startLinePoint;
        }
    }

    return Interpolated_Angles;
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

slice& slice::Toggle_Ground_Grid(const Setting_Actions Enable_Disable){
    switch(Enable_Disable){
        case Setting_Actions::Enable:
        case Setting_Actions::Disable:
            config.Ground_Grid = Enable_Disable;
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
 * #         Visualisation Settings         #
 * ##########################################*/

void slice::Comp_Axis_Guides_3D(){
    Settings Settings_Copy = config;
    if(Settings_Copy.Axis_Guides_3D == Setting_Actions::Enable){
        const float AxisLength = 10000.0f;
        DrawLine3D((Vector3){0, 0, 0}, (Vector3){ AxisLength, 0, 0 }, Settings_Copy.Axis_Guides_3D_X_P);
        DrawLine3D((Vector3){0, 0, 0}, (Vector3){-AxisLength, 0, 0 }, Settings_Copy.Axis_Guides_3D_X_N);

        DrawLine3D((Vector3){0, 0, 0}, (Vector3){0,  AxisLength, 0 }, Settings_Copy.Axis_Guides_3D_Y_P);
        DrawLine3D((Vector3){0, 0, 0}, (Vector3){0, -AxisLength, 0 }, Settings_Copy.Axis_Guides_3D_Y_N);

        DrawLine3D((Vector3){0, 0, 0}, (Vector3){0, 0,  AxisLength }, Settings_Copy.Axis_Guides_3D_Z_P);
        DrawLine3D((Vector3){0, 0, 0}, (Vector3){0, 0, -AxisLength }, Settings_Copy.Axis_Guides_3D_Z_N);
    }
}

void slice::Comp_Ground_Grid(){
    Settings Settings_Copy = config;
    if(Settings_Copy.Ground_Grid != Setting_Actions::Enable){return;}

    float Grid_X_PN_Distance = Settings_Copy.Ground_Grid_Size.x;
    float Grid_Z_PN_Distance = Settings_Copy.Ground_Grid_Size.y;

    float MicroGrid_Size = Settings_Copy.Ground_Grid_Spacing/Settings_Copy.Ground_Grid_MicroSpacing_Count;

    bool Enable_Start_Line = 1;
    if(Settings_Copy.Axis_Guides_3D == Setting_Actions::Enable){Enable_Start_Line = 0;}

    for(float i = 0; i < Grid_X_PN_Distance; i += Settings_Copy.Ground_Grid_Spacing){
        if(!Enable_Start_Line && (i==0)){
        }else{
            DrawLine3D((Vector3){ i, 0, -Grid_Z_PN_Distance}, (Vector3){ i, 0, Grid_Z_PN_Distance}, Settings_Copy.Ground_Grid_Colour);
            DrawLine3D((Vector3){-i, 0, -Grid_Z_PN_Distance}, (Vector3){-i, 0, Grid_Z_PN_Distance}, Settings_Copy.Ground_Grid_Colour);
        }
        
        for(size_t j = 1; j < Settings_Copy.Ground_Grid_MicroSpacing_Count; j++){
            DrawLine3D((Vector3){ i + (j * MicroGrid_Size), 0, -Grid_Z_PN_Distance}, (Vector3){ i + (j * MicroGrid_Size), 0, Grid_Z_PN_Distance}, Settings_Copy.Ground_Grid_MicroSpacing_Colour);
            DrawLine3D((Vector3){-i - (j * MicroGrid_Size), 0, -Grid_Z_PN_Distance}, (Vector3){-i - (j * MicroGrid_Size), 0, Grid_Z_PN_Distance}, Settings_Copy.Ground_Grid_MicroSpacing_Colour);
        }
    }

    for(float i = 0; i < Grid_Z_PN_Distance; i += Settings_Copy.Ground_Grid_Spacing){
        if(!Enable_Start_Line && (i==0)){
        }else{
            DrawLine3D((Vector3){-Grid_Z_PN_Distance, 0, i }, (Vector3){Grid_Z_PN_Distance, 0, i }, Settings_Copy.Ground_Grid_Colour);
            DrawLine3D((Vector3){-Grid_Z_PN_Distance, 0, -i}, (Vector3){Grid_Z_PN_Distance, 0, -i}, Settings_Copy.Ground_Grid_Colour);
        }
        
        for(size_t j = 1; j < Settings_Copy.Ground_Grid_MicroSpacing_Count; j++){
            DrawLine3D((Vector3){-Grid_Z_PN_Distance, 0, i + (j * MicroGrid_Size)}, (Vector3){Grid_Z_PN_Distance, 0, i + (j * MicroGrid_Size)}, Settings_Copy.Ground_Grid_MicroSpacing_Colour);
            DrawLine3D((Vector3){-Grid_Z_PN_Distance, 0,-i - (j * MicroGrid_Size)}, (Vector3){Grid_Z_PN_Distance, 0,-i - (j * MicroGrid_Size)}, Settings_Copy.Ground_Grid_MicroSpacing_Colour);
        }
    }
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
    if(Distance > 0.01){std::cout << "Small slicing distances will cause lag, are you sure you need this level of precision?" << std::endl;}
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

slice& slice::Set_Ending_Position(const Setting_Values& Position){
    config.Ending_Position = Position;
    return *this;
}

slice& slice::Set_Ending_Rotation(const Setting_Values& Rotation){
    config.Ending_Rotation = Rotation;
    return *this;
}

void slice::import_Settings(Settings setting){
    config = setting;
}

Settings slice::return_Settings(){
    return config;
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

std::vector<std::vector<Line>> slice::Add_Start_End_Positions(std::vector<std::vector<Line>>& ToolPaths){
    if(ToolPaths.empty()){return ToolPaths;}

    std::vector<std::vector<Line>> All_ToolPaths = ToolPaths;

    // Local Copy of Settings
    Settings Settings_Copy = config;

    // Calculate Non-Auto (Manual) Positions
    Vector3 Starting_Pos = Settings_Copy.Starting_Position.value3D;
    Vector3 Starting_Rot = Settings_Copy.Starting_Rotation.value3D;
    Vector3 Ending_Pos = Settings_Copy.Ending_Position.value3D;
    Vector3 Ending_Rot = Settings_Copy.Ending_Rotation.value3D;

    Line Starting_Line, Ending_Line;

    // Update position and normal seperately according to which one is in auto mode
    if(Settings_Copy.Starting_Position.mode){
        Starting_Line.startLinePoint.Position = All_ToolPaths.front().front().startLinePoint.Position;
        Starting_Line.endLinePoint.Position   = All_ToolPaths.front().front().startLinePoint.Position;
    }else{
        Starting_Line.startLinePoint.Position = Starting_Pos;
        Starting_Line.endLinePoint.Position   = All_ToolPaths.front().front().startLinePoint.Position;
    }
    
    if(Settings_Copy.Starting_Rotation.mode){
        Starting_Line.startLinePoint.Normal = All_ToolPaths.front().front().startLinePoint.Normal;
        Starting_Line.endLinePoint.Normal   = All_ToolPaths.front().front().startLinePoint.Normal;
    }else{
        Starting_Line.startLinePoint.Normal = Starting_Rot;
        Starting_Line.endLinePoint.Normal   = All_ToolPaths.front().front().startLinePoint.Normal;
    }
    
    if(Settings_Copy.Ending_Position.mode){
        Ending_Line.startLinePoint.Position = All_ToolPaths.back().back().endLinePoint.Position;
        Ending_Line.endLinePoint.Position   = All_ToolPaths.back().back().endLinePoint.Position;
    }else{
        Ending_Line.startLinePoint.Position = All_ToolPaths.back().back().endLinePoint.Position;
        Ending_Line.endLinePoint.Position   = Ending_Pos;
    }
    
    if(Settings_Copy.Ending_Rotation.mode){
        Ending_Line.startLinePoint.Normal = All_ToolPaths.back().back().endLinePoint.Normal;
        Ending_Line.endLinePoint.Normal   = All_ToolPaths.back().back().endLinePoint.Normal;
    }else{
        Ending_Line.startLinePoint.Normal = All_ToolPaths.back().back().endLinePoint.Normal;
        Ending_Line.endLinePoint.Normal   = Ending_Rot;
    }

    std::vector<Line> Starting_Line_Vector = {Starting_Line};
    std::vector<Line> Ending_Line_Vector   = {Ending_Line};

    if( Settings_Copy.Starting_Position.mode && Settings_Copy.Starting_Rotation.mode &&
        Settings_Copy.Ending_Position.mode   && Settings_Copy.Ending_Rotation.mode      ){
        return All_ToolPaths;
    }else{
        All_ToolPaths.insert(All_ToolPaths.begin(), Starting_Line_Vector);
        All_ToolPaths.push_back(Ending_Line_Vector);
    }
    
    // Return Modified Toolpath
    return All_ToolPaths;
}

std::vector<std::vector<Line>> slice::Interpolate_Max_Angle_Displacement(std::vector<std::vector<Line>>& ToolPaths){
    if(ToolPaths.empty()){return ToolPaths;}

    // Local Copy of Settings
    Settings Settings_Copy = config;
    float Max_Angle = Settings_Copy.Max_Angular_Increment;

    std::vector<std::vector<Line>> Result;
    Result.reserve(ToolPaths.size());

    // Walk through each top-level toolpath
    for(size_t i = 0; i < ToolPaths.size(); ++i){
        const std::vector<Line>& currentSlice = ToolPaths[i];
        std::vector<Line> processedSlice;

        for(size_t j = 0; j < currentSlice.size(); ++j) {
            const Line& currentLine = currentSlice[j];
            processedSlice.push_back(currentLine);

            // Determine if a next line exists (could be in same slice or next one)
            bool nextInSameSlice = (j + 1 < currentSlice.size());
            bool nextInNextSlice = (!nextInSameSlice && i + 1 < ToolPaths.size() && !ToolPaths[i + 1].empty());

            if (nextInSameSlice || nextInNextSlice) {
                Line nextLine = nextInSameSlice ? currentSlice[j + 1] : ToolPaths[i + 1][0];

                // Interpolate normals from currentLine.end → nextLine.start
                std::vector<Line> interpolated = Interpolate_Normal_Angles(currentLine, nextLine, Max_Angle);
                if (!interpolated.empty()) {
                    processedSlice.insert(processedSlice.end(), interpolated.begin(), interpolated.end());
                }
            }
        }

        std::cout << "Angle Interpolations" << processedSlice.size() << std::endl;

        Result.push_back(processedSlice);
    }

    return Result;
}

std::vector<Line> slice::Toolpath_Flattener(std::vector<std::vector<Line>>& ToolPaths){
    if(ToolPaths.empty()){return {};}
    std::vector<Line> Flattened_Toolpath;
    Flattened_Toolpath.clear();

    for(const auto& segment : ToolPaths){
        Flattened_Toolpath.insert(Flattened_Toolpath.end(), segment.begin(), segment.end());
    }

    return Flattened_Toolpath;
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

std::vector<std::vector<Line>> slice::Add_Start_End_Positions(){
    Current_Toolpath = Add_Start_End_Positions(Current_Toolpath);
    return Current_Toolpath;
}

std::vector<std::vector<Line>> slice::Interpolate_Max_Angle_Displacement(){
    Current_Toolpath = Interpolate_Max_Angle_Displacement(Current_Toolpath);
    return Current_Toolpath;
}

std::vector<Line> slice::Toolpath_Flattener(){
    return Toolpath_Flattener(Current_Toolpath);
}