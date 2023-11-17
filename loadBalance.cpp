#include <iostream>
#include <limits>
#include "path.hpp"
#include <vector>

namespace load_balancer{

    int balance(int data, std::vector<Path> paths){
        int min = std::numeric_limits<int>::min();
        int min_index = 0;
        for(int i = 0; i < paths.size(); i++){
            if(paths[i].utilization < min){
                min = paths[i].utilization;
                min_index = i;
            }
        }
        return min_index;
    }
}