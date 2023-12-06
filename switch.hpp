#pragma once
#include <iostream>
#include "path.hpp"
#include <vector>
#include <map>
#include "loadBalance.hpp"

class Host;

class Switch {


public:
    Switch();
    Switch(int id, std::vector<Path>& paths);
    int id;
    std::vector<Path> paths;
    std::map<int, int> all_reduce_map;
    void receive(int reduce_id, int data, std::map<int, Host> &host_map, std::map<int, Switch> &switch_map); 
    void send(int reduce_id, int data, std::map<int, Host> &host_map, std::map<int, Switch> &switch_map);
    void addPath(Path path);
    
};