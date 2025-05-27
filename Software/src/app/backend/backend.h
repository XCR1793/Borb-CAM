#ifndef BACKEND_H
#define BACKEND_H

#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <vector>
#include <string>
#include <raylib.h>
#include <raymath.h>
#include <meshmanagement.h>
#include <pathing.h>
#include <slice.h>
#include <thread>
#include <mutex>

enum step{
    Generate_Surface_Toolpath,
    Cull_Toolpath_to_Obstacle,
    Generate_Start_End_Rays,
    Optimise_Start_End_Positions,
    Optimise_Start_End_Linkages,
    Add_Custom_Start_End_Positions,
    Restrict_Max_Angle_per_Move
};

struct Toolpath_Data{
    Settings settings;
    Model model;
    mesh mesh;
    std::vector<std::vector<Line>> Toolpath;
    std::vector<std::pair<int, BoundingBox>> Obstacles;
    std::vector<std::pair<int, BoundingBox>> CullBoxes;
};

class backend{
    public:

    /**##########################################
     * #              API commands              #
     * ##########################################*/

    // Schedule Process
    bool schedule(step s);
    bool schedule(step s, int id);

    // Set Settings
    void set_settings(Settings settings);

    // Set Obstacles
    bool set_obstacles(int id, BoundingBox obstacle);

    // Set Cull Boxes
    bool set_culling_boxes(int id, BoundingBox cull_boxes);

    // Clear Scheduled Processes
    void clear_schedule();

    // Run the schedule
    void run();

    

    // Halt all processes and shutdown all current threads
    void halt();

    private:
    std::mutex dataMutex;
    bool run_is_Active = 0;
};

#endif