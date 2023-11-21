#include "path.hpp"
#include <chrono>
#include <thread>
#include <iostream>
// #include <string>


Path::Path(std::string lower_node, std::string upper_node):lower_node(lower_node), upper_node(upper_node){
        ;
        }

void Path::send_data(int data, bool reduce){ 
    
    //FIX
    std::this_thread::sleep_for(std::chrono::seconds(int(utilization*3)));
    // if (reduce){
    //     upper_node.receive_data(data, reduce);
    // }else{
    //     upper_node.receive_data(data, reduce);
    // }
}
