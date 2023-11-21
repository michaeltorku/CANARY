#include <iostream>
#include <vector>

#include "host.hpp"
#include "switch.hpp"
#include "loadBalance.hpp"



Host::Host(){
}

Host::Host(int id, int data, std::vector<Path> paths){
    this->id = id;
    this->data = data;
    this->paths = paths;
}

void Host::receive(int reduce_id, int data){
    all_reduce_map[reduce_id] += data;
}

void Host::addPath(Path path) {
    paths.push_back(path);
}

void Host::send(int reduce_id, std::map<int, Host> &host_map, std::map<int, Switch> &switch_map){
    int p_idx = load_balancer::balance(this->paths);
    Path selected_path = paths[p_idx];
    // selected_path.upper_node
    std::cout << selected_path.lower_node << " " << selected_path.upper_node << std::endl;
    double delay = selected_path.utilization;
    int target_node_id = selected_path.upper_node[1] - '0';
    if (selected_path.upper_node[0] == 'S'){
        Switch target = switch_map.at(target_node_id);
        target.receive(reduce_id, all_reduce_map[reduce_id]);
    }else{
        Host target = host_map.at(target_node_id);
        target.receive(reduce_id, all_reduce_map[reduce_id]);
    }
    


}