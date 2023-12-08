#pragma once
#include <iostream>
#include "path.hpp"
#include <map>
#include <thread>
#include <vector>
#include <mutex>

class Switch;

class Host {
public:
    Host(int id, int data, std::vector<Path>& paths);
    void receive(int reduce_id, int data, std::map<int, Host> &host_map, std::map<int, Switch> &switch_map,  std::mutex & host_map_mutex, std::mutex & switch_map_mutex);
    std::map<int, int> all_reduce_descriptor;
    void send(int reduce_id, int data, std::map<int, Host> &host_map, std::map<int, Switch> &switch_map, std::mutex & host_map_mutex, std::mutex & switch_map_mutex);
    std::vector<Path> paths;
    void addPath(Path & path);
    std::vector<std::thread> executions;
    void AsyncHostSend(std::vector<Path>& paths, int p_idx, int reduce_id, std::map<int, int>& all_reduce_descriptor, std::map<int, Host>& host_map, std::map<int, Switch>& switch_map,  std::mutex & host_map_mutex, std::mutex & switch_map_mutex);

    Host(const Host&) = delete;
    Host& operator=(const Host&) = delete;
    Host(Host&&) = delete;
    Host& operator=(Host&&) = delete;

    // Destructor
    ~Host();
private:
    int id;
    int data;
    int all_reduce_id;
    std::mutex descriptor_mutex;
    std::mutex paths_mutex;
};