#include <iostream>
#include <vector>
#include <thread>
#include "host.hpp"
#include "switch.hpp"
#include "loadBalance.hpp"

Host::Host(int id, int data, std::vector<Path>& paths){
    this->id = id;
    this->data = data;
    this->paths = paths;
    descriptor_map = {};
}

// void Host::receive(int reduce_id, int data, std::unordered_map<int, Host> &host_map, std::unordered_map<int, Switch> &switch_map){
//     descriptor_map[reduce_id] += data;
//     send(reduce_id, data, host_map, switch_map);
// }

void Host::addPath(Path &path) {
    paths.push_back(path);
}

// void Host::send(int reduce_id, int data, std::unordered_map<int, Host> &host_map, std::unordered_map<int, Switch> &switch_map){
//     int p_idx = load_balancer::balance(this->paths);
//     Path &selected_path = paths[p_idx];

//     double delay = selected_path.utilization;
//     double tmp = selected_path.utilization;
//     selected_path.utilization +=  tmp;
//     std::this_thread::sleep_for(std::chrono::seconds((int)(delay*5)));
//     int target_node_id = selected_path.upper_node[1] - '0';
//     if (selected_path.upper_node[0] == 'S'){
//         Switch &target = switch_map.at(target_node_id);
//         target.receive(reduce_id, descriptor_map[reduce_id], host_map, switch_map);
//     }else{
//         Host &target = host_map.at(target_node_id);
//         target.receive(reduce_id, descriptor_map[reduce_id], host_map, switch_map);
//     }
//     // selected_path.utilization -=  tmp;


// }