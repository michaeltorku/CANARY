
#include "switch.hpp"
#include "host.hpp"
#include <future>



Switch::Switch(int id, std::vector<Path> paths){
    this->id = id;
    this->paths = paths;
}

Switch::Switch(){
}

Switch::~Switch(){
    for (auto& th : executions) {
        if (th.joinable()) {
            th.join();
        }
    }
}

void Switch::receive(int reduce_id, int data, std::map<int, Host> &host_map, std::map<int, Switch> &switch_map, std::mutex & host_map_mutex, std::mutex & switch_map_mutex){
    all_reduce_descriptor[reduce_id] += data;
    this->send(reduce_id, data, host_map, switch_map,host_map_mutex, switch_map_mutex);
}

void Switch::addPath(Path path) {
    paths.push_back(path);
}

void Switch::AsyncSwitchSend(std::vector<Path>& paths, int p_idx, int reduce_id, std::map<int, int>&all_reduce_descriptor, std::map<int, Host>& host_map, std::map<int, Switch>& switch_map, std::mutex & host_map_mutex, std::mutex & switch_map_mutex) {
    std::lock_guard<std::mutex> p_guard(paths_mutex);
    std::lock_guard<std::mutex> h_guard(host_map_mutex);
    std::lock_guard<std::mutex> s_guard(switch_map_mutex);
    std::lock_guard<std::mutex> a_guard(descriptor_mutex);
    Path& selected_path = paths[p_idx];
    int target_node_id = selected_path.upper_node[1] - '0';
    double delay = selected_path.utilization;
    std::this_thread::sleep_for(std::chrono::seconds((int)delay*5));
    double tmp = selected_path.utilization;
    selected_path.utilization +=  tmp;
    if (selected_path.upper_node[0] == 'S'){
        Switch &target = switch_map.at(target_node_id);
        // target.receive(reduce_id, all_reduce_descriptor[reduce_id], host_map, switch_map, host_map_mutex, switch_map_mutex);
        auto target_future = std::async(std::launch::async, 
                                    &Switch::receive,
                                    &target, // pointer to the target object
                                    reduce_id, 
                                    all_reduce_descriptor[reduce_id], 
                                    std::ref(host_map), 
                                    std::ref(switch_map), 
                                    std::ref(host_map_mutex), 
                                    std::ref(switch_map_mutex));
        
    }else{
        Host &target = host_map.at(target_node_id);
        // target.receive(reduce_id, all_reduce_descriptor[reduce_id], host_map, switch_map, host_map_mutex, switch_map_mutex);
        auto target_future = std::async(std::launch::async, 
                                    &Host::receive,
                                    &target, 
                                    reduce_id, 
                                    all_reduce_descriptor[reduce_id], 
                                    std::ref(host_map), 
                                    std::ref(switch_map), 
                                    std::ref(host_map_mutex), 
                                    std::ref(switch_map_mutex));
    }
    selected_path.utilization =  std::max(selected_path.utilization-tmp, 0.3);
}

void Switch::send(int reduce_id, int data, std::map<int, Host> &host_map, std::map<int, Switch> &switch_map, std::mutex & host_map_mutex, std::mutex & switch_map_mutex){
    if (this->paths.size() == 0){
        return;
    }
    int p_idx = load_balancer::balance(this->paths);
    
    auto future = std::async(std::launch::async, &Switch::AsyncSwitchSend, this, std::ref(paths), p_idx, reduce_id, std::ref(all_reduce_descriptor), std::ref(host_map), std::ref(switch_map), std::ref(host_map_mutex), std::ref(switch_map_mutex));
    // executions.emplace_back(AsyncSwitchSend, std::ref(paths), p_idx, reduce_id, std::ref(all_reduce_descriptor), std::ref(host_map), std::ref(switch_map), std::ref(host_map_mutex), std::ref(switch_map_mutex));
}
