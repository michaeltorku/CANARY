#include "path.hpp"
#include <chrono>
#include <thread>


void Path::send_data(int data, bool reduce){
    std::this_thread::sleep_for(std::chrono::seconds(int(utilization*3)));
    if (reduce){
        upper_host.receive_data(data, reduce);
    }else{
        lower_host.receive_data(data, reduce);
    }
}
