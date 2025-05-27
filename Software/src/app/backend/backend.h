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
    Unknown_Instruction,
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
    std::vector<std::pair<step, int>> queue;
};

class backend{
    public:
    backend() = default;

    ~backend() {
        halt();  // Makes sure thread is joined before backend is destroyed
    }

    slice slicing_tools;

    /**##########################################
     * #              API commands              #
     * ##########################################*/

    // Schedule Process
    bool schedule(step s);
    bool schedule(step s, BoundingBox boundingbox);

    // Set Settings
    void set_settings(Settings settings);

    // Set Working Model
    void set_model(Model model);

    // Set Obstacles
    bool set_obstacles(int id, BoundingBox obstacle);

    // Set Cull Boxes
    bool set_culling_boxes(int id, BoundingBox cull_boxes);

    // Clear Scheduled Processes
    void clear_schedule();

    // Run the schedule
    void run_schedule();

    // Run the program
    void run();

    // Halt all processes and shutdown all current threads
    void halt();

    // Return progress and values
    Toolpath_Data return_config();
    std::pair<step, int> return_progress();
    bool return_run_status();
    bool return_worker_status();

    private:
    /**##########################################
     * #              API helpers               #
     * ##########################################*/

    void worker();

    std::pair<BoundingBox, bool> get_obstacle_by_id(int id, std::vector<std::pair<int, BoundingBox>> vector);

    bool perform_process(step step, int id);

    private:
    // Thread Variables
    std::thread workerThread;
    std::mutex dataMutex;
    bool run_is_Active = false;
    bool worker_finished = true;
    bool start_run = false;

    // Thread Returns
    step Current_Step = Unknown_Instruction;
    int Worker_step = 0;

    // System Variables
    Toolpath_Data system_data_;
    int unique_id = 0;
};

#endif