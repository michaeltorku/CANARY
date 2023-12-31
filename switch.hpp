#pragma once
#include <iostream>
#include "path.hpp"
#include <vector>
#include <unordered_map>
#include "loadBalance.hpp"
#include "packet.hpp"

class Host;

class Switch {


public:
    Switch();
    Switch(int id, std::vector<Path>& paths);
    int id;
    std::vector<Path> paths;
    std::unordered_map<int, Packet> descriptor_map;
    // friend void receive(Switch& toggle, int reduce_id, Packet data); 
    // friend void send(Switch& toggle, int reduce_id, Packet data);
    friend std::vector<Path>& getPaths(Switch & s);
    void addPath(Path path);
    
};