#include "backend.h"

/**##########################################
 * #              API commands              #
 * ##########################################*/

void backend::run(){
    std::lock_guard<std::mutex> lock(dataMutex);

    // Start worker thread if not active and finished
    if(!run_is_Active && worker_finished){
        run_is_Active = true;
        worker_finished = false;
        workerThread = std::thread(&backend::worker, this);
        return;
    }

    // If worker finished, join and cleanup
    if(run_is_Active && worker_finished){
        if(workerThread.joinable()){
            workerThread.join();
        }
        run_is_Active = false;
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

/**##########################################
 * #              API helpers               #
 * ##########################################*/

void backend::worker(){
    // Simulate work
    while(true){
        {
            std::lock_guard<std::mutex> lock(dataMutex);
            if (!run_is_Active) {
                break;  // Stop requested
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(3));
        test_value ++;
    }


    // Set finished flag inside mutex
    {
        std::lock_guard<std::mutex> lock(dataMutex);
        worker_finished = true;
    }
}

int backend::return_value_Test(){
    std::lock_guard<std::mutex> lock (dataMutex);
    return test_value;
}