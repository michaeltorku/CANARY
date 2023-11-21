
#include "switch.hpp"
#include "host.hpp"
// #include "path.hpp"
// #include <vector>

Switch::Switch(int id, std::vector<Path> paths){
    this->id = id;
    this->paths = paths;
}

Switch::Switch(){
}

void Switch::receive(int reduce_id, int data, std::map<int, Host> &host_map, std::map<int, Switch> &switch_map){
    std::cout << "B4 node id: " << this->id  << " data: " << all_reduce_map[reduce_id] << std::endl;
    all_reduce_map[reduce_id] += data;
    this->send(reduce_id, data, host_map, switch_map);
    std::cout << "AR node id: " << this->id  << " data: " << all_reduce_map[reduce_id] << std::endl;
}

void Switch::addPath(Path path) {
    paths.push_back(path);
}

void Switch::send(int reduce_id, int data, std::map<int, Host> &host_map, std::map<int, Switch> &switch_map){
    if (this->paths.size() == 0){
        std::cout << "no paths" << std::endl;
        // for (auto item: all_reduce_map){
        //     std::cout << item.first << "-> " << item.second << std::endl;
        // }
        return;
    }

    int p_idx = load_balancer::balance(this->paths);
    std::cout << "p_idx: " << p_idx << "size: " << this->paths.size() << std::endl;
    for (int i = 0; i < paths.size(); i++){
        std::cout << paths[i].lower_node << " " << paths[i].upper_node << std::endl;
    }
    Path selected_path = paths[p_idx];
    // // selected_path.upper_node
    std::cout << "going from " << selected_path.lower_node << " to " << selected_path.upper_node << std::endl;
    double delay = selected_path.utilization;
    int target_node_id = selected_path.upper_node[1] - '0';
    if (selected_path.upper_node[0] == 'S'){
        Switch &target = switch_map.at(target_node_id);
        target.receive(reduce_id, data, host_map, switch_map);
    }else{
        Host &target = host_map.at(target_node_id);
        target.receive(reduce_id, data, host_map, switch_map);
    }
}
