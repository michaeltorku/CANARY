#pragma once
#include <iostream>
#include "path.hpp"
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include "loadBalance.hpp"

class Host;

class Switch {


public:
    Switch();
    Switch(int id, std::vector<Path> paths);
    int id;
    std::vector<Path> paths;
    std::vector<std::thread> executions;
    std::map<int, int> all_reduce_descriptor;
    void receive(int reduce_id, int data, std::map<int, Host> &host_map, std::map<int, Switch> &switch_map, std::mutex & host_map_mutex, std::mutex & switch_map_mutex); 
    void send(int reduce_id, int data, std::map<int, Host> &host_map, std::map<int, Switch> &switch_map, std::mutex & host_map_mutex, std::mutex & switch_map_mutex);
    void addPath(Path path);
    void AsyncSwitchSend(std::vector<Path>& paths, int p_idx, int reduce_id, std::map<int, int>&all_reduce_descriptor, std::map<int, Host>& host_map, std::map<int, Switch>& switch_map, std::mutex & host_map_mutex, std::mutex & switch_map_mutex);
    ~Switch();

    std::mutex descriptor_mutex;
    std::mutex paths_mutex;  
};