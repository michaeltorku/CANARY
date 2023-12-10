#pragma once
#include <iostream>
#include "path.hpp"
#include "packet.hpp"
#include <map>

#include <vector>

class Switch;

class Host {
public:
    Host(int id, int data, std::vector<Path>& paths);
    friend void receive(Host& host, int reduce_id, int data);
    std::map<int, Packet> descriptor_map; //reduce_id to packet
    friend void send(Host& host, int reduce_id, int data);
    std::vector<Path> paths;
    void addPath(Path & path);
private:
    int id;
    int data;
    int all_reduce_id;
};