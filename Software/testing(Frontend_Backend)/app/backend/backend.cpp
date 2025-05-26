#include "backend.h"

/**##########################################
 * #              API commands              #
 * ##########################################*/

bool Backend::schedule(step s){
    switch(s){
        case Generate_Surface_Toolpath:
        case Optimise_Start_End_Positions:
        case Optimise_Start_End_Linkages:
        case Add_Custom_Start_End_Positions:
        case Restrict_Max_Angle_per_Move:
            queue_.push_back(std::make_pair(s, 0));
            break;
        default:
            return false;
    }
    return true;
}

bool Backend::schedule(step s, int id){
    queue_.push_back(std::make_pair(s, id));
    return true;
}

void Backend::set_settings(Settings settings){
    system_data_.settings = settings;
}

void Backend::set_model(Model model){
    system_data_.model = model;
}

bool Backend::set_obstacles(int id, BoundingBox obstacle){
    for(const auto& existing_obstacle : system_data_.Obstacles){
        if(existing_obstacle.first == id){
            return false;
        }
    }

    system_data_.Obstacles.push_back({id, obstacle});
    return true;
}


bool Backend::set_culling_boxes(int id, BoundingBox cull_boxes){
    for(const auto& existing_cull : system_data_.CullBoxes){
        if(existing_cull.first == id){
            return false;
        }
    }

    system_data_.CullBoxes.push_back({id, cull_boxes});
    return true;
}

void Backend::clear_schedule(){
    queue_.clear();
}

// void Backend::run(){
//     std::lock_guard<std::mutex> lock(run_mutex_);
//     if (state_running_) return;

//     state_running_ = true;

//     worker_thread_ = std::thread([this](){
//         for(const auto& [s, id] : queue_){
//             if (progress_callback_) progress_callback_(s, id, false); // started

//             bool success = perform_process(s, id);

//             if (progress_callback_) progress_callback_(s, id, success); // finished
//         }

//         if(progress_callback_){
//             progress_callback_(static_cast<step>(-1), -1, true); // done
//         }

//         std::lock_guard<std::mutex> lock(run_mutex_);
//         state_running_ = false;
//     });

//     worker_thread_.detach();
// }

void Backend::run() {
    std::lock_guard<std::mutex> lock(run_mutex_);
    if (state_running_) {
        std::cout << "[Backend] Already running!\n";
        return;
    }

    state_running_ = true;

    std::cout << "[Backend] Starting worker thread...\n";

    worker_thread_ = std::thread([this]() {
        for (const auto& [s, id] : queue_) {
            std::cout << "[Backend] Executing step " << s << "\n";

            if (progress_callback_) progress_callback_(s, id, false);

            bool success = perform_process(s, id);

            if (progress_callback_) progress_callback_(s, id, success);

            std::cout << "[Backend] Step " << s << " done? " << std::boolalpha << success << "\n";
        }

        {
            std::lock_guard<std::mutex> lock(run_mutex_);
            state_running_ = false;
        }

        std::cout << "[Backend] All steps done!\n";
    });

    worker_thread_.detach();
}

void Backend::halt() {
    std::lock_guard<std::mutex> lock(run_mutex_);
}

void Backend::progress_callback(std::function<void(step, int, bool)> cb){
    progress_callback_ = cb;
}

std::vector<std::vector<Line>>* Backend::return_toolpath(){
    return &system_data_.Toolpath;
}

/**##########################################
 * #               API helpers              #
 * ##########################################*/

std::pair<BoundingBox, bool> Backend::get_obstacle_by_id(int id, std::vector<std::pair<int, BoundingBox>> vector){
    for(auto it : vector){
        if(it.first == id){
            return std::make_pair(it.second, true);
        }
    }
    return std::make_pair(BoundingBox{}, false);
}

bool Backend::perform_process(step step, int id) {
    std::pair<BoundingBox, bool> CullingBox;
    std::pair<BoundingBox, bool> AABB_Box;

    switch(step){
        case Generate_Surface_Toolpath:
            std::cout << "[Backend] Running Generate_Surface_Toolpath...\n";
            if(system_data_.model.meshCount == 0) {
                std::cout << "[Backend] Model is empty\n";
                return false;
            }
            system_data_.Toolpath = slice_tools_.Generate_Surface_Toolpath(system_data_.model);
            std::cout << "[Backend] Finished Generate_Surface_Toolpath, size: "
                      << system_data_.Toolpath.size() << "\n";
            break;

        case Cull_Toolpath_to_Obstacle:
            if(system_data_.Obstacles.empty() || system_data_.Toolpath.empty()) return false;
            CullingBox = get_obstacle_by_id(id, system_data_.Obstacles);
            if(!CullingBox.second) return false;
            system_data_.Toolpath = slice_tools_.Cull_Toolpath_by_Box(system_data_.Toolpath, CullingBox.first);
            break;

        case Generate_Start_End_Rays:
            if(system_data_.CullBoxes.empty() || system_data_.Toolpath.empty()) return false;
            AABB_Box = get_obstacle_by_id(id, system_data_.CullBoxes);
            if(!AABB_Box.second) return false;
            system_data_.Toolpath = slice_tools_.Apply_AABB_Rays(system_data_.Toolpath, AABB_Box.first);
            break;

        case Optimise_Start_End_Positions:
            if(system_data_.Toolpath.empty()) return false;
            system_data_.Toolpath = slice_tools_.Optimise_Start_End_Positions(system_data_.Toolpath);
            break;

        case Optimise_Start_End_Linkages:
            if(system_data_.Toolpath.empty()) return false;
            system_data_.Toolpath = slice_tools_.Optimise_Start_End_Linkages(system_data_.Toolpath);
            break;

        case Add_Custom_Start_End_Positions:
            if(system_data_.Toolpath.empty()) return false;
            system_data_.Toolpath = slice_tools_.Add_Start_End_Positions(system_data_.Toolpath);
            break;

        case Restrict_Max_Angle_per_Move:
            if(system_data_.Toolpath.empty()) return false;
            system_data_.Toolpath = slice_tools_.Interpolate_Max_Angle_Displacement(system_data_.Toolpath);
            break;

        default:
            return false;
    }

    return true;
}
