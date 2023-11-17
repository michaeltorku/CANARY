#pragma once
#include <iostream>
#include "switch.hpp"
#include "path.hpp"
#include "node.hpp"


class Host: public Node{
public:
    Host(int id, int data, int all_reduce_id, std::vector<Path> paths);
    // void Receive(const Switch& other, int data); //change to packet
    void send(); 
    // void request();

private:
    int id;
    int data;
    int all_reduce_id;
};