
#include "switch.hpp"
#include "host.hpp"



Switch::Switch(int id, std::vector<Path> paths){
    this->id = id;
    this->paths = paths;
}

Switch::Switch(){
}

void Switch::receive(int reduce_id, int data, std::map<int, Host> &host_map, std::map<int, Switch> &switch_map){
    all_reduce_map[reduce_id] += data;
    this->send(reduce_id, data, host_map, switch_map);
}

void Switch::addPath(Path path) {
    paths.push_back(path);
}

void AsyncSwitchSend(int target_node_id, Path& selected_path, int &reduce_id, std::map<int, int>all_reduce_map, std::map<int, Host>& host_map, std::map<int, Switch>& switch_map) {
    double delay = selected_path.utilization;
    std::this_thread::sleep_for(std::chrono::seconds((int)delay*5));
    double tmp = selected_path.utilization;
    selected_path.utilization +=  tmp;
    if (selected_path.upper_node[0] == 'S'){
        Switch &target = switch_map.at(target_node_id);
        target.receive(reduce_id, all_reduce_map[reduce_id], host_map, switch_map);
    }else{
        Host &target = host_map.at(target_node_id);
        target.receive(reduce_id, all_reduce_map[reduce_id], host_map, switch_map);
    }
    selected_path.utilization =  std::max(selected_path.utilization-tmp, 0.3);
}

void Switch::send(int reduce_id, int data, std::map<int, Host> &host_map, std::map<int, Switch> &switch_map){
    if (this->paths.size() == 0){
        return;
    }
    int p_idx = load_balancer::balance(this->paths);
    Path selected_path = paths[p_idx];
    int target_node_id = selected_path.upper_node[1] - '0';
    executions.emplace_back(AsyncSwitchSend, target_node_id, std::ref(selected_path), std::ref(reduce_id), std::ref(all_reduce_map), std::ref(host_map), std::ref(switch_map));
}
