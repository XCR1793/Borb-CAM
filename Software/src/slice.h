#ifndef SLICE
#define SLICE

#include <raylib.h>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <limits>
#include "meshmanagement.h"
#include "pathing.h"



enum TSP_Algorithm{
    Nearest_Neighbor
};

enum Gcode_Rotation_Order{
    ABC,
    ACB,
    BAC,
    BCA,
    CAB,
    CBA
};

enum Setting_Actions{
    Enable,     // Enabling a thing, function or action
    Disable     // Disabling a thing, function or action
};

enum Setting_Units{
    Metric,     // Default for system
    Imperial,
    mm,         // Default for metric
    cm,
    m,
    mil,        // Default for imperial
    inch,
    foot,
    degrees,    // Default for rotation (internal math is done in radians but converted to deg)
    rad,
    milliseconds,
    seconds,    // Default Time Value
    minutes,
    hours
};

struct Setting_Values{
    bool mode = 0;
    float value = 0;
    Vector3 value3D = {0, 0, 0};
    float increment = 0;
};

struct Settings{
    // System Settings
    std::string Title = "Default Settings";
    std::string Description = "These are the default settings";
    std::string Misc = "Any miscellaneous text";
    Setting_Units Unit = Setting_Units::Metric; // Either Metric or Imperial
    Setting_Units Unit_Precision = Setting_Units::mm; // The unit of precision
    Setting_Units Unit_Rotation = Setting_Units::degrees; // The rotational output units
    Setting_Units Unit_Time = Setting_Units::seconds;
    float Epsilon_Precision = 0.0001f; // Precision used for collision & other internal calculations

    // Model Settings
    Vector3 Absolute_Position = {0, 0, 0};      // X, Y, Z
    Vector3 Model_Origin_Rotation = {0, 0, 0};  // Around X, Y, Z Axis
    float Absolute_Model_Scale = 1;

    // Slicer Settings
    Vector4 SlicingPlane = {0, 0, 0, 0};    // Plane Coeeficient: X, Y, Z, D (distance offset)
    float SlicingDistance = 0.2f;
    Setting_Values SlicingStartDistance = {.mode = 1};       // Mode 1 means its Auto (mode, value) only
    Setting_Values SlicingEndDistance = {.mode = 1};          // Mode 1 means its Auto (mode, value) only
    float Max_Speed = 100.0f;               // 100mm/s default
    float Max_Acceleration = 2000.0f;       // 2000mm/s^2 default
    float Max_Angular_Increment = 0.001f; // 0.0872664625f;    // 5 Deg in rad increments (Max amount of turn per action)
    Setting_Values Starting_Position = {.mode = 1}; // Mode 1 = Auto (start where it is), (mode, value3D, increment) only
    Setting_Values Starting_Rotation = {.mode = 1}; // Mode 1 = Auto (start where it is), (mode, value3D, increment) only
    Setting_Values Ending_Position = {.mode = 1}; // Mode 1 = Auto (finish where it is), (mode, value3D, increment) only
    Setting_Values Ending_Rotation = {.mode = 1}; // Mode 1 = Auto (finish where it is), (mode, value3D, increment) only

    // Visualisation Settings
    Setting_Actions Axis_Guides_3D = Setting_Actions::Enable; // X Y Z Guides in 3D Space (Enable/Disable) only
    Color Axis_Guides_3D_X_P = {255, 40, 40, 160};
    Color Axis_Guides_3D_X_N = {160, 60, 60, 127};
    Color Axis_Guides_3D_Y_P = {40, 255, 100, 160};
    Color Axis_Guides_3D_Y_N = {60, 180, 80, 127};
    Color Axis_Guides_3D_Z_P = {40, 80, 255, 160};
    Color Axis_Guides_3D_Z_N = {60, 100, 180, 127};

    Setting_Actions Ground_Grid = Setting_Actions::Enable; // 2D grid placed along the XZ Plane
    Vector2 Ground_Grid_Size = {100, 100};
    float Ground_Grid_Spacing = 10;
    float Ground_Grid_MicroSpacing_Count = 10;
    Color Ground_Grid_Colour = {120, 120, 120, 127};
    Color Ground_Grid_MicroSpacing_Colour = {120, 120, 120, 50};

    // Output Settings
    std::string OutputDir = "src/";
    std::string OutputFileName = "OwO";
    std::string OutputFileType = ".nc";

    // Misc Settings
    Gcode_Rotation_Order GcodeRotOrder = Gcode_Rotation_Order::ABC;
};

class slice{
    public:
    /**##########################################
     * #               Maths Tools              #
     * ##########################################*/

    // Convert Rotation X Y to Coefficient A B C
    Vector3 rotation_coefficient(float Rotation_X, float Rotation_Y);

    // Distance to Plane
    float distance_to_plane(Vector3 point, Vector4 Coeff_abcd);

    // New Vector that is a certain angle from Vector A but lies in the plane between Vector A and B
    Vector3 rotate_Vector(Vector3 A, Vector3 B, float rotation_radians);

    /**##########################################
     * #            Algorithms Tools            #
     * ##########################################*/

