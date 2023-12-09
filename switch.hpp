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
    friend void receive(Switch& toggle, int reduce_id, int data); 
    friend void send(Switch& toggle, int reduce_id, int data);
    void addPath(Path path);
    
};