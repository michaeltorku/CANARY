#pragma once
#include <iostream>
#include "path.hpp"
#include "packet.hpp"
#include <unordered_map>

#include <vector>

class Switch;

class Host {
public:
    Host(int id, int data, std::vector<Path>& paths);
    // friend void receive(Host& host, int reduce_id, Packet data);
    std::unordered_map<int, Packet> descriptor_map; //reduce_id to packet
    friend void send(Host& host, int reduce_id, Packet data, std::string reduce_ip_address);
    std::vector<Path> paths;
    void addPath(Path & path);
    friend std::vector<Path>& getPaths(Host & h);
private:
    int id;
    int data;
    int all_reduce_id;
};