    // // TSP Algorithm: Selector
    // std::vector<int> TSP(std::vector<Point> points, TSP_Algorithm tsp);

    // // TSP Algorithm: Nearest Neighbor
    // std::vector<Line> NearestNeighborPath(std::vector<Line> lines);

    /**##########################################
     * #            Helper Functions            #
     * ##########################################*/

    Vector2 Box_Corner_Distances(BoundingBox Box, Vector4 Coeff_abcd);

    std::vector<Line> Flip_Vector(std::vector<Line>& Vector);

    std::vector<Line> Modify_Starting_Pos(std::vector<Line>& Vector, int starting_id);

    std::vector<Line> Interpolate_Normal_Angles(Line A, Line B, const float Max_Angle_Displacement);

    /**##########################################
     * #             System Settings            #
     * ##########################################*/
    
    // Metadata
    slice& Set_Title(const std::string& New_Title);
    slice& Set_Description(const std::string& New_Description);
    slice& Set_Misc_Text(const std::string& New_Misc_Text);

    slice& Set_Output_Directory(const std::string& Output_Directory);
    slice& Set_Output_Name(const std::string& Output_Name);
    slice& Set_Output_Type(const std::string& Output_Type);

    slice& Set_Unit(const Setting_Units& Unit_Type);
    slice& Set_Unit_Precision(const Setting_Units& Unit_Precision);
    slice& Set_Unit_Rotation(const Setting_Units& Unit_Rotation);
    slice& Set_Unit_Time(const Setting_Units& Unit_Time);
    slice& Set_Epsilon_Precision(const float& Epsilon_Precision);

    slice& Toggle_Axis_Guides_3D(const Setting_Actions Enable_Disable);
    slice& Toggle_Ground_Grid(const Setting_Actions Enable_Disable);

    slice& Set_Gcode_Rotation_Order(const Gcode_Rotation_Order Gcode_Rotation_Order);

    /**##########################################
     * #         Visualisation Settings         #
     * ##########################################*/

    void Comp_Axis_Guides_3D();
    void Comp_Ground_Grid();

    /**##########################################
     * #             Slicer Settings            #
     * ##########################################*/

    slice& Set_Slicing_Plane(const Vector4& Plane);
    slice& Set_Slicing_Plane(const Vector2& Rotation_X_Y, const float& Plane_Distance);
    slice& Set_Slicing_Distance(const float& Distance);
    slice& Set_Slicing_Start_Distance(const Setting_Values& Start_Distance);
    slice& Set_Slicing_End_Distance(const Setting_Values& End_Distance);
    
    slice& Set_Max_Speed(const float& Speed);
    slice& Set_Max_Acceleration(const float& Acceleration);
    slice& Set_Max_Angular_Increment(const float& Angular_Increment);
    
    slice& Set_Starting_Position(const Setting_Values& Position);
    slice& Set_Starting_Rotation(const Setting_Values& Rotation);
    slice& Set_Ending_Position(const Setting_Values& Position);
    slice& Set_Ending_Rotation(const Setting_Values& Rotation);

    /**##########################################
     * #              Slicing Tools             #
     * ##########################################*/

    std::vector<std::vector<Line>> Generate_Surface_Toolpath(Model model);
    std::vector<std::vector<Line>> Cull_Toolpath_by_Box(std::vector<std::vector<Line>>& ToolPaths, BoundingBox cullBox);
    std::vector<std::vector<Line>> Apply_AABB_Rays(std::vector<std::vector<Line>>& ToolPaths, BoundingBox AABB_Box);
    std::vector<std::vector<Line>> Optimise_Start_End_Positions(std::vector<std::vector<Line>>& ToolPaths);
    std::vector<std::vector<Line>> Optimise_Start_End_Linkages(std::vector<std::vector<Line>>& ToolPaths);
    std::vector<std::vector<Line>> Add_Start_End_Positions(std::vector<std::vector<Line>>& ToolPaths);
    std::vector<std::vector<Line>> Interpolate_Max_Angle_Displacement(std::vector<std::vector<Line>>& ToolPaths);

    /**##########################################
     * #        Slicing Tools (Buffered)        #
     * ##########################################*/

    std::vector<std::vector<Line>> Clear_Toolpath();
    std::vector<std::vector<Line>> Load_Toolpath(std::vector<std::vector<Line>>& ToolPaths);
    std::vector<std::vector<Line>> Return_Toolpath();

    std::vector<std::vector<Line>> Cull_Toolpath_by_Box(BoundingBox cullBox);
    std::vector<std::vector<Line>> Apply_AABB_Rays(BoundingBox AABB_Box);
    std::vector<std::vector<Line>> Optimise_Start_End_Positions();
    std::vector<std::vector<Line>> Optimise_Start_End_Linkages();
    std::vector<std::vector<Line>> Add_Start_End_Positions();
    std::vector<std::vector<Line>> Interpolate_Max_Angle_Displacement();


    private:
    Settings config;
    mesh mesh_Class;
    std::vector<std::vector<Line>> Current_Toolpath;

};

#endif