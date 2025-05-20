#ifndef SLICE
#define SLICE

#include <raylib.h>
#include <vector>
#include <string>
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

enum Output_Gcode_Rotation_Order{
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
    mil,
    inch,       // Default for imperial
    foot,
    degrees,    // Default for rotation (internal math is done in radians but converted to deg)
    rad,
    millisecond,
    seconds,    // Default Time Value
    minutes,
    hours
};

struct Setting_Values{
    bool mode = 1;
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
    float SlicingDistance = 5.0f;
    Setting_Values SicingStartDistance = {.mode = 1};       // Mode 1 means its Auto (mode, value) only
    Setting_Values SlicingEndHeight = {.mode = 1};          // Mode 1 means its Auto (mode, value) only
    float Max_Speed = 100.0f;               // 100mm/s default
    float Max_Acceleration = 2000.0f;       // 2000mm/s^2 default
    float Max_Angular_Increment = 0.0872664625f;    // 5 Deg in rad increments (Max amount of turn per action)
    Setting_Values Starting_Position = {.mode = 1}; // Mode 1 = Auto (start where it is), (mode, value3D, increment) only
    Setting_Values Starting_Rotation = {.mode = 1}; // Mode 1 = Auto (start where it is), (mode, value3D, increment) only
    Setting_Values Finishing_Position = {.mode = 1}; // Mode 1 = Auto (finish where it is), (mode, value3D, increment) only
    Setting_Values Finishing_Rotation = {.mode = 1}; // Mode 1 = Auto (finish where it is), (mode, value3D, increment) only

    // Visualisation Settings
    Setting_Actions Axis_Guides_3D = Setting_Actions::Enable; // X Y Z Guides in 3D Space (Enable/Disable) only

    // Output Settings
    std::string OutputDir = "src/";
    std::string OutputFileName = "OwO";
    std::string OutputFileType = ".nc";
    Output_Gcode_Rotation_Order OutputRotOrder = Output_Gcode_Rotation_Order::ABC;

    // Misc Settings
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