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

// // TSP Selector
// std::vector<Line> slice::TSP(std::vector<Point> points, TSP_Types tsp){
//     switch(tsp){
//         case Nearest_Neighbor:
//             return Generate_TSP_Lines_FromPoints(points);
//         default:
//             return{};
//     }
// }

// // TSP Nearest Neighbor
// std::vector<Line> slice::Generate_TSP_Lines_FromPoints(std::vector<Point> points){
//     std::vector<Line> tspLines;
//     int n = points.size();
//     if(n < 2) return tspLines;

//     std::vector<bool> visited(n, false);
//     std::vector<int> path;
//     path.push_back(0);
//     visited[0] = true;

//     for(int step = 1; step < n; ++step){
//         int last = path.back();
//         float minDist = FLT_MAX;
//         int nextIdx = -1;

//         for(int i = 0; i < n; ++i){
//             if(!visited[i]){
//                 float dist = model_list.pointToPointDistance(points[last].Position, points[i].Position);
//                 if(dist < minDist){
//                     minDist = dist;
//                     nextIdx = i;
//                 }
//             }
//         }

//         if(nextIdx >= 0){
//             visited[nextIdx] = true;
//             path.push_back(nextIdx);
//         }
//     }

//     for (size_t i = 0; i < path.size() - 1; ++i) {
//         Line tspLine;
//         tspLine.startLinePoint = points[path[i]];
//         tspLine.endLinePoint = points[path[i + 1]];
//         tspLine.type = 2;
//         tspLines.push_back(tspLine);
//     }

//     return tspLines;
// }

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
