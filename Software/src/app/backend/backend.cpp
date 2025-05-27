    #include "backend.h"

    /**##########################################
     * #              API commands              #
     * ##########################################*/

    bool backend::schedule(step s){
        switch(s){
            case Generate_Surface_Toolpath:
            case Optimise_Start_End_Positions:
            case Optimise_Start_End_Linkages:
            case Add_Custom_Start_End_Positions:
            case Restrict_Max_Angle_per_Move:
                system_data_.queue.push_back(std::make_pair(s, 0));
                break;
            default:
                return false;
        }
        return true;
    }

    bool backend::schedule(step s, BoundingBox boundingbox){
        system_data_.queue.push_back(std::make_pair(s, unique_id));
        switch(s){
            case Cull_Toolpath_to_Obstacle:
                system_data_.Obstacles.push_back(std::make_pair(unique_id, boundingbox));
                unique_id ++;
                break;
            case Generate_Start_End_Rays:
                system_data_.CullBoxes.push_back(std::make_pair(unique_id, boundingbox));
                unique_id ++;
                break;
            default:
                std::cout << "BoundingBox datatype cant be used by scheduled step" << std::endl;
                return false;
        }
        return true;
    }

    void backend::set_settings(Settings settings){
        system_data_.settings = settings;
        slicing_tools.import_Settings(settings);
    }

    void backend::set_model(Model model){
        system_data_.model = model;
    }

    bool backend::set_obstacles(int id, BoundingBox obstacle){
        for(const auto& existing_obstacle : system_data_.Obstacles){
            if(existing_obstacle.first == id){
                return false;
            }
        }

        system_data_.Obstacles.push_back({id, obstacle});
        return true;
    }

    bool backend::set_culling_boxes(int id, BoundingBox cull_boxes){
        for(const auto& existing_cull : system_data_.CullBoxes){
            if(existing_cull.first == id){
                return false;
            }
        }

        system_data_.CullBoxes.push_back({id, cull_boxes});
        return true;
    }

    void backend::clear_schedule(){
        system_data_.queue.clear();
    }

    void backend::run_schedule(){
        start_run = true;
        backend::run();
    }

    void backend::run(){
        std::lock_guard<std::mutex> lock(dataMutex);

        // Start worker thread if not active and finished
        if(!run_is_Active && worker_finished && start_run){
            start_run = false;
            run_is_Active = true;
            worker_finished = false;
            workerThread = std::thread(&backend::worker, this);
            std::cout << "=========================== Run Started ===========================" << std::endl;
            std::cout << "Queue Size: " << system_data_.queue.size() << std::endl;
            return;
        }

        // If worker finished, join and cleanup
        if(run_is_Active && worker_finished){
            if(workerThread.joinable()){
                workerThread.join();
            }
            run_is_Active = false;
            std::cout << "Steps Run: " << Worker_step << std::endl;
            std::cout << "=========================== Run Finished ===========================" << std::endl;
        }
    }

    void backend::halt(){
        {
            std::lock_guard<std::mutex> lock(dataMutex);
            run_is_Active = false;
        }

        if(workerThread.joinable()){
            workerThread.join();
        }
    }

    Toolpath_Data backend::return_config(){
        return system_data_;
    }

    std::pair<step, int> backend::return_progress(){
        std::lock_guard<std::mutex> lock(dataMutex);
        return std::make_pair(Current_Step, Worker_step);
    }

    bool backend::return_run_status(){
        std::lock_guard<std::mutex> lock(dataMutex);
        return run_is_Active;
    }

    bool backend::return_worker_status(){
        std::lock_guard<std::mutex> lock(dataMutex);
        return worker_finished;
    }

    /**##########################################
     * #              API helpers               #
     * ##########################################*/

    void backend::worker(){
        size_t i = 0;
        while(true){
            step currentStep;
            int unique_id_processing;

            {
                std::lock_guard<std::mutex> lock(dataMutex);
                if(!run_is_Active || i >= system_data_.queue.size()){break;}

                currentStep = system_data_.queue[i].first;
                unique_id_processing = system_data_.queue[i].second;
                Current_Step = currentStep;
                Worker_step = i;
            }

            if(!perform_process(currentStep, unique_id_processing)){break;}

            {
                std::lock_guard<std::mutex> lock(dataMutex);
                Worker_step++;
            }

            i++;
        }

        {
            std::lock_guard<std::mutex> lock(dataMutex);
            worker_finished = true;
        }
    }

    std::pair<BoundingBox, bool> backend::get_obstacle_by_id(int id, std::vector<std::pair<int, BoundingBox>> vector){
        for(auto it : vector){
            if(it.first == id){
                return std::make_pair(it.second, true);
            }
        }
        return std::make_pair(BoundingBox{}, false);
    }

    bool backend::perform_process(step step, int id) {
        std::pair<BoundingBox, bool> CullingBox;
        std::pair<BoundingBox, bool> AABB_Box;

        switch(step){
            case Generate_Surface_Toolpath:
                std::cout << "[Backend] Running Generate_Surface_Toolpath...\n";
                if(system_data_.model.meshCount == 0) {
                    std::cout << "[Backend] Model is empty\n";
                    return false;
                }
                system_data_.Toolpath = slicing_tools.Generate_Surface_Toolpath(system_data_.model);
                std::cout << "[Backend] Finished Generate_Surface_Toolpath, size: "
                        << system_data_.Toolpath.size() << "\n";
                break;

            case Cull_Toolpath_to_Obstacle:
                if(system_data_.Obstacles.empty() || system_data_.Toolpath.empty()) return false;
                CullingBox = get_obstacle_by_id(id, system_data_.Obstacles);
                if(!CullingBox.second) return false;
                system_data_.Toolpath = slicing_tools.Cull_Toolpath_by_Box(system_data_.Toolpath, CullingBox.first);
                break;

            case Generate_Start_End_Rays:
                if(system_data_.CullBoxes.empty() || system_data_.Toolpath.empty()) return false;
                AABB_Box = get_obstacle_by_id(id, system_data_.CullBoxes);
                if(!AABB_Box.second) return false;
                system_data_.Toolpath = slicing_tools.Apply_AABB_Rays(system_data_.Toolpath, AABB_Box.first);
                break;

            case Optimise_Start_End_Positions:
                if(system_data_.Toolpath.empty()) return false;
                system_data_.Toolpath = slicing_tools.Optimise_Start_End_Positions(system_data_.Toolpath);
                break;

            case Optimise_Start_End_Linkages:
                if(system_data_.Toolpath.empty()) return false;
                system_data_.Toolpath = slicing_tools.Optimise_Start_End_Linkages(system_data_.Toolpath);
                break;

            case Add_Custom_Start_End_Positions:
                if(system_data_.Toolpath.empty()) return false;
                system_data_.Toolpath = slicing_tools.Add_Start_End_Positions(system_data_.Toolpath);
                break;

            case Restrict_Max_Angle_per_Move:
                if(system_data_.Toolpath.empty()) return false;
                system_data_.Toolpath = slicing_tools.Interpolate_Max_Angle_Displacement(system_data_.Toolpath);
                break;

            default:
                return false;
        }

        return true;
    }
