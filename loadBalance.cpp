#include "loadBalance.hpp"
// #include <functional>

static int index = 0;

int least_bandwidth(std::vector<Path> paths){
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

int round_robin(std::vector<Path> paths){
    int numPaths = paths.size();
    int selectedPath = index % numPaths;
    index++;
    return selectedPath;
}

int IP_Hash(std::vector<Path> paths, std::string clientIP){
    std::hash<std::string> hasher;

    auto hashValue = hasher(clientIP);
    int nodeIndex = hashValue % paths.size();
    return nodeIndex;

}


int load_balancer::balance(std::vector<Path> paths, int algorithm, std::string clientIP){
switch(algorithm) {
    case 1: //least bandwidth
        return least_bandwidth(paths);
    case 2: //round robin
        return round_robin(paths);
    case 3:
        // IP Hash
        return IP_Hash(paths, clientIP);
    default:
        // Code for default case
        break;
}
    
}
