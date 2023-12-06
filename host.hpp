#pragma once
#include <iostream>
#include "path.hpp"
#include <map>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <vector>

class Switch;

class Host {
public:
    Host(int id, int data, std::vector<Path>& paths, int load_balance_algorithm);
    void receive(int reduce_id, int data, std::map<int, Host> &host_map, std::map<int, Switch> &switch_map);
    std::map<int, int> all_reduce_map;
    void send(int reduce_id, int data, std::map<int, Host> &host_map, std::map<int, Switch> &switch_map);
    std::vector<Path> paths;
    void addPath(Path & path);
private:
    int id;
    int data;
    int all_reduce_id;
    int load_balance_algorithm;
    boost::uuids::uuid IP;
};