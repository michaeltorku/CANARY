#include "loadBalance.hpp"



int load_balancer::balance(std::vector<Path> paths){
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
