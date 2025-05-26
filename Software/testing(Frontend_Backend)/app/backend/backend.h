#ifndef BACKEND_H
#define BACKEND_H

#include <vector>
#include <chrono>
#include <raylib.h>
#include <meshmanagement.h>
#include <pathing.h>
#include <slice.h>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>

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

class Backend{
    public:

    /**##########################################
     * #              API commands              #
     * ##########################################*/

    // Schedule Process
    bool schedule(step s);
    bool schedule(step s, int id);

    void set_settings(Settings settings);

    void set_model(Model model);

    bool set_obstacles(int id, BoundingBox obstacle);

    bool set_culling_boxes(int id, BoundingBox cull_boxes);

    void clear_schedule();

    void run();

    void halt();

    void progress_callback(std::function<void(step, int, bool)> cb);

    std::vector<std::vector<Line>>* return_toolpath();

    private:
    /**##########################################
     * #               API helpers              #
     * ##########################################*/
    
    std::pair<BoundingBox, bool> get_obstacle_by_id(int id, std::vector<std::pair<int, BoundingBox>> vector);

    bool perform_process(step step, int id);

    private:
    std::vector<std::pair<step, int>> queue_;
    Toolpath_Data system_data_;
    slice slice_tools_;

    std::thread worker_thread_;
    std::function<void(step, int, bool)> progress_callback_;
    bool state_running_ = false;
    std::mutex run_mutex_;

};

#endif
